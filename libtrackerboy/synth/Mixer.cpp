
#include "trackerboy/synth/Mixer.hpp"

namespace {


static const float GAIN_TABLE[16] = {
    0.0f,               // Envelope = 0, no gain
    1.0f / 64.0f,
    2.0f / 64.0f,
    3.0f / 64.0f,
    4.0f / 64.0f,
    5.0f / 64.0f,
    6.0f / 64.0f,
    7.0f / 64.0f,
    8.0f / 64.0f,
    9.0f / 64.0f,
    10.0f / 64.0f,
    11.0f / 64.0f,
    12.0f / 64.0f,
    13.0f / 64.0f,
    14.0f / 64.0f,
    15.0f / 64.0f       // Envelope = 0xF, max gain approx 1/4
};


}


namespace trackerboy {

Mixer::Mixer(HardwareFile &hf) :
    mHf(hf),
    mOutputStat(Gbs::OUT_OFF)
{
}

//void Mixer::getOutput(int16_t in1, int16_t in2, int16_t in3, int16_t in4, int16_t &outLeft, int16_t &outRight) {
//    
//    #define applyVolume(var, volume) do { \
//        switch (volume) { \
//            case 0x0: \
//                var >>= 3; \
//                break; \
//            case 0x1: \
//                var >>= 2; \
//                break; \
//            case 0x2: \
//                var = (var * 3) >> 3; \
//                break; \
//            case 0x3: \
//                var >>= 1; \
//                break; \
//            case 0x4: \
//                var = (static_cast<int32_t>(var) * 5) >> 3; \
//                break; \
//            case 0x5: \
//                var = (var * 3) >> 2; \
//                break; \
//            case 0x6: \
//                var = (static_cast<int32_t>(var) * 7) >> 3; \
//                break; \
//            case 0x7: \
//                break; \
//        } } while (false)
//    
//    int16_t left = 0, right = 0;
//    if (mS01enable) {
//        if (mOutputStat & Gbs::OUT_LEFT1) {
//            left += in1;
//        }
//        if (mOutputStat & Gbs::OUT_LEFT2) {
//            left += in2;
//        }
//        if (mOutputStat & Gbs::OUT_LEFT3) {
//            left += in3;
//        }
//        if (mOutputStat & Gbs::OUT_LEFT4) {
//            left += in4;
//        }
//
//        applyVolume(left, mS01vol);
//    }
//    if (mS02enable) {
//        if (mOutputStat & Gbs::OUT_RIGHT1) {
//            right += in1;
//        }
//        if (mOutputStat & Gbs::OUT_RIGHT2) {
//            right += in2;
//        }
//        if (mOutputStat & Gbs::OUT_RIGHT3) {
//            right += in3;
//        }
//        if (mOutputStat & Gbs::OUT_RIGHT4) {
//            right += in4;
//        }
//
//        applyVolume(right, mS02vol);
//    }
//
//    outLeft = left;
//    outRight = right;
//}

void Mixer::setEnable(Gbs::OutputFlags flags) {
    mOutputStat = flags;
}

void Mixer::setEnable(ChType ch, Gbs::Terminal term, bool enabled) {
    uint8_t flag = 0;
    if (term & Gbs::TERM_LEFT) {
        flag = 1 << static_cast<uint8_t>(ch);
    }

    if (term & Gbs::TERM_RIGHT) {
        flag |= 16 << static_cast<uint8_t>(ch);
    }

    if (enabled) {
        mOutputStat |= flag;
    } else {
        mOutputStat &= ~flag;
    }
}


void Mixer::sum(ChType ch, float in[], float out[], size_t nsamples) {
    Envelope *env = nullptr;

    bool leftEnabled = mOutputStat & (1 << static_cast<unsigned>(ch));
    bool rightEnabled = mOutputStat & (16 << static_cast<unsigned>(ch));
    
    switch (ch) {
        case ChType::ch1:
            env = &mHf.env1;
            break;
        case ChType::ch2:
            env = &mHf.env2;
            break;
        case ChType::ch3:
            // no envelope for CH3
            break;
        case ChType::ch4:
            env = &mHf.env4;
            break;

    }

    float gain;
    if (env == nullptr) {
        gain = GAIN_TABLE[15];
    } else {
        gain = GAIN_TABLE[env->value()];
    }

    // now apply the gain and sum into the output
    float *outptr = out;
    float *inptr = in;

    for (size_t i = 0; i != nsamples; ++i) {
        float sample = gain * *inptr++;
        *outptr++ += (leftEnabled) ? sample : 0.0f;
        *outptr++ += (rightEnabled) ? sample : 0.0f;
    }
}

}