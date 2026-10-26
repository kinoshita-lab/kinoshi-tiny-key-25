/**
 * @file	user_config.cpp
 * @brief	Persistent user configuration storage for Tiny KinoKey 25
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 * Copyright (c) 2026 Kinoshita Laboratory All rights reserved.
 */
#include <Arduino.h>
#include <EEPROM.h>
#include <type_traits>

#include "user_config.h"

namespace kinoshita_lab::kinoshi_tiny_key_25::user_config
{
namespace
{
constexpr uint8_t kStorageVersion  = 2;
constexpr int kEepromAddress       = 0;
constexpr size_t kEepromSize       = 256;

// On-flash layout. Validity is verified by the checksum (CRC-8 over version and
// payload) plus a version match, instead of a magic number.
struct StoredConfig
{
    uint8_t version;
    uint8_t checksum;
    UserConfig payload;
};

static_assert(std::is_trivially_copyable_v<UserConfig>, "UserConfig must be stored as raw bytes");
static_assert(sizeof(StoredConfig) <= kEepromSize, "StoredConfig does not fit in EEPROM");

uint8_t crc8Update(uint8_t checksum, const uint8_t byte)
{
    checksum ^= byte;
    for (int bit = 0; bit < 8; ++bit) {
        if ((checksum & 0x80u) != 0u) {
            checksum = static_cast<uint8_t>((checksum << 1) ^ 0x07u);
        } else {
            checksum = static_cast<uint8_t>(checksum << 1);
        }
    }
    return checksum;
}

uint8_t computeChecksum(const StoredConfig& stored)
{
    uint8_t checksum = crc8Update(0, stored.version);
    const auto* payload_bytes = reinterpret_cast<const uint8_t*>(&stored.payload);
    for (size_t i = 0; i < sizeof(stored.payload); ++i) {
        checksum = crc8Update(checksum, payload_bytes[i]);
    }
    return checksum;
}

bool isValidButton(const ButtonConfig& button)
{
    return (button.function == kButtonFunctionPitchBend || button.function == kButtonFunctionCc) &&
           (button.mode == kButtonModeMomentary || button.mode == kButtonModeToggle) &&
           button.cc_number <= kMaxDataValue &&
           button.on_value <= kMaxDataValue &&
           button.off_value <= kMaxDataValue;
}

bool isValidChannel(const uint8_t channel)
{
    return channel >= kMinMidiChannel && channel <= kMaxMidiChannel;
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

bool isValid(const UserConfig& candidate)
{
    return isValidChannel(candidate.usb_midi_channel) &&
           isValidChannel(candidate.trs_midi_channel) &&
           candidate.velocity >= kMinVelocity && candidate.velocity <= kMaxVelocity &&
           isValidButton(candidate.mod) && candidate.mod.function == kButtonFunctionCc &&
           isValidButton(candidate.pitch_up) &&
           isValidButton(candidate.pitch_down) &&
           isValidButton(candidate.sustain) && candidate.sustain.function == kButtonFunctionCc;
}

bool load()
{
    ensureEepromBegan();

    StoredConfig stored = {};
    EEPROM.get(kEepromAddress, stored);

    const bool valid = stored.version == kStorageVersion &&
                       stored.checksum == computeChecksum(stored) &&
                       isValid(stored.payload);

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
    stored.version = kStorageVersion;
    stored.payload = config;
    stored.checksum = computeChecksum(stored);

    EEPROM.put(kEepromAddress, stored);
    return EEPROM.commit();
}

bool reset()
{
    config = UserConfig{};
    return save();
}
}  // namespace kinoshita_lab::kinoshi_tiny_key_25::user_config
