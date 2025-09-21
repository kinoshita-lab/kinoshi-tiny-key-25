/**
 * @file	leds.h
 * @brief	LED control for Tiny KinoKey 25
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 * Copyright (c) 2025 Kinoshita Laboratory All rights reserved.
 */
#pragma once
#ifndef LEDS_H
#define LEDS_H

#include <cstdint>

namespace kinoshita_lab::tiny_kino_key_25::leds
{
struct Color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

enum
{
    kLedBuiltin,
    kLedOctave,
};
void initialize();

void setOctaveLed(const int octave);
} // namespace kinoshita_lab::tiny_kino_key_25::leds

#endif // LEDS_H
