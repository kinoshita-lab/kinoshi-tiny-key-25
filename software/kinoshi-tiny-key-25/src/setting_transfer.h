/**
 * @file	setting_transfer.h
 * @brief	Setting transfer with the editor over NRPN for Tiny KinoKey 25
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 * Copyright (c) 2026 Kinoshita Laboratory All rights reserved.
 *
 * Implements the setting data of the Kinoshi-Tiny Key 25 MIDI Implementation
 * (MIDI_Implementation.txt at the repository root).
 * Setting data is received and transmitted on USB MIDI, channel 1 only.
 */
#pragma once
#ifndef SETTING_TRANSFER_H
#define SETTING_TRANSFER_H

#include <cstdint>

namespace kinoshita_lab::kinoshi_tiny_key_25::setting_transfer
{
// Called after a setting value has been changed by the editor.
using SettingsChangedHandler = void (*)();
// Called when the editor starts transmitting settings (TRANSFER START).
using TransferStartedHandler = void (*)();

/// Registers the USB MIDI receive handler.
void initialize(SettingsChangedHandler settings_changed_handler, TransferStartedHandler transfer_started_handler);

/// Feeds a Control Change received on USB MIDI.
void handleControlChange(uint8_t channel, uint8_t number, uint8_t value);
}  // namespace kinoshita_lab::kinoshi_tiny_key_25::setting_transfer

#endif  // SETTING_TRANSFER_H
