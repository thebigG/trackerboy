
#pragma once

#include "trackerboy/gbs.hpp"


namespace trackerboy {

class NoiseGen {

public:

    NoiseGen();

    void generate(float buf[], size_t nsamples, float cps);

    void reset();

    void setNoise(uint8_t reg);


private:

    uint8_t mScf;
    Gbs::NoiseSteps mStepSelection;
    uint8_t mDrf;
    uint16_t mLfsr;
    unsigned mShiftCounter;
    unsigned mShiftCounterMax;
    // the fractional part that is truncated when generating samples
    float mDrift;
};



}