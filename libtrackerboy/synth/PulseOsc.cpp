
#include "trackerboy/synth/PulseOsc.hpp"

namespace trackerboy {


PulseOsc::PulseOsc(float samplingRate) :
    Osc(samplingRate, 4, 8),
    mDuty(Gbs::DEFAULT_DUTY)
{
   mDeltaBuf.push_back(Delta());
   mDeltaBuf.push_back(Delta());
   setDeltaBuf();
}

void PulseOsc::setDuty(Gbs::Duty duty) {
    if (duty != mDuty) {
        mDuty = duty;
        setDeltaBuf();
    }
    
}


void PulseOsc::setDeltaBuf() {
    Delta &first = mDeltaBuf[0];
    Delta &second = mDeltaBuf[1];

    switch (mDuty) {
        case Gbs::DUTY_125:
            // 0000000F -> 0, 0, 0, 0, 0, 0, -F, +F
            // 00 00 00 0F -> -F, 0, 0, 0, 0, 0, 0, +F
            first.location = 0;
            first.change = -VOLUME_MAX;
            first.before = VOLUME_MAX;
            second.location = 7;
            second.change = VOLUME_MAX;
            second.before = 0.0f;
            break;
        case Gbs::DUTY_25:
            // F000000F -> -F, 0, 0, 0, 0, 0, +F, 0
            // F0 00 00 0F -> 0, -F, 0, 0, 0, 0, 0, +F
            first.location = 1;
            first.change = -VOLUME_MAX;
            first.before = VOLUME_MAX;
            second.location = 7;
            second.change = VOLUME_MAX;
            second.before = 0.0f;
            break;
        case Gbs::DUTY_50:
            // F0000FFF -> -F, 0, 0, 0, +F, 0, 0, 0
            // F0 00 0F FF -> 0, -F, 0, 0, 0, +F, 0, 0
            first.location = 1;
            first.change = -VOLUME_MAX;
            first.before = VOLUME_MAX;
            second.location = 5;
            second.change = VOLUME_MAX;
            second.before = 0.0f;
            break;
        case Gbs::DUTY_75:
            // 0FFFFFF0 -> +F, 0, 0, 0, 0, 0, -F, 0
            // 0F FF FF F0 -> 0, +F, 0, 0, 0, 0, 0, -F
            first.location = 1;
            first.change = VOLUME_MAX;
            first.before = 0.0f;
            second.location = 7;
            second.change = -VOLUME_MAX;
            second.before = VOLUME_MAX;
            break;
        default:
            break;
    }

    mRecalc = true;
}



}