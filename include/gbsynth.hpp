
#pragma once

#include <cstdint>
#include <vector>
using std::uint8_t;
using std::uint16_t;
using std::vector;

namespace gbsynth {

    enum Duty {
        DUTY_125 = 0,
        DUTY_25  = 1,
        DUTY_50  = 2,
        DUTY_75  = 3
    };

    enum EnvMode {
        ENV_ATTENUATE = 0,
        ENV_AMPLIFY = 1
    };

    enum SweepMode {
        SWEEP_ADDITION = 0,
        SWEEP_SUBTRACTION = 1
    };

    enum StepCount {
        STEPS_15 = 0,
        STEPS_7 = 1
    };

    enum WaveformLevel {
        WAVE_MUTE    = 0,
        WAVE_WHOLE   = 1,
        WAVE_HALF    = 2,
        WAVE_QUARTER = 3
    };

    enum Terminal {
        TERM_S01,
        TERM_S02,
        TERM_LEFT = TERM_S01,   // todo: verify that s01 = left
        TERM_RIGHT = TERM_S02
    };

    enum ChType {
        CH_SOUND1 = 0,
        CH_SOUND2 = 1,
        CH_SOUND3 = 2,
        CH_SOUND4 = 3
    };

    enum OutputFlags : uint8_t {
        OUT_SOUND1_LEFT     = 0x1,
        OUT_SOUND2_LEFT     = 0x2,
        OUT_SOUND3_LEFT     = 0x4,
        OUT_SOUND4_LEFT     = 0x8,
        OUT_SOUND1_RIGHT    = 0x10,
        OUT_SOUND2_RIGHT    = 0x20,
        OUT_SOUND3_RIGHT    = 0x40,
        OUT_SOUND4_RIGHT    = 0x80,
        OUT_SOUND1_BOTH     = OUT_SOUND1_LEFT | OUT_SOUND1_RIGHT,
        OUT_SOUND2_BOTH     = OUT_SOUND2_LEFT | OUT_SOUND2_RIGHT,
        OUT_SOUND3_BOTH     = OUT_SOUND3_LEFT | OUT_SOUND3_RIGHT,
        OUT_SOUND4_BOTH     = OUT_SOUND4_LEFT | OUT_SOUND4_RIGHT,
        OUT_ALL             = 0xFF
    };

    enum Constants {
        // maximum values for parameters
        MAX_SWEEP_TIME      = 0x7,
        MAX_SWEEP_SHIFT     = 0x7,
        MAX_LENGTH          = 0x3F,
        MAX_ENV_STEPS       = 0xF,
        MAX_ENV_LENGTH      = 0x7,
        MAX_FREQUENCY       = 0x7FF,
        MAX_WAVE_LENGTH     = 0xFF,
        MAX_VOLUME          = 0x7,
        MAX_SCF             = 0xD,

        // defaults
        DEFAULT_FREQUENCY   = 0,
        DEFAULT_DUTY        = DUTY_75,
        DEFAULT_ENV_STEPS   = 0,
        DEFAULT_ENV_LENGTH  = 0,
        DEFAULT_ENV_MODE    = ENV_ATTENUATE,
        DEFAULT_SWEEP_TIME  = MAX_SWEEP_TIME,
        DEFAULT_SWEEP_MODE  = SWEEP_ADDITION,
        DEFAULT_SWEEP_SHIFT = 0,
        DEFAULT_WAVE_LEVEL  = WAVE_WHOLE,
        DEFAULT_TERM_ENABLE = false,
        DEFAULT_TERM_VOLUME = MAX_VOLUME,
        DEFAULT_SCF         = 0,
        DEFAULT_STEP_COUNT  = STEPS_15,
        DEFAULT_DRF         = 0,

        // sample values
        SAMPLE_GND          = 0x8,
        SAMPLE_MAX          = 0xF,
        SAMPLE_MIN          = 0x0,

        // # of entries in waveform ram (sound3)
        WAVE_SIZE           = 32,
        WAVE_RAMSIZE        = 16
    };

    class Channel {
        uint8_t lengthCounter;
        bool continuous;
        bool enabled;

