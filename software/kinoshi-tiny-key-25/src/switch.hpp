/**
 * @file	switch.hpp
 * @brief    Switch scanning for Tiny KinoKey 25	
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 * Copyright (c) 2025 Kinoshita Laboratory All rights reserved.
 */
#pragma once
#ifndef SWITCH_HPP
#define SWITCH_HPP

#include <cstdint>
#include <cstdlib>
#include <Arduino.h>
#include <functional>
namespace kinoshita_lab::kinoshi_tiny_key_25::switches
{
// product specific switch scanner
class Switches
{
public:
    enum SwitchIds
    {
        kSwitchIdC1 = 0,

        kSwitchIdGs1,
        kSwitchIdG1,
        kSwitchIdFs1,
        kSwitchIdF1,
        kSwitchIdE1,
        kSwitchIdDs1,
        kSwitchIdD1,
        kSwitchIdCs1,

        kSwitchIdE2,
        kSwitchIdDs2,
        kSwitchIdD2,
        kSwitchIdCs2,
        kSwitchIdC2,
        kSwitchIdB1,
        kSwitchIdAs1,
        kSwitchIdA1,

        kSwitchIdC3,
        kSwitchIdB2,
        kSwitchIdAs2,
        kSwitchIdA2,
        kSwitchIdGs2,
        kSwitchIdG2,
        kSwitchIdFs2,
        kSwitchIdF2,

        kSwitchIdSustain,
        kSwitchIdPitchBendPlus,
        kSwitchIdPitchBendMinus,
        kSwitchIdModulation,
        kSwitchIdOctMinus,
        kSwitchIdOctPlus,

        kNumSwitches,
    };

    enum
    {                                 // misc. constants
        kNumRequiredClockCycles = 16, // U4, U5 is cascaded, so 16 clock cycles are required to read all switches
        kScanPeriod             = 10, // ms

    };
    using SwitchHandler = std::function<void(uint32_t switch_index, const int off_on)>;

    enum InternalState
    {
        Init,
        LoadStart,
        ReadEachBits,
        WaitNext,
        NumNormalStates,
        UnknownState = 0xff,
    };

    Switches(
        const uint8_t npl_pin, const uint8_t clock_pin,
        const uint8_t output1_pin, const uint8_t output2_pin, const uint8_t output3_pin, SwitchHandler handler = nullptr)
        : pins_(npl_pin, clock_pin, output1_pin, output2_pin, output3_pin),
          handler_(handler)
    {

        // initialize switch status
        for (auto i = 0u; i < kNumSwitches; ++i) {
            scan_buffers_[i]        = 1;
            former_scan_buffers_[i] = 1;
            switch_status_[i]       = 1;
        }
        pinMode(npl_pin, OUTPUT);
        pinMode(clock_pin, OUTPUT);
        pinMode(output1_pin, INPUT_PULLUP);
        pinMode(output2_pin, INPUT_PULLUP);
        pinMode(output3_pin, INPUT_PULLUP);

        setState(Init);
    }

    void setHandler(SwitchHandler handler)
    {
        handler_ = handler;
    }

    virtual ~Switches() = default;

    uint8_t toSwitchId(const uint8_t ic_index, const uint8_t bit_index, const uint8_t read_index) const
    {
        if (ic_index >= kNumRequiredClockCycles / 8 || bit_index >= 8) {
            return kNumSwitches; // out of range
        }
        if (ic_index == 0) {       // U2, U3, U5
            if (read_index == 0) { // U3
                switch (bit_index) {
                case 0:
                    return kSwitchIdCs1;
                case 1:
                    return kSwitchIdD1;
                case 2:
                    return kSwitchIdDs1;
                case 3:
                    return kSwitchIdE1;
                case 4:
                    return kSwitchIdF1;
                case 5:
                    return kSwitchIdFs1;
                case 6:
                    return kSwitchIdG1;
                case 7:
                    return kSwitchIdGs1;
                default:
                    return kNumSwitches; // out of range
                }
            }
            if (read_index == 1) { // U5
                switch (bit_index) {
                case 0:
                    return kSwitchIdF2;
                case 1:
                    return kSwitchIdFs2;
                case 2:
                    return kSwitchIdG2;
                case 3:
                    return kSwitchIdGs2;
                case 4:
                    return kSwitchIdA2;
                case 5:
                    return kSwitchIdAs2;
                case 6:
                    return kSwitchIdB2;
                case 7:
                    return kSwitchIdC3;
                default:
                    return kNumSwitches; // out of range
                }
            }
            if (read_index == 2) { // U2
                switch (bit_index) {
                case 2:
                    return kSwitchIdSustain;
                case 3:
                    return kSwitchIdPitchBendPlus;
                case 4:
                    return kSwitchIdPitchBendMinus;
                case 5:
                    return kSwitchIdModulation;
                case 6:
                    return kSwitchIdOctMinus;
                case 7:
                    return kSwitchIdOctPlus;
                default:
                    return kNumSwitches; // out of range
                }
            }
        }
        if (ic_index == 1) {       // KEY_C1, U4
            if (read_index == 0) { // KEY_C1
                switch (bit_index) {
                case 0:
                    return kSwitchIdC1;
                default:
                    return kNumSwitches; // out of range
                }
            }
            if (read_index == 1) { // U4
                switch (bit_index) {
                case 0:
                    return kSwitchIdA1;
                case 1:
                    return kSwitchIdAs1;
                case 2:
                    return kSwitchIdB1;
                case 3:
                    return kSwitchIdC2;
                case 4:
                    return kSwitchIdCs2;
                case 5:
                    return kSwitchIdD2;
                case 6:
                    return kSwitchIdDs2;
                case 7:
                    return kSwitchIdE2;
                default:
                    return kNumSwitches;
                }
            }
        }
        return kNumSwitches; // out of range
    }

