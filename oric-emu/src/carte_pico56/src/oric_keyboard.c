#include "oric_keyboard.h"

/* =========================================================
   GLOBAL STATE
   ========================================================= */

oricKyb kbdState;

oricKyb nesState;

uint8_t tableNes[8][3] =
{
    {  1, 7, 4 },
    {  2, 5, 4 },
    {  4, 6, 4 },
    {  8, 3, 4 },
    { 16, 4, 4 },
    { 32, 4, 7 },
    { 64, 0, 4 },
    {128, 0, 4 }
};

static const uint8_t colonnesKey[8]={0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};

/* =========================================================
   FLAGS
   ========================================================= */

static uint8_t shiftLeftPressed  = 0;
static uint8_t shiftRightPressed = 0;
static uint8_t ctrlPressed       = 0;

static uint8_t releaseCode  = 0;
static uint8_t extendedCode = 0;

/* =========================================================
   KEY MAP
   ========================================================= */

typedef struct
{
    uint8_t sc;
    uint8_t ext;
    uint8_t col;
    uint8_t row;
} KeyMap;



static const KeyMap keyMap[] =
{
  /* =========================================================
       ROW 0 : "7N5V 1X3"
       ========================================================= */

    {0x3d,0,0xfe,0}, // 7
    {0x31,0,0xfd,0}, // N
    {0x2e,0,0xfb,0}, // 5
    {0x2a,0,0xf7,0}, // V
    {0x14,1,0xef,0}, // right ctrl test
    {0x16,0,0xdf,0}, // 1
    {0x22,0,0xbf,0}, // X
    {0x26,0,0x7f,0}, // 3

    
    /* =========================================================
       ROW 1 : "JTRF  QD"
       ========================================================= */
    {0x3B,0,0xfe,1}, // J
    {0x2C,0,0xfd,1}, // T
    {0x2D,0,0xfb,1}, // R
    {0x2b,0,0xf7,1}, // F
    //{0x29,0,0xef,1}, // NONE
    {0x76,0,0xdf,1}, // ESC
    {0x15,0,0xbf,1}, // Q
    {0x23,0,0x7f,1}, // D

    /* =========================================================
       ROW 2 : "M6B4 Z2C"
       ========================================================= */
    {0x3A,0,0xfe,2}, // M
    {0x36,0,0xfd,2}, // 6
    {0x32,0,0xfb,2}, // B
    {0x25,0,0xf7,2}, // 4
    {0x14,0,0xef,2}, // CTRL
    {0x1A,0,0xdf,2}, // Z
    {0x1E,0,0xbf,2}, // 2
    {0x21,0,0x7f,2}, // C

    /* =========================================================
       ROW 3 : "K9;-  \'"
       ========================================================= */
    {0x42,0,0xfe,3}, // K
    {0x46,0,0xfd,3}, // 9
    {0x4C,0,0xfb,3}, // ;
    {0x4E,0,0xf7,3}, // -
    //{0x29,0,0xef,3}, // none
    //{0x5D,0,0xdf,3}, // je sais pas
    {0x5D,0,0xbf,3}, // \ a vérifier
    {0x52,0,0x7f,3}, // '

    /* =========================================================
       ROW 4 : " <>     "
       ========================================================= */
    {0x29,0,0xfe,4}, // SPACE
    {0x41,0,0xfd,4}, // ,
    {0x49,0,0xfb,4}, // .
    {0x75,1,0xf7,4}, // UP
    {0x12,0,0xef,4}, // left shift
    {0x6b,1,0xdf,4}, // LEFT
    {0x72,1,0xbf,4}, // DOWN
    {0x74,1,0x7f,4}, // RIGHT

    /* =========================================================
       ROW 5 : "UIOP  ]["
       ========================================================= */
    {0x3C,0,0xfe,5}, // U
    {0x43,0,0xfd,5}, // I
    {0x44,0,0xfb,5}, // O
    {0x4D,0,0xf7,5}, // P
    {0x11,0,0xef,5}, // FCT
    {0x66,0,0xdf,5}, // del
    {0x54,0,0xbf,5}, // [
    {0x5B,0,0x7f,5}, // ]
    
    /* =========================================================
       ROW 6 : "YHGE ASW"
       ========================================================= */
    {0x2c,0,0xfe,6}, // Y
    {0x33,0,0xfd,6}, // H
    {0x34,0,0xfb,6}, // G
    {0x24,0,0xf7,6}, // E
    //{0x29,0xef,4,6}, // NONE
    {0x1C,0,0xdf,6}, // A
    {0x1B,0,0xbf,6}, // S
    {0x1D,0,0x7f,6}, // W

    /* =========================================================
       ROW 7 : "8L0/   ="
       ========================================================= */
   {0x3e,0,0xfe,7}, // 8
   {0x4B,0,0xfd,7}, // L
   {0x45,0,0xfb,7}, // 0
   {0x4A,0,0xf7,7}, // /
   {0x59,0,0xef,7}, // RIGHT shift
   {0x5a,0,0xdf,7}, // return
   //{0x00,0,0xbf,7}, // caps
   {0x55,0,0x7f,7}, // =```

    /* =========================================================
     Pavé numérique
      ========================================================= */
    {0x70,0,0xfb,7}, // 0
    {0x69,0,0xdf,0}, // 1
    {0x72,0,0xbf,2}, // 2
    {0x7a,0,0x7f,0}, // 3
    {0x6b,0,0xf7,2}, // 4
    {0x73,0,0xfb,0}, // 5
    {0x74,0,0xfd,2}, // 6
    {0x6c,0,0xfe,0}, // 7
    {0x75,0,0xfe,7}, // 8 
    {0x7d,0,0xfd,3}, // 9
    
    {0x7b,0,0xf7,3}, // -
    {0x71,0,0xfb,4}, // .
    {0x4A,1,0xf7,7}, // /
    
    {0x5a,1,0xdf,7} // return 

    
};


