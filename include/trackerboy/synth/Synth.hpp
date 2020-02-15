#pragma once

#include "HardwareFile.hpp"
#include "Mixer.hpp"
#include "Sequencer.hpp"


namespace trackerboy {

class Synth {
    
public:

    Synth(float samplingRate);

    HardwareFile& hardware();
    Mixer& mixer();
    Sequencer& sequencer();

    void fill(float buf[], size_t nsamples);

private:

    float mSamplingRate;

    HardwareFile mHf;
    Mixer mMixer;
    Sequencer mSequencer;


};

}