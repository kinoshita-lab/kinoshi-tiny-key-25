/**
 * @file	setting_transfer.cpp
 * @brief	Setting transfer with the editor over NRPN for Tiny KinoKey 25
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 * Copyright (c) 2026 Kinoshita Laboratory All rights reserved.
 */
#include <Arduino.h>

#include "setting_transfer.h"
#include "device_config.h"
#include "midi_process.h"
#include "user_config.h"

namespace kinoshita_lab::kinoshi_tiny_key_25::setting_transfer
{
namespace
{
constexpr uint8_t kSettingChannel = 1;

// Control Change numbers
constexpr uint8_t kCcDataEntryMsb = 0x06;
constexpr uint8_t kCcNrpnLsb      = 0x62;
constexpr uint8_t kCcNrpnMsb      = 0x63;
constexpr uint8_t kCcRpnLsb       = 0x64;
constexpr uint8_t kCcRpnMsb       = 0x65;
constexpr uint8_t kRpnNull        = 0x7f;

// TABLE 2 : System Command. Each takes value 00.
constexpr uint8_t kSystemCommandMsb  = 0x60;
constexpr uint8_t kDumpRequestLsb    = 0x00;
constexpr uint8_t kDumpCompleteLsb   = 0x01;
constexpr uint8_t kSaveLsb           = 0x02;
constexpr uint8_t kAckLsb            = 0x03;
constexpr uint8_t kTransferStartLsb  = 0x04;
constexpr uint8_t kVersionRequestLsb = 0x05;

// TABLE 3 : Device Information (transmit only)
constexpr uint8_t kDeviceInfoMsb      = 0x61;
constexpr uint8_t kVersionMajorLsb    = 0x00;
constexpr uint8_t kVersionMinorLsb    = 0x01;
constexpr uint8_t kVersionPatchLsb    = 0x02;
constexpr int16_t kNoParameterNumber = -1;

using user_config::UserConfig;

struct Parameter
{
    uint8_t msb;
    uint8_t lsb;
    uint8_t min;  // NRPN value range
    uint8_t max;
    uint8_t offset;  // stored value = NRPN value + offset
    uint8_t* (*field)(UserConfig& config);
};

// TABLE 1 : Setting Parameter, in dump order.
// clang-format off
constexpr Parameter kParameters[] = {
    {0x40, 0x00, 0x00, 0x0f, 1, [](UserConfig& c) { return &c.usb_midi_channel; }},
    {0x41, 0x00, 0x00, 0x0f, 1, [](UserConfig& c) { return &c.trs_midi_channel; }},
    {0x48, 0x01, 0x01, 0x7f, 0, [](UserConfig& c) { return &c.velocity; }},
    {0x50, 0x11, 0x00, 0x01, 0, [](UserConfig& c) { return &c.mod.mode; }},
    {0x50, 0x12, 0x00, 0x7f, 0, [](UserConfig& c) { return &c.mod.cc_number; }},
    {0x50, 0x13, 0x00, 0x7f, 0, [](UserConfig& c) { return &c.mod.on_value; }},
    {0x50, 0x14, 0x00, 0x7f, 0, [](UserConfig& c) { return &c.mod.off_value; }},
    {0x51, 0x10, 0x00, 0x01, 0, [](UserConfig& c) { return &c.pitch_up.function; }},
    {0x51, 0x11, 0x00, 0x01, 0, [](UserConfig& c) { return &c.pitch_up.mode; }},
    {0x51, 0x12, 0x00, 0x7f, 0, [](UserConfig& c) { return &c.pitch_up.cc_number; }},
    {0x51, 0x13, 0x00, 0x7f, 0, [](UserConfig& c) { return &c.pitch_up.on_value; }},
    {0x51, 0x14, 0x00, 0x7f, 0, [](UserConfig& c) { return &c.pitch_up.off_value; }},
    {0x52, 0x10, 0x00, 0x01, 0, [](UserConfig& c) { return &c.pitch_down.function; }},
    {0x52, 0x11, 0x00, 0x01, 0, [](UserConfig& c) { return &c.pitch_down.mode; }},
    {0x52, 0x12, 0x00, 0x7f, 0, [](UserConfig& c) { return &c.pitch_down.cc_number; }},
    {0x52, 0x13, 0x00, 0x7f, 0, [](UserConfig& c) { return &c.pitch_down.on_value; }},
    {0x52, 0x14, 0x00, 0x7f, 0, [](UserConfig& c) { return &c.pitch_down.off_value; }},
    {0x53, 0x11, 0x00, 0x01, 0, [](UserConfig& c) { return &c.sustain.mode; }},
    {0x53, 0x12, 0x00, 0x7f, 0, [](UserConfig& c) { return &c.sustain.cc_number; }},
    {0x53, 0x13, 0x00, 0x7f, 0, [](UserConfig& c) { return &c.sustain.on_value; }},
    {0x53, 0x14, 0x00, 0x7f, 0, [](UserConfig& c) { return &c.sustain.off_value; }},
};
// clang-format on

SettingsChangedHandler settings_changed_handler_ = nullptr;
TransferStartedHandler transfer_started_handler_ = nullptr;

// NRPN parameter currently addressed by the editor (cleared by RPN Null).
int16_t nrpn_msb_ = kNoParameterNumber;
int16_t nrpn_lsb_ = kNoParameterNumber;

void sendParameter(const uint8_t msb, const uint8_t lsb, const uint8_t value)
{
    midi_process::sendUsbControlChange(kCcNrpnMsb, msb, kSettingChannel);
    midi_process::sendUsbControlChange(kCcNrpnLsb, lsb, kSettingChannel);
    midi_process::sendUsbControlChange(kCcDataEntryMsb, value, kSettingChannel);
    midi_process::sendUsbControlChange(kCcRpnMsb, kRpnNull, kSettingChannel);
    midi_process::sendUsbControlChange(kCcRpnLsb, kRpnNull, kSettingChannel);
}

void sendDump()
{
    for (const auto& parameter : kParameters) {
        const auto stored = *parameter.field(user_config::config);
        sendParameter(parameter.msb, parameter.lsb, static_cast<uint8_t>(stored - parameter.offset));
    }
    sendParameter(kSystemCommandMsb, kDumpCompleteLsb, 0x00);
    Serial.printf("Setting dump sent\n");
}

void sendVersion()
{
    sendParameter(kDeviceInfoMsb, kVersionMajorLsb, device_config::kFirmwareVersionMajor);
    sendParameter(kDeviceInfoMsb, kVersionMinorLsb, device_config::kFirmwareVersionMinor);
    sendParameter(kDeviceInfoMsb, kVersionPatchLsb, device_config::kFirmwareVersionPatch);
    Serial.printf("Firmware version sent: %d.%d.%d\n", device_config::kFirmwareVersionMajor,
                  device_config::kFirmwareVersionMinor, device_config::kFirmwareVersionPatch);
}

void handleSystemCommand(const uint8_t lsb, const uint8_t value)
{
    if (value != 0x00) {
        return;
    }
    switch (lsb) {
        case kDumpRequestLsb:
            sendDump();
            break;
        case kSaveLsb: {
            const auto saved = user_config::save();
            Serial.printf("Settings save %s\n", saved ? "succeeded" : "failed");
            if (saved) {
                sendParameter(kSystemCommandMsb, kAckLsb, 0x00);
            }
        } break;
        case kTransferStartLsb:
            Serial.printf("Setting transfer started\n");
            if (transfer_started_handler_) {
                transfer_started_handler_();
            }
            break;
        case kVersionRequestLsb:
            sendVersion();
            break;
        default:
            break;
    }
}

void applyParameter(const uint8_t msb, const uint8_t lsb, const uint8_t value)
{
    for (const auto& parameter : kParameters) {
        if (parameter.msb != msb || parameter.lsb != lsb) {
            continue;
        }
        // out-of-range values are ignored
        if (value < parameter.min || value > parameter.max) {
            return;
        }
        *parameter.field(user_config::config) = static_cast<uint8_t>(value + parameter.offset);
        Serial.printf("Setting [%02x %02x] = %d\n", msb, lsb, value);
        if (settings_changed_handler_) {
            settings_changed_handler_();
        }
        return;
    }
    // undefined parameters are ignored
}
}  // namespace

void initialize(SettingsChangedHandler settings_changed_handler, TransferStartedHandler transfer_started_handler)
{
    settings_changed_handler_ = settings_changed_handler;
    transfer_started_handler_ = transfer_started_handler;
    nrpn_msb_                 = kNoParameterNumber;
    nrpn_lsb_                 = kNoParameterNumber;
    midi_process::setUsbControlChangeHandler(handleControlChange);
}

void handleControlChange(const uint8_t channel, const uint8_t number, const uint8_t value)
{
    if (channel != kSettingChannel) {
        return;
    }

    switch (number) {
        case kCcNrpnMsb:
            nrpn_msb_ = value;
            return;
        case kCcNrpnLsb:
            nrpn_lsb_ = value;
            return;
        case kCcRpnMsb:
        case kCcRpnLsb:
            // RPN Null, or an RPN being selected: either way no NRPN is addressed anymore
            nrpn_msb_ = kNoParameterNumber;
            nrpn_lsb_ = kNoParameterNumber;
            return;
        case kCcDataEntryMsb:
            break;
        default:
            return;
    }

    if (nrpn_msb_ == kNoParameterNumber || nrpn_lsb_ == kNoParameterNumber) {
        return;
    }

    const auto msb = static_cast<uint8_t>(nrpn_msb_);
    const auto lsb = static_cast<uint8_t>(nrpn_lsb_);
    if (msb == kSystemCommandMsb) {
        handleSystemCommand(lsb, value);
    } else {
        applyParameter(msb, lsb, value);
    }
}
}  // namespace kinoshita_lab::kinoshi_tiny_key_25::setting_transfer
