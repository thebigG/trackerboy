
#include "gbsynth.h"

#include "tables.h"

// each channel has a maximum volume of 0.2, so maximum volume of all channels is 0.8
#define VOL_MULTIPLIER 0.2f

enum OutputStat {
    CTRL_SOUND1_LEFT = 0x1,
    CTRL_SOUND2_LEFT = 0x2,
    CTRL_SOUND3_LEFT = 0x4,
    CTRL_SOUND4_LEFT = 0x8,
    CTRL_SOUND1_RIGHT = 0x10,
    CTRL_SOUND2_RIGHT = 0x20,
    CTRL_SOUND3_RIGHT = 0x40,
    CTRL_SOUND4_RIGHT = 0x80
};

namespace gbsynth {

    Mixer::Mixer() :
        terminalEnable{DEFAULT_TERM_ENABLE},
        terminalVolumes{DEFAULT_TERM_VOLUME},
        outputStat((OutputFlags)0)
    {
    }

    void Mixer::getOutput(float in1, float in2, float in3, float in4, float &outLeft, float &outRight) {
        float left = 0.0f, right = 0.0f;
        if (terminalEnable[TERM_LEFT]) {
            if (outputStat & CTRL_SOUND1_LEFT) {
                left += in1 * VOL_MULTIPLIER;
            }
            if (outputStat & CTRL_SOUND2_LEFT) {
                left += in2 * VOL_MULTIPLIER;
            }
            if (outputStat & CTRL_SOUND3_LEFT) {
                left += in3 * VOL_MULTIPLIER;
            }
            if (outputStat & CTRL_SOUND4_LEFT) {
                left += in4 * VOL_MULTIPLIER;
            }
        }
        if (terminalEnable[TERM_RIGHT]) {
            if (outputStat & CTRL_SOUND1_RIGHT) {
                right += in1 * VOL_MULTIPLIER;
            }
            if (outputStat & CTRL_SOUND2_RIGHT) {
                right += in2 * VOL_MULTIPLIER;
            }
            if (outputStat & CTRL_SOUND3_RIGHT) {
                right += in3 * VOL_MULTIPLIER;
            }
            if (outputStat & CTRL_SOUND4_RIGHT) {
                right += in4 * VOL_MULTIPLIER;
            }
        }
        outLeft = left * VOLUME_TABLE[terminalVolumes[TERM_LEFT]];
        outRight = right * VOLUME_TABLE[terminalVolumes[TERM_RIGHT]];
    }

    void Mixer::setTerminalEnable(Terminal term, bool enabled) {
        terminalEnable[term] = enabled;
    }

    void Mixer::setTerminalVolume(Terminal term, uint8_t volume) {
        if (volume > MAX_VOLUME) {
            volume = MAX_VOLUME;
        }
        terminalVolumes[term] = volume;
    }

    void Mixer::setEnable(OutputFlags flags) {
        outputStat = flags;
    }

    void Mixer::setEnable(ChType ch, Terminal term, bool enabled) {
        uint8_t flag = 1 << ch;
        if (term == TERM_S02) {
            flag <<= 4;
        }

        if (enabled) {
            outputStat |= flag;
        } else {
            outputStat &= ~flag;
        }
    }


}