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

namespace kinoshita_lab::tiny_kino_key_25::midi_process
{
void initialize();
void loop();

void sendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel);
void sendNoteOff(uint8_t note, uint8_t velocity, uint8_t channel);
void sendMoulation(bool on, uint8_t channel);
void sendPitchBend(int16_t value, uint8_t channel);
void sendSustain(bool on, uint8_t channel);
}  // namespace kinoshita_lab::tiny_kino_key_25::midi_process

#endif  // MIDI_H
