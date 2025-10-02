/**
 * @file	application.cpp
 * @brief	Application logic for Tiny KinoKey 25
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 * Copyright (c) 2025 Kinoshita Laboratory All rights reserved.
 */
#include <Arduino.h>
#include <algorithm>
#include "application.h"
#include "leds.h"
#include "config.h"
#include "switch.hpp"
#include "midi_process.h"
#include "pins.h"

extern "C" {
#include "pico/bootrom.h"
}

namespace kinoshita_lab::tiny_kino_key_25::application
{
namespace
{

switches::Switches switches_(pins::kPinPl, pins::kPinCp, pins::kPinSerialOut1, pins::kPinSerialOut2, pins::kPinSerialOut3, nullptr);

struct Status
{
    bool timer_fired            = false;
    int current_octave          = config::kDefaultOctave;
    uint8_t noteon_velocity     = INT8_MAX;
    int16_t pitch_bend_value    = 0;  // center
    int pitch_bend_direction    = 0;  // -1, 0, 1
    bool should_send_pitch_bend = false;
    const float pitch_bend_step = (8192.f / (config::pitch_bend_time / 1000.f)) / (1000.f * 1000.f / config::kApplicationTimerIntervalUs);  // per timer tick
    struct KeyboardStatus
    {
        bool shouldSend;
        bool offOn;
        int8_t noteOnNoteNumber = -1;  // MIDI note number for "Note On" event
    };
    KeyboardStatus keyboard_status[config::kNumKeyboardKeys];
};
Status status_;

void setOctaveWithDelta(const int delta)
{
    const auto prev    = status_.current_octave;
    const auto new_val = std::clamp<int>(prev + delta, config::kMinOctave, config::kMaxOctave);
    if (new_val == prev) {
        return;
    }
    Serial.printf("Octave changed: %d -> %d\n", prev, new_val);
    leds::setOctaveLed(new_val);
    status_.current_octave = new_val;
}
}
void initialize()
{
    status_.current_octave = config::kDefaultOctave;
    // initial switch read
    switches_.forceScan();

    // read switch status whether to enter update mode
    auto in_update_mode = switches_.switchIsOn(switches::Switches::kSwitchIdModulation) &&
                          switches_.switchIsOn(switches::Switches::kSwitchIdOctMinus);

    if (in_update_mode) {
        // enter update mode
        reset_usb_boot(0,0);
        return;
    }

    switches_.setHandler(switchStateChanged);
    leds::initialize();
    leds::setOctaveLed(status_.current_octave);
    midi_process::initialize();
}

void timerFired()
{
    status_.timer_fired = true;  // rough timer flag
    // process pitch bend(cannot process in idle loop because of timing)
    // calc value, send on loop
    if (status_.pitch_bend_direction != 0) {
        const auto new_pitch_bend_value = std::clamp<int16_t>(status_.pitch_bend_value + status_.pitch_bend_direction * status_.pitch_bend_step, -8192, 8191);
        if (new_pitch_bend_value == status_.pitch_bend_value) {
            // reached max/min
            return;
        }
        status_.pitch_bend_value       = new_pitch_bend_value;
        status_.should_send_pitch_bend = true;
    }
}

void processKeyboard()
{
    for (auto i = 0; i < config::kNumKeyboardKeys; ++i) {
        if (status_.keyboard_status[i].shouldSend) {
            status_.keyboard_status[i].shouldSend = false;
            if (status_.keyboard_status[i].offOn) {
                // on
                constexpr int num_note_per_octave           = 12;
                const auto on_note_number                   = (status_.current_octave + 1) * num_note_per_octave + i;
                status_.keyboard_status[i].noteOnNoteNumber = on_note_number;
                midi_process::sendNoteOn(on_note_number, status_.noteon_velocity, config::kMidiChannel);
                Serial.printf("Note On sent: note=%d, velocity=%d, channel=%d\n",
                              on_note_number, status_.noteon_velocity, config::kMidiChannel);
            } else {  // off
                const auto off_note_number = status_.keyboard_status[i].noteOnNoteNumber;
                if (off_note_number >= 0) {
                    midi_process::sendNoteOff(off_note_number, 0, config::kMidiChannel);
                    Serial.printf("Note Off sent: note=%d, velocity=0, channel=%d\n",
                                  off_note_number, config::kMidiChannel);
                    status_.keyboard_status[i].noteOnNoteNumber = -1;
                }
            }
        }
    }
}

void switchStateChanged(uint32_t switch_index, const int off_on)
{
    Serial.printf("Switch %d is %s\n", switch_index, off_on ? "ON" : "OFF");

    switch (switch_index) {
        case switches::Switches::kSwitchIdC1... switches::Switches::kSwitchIdF2: {  // keyboard keys
            const auto key_index                          = switch_index - switches::Switches::kSwitchIdC1;
            status_.keyboard_status[key_index].shouldSend = true;
            status_.keyboard_status[key_index].offOn      = off_on;
        }
            return;
        case switches::Switches::kSwitchIdSustain:
            midi_process::sendSustain(off_on != 0, config::kMidiChannel);
            return;
        case switches::Switches::kSwitchIdModulation:
            midi_process::sendMoulation(off_on != 0, config::kMidiChannel);
            return;
        case switches::Switches::kSwitchIdPitchBendPlus:
            status_.pitch_bend_direction   = off_on ? 1 : 0;
            status_.should_send_pitch_bend = true;  // send immediately
            return;
        case switches::Switches::kSwitchIdPitchBendMinus:
            status_.pitch_bend_direction   = off_on ? -1 : 0;
            status_.should_send_pitch_bend = true;  // send immediately
            return;
        default:
            break;
    }

    // on trigger switches
    if (off_on) {
        switch (switch_index) {
            case switches::Switches::kSwitchIdOctMinus:
                setOctaveWithDelta(-1);
                break;
            case switches::Switches::kSwitchIdOctPlus:
                setOctaveWithDelta(1);
                break;
            default:
                break;
        }
        return;
    }
}
void processPitchBend();

void loop()
{
    switches_.update();
    processTimerTick();
    processKeyboard();
    processPitchBend();
    midi_process::loop();
}
void processPitchBend()
{
    if (!status_.should_send_pitch_bend) {
        return;
    }
    status_.should_send_pitch_bend = false;

    if (status_.pitch_bend_direction == 0) {
        status_.pitch_bend_value = 0;  // reset to center when stopped
    }

    midi_process::sendPitchBend(status_.pitch_bend_value, config::kMidiChannel);
}

void processTimerTick()
{
    if (!status_.timer_fired) {
        return;
    }
    status_.timer_fired = false;
}
}  // namespace kinoshita_lab::tiny_kino_key_25::application