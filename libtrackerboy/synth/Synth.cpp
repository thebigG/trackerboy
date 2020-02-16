
#include "trackerboy/synth/Synth.hpp"
#include "trackerboy/gbs.hpp"

#include <algorithm>
#include <climits>

constexpr unsigned CYCLES_PER_TRIGGER = 8192;


namespace trackerboy {

// Frame sequencer
// this part of the APU controls when components such as sweep, envelope
// and length counters are triggered. The sequencer itself is stepped every
// 8192 cycles or at 512 Hz. The table below shows which steps the components
// are triggered (except for length counters).
//
// Step:                 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
// --------------------------+---+---+---+---+---+---+---+-------------------
// Sweep        (128 Hz) |         x               x       
// envelope     ( 64 Hz) |                             x 
//
// The synthesizer emulates this by generating samples before each trigger,
// applying the trigger, and then repeating until the buffer is filled.
//
// The time between each trigger is stored in the mTriggerTimings array. For
// the first element, this value is the number of samples needed until the
// first trigger (which is Sweep @ step 2).
//

Synth::TriggerType const Synth::TRIGGER_SEQUENCE[] = {
    TriggerType::SWEEP,     // Sweep    @ step 2
    TriggerType::SWEEP,     // Sweep    @ step 6
    TriggerType::ENV        // Envelope @ step 7
};


Synth::Synth(float samplingRate) :
    mSamplingRate(samplingRate),
    mHf(samplingRate),
    mMixer(mHf),
    mCyclesPerSample(Gbs::CLOCK_SPEED / samplingRate),
    mTriggerTimings{
        (CYCLES_PER_TRIGGER * 3) / mCyclesPerSample, // 3 steps from 7 to 2
        (CYCLES_PER_TRIGGER * 4) / mCyclesPerSample, // 4 steps from 2 to 6
        (CYCLES_PER_TRIGGER * 1) / mCyclesPerSample  // 1 step from  6 to 7
    },
    mSampleCounter(0.0f),
    mSamplesToTrigger(mTriggerTimings[0]),
    mTriggerIndex(0),
    mInputBuffer(1 + static_cast<size_t>((CYCLES_PER_TRIGGER * 4) / mCyclesPerSample))
{
    // NOTE: mTriggerTimings will need to be recalculated if the sampling rate
    // changes. Currently there is no way to change it after construction,
    // although this may change in the future.
}

HardwareFile& Synth::hardware() {
    return mHf;
}

Mixer& Synth::mixer() {
    return mMixer;
}

void Synth::fill(float buf[], size_t nsamples) {
    
    float *inbuf = mInputBuffer.data();
    
    while (nsamples != 0) {

        float samples = mSamplesToTrigger - mSampleCounter;
        size_t samplesWhole = static_cast<size_t>(samples);

        TriggerType trigger;

        if (samplesWhole <= nsamples) {
            // enough samples to hit the trigger, set it
            trigger = TRIGGER_SEQUENCE[mTriggerIndex];
            // update index to the next one, rollback to start if needed
            if (++mTriggerIndex == TRIGGER_COUNT) {
                mTriggerIndex = 0;
            }

            // figure out the timing for the next trigger
            // add the fractional part since we can only generate whole samples
            mSamplesToTrigger = mTriggerTimings[mTriggerIndex] + (samples - samplesWhole);
            mSampleCounter = 0.0f;

        } else {
            // not enough samples in buffer to hit the trigger
            // this also means we are at the end of the buffer
            samplesWhole = nsamples;
            // adjust the counter for the next time
            mSampleCounter += samplesWhole;
            // don't trigger anything
            trigger = TriggerType::NONE;
        }

        // generate the samples and sum into buf for each oscillator/generator
        // NOTE: polymorphism isn't used here since we will always have this
        //       setup for every Synth. (it's also faster this way too)
        // TODO: optimize when envelope is 0 or channel is muted (we don't
        //       need to generate and mix)
        mHf.osc1.generate(inbuf, samplesWhole);
        mMixer.sum(ChType::ch1, inbuf, buf, samplesWhole);

        mHf.osc2.generate(inbuf, samplesWhole);
        mMixer.sum(ChType::ch2, inbuf, buf, samplesWhole);

        mHf.osc3.generate(inbuf, samplesWhole);
        mMixer.sum(ChType::ch3, inbuf, buf, samplesWhole);

        mHf.gen4.generate(inbuf, samplesWhole, mCyclesPerSample);
        mMixer.sum(ChType::ch4, inbuf, buf, samplesWhole);

        // do trigger
        switch (trigger) {
            case TriggerType::NONE:
                break;
            case TriggerType::ENV:
                mHf.env1.trigger();
                mHf.env2.trigger();
                mHf.env4.trigger();
                break;
            case TriggerType::SWEEP:
                mHf.sweep1.trigger();
                break;
        }
        
        nsamples -= samplesWhole;
        // interleaved output buffer, 2 channels per sample
        buf += (samplesWhole * 2);
    }

}

}