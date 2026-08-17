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
    kMinMidiChannel = 1,
    kMaxMidiChannel = 16,
    kDefaultMidiChannel = 1,
};

struct UserConfig
{
    uint8_t midi_channel = kDefaultMidiChannel;
};

inline UserConfig config;
}  // namespace kinoshita_lab::kinoshi_tiny_key_25::user_config

#endif  // USER_CONFIG_H