#define MAP_SIZE (sizeof(keyMap)/sizeof(keyMap[0]))

/* =========================================================
   HELPERS
   ========================================================= */

void clearKeys(void)
{
    kbdState.nbTouchesPress = 0;

    kbdState.key[0].colonne = 0;
    kbdState.key[0].ligne   = 0;

    kbdState.key[1].colonne = 0;
    kbdState.key[1].ligne   = 0;
}

static void addKey(uint8_t col, uint8_t row)
{
    if (kbdState.nbTouchesPress >= 2)
        return;

    kbdState.key[kbdState.nbTouchesPress].colonne = col;
    kbdState.key[kbdState.nbTouchesPress].ligne   = row;

    kbdState.nbTouchesPress++;
}

static void rebuildKeyboardState(void)
{
    clearKeys();

    if (shiftLeftPressed)
        addKey(0xEF,4);

    if (shiftRightPressed)
        addKey(0XEF,7);

    if (ctrlPressed)
        addKey(0XEF,2);
}

/* =========================================================
   PUBLIC API
   ========================================================= */

void oric_keyboard_init(void)
{
    clearKeys();
}

void oric_keyboard_process(uint8_t sc)
{
    if (!sc)
        return;

    if (sc == 0xE0)
    {
        extendedCode = 1;
        return;
    }

    if (sc == 0xF0)
    {
        releaseCode = 1;
        return;
    }

    for (int i = 0; i < MAP_SIZE; i++)
    {
        if (keyMap[i].sc != sc)
            continue;

        if (keyMap[i].ext != extendedCode)
            continue;

        /* RELEASE */

        if (releaseCode)
        {
            if (sc == 0x12)
                shiftLeftPressed = 0;

            if (sc == 0x59)
                shiftRightPressed = 0;

            if (sc == 0x14)
                ctrlPressed = 0;

            rebuildKeyboardState();

            releaseCode  = 0;
            extendedCode = 0;

            return;
        }

        /* PRESS */

        if (sc == 0x12)
        {
            shiftLeftPressed = 1;

            rebuildKeyboardState();

            releaseCode  = 0;
            extendedCode = 0;

            return;
        }

        if (sc == 0x59)
        {
            shiftRightPressed = 1;

            rebuildKeyboardState();

            releaseCode  = 0;
            extendedCode = 0;

            return;
        }

        if (sc == 0x14)
        {
            ctrlPressed = 1;

            rebuildKeyboardState();

            releaseCode  = 0;
            extendedCode = 0;

            return;
        }

        rebuildKeyboardState();

        addKey(keyMap[i].col, keyMap[i].row);

        releaseCode  = 0;
        extendedCode = 0;

        return;
    }

    releaseCode  = 0;
    extendedCode = 0;
}
/*
 mode NES
 */

static void clearKeysNes(void)
{
    nesState.nbTouchesPress = 0;

    nesState.key[0].colonne = 0;
    nesState.key[0].ligne   = 0;

    nesState.key[1].colonne = 0;
    nesState.key[1].ligne   = 0;
}

static void addKeyNes(uint8_t col, uint8_t row)
{
    if (nesState.nbTouchesPress >= 2)
        return;

    nesState.key[nesState.nbTouchesPress].colonne = col;
    nesState.key[nesState.nbTouchesPress].ligne   = row;

    nesState.nbTouchesPress++;
}

void oric_keyboard_nes(uint8_t nesSt)
{
    // reset clavier à chaque frame NES
    clearKeysNes();

    // =====================================================
    // NES bits actifs à 0 (donc 0 = pressed)
    // =====================================================

    for (int n = 0; n < 8; n++) {
        if (!(nesSt & tableNes[n][0])) {
            addKeyNes(colonnesKey[tableNes[n][1]], tableNes[n][2]);
        }
    }
        
    /*
    // -----------------------------------------------------
    // UP
    // -----------------------------------------------------
    if (!(nesSt & 0x08))
    {
        addKeyNes(0xbf, 0);
    }

    // -----------------------------------------------------
    // DOWN
    // -----------------------------------------------------
    if (!(nesSt & 0x04))
    {
        addKeyNes(0xdf, 4);
    }
    
    // -----------------------------------------------------
    // LEFT
    // -----------------------------------------------------
    if (!(nesSt & 0x02))
    {
        addKeyNes(0xDF, 4);
    }

    // -----------------------------------------------------
    // RIGHT
    // -----------------------------------------------------
    if (!(nesSt & 0x01))
    {
        addKeyNes(0xbf, 4);
    }

    // -----------------------------------------------------
    // A button
    // -----------------------------------------------------
    if (!(nesSt & 0x80))
    {
        addKeyNes(0xFE, 4);
    }
     * */
    
}
/*
Actif niveau BAS FF : aucune touche appuyée
D7      D6      D5      D4      D3      D2      D1      D0 
A       B       Select  Start   Haut    Bas     Gauche  Droite
 */


