/*
 * Troy's HBC-56 Emulator
 * Copyright (c) 2021 Troy Schrapel
 * Adapted for ORIC ATMOS
 * F4GOH 2026
 */


#ifndef _ORIC_CONFIG_H_
#define _ORIC_CONFIG_H_


/* emulator configuration values 
  -------------------------------------------------------------------------- */
#define ORIC_HAVE_THREADS      0

#define ORIC_CLOCK_FREQ        1000000   /* half of 7.3728 */
//#define ORIC_AUDIO_FREQ        48000
//#define ORIC_MAX_DEVICES       16

/* memory map configuration values 
  -------------------------------------------------------------------------- */
#define ORIC_RAM_START         0x0000
#define ORIC_RAM_SIZE          0xc000

#define ORIC_ROM_START         0xc000
#define ORIC_ROM_SIZE          0x4000

#define ORIC_IO_START          0x0300
#define ORIC_IO_SIZE           0x0010

#define ORIC_MONITOR_SIZE      0x0460 //1120
/* device configuration values 
  -------------------------------------------------------------------------- */
#define ORIC_HAVE_AY_3_8910    1
#define ORIC_AY_3_8910_COUNT   1
#define ORIC_AY38910_CLOCK     1000000

#define ORIC_HAVE_VIA          1
#define ORIC_VIA_PORT          0x0300

/* computed configuration values (shouldn't need to touch these) 
  -------------------------------------------------------------------------- */
#define ORIC_RAM_END           (ORIC_RAM_START + ORIC_RAM_SIZE) /* one past end */
#define ORIC_ROM_END           (ORIC_ROM_START + ORIC_ROM_SIZE) /* one past end */
#define ORIC_IO_PORT_MASK      (ORIC_IO_SIZE - 1)

#define ORIC_IO_ADDRESS(p)     (ORIC_IO_START | (p & ORIC_IO_PORT_MASK))


#endif