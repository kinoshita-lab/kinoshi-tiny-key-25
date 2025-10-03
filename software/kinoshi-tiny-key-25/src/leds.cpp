/**
 * @file	leds.cpp
 * @brief	LED control for Tiny KinoKey 25
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 * Copyright (c) 2025 Kinoshita Laboratory All rights reserved.
 */
#include "FastLED.h"
#include "leds.h"
#include "pins.h"
#include "config.h"
namespace
{
CRGB oct_led[1];
}

namespace kinoshita_lab::kinoshi_tiny_key_25::leds
{
void initialize()
{
    FastLED.addLeds<NEOPIXEL, pins::kPinOctaveNeoPixel>(oct_led, 1);
    FastLED.setBrightness(20);
    FastLED.show();
}

void setOctaveLed(const int octave)
{ 
    if (octave < config::kMinOctave || octave > config::kMaxOctave) {
        return;
    }
    const auto index = octave - config::kMinOctave;
    const auto& c   = config::kOctaveColors[index];
    oct_led[0].setRGB(c.r, c.g, c.b);
    FastLED.show();
}
}