/*
 * Troy's HBC-56 Emulator
 * Copyright (c) 2021 Troy Schrapel
 * Adapted for ORIC ATMOS
 * F4GOH 2026
 */




#pragma once

#include <inttypes.h>
#include <stdbool.h>

extern uint8_t ram[];
extern uint8_t ramMoni[];

typedef enum
{
    CMD_NONE,
    CMD_NEXT,      // Entrée seule
    CMD_DUMP,      // m <addr>
    CMD_DISASM,    // d <addr>
    CMD_WRITEFILE, // fw <addr> <len> <file>
    CMD_MODMEM,     // mm <addr> <valeur>
    CMD_SNAPLOAD,   // SNL filename
    CMD_SNAPSAVE,    // SNS filename
    CMD_BMP,         // BMP filename
    CMD_I2C         // I2C
            
} CommandType;

typedef struct
{
    CommandType type;

    uint16_t addr;
    uint16_t len;
    uint8_t  valeur;

    char filename[16];

} Command;

typedef enum
{
    BUS_NONE = 0,
    BUS_LED,
    BUS_WRITE,
    BUS_READ,        
}busType;
            

typedef struct
{
    busType type;
    uint8_t addr;
    uint8_t  valeur;
    char variable;
} BusCmd;



#define CMD_MAX 32
#define CMD_ROW_OFFSET 27*40+3

typedef struct
{
    char buf[CMD_MAX];
    uint8_t len;
    uint8_t cursor;
} CmdLine;



void busInit();
void busMainLoop();
uint8_t busRead(uint16_t addr, bool isDbg);
