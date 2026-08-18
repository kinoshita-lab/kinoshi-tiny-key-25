/**
 * @file	user_config.cpp
 * @brief	Persistent user configuration storage for Tiny KinoKey 25
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 * Copyright (c) 2026 Kinoshita Laboratory All rights reserved.
 */
#include <Arduino.h>
#include <EEPROM.h>
#include <cstddef>

#include "user_config.h"

namespace kinoshita_lab::kinoshi_tiny_key_25::user_config
{
namespace
{
constexpr uint16_t kStorageMagic   = 0x4B43;  // "KC" (Kinoshi Config)
constexpr uint8_t kStorageVersion  = 1;
constexpr int kEepromAddress       = 0;
constexpr size_t kEepromSize       = 256;

struct StoredConfig
{
    uint16_t magic;
    uint8_t version;
    uint8_t checksum;
    UserConfig payload;
};

uint8_t computeChecksum(const StoredConfig& stored)
{
    const uint8_t bytes[] = {
        static_cast<uint8_t>(stored.magic & 0xff),
        static_cast<uint8_t>(stored.magic >> 8),
        stored.version,
        stored.payload.midi_channel,
    };

    uint8_t checksum = 0;
    for (const auto byte : bytes) {
        checksum ^= byte;
        for (int bit = 0; bit < 8; ++bit) {
            if ((checksum & 0x80u) != 0u) {
                checksum = static_cast<uint8_t>((checksum << 1) ^ 0x07u);
            } else {
                checksum = static_cast<uint8_t>(checksum << 1);
            }
        }
    }

    return checksum;
}

void ensureEepromBegan()
{
    static bool began = false;
    if (!began) {
        EEPROM.begin(kEepromSize);
        began = true;
    }
}
}  // namespace

bool load()
{
    ensureEepromBegan();

    StoredConfig stored = {};
    EEPROM.get(kEepromAddress, stored);

    const bool valid = stored.magic == kStorageMagic &&
                       stored.version == kStorageVersion &&
                       stored.checksum == computeChecksum(stored) &&
                       stored.payload.midi_channel >= kMinMidiChannel &&
                       stored.payload.midi_channel <= kMaxMidiChannel;

    if (!valid) {
        config = UserConfig{};
        return false;
    }

    config = stored.payload;
    return true;
}

bool save()
{
    ensureEepromBegan();

    StoredConfig stored = {};
    stored.magic = kStorageMagic;
    stored.version = kStorageVersion;
    stored.payload = config;
    stored.checksum = computeChecksum(stored);

    EEPROM.put(kEepromAddress, stored);
    return EEPROM.commit();
}
}  // namespace kinoshita_lab::kinoshi_tiny_key_25::user_config
