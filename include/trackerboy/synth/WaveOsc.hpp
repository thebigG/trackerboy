#pragma once

#include "trackerboy/gbs.hpp"
#include "trackerboy/synth/Osc.hpp"


namespace trackerboy {


class WaveOsc : public Osc {

public:
    WaveOsc(float samplingRate);

    void setVolume(Gbs::WaveVolume volume);

    //void setWaveform()

};

}