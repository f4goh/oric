/*
 * Troy's HBC-56 Emulator
 * Copyright (c) 2021 Troy Schrapel
 * Adapted for ORIC ATMOS FOR ONE GAME PAD
 * F4GOH 2026
 */

#pragma once

#include <inttypes.h>
#include <stdbool.h>

#define KBD_INT 2

bool ps2kbd_begin();
uint8_t ps2kbd_read();
void ps2kbd_write(uint8_t value);

extern bool isShifted;
extern bool isControlled;

extern bool kbdQueueEmpty();

extern void kbdQueuePush(uint8_t scancode);

extern uint8_t kbdQueuePop();

extern char processAsciiToPs2(char c);

extern uint8_t ascii2Ps2[128];