    public:
        void disable();
        uint8_t getCurrentSample();
        virtual float getCurrentVolume();
        void setLength(uint8_t length);
        void setContinuousOutput(bool continuous);
        void lengthStep();
        virtual void reset();
        virtual void step(unsigned cycles);

    protected:
        uint8_t currentSample;
        uint8_t length;

        Channel();

        virtual uint8_t generate(unsigned cycles) = 0;
    };

    class EnvChannel : public Channel {
        uint8_t envCounter;
        uint8_t envelope;

    public:
        float getCurrentVolume() override;
        void setEnvStep(uint8_t step);
        void setEnvMode(EnvMode mode);
        void setEnvLength(uint8_t length);
        void envStep();
        virtual void reset() override;

    protected:
        uint8_t envSteps;
        EnvMode envMode;
        uint8_t envLength;

        EnvChannel();
    };

    class FreqChannel {
        unsigned multiplier;

    protected:
        uint16_t frequency;
        unsigned freqCounter;
        unsigned freqCounterMax;

    public:
        FreqChannel(unsigned multiplier);

        uint16_t getFrequency();
        void setFrequency(uint16_t frequency);
        void setFrequency(float frequency);
    };

    class PulseChannel : public EnvChannel, public FreqChannel {
        Duty duty;
        unsigned dutyCounter;

    public:
        PulseChannel();

        void setDuty(Duty duty);
        void reset() override;

    protected:
        uint8_t generate(unsigned cycles) override;
    
    };

    class WaveChannel : public Channel, public FreqChannel {
        WaveformLevel outputLevel;
        uint8_t wavedata[WAVE_RAMSIZE];
        unsigned waveIndex;

    public:
        WaveChannel();

        void setOutputLevel(WaveformLevel level);
        void setWaveform(uint8_t buf[WAVE_RAMSIZE]);
        void reset() override;

    protected:
        uint8_t generate(unsigned cycles) override;
    };

    class NoiseChannel : public EnvChannel {
        uint8_t scf;
        StepCount stepSelection;
        uint8_t drf;

        uint16_t lfsr;
        unsigned shiftCounter;
        unsigned shiftCounterMax;
        
    public:
        NoiseChannel();

        void setScf(uint8_t scf);
        void setStepSelection(StepCount count);
        void setDrf(uint8_t drf);

        void reset() override;

    protected:
        uint8_t generate(unsigned cycles) override;

    };


    class Sweep {
        SweepMode sweepMode;
        uint8_t sweepTime;
        uint8_t sweepShift;

        PulseChannel &ch;

        uint8_t counter;

    public:
        Sweep(PulseChannel &ch);

        void setSweepTime(uint8_t ts);
        void setSweepMode(SweepMode mode);
        void setSweepShift(uint8_t n);
        void reset();
        void step();
    };

    struct ChannelFile {
        PulseChannel ch1;
        PulseChannel ch2;
        WaveChannel ch3;
        NoiseChannel ch4;
    };

    class Sequencer {
        unsigned freqCounter;
        unsigned stepCounter;
        Sweep &sweep;
        ChannelFile &cf;

    public:
        Sequencer(ChannelFile &cf, Sweep &sweep);

        void step(unsigned cycles);
        void reset();
    };

    class Mixer {
        bool s01enable, s02enable;
        uint8_t s01vol, s02vol;
        uint8_t outputStat;

    public:
        Mixer();

        void setTerminalEnable(Terminal term, bool enabled);
        void setTerminalVolume(Terminal term, uint8_t volume);
        void setEnable(OutputFlags flags);
        void setEnable(ChType ch, Terminal term, bool enabled);

        void getOutput(float in1, float in2, float in3, float in4, float &outLeft, float &outRight);
    };

    class Synth {
        Sweep sweep;
        ChannelFile cf;
        Mixer mixer;
        Sequencer sequencer;

        float samplingRate;
        unsigned stepsPerSample;

    public:
        Synth(float samplingRate);

        Sweep& getSweep();
        ChannelFile& getChannels();
        Mixer& getMixer();
        Sequencer& getSequencer();

        void fill(float leftBuf[], float rightBuf[], size_t nsamples);
    };


    // utility functions

    float fromGbFreq(uint16_t value);

    uint16_t toGbFreq(float value);
}