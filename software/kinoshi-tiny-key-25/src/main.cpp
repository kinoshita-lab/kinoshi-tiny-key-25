/**
 * @file	main.cpp
 * @brief	Main loop for Tiny KinoKey 25
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 * Copyright (c) 2025 Kinoshita Laboratory All rights reserved.
 */
#include <Arduino.h>

#include <pico/stdlib.h>
#include "application.h"
#include "config.h"
using namespace kinoshita_lab::kinoshi_tiny_key_25;

// timer callbacks
struct repeating_timer timer;
bool timer_callback(struct repeating_timer* t)
{
    application::timerFired();
    return true;
}

void setup()
{
    Serial.begin(115200);

    application::initialize();
    // start timer
    add_repeating_timer_us(config::kApplicationTimerIntervalUs, timer_callback, NULL, &timer);

}

void loop()
{
    kinoshita_lab::kinoshi_tiny_key_25::application::loop();
}
