
#include "trackerboy/synth/Synth.hpp"
#include "trackerboy/gbs.hpp"

namespace trackerboy {


Synth::Synth(float samplingRate) :
    mSamplingRate(samplingRate),
    mHf(samplingRate),
    mMixer(mHf),
    mSequencer(mHf)
{
}

HardwareFile& Synth::hardware() {
    return mHf;
}

Mixer& Synth::mixer() {
    return mMixer;
}

Sequencer& Synth::sequencer() {
    return mSequencer;
}

void Synth::fill(float buf[], size_t nsamples) {
    for (size_t i = 0, j = nsamples * 2; i != j; i += 2) {
        //step(buf[i], buf[i + 1]);
    }
}

}