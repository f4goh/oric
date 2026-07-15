/*
 * Troy's HBC-56 Emulator
 * Copyright (c) 2021 Troy Schrapel
 * Adapted for ORIC ATMOS FOR ONE GAME PAD
 * F4GOH 2026
 */

#pragma once

#include "hardware/pio.h"

void pio_set_x(PIO pio, uint sm, uint32_t x);
void pio_set_y(PIO pio, uint sm, uint32_t y);
