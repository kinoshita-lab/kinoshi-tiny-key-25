/**
 * @file	pins.h
 * @brief   Pin definitions for Tiny KinoKey 25	
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 * Copyright (c) 2025 Kinoshita Laboratory All rights reserved.
 */
#pragma once
#ifndef PINS_H
#define PINS_H

#include <Arduino.h>

namespace kinoshita_lab::kinoshi_tiny_key_25::pins
{
// Pin Constants
enum
{
    kPinSerialOut3  = D28,
    kPinSerialOut1  = D27,
    kPinSerialOut2  = D26,
    kPinPl          = D15,
    kPinCp          = D14,

    kPinZeroNeoPixel = D16,
    kPinOctaveNeoPixel = D29,
};

} // namespace kinoshita_lab::tiny_kino_key_25
#endif // PINS_H
