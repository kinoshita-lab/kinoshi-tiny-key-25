/**
 * @file	config.h
 * @brief   configuration and constants for Tiny KinoKey 25
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 * Copyright (c) 2025 Kinoshita Laboratory All rights reserved.
 */
#pragma once
#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>
#include "leds.h"
namespace kinoshita_lab::kinoshi_tiny_key_25::config
{
// USB configuration
constexpr char kUsbManufacturerString[]   = "Kinoshita Laboratory";
constexpr char kUsbProductDescriptor[]    = "Kinoshi-Tiny Key 25";
constexpr char kUsbSerialDescriptor[]     = "0xdeadbeef";
constexpr char kUsbMidiStringDescriptor[] = "Kinoshi-Tiny Key 25"; // TODO: bug? long desciptor stops USB functionality. should investigate.

// MIDI configuration
enum
{
    kMidiChannel = 1,  // TODO: make it configurable via NRPN
};

// Pitch Bend configuration
constexpr int pitch_bend_time = 250; // ms to reach from center to max/min TODO: make it configurable via NRPN

// Keyboard basic configuration
enum
{
    kNumKeyboardKeys = 25,
    kMinOctave       = -1,
    kMaxOctave       = 9,
    kDefaultOctave   = 4,
    kNumOctaves      = (kMaxOctave - kMinOctave + 1),
};

enum
{
    kNumKeys = 25,
};

// application timer configuration
constexpr uint32_t kApplicationTimerIntervalUs = 500; // 500us

// color config for octave led
constexpr leds::Color kOctaveColors[kNumOctaves] = {
    {0x00, 0x00, 0x00},  // -1 black
    {0xb0, 0x00, 0x00},  // 0 red
    {0xff, 0x1f, 0x8a},  // 1  (OKHSL 0 100 60)
    {0xfe, 0x42, 0x00},  // 2  (OKHSL 35 100 60)
    {0xc7, 0x7f, 0x00},  // 3  (OKHSL 70 100 60)
    {0x6c, 0xa5, 0x00},  // 4  (OKHSL 142 100 60)
    {0x00, 0xb0, 0x00},  // 5  green // 264
    {0x00, 0xa8, 0x94},  // 6  (OKHSL 180 100 60)
    {0x1a, 0x55, 0xfc},  // 7  blue
    {0x93, 0x74, 0xff},  // 8  (OKHSL 270 100 60)
    {0xff, 0xff, 0xff},  // 9  white

};
}

#endif  // CONFIG_H
