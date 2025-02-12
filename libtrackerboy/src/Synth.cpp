
#include "trackerboy/Synth.hpp"

#include <cmath>


namespace trackerboy {


Synth::Synth(int samplerate, float framerate) noexcept :
    mApu(samplerate, static_cast<size_t>(samplerate / framerate) + 1),
    mSamplerate(samplerate),
    mFramerate(framerate),
    mCyclesPerFrame(gbapu::constants::CLOCK_SPEED<float> / mFramerate),
    mCycleOffset(0.0f),
    mFrameSize(0),
    mResizeRequired(true)
{
    setupBuffers();
}

gbapu::Apu& Synth::apu() noexcept {
    return mApu;
}

size_t Synth::framesize() const noexcept {
    return mFrameSize;
}

void Synth::run() noexcept {

    // determine number of cycles to run for the next frame
    float cycles = mCyclesPerFrame + mCycleOffset;
    float wholeCycles;
    mCycleOffset = modff(cycles, &wholeCycles);

    // step to the end of the frame
    mApu.stepTo(static_cast<uint32_t>(wholeCycles));
    mApu.endFrame();

}


void Synth::reset() noexcept {
    mApu.reset();
    mApu.clearSamples();
    mCycleOffset = 0.0f;

    // turn sound on
    mApu.writeRegister(gbapu::Apu::REG_NR52, 0x80, 0);
    mApu.writeRegister(gbapu::Apu::REG_NR50, 0x77, 0);
}

void Synth::setFramerate(float framerate) {
    if (mFramerate != framerate) {
        mFramerate = framerate;
        mResizeRequired = true;
    }
}

int Synth::samplerate() const noexcept {
    return mSamplerate;
}

void Synth::setSamplerate(int samplerate) {
    if (mSamplerate != samplerate) {
        mSamplerate = samplerate;
        mResizeRequired = true;
    }
}

void Synth::setupBuffers() {
    if (mResizeRequired) {
        mCyclesPerFrame = gbapu::constants::CLOCK_SPEED<float> / mFramerate;
        mFrameSize = static_cast<size_t>(mSamplerate / mFramerate) + 1;

        mApu.setSamplerate(mSamplerate);
        mApu.setBuffersize(mFrameSize);
        mApu.resizeBuffer();

        reset();
        mResizeRequired = false;
    }

    
}

}
