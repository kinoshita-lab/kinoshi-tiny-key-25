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
#include "device_config.h"
#include "user_config.h"

namespace kinoshita_lab::kinoshi_tiny_key_25::midi_process
{
namespace
{
Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI_USB);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI_Serial);

// Max time to wait for room in the USB MIDI TX FIFO before dropping a message.
constexpr uint32_t kUsbWriteTimeoutMs = 100;

bool isValidChannel(const uint8_t channel)
{
    return channel >= 1 && channel <= 16;
}
}
void initialize()
{
    // Manual begin() is required on core without built-in support e.g. mbed rp2040
    TinyUSBDevice.setManufacturerDescriptor(device_config::kUsbManufacturerString);
    TinyUSBDevice.setProductDescriptor(device_config::kUsbProductDescriptor);
    TinyUSBDevice.setSerialDescriptor(device_config::kUsbSerialDescriptor);
    if (!TinyUSBDevice.isInitialized()) {
        TinyUSBDevice.begin(0);
    }

    usb_midi.setStringDescriptor(device_config::kUsbMidiStringDescriptor);

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
    while (MIDI_USB.read()) {
        // dispatches to the registered handlers
    }
}
void setUsbControlChangeHandler(ControlChangeHandler handler)
{
    MIDI_USB.setHandleControlChange(handler);
}
void sendNoteOn(uint8_t note, uint8_t velocity)
{
    if (note > 127 || velocity > 127) {
        return;
    }
    const auto& config = user_config::config;
    if (isValidChannel(config.usb_midi_channel)) {
        MIDI_USB.sendNoteOn(note, velocity, config.usb_midi_channel);
    }
    if (isValidChannel(config.trs_midi_channel)) {
        MIDI_Serial.sendNoteOn(note, velocity, config.trs_midi_channel);
    }
}
void sendNoteOff(uint8_t note, uint8_t velocity)
{
    if (note > 127 || velocity > 127) {
        return;
    }
    const auto& config = user_config::config;
    if (isValidChannel(config.usb_midi_channel)) {
        MIDI_USB.sendNoteOff(note, velocity, config.usb_midi_channel);
    }
    if (isValidChannel(config.trs_midi_channel)) {
        MIDI_Serial.sendNoteOff(note, velocity, config.trs_midi_channel);
    }
}
void sendControlChange(uint8_t number, uint8_t value)
{
    if (number > 127 || value > 127) {
        return;
    }
    const auto& config = user_config::config;
    if (isValidChannel(config.usb_midi_channel)) {
        MIDI_USB.sendControlChange(number, value, config.usb_midi_channel);
    }
    if (isValidChannel(config.trs_midi_channel)) {
        MIDI_Serial.sendControlChange(number, value, config.trs_midi_channel);
    }
}
void sendPitchBend(int16_t value)
{
    if (value < -8192 || value > 8191) {
        return;
    }
    const auto& config = user_config::config;
    if (isValidChannel(config.usb_midi_channel)) {
        MIDI_USB.sendPitchBend(value, config.usb_midi_channel);
    }
    if (isValidChannel(config.trs_midi_channel)) {
        MIDI_Serial.sendPitchBend(value, config.trs_midi_channel);
    }
}
void sendUsbControlChange(uint8_t number, uint8_t value, uint8_t channel)
{
    if (number > 127 || value > 127 || !isValidChannel(channel)) {
        return;
    }
    if (!TinyUSBDevice.mounted()) {
        return;
    }

    // The TX FIFO holds only 16 messages, so a burst (e.g. a setting dump) has to
    // wait until the host drains it. Write whole packets so a message is never split.
    constexpr uint8_t kCodeIndexControlChange = 0x0b;
    const uint8_t packet[4]                   = {kCodeIndexControlChange, static_cast<uint8_t>(0xb0 | (channel - 1)), number, value};
    const auto start                          = millis();
    while (!usb_midi.writePacket(packet)) {
        if (millis() - start >= kUsbWriteTimeoutMs) {
            return;
        }
        TinyUSBDevice.task();
    }
}
}  // namespace kinoshita_lab::kinoshi_tiny_key_25::midi_process
