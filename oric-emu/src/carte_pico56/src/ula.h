#ifndef ULA_H
#define ULA_H

#include <stdint.h>
#include <stdbool.h>

//#include "videoMemLores.h"
//#include "videoMemHires.h"
#include "bus.h"


#define ORIC_FREQUENCY     1000000
#define ORIC_MAX_TAPE_SIZE (1 << 16)

#define ORIC_SCREEN_WIDTH  240
#define ORIC_SCREEN_HEIGHT 224

#define ORIC_FRAMEBUFFER_SIZE (ORIC_SCREEN_WIDTH / 2)

#define PATTR_50HZ  0x02
#define PATTR_HIRES 0x04

#define LATTR_ALT   0x01
#define LATTR_DSIZE 0x02
#define LATTR_BLINK 0x04

//#define RAM_SIZE 0xC000


typedef struct
{
    //uint8_t ram[RAM_SIZE];
    int blink_counter;
    uint8_t pattr;
    uint8_t pattrSave;
    bool enMonitor;
    uint8_t fb[ORIC_FRAMEBUFFER_SIZE];
} Ula;


extern volatile uint8_t ula_kbdScancode;
extern volatile uint8_t ula_nesValue;
extern volatile uint8_t ula_nesSt;

void ula_init(Ula* u);
void ula_reset(Ula* u);
void ula_monitor(Ula* u,bool _enMonitor);


void ula_scanline(Ula* u, uint16_t y, uint16_t* out);
void ula_endframe(Ula* u);

#endif