#pragma once

#include "trackerboy/ChType.hpp"
#include "trackerboy/gbs.hpp"
#include "trackerboy/synth/HardwareFile.hpp"

#include <cstdint>
#include <type_traits>


namespace trackerboy {

class Mixer {

public:

    Mixer(HardwareFile &hf);

    void sum(ChType ch, float in[], float out[], size_t nsamples);

    void getOutput(int16_t in1, int16_t in2, int16_t in3, int16_t in4, int16_t &outLeft, int16_t &outRight);
    void setEnable(Gbs::OutputFlags flags);
    void setEnable(ChType ch, Gbs::Terminal term, bool enabled);

private:
    HardwareFile &mHf;
    std::underlying_type<Gbs::OutputFlags>::type mOutputStat;

};

}