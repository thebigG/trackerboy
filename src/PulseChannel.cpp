
#include "gbsynth.h"

#define DUTY_SIZE 8

namespace gbsynth {

    static uint8_t DUTY_TABLE[][DUTY_SIZE] = {
        // Duty 12.5%:  00000001
        {SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MAX},
        // Duty 25%:    10000001
        {SAMPLE_MAX, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MAX},
        // Duty 50%:    10000111
        {SAMPLE_MAX, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MAX, SAMPLE_MAX, SAMPLE_MAX},
        // Duty 75%:    01111110
        {SAMPLE_MIN, SAMPLE_MAX, SAMPLE_MAX, SAMPLE_MAX, SAMPLE_MAX, SAMPLE_MAX, SAMPLE_MAX, SAMPLE_MIN}
    };

    PulseChannel::PulseChannel() : EnvChannel(), FreqChannel() {        
        duty = (Duty)DEFAULT_DUTY;
        periodCounter = 0;
        dutyCounter = 0;
        nextsample = DUTY_TABLE[duty][0];
    }

    /*void PulseChannel::getRegisters(ChRegUnion &reg) {
        uint8_t nrx1 = length;
        nrx1 |= duty << 6;
        uint8_t nrx2 = encodeEnvRegister();
        uint8_t nrx3 = frequency & 0xFF;
        uint8_t nrx4 = frequency >> 8;
        if (!continuous) {
            nrx4 |= 0x40;
        }

        if (sweepEnabled) {
            reg.ch1.nr10 = (sweepTime << 4) | (sweepMode << 3) | sweepShift;
            reg.ch1.nr11 = nrx1;
            reg.ch1.nr12 = nrx2;
            reg.ch1.nr13 = nrx3;
            reg.ch1.nr14 = nrx4;
        } else {
            reg.ch2.nr21 = nrx1;
            reg.ch2.nr22 = nrx2;
            reg.ch2.nr23 = nrx3;
            reg.ch2.nr24 = nrx4;
        }

    }*/

    void PulseChannel::reset() {
        EnvChannel::reset();
        periodCounter = 0;
        dutyCounter = 0;
        nextsample = DUTY_TABLE[duty][0];
    }

    void PulseChannel::setDuty(Duty duty) {
        this->duty = duty;
    }

    uint8_t PulseChannel::generate() {
        if (periodCounter == 0) {
            periodCounter = (2048 - frequency) * 4;
            nextsample = DUTY_TABLE[duty][dutyCounter];
            if (++dutyCounter == DUTY_SIZE) {
                dutyCounter = 0;
            }
        } else {
            --periodCounter;
        }

        return apply(nextsample);
    }

}