/*
 * Troy's HBC-56 Emulator
 * Copyright (c) 2021 Troy Schrapel
 * Adapted for ORIC ATMOS
 * F4GOH 2026
 */

#pragma once

#include <inttypes.h>
//#include "music.h"

#define AY38910_CLOCK 1000000
#define SAMPLE_FREQUENCY 31480

#define AUDIO_PIN 28
#define ALPHA 60  //pour le filtre numérique audio


void audioInit(int psgClock, int sampleRate);

void audioUpdate();

uint8_t audioReadPsg0();

void audioWritePsg0(uint16_t addr, uint8_t val);

uint8_t audioSize();
uint8_t* audioSave(void);
void audioLoad(const uint8_t* buffer);