    void update()
    {
        switch (status_) {
        case Init:
            setState(LoadStart);
            break;
        case LoadStart:
            setState(ReadEachBits);
            break;
        case ReadEachBits: {
            for (auto ic_index = 0u; ic_index < kNumRequiredClockCycles / 8; ic_index++) {
                constexpr auto num_bits = 8;
                for (auto i = 0u; i < num_bits; ++i) {
                    digitalWrite(pins_.clock_pin, LOW);
                    const auto read1_data = digitalRead(pins_.output1_pin);
                    const auto read2_data = digitalRead(pins_.output2_pin);
                    const auto read3_data = digitalRead(pins_.output3_pin);
                    const auto switch1Id  = toSwitchId(ic_index, i, 0);
                    const auto switch2Id  = toSwitchId(ic_index, i, 1);
                    const auto switch3Id  = toSwitchId(ic_index, i, 2);

                    if (switch1Id < kNumSwitches) {
                        scan_buffers_[switch1Id] = read1_data;
                    }
                    if (switch2Id < kNumSwitches) {
                        scan_buffers_[switch2Id] = read2_data;
                    }
                    if (switch3Id < kNumSwitches) {
                        scan_buffers_[switch3Id] = read3_data;
                    }
                    if (read3_data == LOW) {
                    //    Serial.printf("bit:%d read3_data: %d => switch id %d\n", i, read3_data, switch3Id);
                    }

                    digitalWrite(pins_.clock_pin, HIGH);
                }
            }
            updateSwitchStatus();
            setState(WaitNext);
            break;
        case WaitNext: {
            const auto current = millis();
            const auto delta   = current - wait_start_;
            if (delta >= kScanPeriod) {
                setState(LoadStart);
            }
        } break;
        default:
            break;
        }
        }
    };

    void forceScan()
    {
        // scan 2 times just in case for robustness
        for (auto i = 0; i < 4; ++i) {
            for (auto i = 0; i < NumNormalStates; ++i) {
                update();
                delay(kScanPeriod);
            }
        }
    }

    bool switchIsOn(const uint32_t switch_index) const
    {
        if (switch_index >= kNumSwitches) {
            return false;
        }

        return switch_status_[switch_index] == 0;
    }

protected:
    uint8_t status_          = UnknownState;
    size_t will_read_switch_ = 0;

    void setState(const int status)
    {
        status_ = status;
        assert(pins_.npl_pin != Pins::INVALID_PIN_CONFIGURATION);
        assert(pins_.clock_pin != Pins::INVALID_PIN_CONFIGURATION);
        assert(pins_.output1_pin != Pins::INVALID_PIN_CONFIGURATION);
        assert(pins_.output2_pin != Pins::INVALID_PIN_CONFIGURATION);
        assert(pins_.output3_pin != Pins::INVALID_PIN_CONFIGURATION);

        switch (status_) {
        case Init:
            digitalWrite(pins_.npl_pin, HIGH);
            digitalWrite(pins_.clock_pin, LOW);
            break;
        case LoadStart:
            digitalWrite(pins_.npl_pin, LOW);
            digitalWrite(pins_.clock_pin, LOW);
            will_read_switch_ = 0;
            break;
        case ReadEachBits:
            digitalWrite(pins_.clock_pin, LOW);
            digitalWrite(pins_.npl_pin, HIGH);
            break;
        case WaitNext:
            wait_start_ = millis();
            digitalWrite(pins_.npl_pin, HIGH);
            digitalWrite(pins_.clock_pin, LOW);
            break;
        default:
            break;
        }
    }

    void updateSwitchStatus()
    {
        for (auto i = 0u; i < kNumSwitches; ++i) {
            if (scan_buffers_[i] == former_scan_buffers_[i]) {
                const auto new_status = scan_buffers_[i];
                auto current_status   = switch_status_[i];
                if (current_status != new_status) {
                    switch_status_[i] = (new_status);

                    const auto notification_status = !switch_status_[i]; // NOTE: inverted!! off = HIGH, on = LOW
                    if (handler_) {
                        handler_(i, notification_status);
                    } else {
                        Serial.printf("Switch %d is %s\n", i, notification_status ? "ON" : "OFF");
                    }
                }
            }
            former_scan_buffers_[i] = scan_buffers_[i];
        }
    }
    struct Pins
    {
        enum
        {
            INVALID_PIN_CONFIGURATION = 0xff,
        };
        Pins(const uint8_t npl_pin, const uint8_t clock_pin,
             const uint8_t input1_pin, const uint8_t input2_pin, const uint8_t input3_pin)
            : npl_pin(npl_pin), clock_pin(clock_pin), output1_pin(input1_pin), output2_pin(input2_pin), output3_pin(input3_pin) {}

        Pins() = default;

        uint8_t npl_pin     = INVALID_PIN_CONFIGURATION;
        uint8_t clock_pin   = INVALID_PIN_CONFIGURATION;
        uint8_t output1_pin = INVALID_PIN_CONFIGURATION;
        uint8_t output2_pin = INVALID_PIN_CONFIGURATION;
        uint8_t output3_pin = INVALID_PIN_CONFIGURATION;
    };
    Pins pins_;
    SwitchHandler handler_ = nullptr;

    uint8_t scan_buffers_[kNumSwitches]        = {0};
    uint8_t former_scan_buffers_[kNumSwitches] = {0};
    uint8_t switch_status_[kNumSwitches]       = {0};
    uint32_t wait_start_                       = 0;

private:
    Switches(const Switches&) {}
};
} // namespace kinoshita_lab::tiny_kino_key_25::switches

#endif // SWITCH_HPP
