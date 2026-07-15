#ifndef ORIC_KEYBOARD_H
#define ORIC_KEYBOARD_H

#include <stdint.h>

/* =========================================================
   ORIC KEY STRUCTURES
   ========================================================= */

typedef struct {
    uint8_t colonne;
    uint8_t ligne;
} oricKey;

typedef struct {
    uint8_t nbTouchesPress;
    oricKey key[2];
} oricKyb;

#define MAX_ROWS 8
#define MAX_COLS 3


/* =========================================================
   GLOBAL STATE
   ========================================================= */

extern oricKyb kbdState;
extern oricKyb nesState;
extern uint8_t tableNes[8][3];

/* =========================================================
   API
   ========================================================= */

void oric_keyboard_init(void);

void oric_keyboard_process(uint8_t scancode);

void oric_keyboard_nes(uint8_t nesSt);
void clearKeys(void);


#endif
