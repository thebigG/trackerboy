
#pragma once

#include "trackerboy/synth/NoiseGen.hpp"

#include <cmath>

constexpr uint16_t LFSR_INIT = 0x7FFF;
#define calcCounterMax(drf, scf) (DRF_TABLE[drf] << (scf+1))

namespace {

static const uint8_t DRF_TABLE[] = {
    8,
    16,
    32,
    48,
    64,
    80,
    96,
    112
};

}

namespace trackerboy {

NoiseGen::NoiseGen(float samplingRate) :
    mScf(Gbs::DEFAULT_SCF),
    mStepSelection(Gbs::DEFAULT_STEP_COUNT),
    mDrf(Gbs::DEFAULT_DRF),
    mLfsr(LFSR_INIT),
    mShiftCounter(0),
    mShiftCounterMax(calcCounterMax(mDrf, mScf)),
    mStepsPerSample(static_cast<unsigned>(Gbs::CLOCK_SPEED / samplingRate))
{
    // NOTES:
    // mStepsPerSample is an unsigned and not float for performance reasons.
    // keeping track of the fractional part would result in more accurate
    // sample generation, however, the difference in output is negligible
}


void NoiseGen::generate(float buf[], size_t nsamples) {
    for (size_t i = 0; i != nsamples; ++i) {
        mShiftCounter += mStepsPerSample;
        unsigned shifts = mShiftCounter / mShiftCounterMax;
        for (unsigned j = 0; j != shifts; ++j) {
            // same as mShiftCounter %= mShiftCounterMax outside of loop
            mShiftCounter -= mShiftCounterMax;
            // xor bits 1 and 0 of the lfsr
            uint8_t result = (mLfsr & 0x1) ^ ((mLfsr >> 1) & 0x1);
            // shift the register
            mLfsr >>= 1;
            // set the resulting xor to bit 15 (feedback)
            mLfsr |= result << 14;
            if (mStepSelection == Gbs::NOISE_STEPS_7) {
                // 7-bit lfsr, set bit 7 with the result
                mLfsr &= ~0x40; // reset bit 7
                mLfsr |= result << 6; // set bit 7 result
            }
        }
        if (mLfsr & 0x1) {
            // output is bit 0 inverted, so if bit 0 == 1, output MIN
            *buf++ = 0.0f;
        } else {
            *buf++ = 1.0f;
        }


    }
}

void NoiseGen::reset() {
    mShiftCounter = 0;
    mLfsr = LFSR_INIT;
}

void NoiseGen::setNoise(uint8_t noiseReg) {
    mDrf = noiseReg & 0x7;
    mStepSelection = static_cast<Gbs::NoiseSteps>((noiseReg >> 3) & 1);
    mScf = noiseReg >> 4;
    mShiftCounterMax = calcCounterMax(mDrf, mScf);
}


}