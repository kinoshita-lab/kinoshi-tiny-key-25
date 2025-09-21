/**
 * @file	midi_process.cpp
 * @brief	MIDI processing implementation
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 * Copyright (c) 2025 Kinoshita Laboratory All rights reserved.
 *
 */
#include "MIDI.h"
#include <Adafruit_TinyUSB.h>
#include "midi_process.h"
#include "config.h"

namespace kinoshita_lab::tiny_kino_key_25::midi_process
{
namespace
{
Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI_USB);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI_Serial);
}
void initialize()
{
    // Manual begin() is required on core without built-in support e.g. mbed rp2040
    TinyUSBDevice.setManufacturerDescriptor(config::kUsbManufacturerString);
    TinyUSBDevice.setProductDescriptor(config::kUsbProductDescriptor);
    TinyUSBDevice.setSerialDescriptor(config::kUsbSerialDescriptor);
    if (!TinyUSBDevice.isInitialized()) {
        TinyUSBDevice.begin(0);
    }

    usb_midi.setStringDescriptor(config::kUsbMidiStringDescriptor);

    MIDI_USB.begin(MIDI_CHANNEL_OMNI);
    MIDI_USB.setThruFilterMode(midi::Thru::Off);
    if (TinyUSBDevice.mounted()) {
        TinyUSBDevice.detach();
        delay(10);
        TinyUSBDevice.attach();
    }
    MIDI_Serial.begin(MIDI_CHANNEL_OMNI);
    MIDI_Serial.setThruFilterMode(midi::Thru::Off);
}
void loop()
{
#ifdef TINYUSB_NEED_POLLING_TASK
    // Manual call tud_task since it isn't called by Core's background
    TinyUSBDevice.task();
#endif
}
void sendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel)
{
    if (note > 127 || velocity > 127 || channel < 1 || channel > 16) {
        return;
    }

    MIDI_USB.sendNoteOn(note, velocity, channel);
    MIDI_Serial.sendNoteOn(note, velocity, channel);
}
void sendNoteOff(uint8_t note, uint8_t velocity, uint8_t channel)
{
    if (note > 127 || velocity > 127 || channel < 1 || channel > 16) {
        return;
    }
     
    MIDI_USB.sendNoteOff(note, velocity, channel);
    MIDI_Serial.sendNoteOff(note, velocity, channel);
}
}