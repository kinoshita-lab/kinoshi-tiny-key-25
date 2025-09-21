/**
 * @file	application.h
 * @brief	Application logic for Tiny KinoKey 25
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 * Copyright (c) 2025 Kinoshita Laboratory All rights reserved.
 */
#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H

#include <cstdint>

namespace kinoshita_lab::tiny_kino_key_25::application
{
void initialize();
void timerFired();
void loop();
void processTimerTick();

void switchStateChanged(uint32_t switch_index, const int off_on);
} // namespace kinoshita_lab::tiny_kino_key_25::application
#endif // APPLICATION_H
