/*
 * Troy's HBC-56 Emulator
 * Copyright (c) 2021 Troy Schrapel
 * Adapted for ORIC ATMOS FOR ONE GAME PAD
 * F4GOH 2026
 */

 #pragma once

 #include <inttypes.h>

bool nes_begin();

void nes_read_start(void);
void nes_read_finish(void);

uint8_t nes_get_state_1();
//uint8_t nes_get_state_2();
