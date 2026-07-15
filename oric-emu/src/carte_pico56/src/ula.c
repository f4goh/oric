#include "ula.h"
#include <string.h>
#include "ps2-kbd.h"
#include "nes-ctrl.h"

// palette 8 couleurs Oric
/*
 * Palette convertie en format 12-bit BGR
 * suivant la fonction colorFromRgb()
 */

extern uint8_t charset[];

static const uint16_t pal[8] =
{
    0x000, // black
    0x00F, // red
    0x0F0, // green
    0x0FF, // yellow
    0xF00, // blue
    0xF0F, // magenta
    0xFF0, // cyan
    0xFFF  // white
};


volatile uint8_t ula_kbdScancode = 0;
volatile uint8_t ula_nesValue = 0xff;
volatile uint8_t ula_nesSt = 0x0;

// -------------------------

void ula_init(Ula* u) {
    //memset(u->ram, 0x00, RAM_SIZE);
    ps2kbd_begin();
    ps2kbd_write(0xed);
    //sleep_ms(10);
    ps2kbd_write(0x02);
    // update nes state
  nes_begin();
  nes_read_start();
}

// -------------------------
void ula_reset(Ula* u)
{
    //memcpy(u->ram+0x9800, lores, 0x2800);
    u->pattr=0;  //LORES
    //memcpy(u->ram+0x9800, hires, 0x2800);
    //u->pattr=PATTR_HIRES;  
    u->blink_counter=0;
}


void ula_monitor(Ula* u, bool en)
{
    if (en && !u->enMonitor)
    {
        u->enMonitor = true;
        /* Sauvegarde de l'état vidéo courant */
        u->pattrSave = u->pattr;
        /* Forcer le mode texte */
        u->pattr &= ~PATTR_HIRES;
    }
    else if (!en && u->enMonitor)
    {
        u->enMonitor = false;
        /* Restauration de l'état vidéo */
        u->pattr = u->pattrSave;
    }
}


// -------------------------
// hires/text Oric timing
// -------------------------
void ula_scanline(Ula* u, uint16_t y, uint16_t* out) {
    //bool blink_state = u->blink_counter & 0x20;  //valeur pour 50hz
    bool blink_state = u->blink_counter & 0x10;  //adaptation pour 60hz


    //uint8_t pattr = u->pattr;
    uint8_t pattr = u->enMonitor ? 0 : u->pattr;

    uint8_t lattr = 0;
    uint8_t fgcol = 7;
    uint8_t bgcol = 0;

    uint8_t* p= u->fb;

    if (y>=ORIC_SCREEN_HEIGHT){
         for (int i=0;i<320;i++) out[i] = 0;
         return;
    }
    
    for (int x = 0; x < 40; x++) {
        // Lookup the byte and, if needed, the pattern data
        uint8_t ch, pat;
        if ((pattr & PATTR_HIRES) && y < 200)
            //ch = pat = u->ram[0xA000 + y * 40 + x];
            //ch = pat = busRead(0xA000 + y * 40 + x,0);    
            ch = pat = ram[0xA000 + y * 40 + x];

        else {
            //ch = u->ram[0xBB80 + (y >> 3) * 40 + x];
            if (!u->enMonitor){
              ch = ram[0xBB80 + (y >> 3) * 40 + x];  //oui
            }
            else{
              ch = ramMoni[(y >> 3) * 40 + x];  
            }
            int off = (lattr & LATTR_DSIZE ? y >> 1 : y) & 7;
            const uint8_t* base;
            if (pattr & PATTR_HIRES)
                if (lattr & LATTR_ALT)
                    //base = u->ram + 0x9C00;
                    base = ram + 0x9C00;  //altcharset hires
                else
                    //base = u->ram + 0x9800;
                    base = ram + 0x9800;  //charset hires
            else if (lattr & LATTR_ALT)
                //base = u->ram + 0xB800;
                base = ram + 0xB800;  //altcharset text
            else
                 if (!u->enMonitor)
                    base = ram + 0xB400;  //charset text
                 else 
                    base = charset;

            pat = base[((ch & 0x7F) << 3) | off];
        }

        // Handle state-chaging attributes
        if (!(ch & 0x60)) {
            pat = 0x00;
            switch (ch & 0x18) {
                case 0x00:
                    fgcol = ch & 7;
                    break;
                case 0x08:
                    lattr = ch & 7;
                    break;
                case 0x10:
                    bgcol = ch & 7;
                    break;
                case 0x18:
                    pattr = ch & 7;
                    break;
            }
        }

        // Pick up the colors for the pattern
        uint8_t c_fgcol = fgcol;
        uint8_t c_bgcol = bgcol;

        // inverse video
        if (ch & 0x80) {
            c_bgcol = c_bgcol ^ 0x07;
            c_fgcol = c_fgcol ^ 0x07;
        }
        // blink
        if ((lattr & LATTR_BLINK) && blink_state) c_fgcol = c_bgcol;

        // Draw the pattern
        uint8_t c;
        c = pat & 0x20 ? c_fgcol : c_bgcol;
        *p = c << 4;
        c = pat & 0x10 ? c_fgcol : c_bgcol;
        *p++ |= c;
        c = pat & 0x08 ? c_fgcol : c_bgcol;
        *p = c << 4;
        c = pat & 0x04 ? c_fgcol : c_bgcol;
        *p++ |= c;
        c = pat & 0x02 ? c_fgcol : c_bgcol;
        *p = c << 4;
        c = pat & 0x01 ? c_fgcol : c_bgcol;
        *p++ |= c;
    }

    int x = 0;
    int i;
    //il faut deux marges de 80 pixels de chaque coté donc 40
    for (i=0;i<40;i++) out[x++] = 0;
    
    for (int i = 0; i < 120; i++) {
        uint8_t v = u->fb[i];

        // pixel gauche (nibble haut)
        uint8_t color = (v >> 4) & 0x07;
        out[x++] = pal[color];

        // pixel droit (nibble bas)
        color = v & 0x07;
        out[x++] = pal[color];
    }
    for (i=0;i<40;i++) out[x++] = 0;
    
    u->pattr = pattr;

    
}

void ula_endframe(Ula* u)
{
    uint8_t c = ps2kbd_read();
    
    if (c != 0)
    {
        ula_kbdScancode = c;
    }
    
    nes_read_finish();
    nes_read_start();
    ula_nesValue = nes_get_state_1();
    ula_nesSt = 1;
   
    //clignotement ula a faire
    //u->blink_counter = (u->blink_counter + 1) & 0x3F; //valeur pour 50Hz
    u->blink_counter = (u->blink_counter + 1) & 0x1F; //adaptation pour 60hz

     
}