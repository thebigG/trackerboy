#pragma once

#include "Channel.hpp"

#include "trackerboy/gbs.hpp"
#include "trackerboy/synth/Waveform.hpp"


namespace trackerboy {

class WaveChannel : public Channel {

public:

    WaveChannel();

    void reset() override;
    void setOutputLevel(Gbs::WaveVolume level);
    void setWaveform(Waveform &wave);
    void setWaveform(const uint8_t wave[Gbs::WAVE_RAMSIZE]);
    void step(unsigned cycles) override;

private:
    Gbs::WaveVolume mOutputLevel;
    uint8_t mWavedata[Gbs::WAVE_RAMSIZE];
    unsigned mWaveIndex;

};

}
