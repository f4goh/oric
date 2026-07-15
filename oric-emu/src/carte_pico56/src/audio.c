/*
 * Project: pico-56 - audio
 *
 * Copyright (c) 2023 Troy Schrapel
 *
 * This code is licensed under the MIT license
 *
 * https://github.com/visrealm/pico-56
 *
 */

#include "audio.h"
#include "emu2149.h"

#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include <stdlib.h>
#include <string.h>

#define PWM_WRAP 1024

static PSG* psg0 = NULL;

static uint audioSlice = 0;
static uint8_t psg0Reg = 0;
static int32_t lp = 512;   // état filtre
static uint32_t music_pos = 0;

/*
 * create a PSG 'object'
 */
static PSG* createPSG(int psgClock, int sampleRate)
{
  PSG* psg = PSG_new(psgClock, sampleRate);
  PSG_setVolumeMode(psg, EMU2149_VOL_AY_3_8910);  // AY-3-8910 mode
  PSG_reset(psg);
  return psg;
}

/*
 * initialize the audio
 */
void audioInit(int psgClock, int sampleRate)
{
  psg0 = createPSG(psgClock, sampleRate);
 
  gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);
  gpio_set_function(AUDIO_PIN + 1, GPIO_FUNC_PWM);

  audioSlice = pwm_gpio_to_slice_num(AUDIO_PIN);

  // set up the PWM
  pwm_set_clkdiv_int_frac(audioSlice, 1, 0);
  pwm_set_wrap(audioSlice, PWM_WRAP);
  pwm_set_both_levels(audioSlice, 0, 0);
  pwm_set_enabled(audioSlice, true);
  /*
   pwm_set_enabled(audioSlice, false);
    pwm_set_clkdiv(audioSlice, 1.0f);
    pwm_set_wrap(audioSlice, 1023);
    pwm_set_both_levels(audioSlice, 0, 0);
    pwm_set_enabled(audioSlice, true);
    */
}

/*
 * update the audio
 *  - called at a frequency matching the sample rate
 */
void audioUpdate()
{
  // generate audio data
  int16_t sample = PSG_calc(psg0);
  

  // convert signed audio -> PWM (12-bit RP2040)
    uint16_t pwm = (uint16_t)((sample >> 4) + 512);
    
    
    // clamp input
    if (pwm < 0) pwm = 0;
    if (pwm > 1023) pwm = 1023;

    // =========================
    // SIMPLE SOFTWARE FILTER
    // =========================
    
    lp = lp + ((ALPHA * (pwm - lp)) >> 8);  // alpha = 20

    pwm = lp;

    if (pwm < 0) pwm = 0;
    if (pwm > 1023) pwm = 1023;
      
    
    // MONO STRICT → même valeur sur A et B
    pwm_set_both_levels(audioSlice, pwm, pwm);
    /*
    // 2) 50 Hz music update
    static uint32_t div = 0;
    static uint32_t DIVIDER = SAMPLE_FREQUENCY / 50;

    if (++div >= DIVIDER)
    {
        div = 0;

        // write full PSG frame (16 registers)
        for (uint8_t reg = 0; reg < 16; reg++)
        {
            uint8_t val = music[music_pos++];

            audioWritePsg0(0, reg);  // latch register
            audioWritePsg0(1, val);  // write value
        }

        if (music_pos >= LENGTH)
            music_pos = 0;
    }
    */
}

/*
 * read the current register value from psg0
 */
uint8_t audioReadPsg0()
{
  return PSG_readReg(psg0, psg0Reg);
}


/*
 * write to psg0
 *  - lsb of addr determines BC1 signal
 */
void audioWritePsg0(uint16_t addr, uint8_t val)
{
  if ((addr & 0x01) == 0)
  {
    psg0Reg = val;
  }
  else
  {
    PSG_writeReg(psg0, psg0Reg, val);
  }
}

uint8_t audioSize(){
    return sizeof(savePSG);
}


uint8_t* audioSave(void)
{
    return (uint8_t*)psg0;
}

void audioLoad(const uint8_t* buffer)
{
   memcpy(psg0, buffer, sizeof(savePSG)); 
}
