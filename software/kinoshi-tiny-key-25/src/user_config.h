/**
 * @file	user_config.h
 * @brief	User configuration storage for Tiny KinoKey 25
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 * Copyright (c) 2026 Kinoshita Laboratory All rights reserved.
 */
#pragma once
#ifndef USER_CONFIG_H
#define USER_CONFIG_H

#include <cstdint>

namespace kinoshita_lab::kinoshi_tiny_key_25::user_config
{
enum
{
    kMinMidiChannel     = 1,
    kMaxMidiChannel     = 16,
    kDefaultMidiChannel = 1,

    kMinVelocity     = 1,
    kMaxVelocity     = 127,
    kDefaultVelocity = 100,

    kMaxDataValue = 127,  // CC number, On / Off value
};

enum ButtonFunction : uint8_t
{
    kButtonFunctionPitchBend = 0,
    kButtonFunctionCc        = 1,
};

enum ButtonMode : uint8_t
{
    kButtonModeMomentary = 0,
    kButtonModeToggle    = 1,
};

struct ButtonConfig
{
    uint8_t function;  // ButtonFunction. Mod and Sustain are always kButtonFunctionCc.
    uint8_t mode;      // ButtonMode
    uint8_t cc_number;
    uint8_t on_value;
    uint8_t off_value;
};

// All members are uint8_t, so the struct has no padding and is stored as is.
struct UserConfig
{
    uint8_t usb_midi_channel = kDefaultMidiChannel;  // 1-16
    uint8_t trs_midi_channel = kDefaultMidiChannel;  // 1-16
    uint8_t velocity         = kDefaultVelocity;
    ButtonConfig mod         = {kButtonFunctionCc, kButtonModeMomentary, 1, 127, 0};
    ButtonConfig pitch_up    = {kButtonFunctionPitchBend, kButtonModeMomentary, 0, 127, 0};
    ButtonConfig pitch_down  = {kButtonFunctionPitchBend, kButtonModeMomentary, 0, 127, 0};
    ButtonConfig sustain     = {kButtonFunctionCc, kButtonModeMomentary, 64, 127, 0};
};

inline UserConfig config;

/// Returns true if every value of the config is in range.
bool isValid(const UserConfig& candidate);

bool load();
bool save();

/// Resets config to defaults and saves to flash. Returns true on success.
bool reset();
}  // namespace kinoshita_lab::kinoshi_tiny_key_25::user_config

#endif  // USER_CONFIG_H
