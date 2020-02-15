#pragma once

#include "trackerboy/synth/HardwareFile.hpp"


namespace trackerboy {

class Sequencer {
    

public:
    Sequencer(HardwareFile &hf);

    void reset();
    unsigned step(unsigned cycles);


private:

    enum TriggerType {
        NONE,
        SWEEP,
        ENV
    };
    struct Trigger {
        unsigned nextIndex;     // next index in the sequence
        unsigned nextFence;      // next wall to stop short
        TriggerType trigger;    // trigger to do
    };

    static Trigger const TRIGGER_SEQUENCE[];
    
    HardwareFile &mHf;
    unsigned mFreqCounter;
    unsigned mFence;
    unsigned mTriggerIndex;
    TriggerType mTrigger;

    

};

}