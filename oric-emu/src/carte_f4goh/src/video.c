
#include "pico/stdlib.h"
#include "hardware/clocks.h"

#include "video.h"
#include "vga.h"
#include "vga-modes.h"
#include "ula.h"
#include "audio.h"
#include <stdint.h>
#include <stdbool.h>


#define MAX_CLOCK 270000L

static Ula ula;

// ------------------------------------------------

static void setClosestClockFreqKhz(uint32_t clockKHz)
{
    if (clockKHz > MAX_CLOCK)
        clockKHz = MAX_CLOCK;

    clockKHz /= 10;
    clockKHz *= 10;

    uint32_t offset = 0;

    while (!set_sys_clock_khz(clockKHz + offset, false))
    {
        offset += 10;

        if (set_sys_clock_khz(clockKHz - offset, false))
            break;
    }
}

// ------------------------------------------------
// CALLBACKS
// ------------------------------------------------

//static void scanline(uint16_t y,VgaParams* params,uint16_t* pixels) {
static void scanline(uint16_t y,VgaParams* params,uint8_t* pixels) {
    ula_scanline(&ula, y, pixels);
}

static void end_frame(uint64_t frame) {
    //ula.endframe();
    ula_endframe(&ula);
}

static void end_scanline() {
    audioUpdate();
}

void monitorEnable(bool monEn){
    ula_monitor(&ula,monEn);
}

uint8_t getpattr(){
    return ula.pattrSave;
}


// ------------------------------------------------

void videoInit() {
    
    ula_init(&ula);
    ula_reset(&ula);
    
    VgaInitParams initParams;

    initParams.params =
            vgaGetParams(VGA_640_480_60HZ, 2);

    initParams.scanlineFn = scanline;
    initParams.endOfFrameFn = end_frame;
    initParams.endOfScanlineFn = end_scanline;

    uint32_t minSysClockFreq =
            vgaMinimumPioClockKHz(&initParams.params);

    uint32_t sysClockFreq = minSysClockFreq;

    while (sysClockFreq + minSysClockFreq < MAX_CLOCK) {
        sysClockFreq += minSysClockFreq;
    }

    setClosestClockFreqKhz(sysClockFreq);

    vgaInit(initParams);
}

void videoSetMode(uint8_t mode) {
    if (mode)
    {
       ula.pattr=PATTR_HIRES; 
    }
    else
    {
        ula.pattr=0;
    }
    
}