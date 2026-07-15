/*
 * Troy's HBC-56 Emulator
 * Copyright (c) 2021 Troy Schrapel
 * Adapted for ORIC ATMOS FOR ONE GAME PAD
 * F4GOH 2026
 */

#pragma once

#include "vga.h"

typedef enum
{
  VGA_640_480_60HZ,
  VGA_640_400_70HZ,
  VGA_800_600_60HZ,
  VGA_1024_768_60HZ,
  VGA_1280_1024_60HZ,
} VgaMode;

extern VgaParams vgaGetParams(VgaMode mode, int pixelScale);
