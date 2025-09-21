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

namespace kinoshita_lab::tiny_kino_key_25::application
{
namespace{

switches::Switches switches_(pins::kPinPl, pins::kPinCp, pins::kPinSerialOut1, pins::kPinSerialOut2, pins::kPinSerialOut3, nullptr);


struct Status
{
    bool timer_fired        = false;
    int current_octave      = config::kDefaultOctave;
    uint8_t noteon_velocity = INT8_MAX;

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
    switches_.setHandler(switchStateChanged);
    leds::initialize();
    leds::setOctaveLed(status_.current_octave);
    midi_process::initialize();
}

void timerFired()
{
    status_.timer_fired = true;
}

void processKeyboard()
{
    for (auto i = 0; i < config::kNumKeyboardKeys; ++i) {
        if (status_.keyboard_status[i].shouldSend) {
            status_.keyboard_status[i].shouldSend = false;
            constexpr int num_note_per_octave     = 12;
            const auto note_number                = (status_.current_octave + 1) * num_note_per_octave + i;
        }
    }
}

void switchStateChanged(uint32_t switch_index, const int off_on)
{
    Serial.printf("Switch %d is %s\n", switch_index, off_on ? "ON" : "OFF");

    // keyboard keys
    switch (switch_index) {
        case switches::Switches::kSwitchIdC1... switches::Switches::kSwitchIdF2: {  // keyboard keys
            const auto key_index                          = switch_index - switches::Switches::kSwitchIdC1;
            status_.keyboard_status[key_index].shouldSend = true;
            status_.keyboard_status[key_index].offOn      = off_on;
        }
            return;
        default:
            break;
    }

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
    // off
    switch (switch_index) {
        default:
            break;
    }
}

void loop()
{
    processTimerTick();
    processKeyboard();
    midi_process::loop();
}
void processTimerTick()
{
    if (!status_.timer_fired) {
        return;
    }
    status_.timer_fired = false;
    
}
}  // namespace kinoshita_lab::tiny_kino_key_25::application