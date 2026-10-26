/**
 * @file	midi_process.h
 * @brief	MIDI processing header
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 * Copyright (c) 2025 Kinoshita Laboratory All rights reserved.
 */
#pragma once
#ifndef MIDI_H
#define MIDI_H

#include <cstdint>

namespace kinoshita_lab::kinoshi_tiny_key_25::midi_process
{
// Called for Control Change received on USB MIDI.
using ControlChangeHandler = void (*)(uint8_t channel, uint8_t number, uint8_t value);

void initialize();
void loop();

void setUsbControlChangeHandler(ControlChangeHandler handler);

// Performance messages are sent to USB MIDI and TRS MIDI, each on its own
// channel (user_config::config.usb_midi_channel / trs_midi_channel).
void sendNoteOn(uint8_t note, uint8_t velocity);
void sendNoteOff(uint8_t note, uint8_t velocity);
void sendControlChange(uint8_t number, uint8_t value);
void sendPitchBend(int16_t value);

// Sends a Control Change to USB MIDI only (used for setting data).
void sendUsbControlChange(uint8_t number, uint8_t value, uint8_t channel);
}  // namespace kinoshita_lab::tiny_kino_key_25::midi_process

#endif  // MIDI_H
