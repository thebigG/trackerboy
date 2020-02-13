
#pragma once

#include "trackerboy/gbs.hpp"


namespace trackerboy {

class NoiseGen {

public:

    NoiseGen(float samplingRate);

    void generate(float buf[], size_t nsamples);

    void reset();

    void setNoise(uint8_t reg);


private:

    uint8_t mScf;
    Gbs::NoiseSteps mStepSelection;
    uint8_t mDrf;
    uint16_t mLfsr;
    unsigned mShiftCounter;
    unsigned mShiftCounterMax;

    float mStepsPerSample;
};



}