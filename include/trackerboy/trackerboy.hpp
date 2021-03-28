/*
** Trackerboy - Gameboy / Gameboy Color music tracker
** Copyright (C) 2019-2020 stoneface86
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
*/

// common typedefs and constants used throughout the library

#pragma once

#include <type_traits>
#include <cstdint>

namespace trackerboy {

enum class System : uint8_t {
    dmg,
    sgb
};

//
// Channel enum
//
enum class ChType : uint8_t {
    ch1 = 0,
    ch2 = 1,
    ch3 = 2,
    ch4 = 3
};

constexpr uint8_t EFFECT_CATEGORY_PATTERN = 0x00;
constexpr uint8_t EFFECT_CATEGORY_TRACK = 0x40;
constexpr uint8_t EFFECT_CATEGORY_FREQ = 0x80;

enum class EffectType : uint8_t {

    // A * indicates the effect is continuous and must be turned off (ie 400)

    // pattern effect (bits 6-7 = 00)
    patternGoto = EFFECT_CATEGORY_PATTERN,  //   1 2 3 4 Bxx begin playing given pattern immediately
    patternHalt,                            //   1 2 3 4 C00 stop playing
    patternSkip,                            //   1 2 3 4 D00 begin playing next pattern immediately
    setTempo,                               //   1 2 3 4 Fxx set the tempo
    sfx,                                    // * 1 2 3 4 Txx play sound effect

    // track effect (bits 6-7 = 01)

    setEnvelope = EFFECT_CATEGORY_TRACK,    //   1 2 3 4 Exx set the persistent envelope/wave id setting
    setTimbre,                              //   1 2 3 4 Vxx set persistent duty/wave volume setting
    setPanning,                             //   1 2 3 4 Ixy set channel panning setting
    setSweep,                               //   1       Hxx set the persistent sweep setting (CH1 only)
    delayedCut,                             //   1 2 3 4 Sxx note cut delayed by xx frames
    delayedNote,                            //   1 2 3 4 Gxx note trigger delayed by xx frames
    lock,                                   //   1 2 3 4 L00 (lock) stop the sound effect on the current channel


    // frequency effect (bits 6-7 = 10)
    arpeggio = EFFECT_CATEGORY_FREQ,        // * 1 2 3   0xy arpeggio with semi tones x and y
    pitchUp,                                // * 1 2 3   1xx pitch slide up
    pitchDown,                              // * 1 2 3   2xx pitch slide down
    autoPortamento,                         // * 1 2 3   3xx automatic portamento
    vibrato,                                // * 1 2 3   4xy vibrato
    vibratoDelay,                           //   1 2 3   5xx delay vibrato xx frames on note trigger
    tuning,                                 //   1 2 3   Pxx fine tuning
    noteSlideUp,                            // * 1 2 3   Qxy note slide up
    noteSlideDown                           // * 1 2 3   Rxy note slide down

};

//
// The speed type determines the tempo during pattern playback. Its unit is
// frames per row in Q4.4 format. Speeds with a fractional component will
// have some rows taking an extra frame.
//
using Speed = uint8_t;

// minimum possible speed, 1.0 frames per row
static constexpr Speed SPEED_MIN = 0x10;

// maximum possible speed, 15.0 frames per row
static constexpr Speed SPEED_MAX = 0xF0;

constexpr size_t MAX_INSTRUMENTS = 256;
constexpr size_t MAX_WAVEFORMS = 256;
constexpr size_t MAX_PATTERNS = 256;

template <typename T>
constexpr T GB_CLOCK_SPEED = T(4194304);

constexpr float GB_FRAMERATE_DMG = 59.7f;
constexpr float GB_FRAMERATE_SGB = 61.1f;

// each channel has 5 registers
constexpr unsigned GB_CHANNEL_REGS = 5;

//
// Maximum frequency setting for channels 1, 2 and 3
//
constexpr uint16_t GB_MAX_FREQUENCY = 2047;

//
// CH3 waveram is 16 bytes
//
constexpr size_t GB_WAVERAM_SIZE = 16;


// convert an enum class to its underlying type using unary operator +
// so instead of
// `static_cast<std::underlying_type<Foo>>(Foo::bar)` is equvalent to `+Foo::bar`
// 
template <typename T>
constexpr auto operator+(T e) noexcept
-> std::enable_if_t<std::is_enum<T>::value, std::underlying_type_t<T>>
{
    return static_cast<std::underlying_type_t<T>>(e);
}

}