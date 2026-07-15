/*
 * Troy's HBC-56 Emulator
 * Copyright (c) 2021 Troy Schrapel
 * Adapted for ORIC ATMOS FOR ONE GAME PAD
 * F4GOH 2026
 */

#pragma once

#include <inttypes.h>

char processAsciiToPs2(char c);

extern uint8_t ascii2Ps2[128];
