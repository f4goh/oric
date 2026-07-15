/*
 * Adaptation MOS6522 VIA
 * Version simplifiée IRQ unique
// 0300-03FF I/O Area
// 0300-030F Internal VIA 6522
// 0310-0310 DK'tronics Joystick Interface (left port)
// 0310-0313 Microdisc FDC WD1793
// 0310-031F Pravetz FDC
// 0314-031B Microdisc additionnal I/O registers
// 031C-031F Internal ACIA 6551 (Telestrat)
// 0320-032F RS232 extension (Atmos)
// 0320-0320 DK'tronics Joystick Interface (right port)
// 0320-032F Second VIA 6522 (Telestrat)
// 0320-03FF Pravetz ROM
// 0330-035F Spare Memory
// 0360-0371 RTC ICM7170 (Telestrat and Atmos)
// 0380-03DF Spare Memory
// 03E0-03E1 Oric Lightpen
// 03E2-03F3 Spare Memory
// 03F4-03FF Jasmin FDC WD1773
 *
 */

#include "vrEmu6502.h"

#define CHIPS_IMPL
#include "via.h"
#include "ula.h"

#include "audio.h"
#include "video.h"

#include "config.h"
#include "bus.h"
#include "oric_keyboard.h"  

#include "pico/stdlib.h"
#include "pico/time.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "DesassembleurWrapper.h"
#include "sd_reader.h"
//#include "6551.h"
//#include "busi2c.h"

// --------------------------------------------------
// RAM / ROM
// --------------------------------------------------


uint8_t __aligned(4) ram[ORIC_RAM_SIZE];
uint8_t __aligned(4) ramMoni[ORIC_MONITOR_SIZE];

extern uint8_t oricRom[];

uint8_t* romPtr()
{
    return oricRom;
}

size_t romSize()
{
    return ORIC_ROM_SIZE;
}

uint8_t buffer[64*1024];
//TAPHeader tap;
tap_headers_t tapHeaders;
uint8_t indexTap=0;
size_t bytes_read = 0;
size_t nb_files = 0;
char tap_filenames[MAX_TAP_FILES][MAX_NAME_LEN];
uint8_t stateCload=0;
uint16_t cptByteTap=0;
uint16_t startByteTap=0;
uint16_t sizeByteTap=0;

uint8_t ps2_release = 0;
uint16_t dump_addr_state = 0;
uint16_t disasm_addr_state = 0x0600;
CommandType witchDump=CMD_NONE;
uint32_t infoCpt=0;
uint8_t infoFlag=0;


// --------------------------------------------------
// CPU / VIA
// --------------------------------------------------

static VrEmu6502* cpu = NULL;
struct via vi;
//struct acia acia = {0};
 
// IRQ unique
static bool cpuIrq = false;
static bool nmi_request = false;


// --------------------------------------------------

#define ORIC_CLOCK_FREQ_MHZ 1.0000

#define MICROSECONDS_PER_BURST 50

#define TICKS_PER_BURST \
    (int)(MICROSECONDS_PER_BURST * ORIC_CLOCK_FREQ_MHZ)

#define CPU_6502_WAI 0xCB


#define  CPU_SNAP_SIZE 15
#define  VIA_SNAP_SIZE 37
#define  PSG_SNAP_SIZE 112

// --------------------------------------------------

void busWrite(uint16_t addr, uint8_t val);

uint8_t busRead(uint16_t addr, bool isDbg);

void cload();
void csave(); 
void moniDump();


bool ps2_is_press(uint8_t sc);
char sc_to_ascii(uint8_t sc);
void cmd_draw(const CmdLine *c);
bool cmd_feed(CmdLine *c, uint8_t sc);
bool cmd_parse(const char *txt, Command *cmd);
const char* skip_spaces(const char *s);
void cmd_error_draw(void);
void cmd_error_clear(void);
void cmd_compact(char *dst, const char *src);
char hexdigit(uint8_t v);
void dumpRam();
void dumpDisasm(void);
void snapSave(const char* filename);
void snapLoad(const char* filename);
void bmpSave(const char* filename);

//void execute_command(const Command *cmd);

// --------------------------------------------------
// INIT
// --------------------------------------------------

int _write(int fd, const char *buf, int len)
{
    uart_write_blocking(
        uart0,
        (const uint8_t *)buf,
        len
    );

    return len;
}


void busInit()
{
    
    
     if (sd_init() != 0) {
        while (1) {
            // fast blink handled in sd_init()
            //delay(1000);
        }
    }
    
    
    
    // CPU     
    cpu = vrEmu6502New(
        CPU_6502,
        busRead,
        busWrite
    );

    // VIA
    via_init(&vi);
    //acia_init(&acia);

    // Audio / Video

    audioInit(
        AY38910_CLOCK,
        SAMPLE_FREQUENCY
    );

    videoInit();
    //uart_init(uart0, 115200);
    //gpio_set_function(16, GPIO_FUNC_UART);
    oric_keyboard_init();
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    //gpio_init(18);
    //gpio_set_dir(18, GPIO_IN);

    // LED éteinte au démarrage (active bas)
    //gpio_put(17, 1);
     oricRom[0xE4AC & (ORIC_ROM_SIZE - 1)]=0xEA; //nop bande amorce
     oricRom[0xE4AD & (ORIC_ROM_SIZE - 1)]=0xEA;
     oricRom[0xE4AE & (ORIC_ROM_SIZE - 1)]=0xEA;
     
     oricRom[0xE6C9 & (ORIC_ROM_SIZE - 1)]=0xA9; //lda xx
     oricRom[0xE6CA & (ORIC_ROM_SIZE - 1)]=0x00; //octet a cet endroit
     oricRom[0xE6CB & (ORIC_ROM_SIZE - 1)]=0x60;

     oricRom[0xE92C & (ORIC_ROM_SIZE - 1)]=0x4C; //jmp $E93B
     oricRom[0xE92D & (ORIC_ROM_SIZE - 1)]=0x3B; //pour csave
     oricRom[0xE92E & (ORIC_ROM_SIZE - 1)]=0xE9;    
    
     
     //serialBegin();
     
}

void __not_in_flash_func(clavier()) {
    uint8_t sc = ula_kbdScancode;

    if (!sc) {
        //gpio_put(17, 1);
        return;
    }
    switch (sc) {
        case 0x05: nmi_request = true;
            break;
        case 0x06: vrEmu6502Reset(cpu);
            via_init(&vi);
            infoFlag=0;
            infoCpt=0;
            break;
        case 0x04: moniDump();
            break;
        default:
            oric_keyboard_process(sc);
    }
    
    
    /*
    if (sc == 0x05) {
        nmi_request = true;
    } else if (sc == 0x06) {
        vrEmu6502Reset(cpu);
        via_init(&vi);
    } else {
        oric_keyboard_process(sc);
    }
     */
    //gpio_put(17, 0); // LED allumée
    /*
    if (kbdState.nbTouchesPress > 0) {
        char buf[20];
        buf[0] = "0123456789abcdef"[kbdState.key[0].colonne >> 4];
        buf[1] = "0123456789abcdef"[kbdState.key[0].colonne & 0x0F];
        buf[2] = ' ';
        buf[3] = "0123456789abcdef"[kbdState.key[0].ligne >> 4];
        buf[4] = "0123456789abcdef"[kbdState.key[0].ligne & 0x0F];
        buf[5] = '\n';
        buf[6] = 0;
        puts(buf);
    }
     */
    ula_kbdScancode = 0; // consommé

}

void __not_in_flash_func(manetteNes()) {
    uint8_t nesSt = ula_nesSt;
    uint8_t nesValue = ula_nesValue;

    if (nesSt==0) {
        //gpio_put(17, 1);
        return;
    }
    oric_keyboard_nes(nesValue);
    /*
    char buf[20];
    buf[0] = "0123456789abcdef"[nesSt >> 4];
    buf[1] = "0123456789abcdef"[nesSt & 0x0F];
    buf[5] = '\n';
    buf[6] = 0;
    puts(buf);
    */
    //gpio_put(17, 0); // LED allumée
    /*
    if (nesState.nbTouchesPress > 0) {
        char buf[20];
        buf[0] = "0123456789abcdef"[nesState.key[0].colonne >> 4];
        buf[1] = "0123456789abcdef"[nesState.key[0].colonne & 0x0F];
        buf[2] = ' ';
        buf[3] = "0123456789abcdef"[nesState.key[0].ligne >> 4];
        buf[4] = "0123456789abcdef"[nesState.key[0].ligne & 0x0F];
        buf[5] = '\n';
        buf[6] = 0;
        puts(buf);
    }
    */
    ula_nesSt = 0; // consommé

}

// --------------------------------------------------
// MAIN LOOP
// --------------------------------------------------

void __not_in_flash_func(busMainLoop)()
{
    vrEmu6502Reset(cpu);

    absolute_time_t currentTime =
        get_absolute_time();

    int i = 0;

    while (1)
    {
    // =====================================================
    // PHASE INPUT / PERIPHERIQUES
    // =====================================================
    // Lecture clavier à chaque frame
    clavier();
    manetteNes();

        if (infoFlag == 0) {
            if (infoCpt == 50000) {
                memcpy(ram + 0xbb80 + 40 * 3 + 2, "\x03\x14ORIC-EMU F4GOH 2026\x17\x00", 23);
                infoFlag = 1;
                ram[0x2f5] = 0x00;
                ram[0x2f6] = 0x80;
                /*
                ram[0x8000] = 0x20;
                ram[0x8001] = 0xe2;
                ram[0x8002] = 0x00;

                ram[0x8003] = 0x20;
                ram[0x8004] = 0xe2;
                ram[0x8005] = 0x00;

                ram[0x8006] = 0x20;
                ram[0x8007] = 0xe2;
                ram[0x8008] = 0x00;
                */
                
                ram[0x8000] = 0x60;
                
                //gpio_put(25, 1);
            }
            infoCpt += 1;
        }
    
        // ------------------------------------------
        // CPU
        // ------------------------------------------

        while (i < TICKS_PER_BURST)
        {
            int cycleTicks =
                vrEmu6502InstCycle(cpu);

            if (vrEmu6502GetCurrentOpcode(cpu)
                == CPU_6502_WAI)
            {
                i += TICKS_PER_BURST;
                break;
            }

            i += cycleTicks;
        }

        i -= TICKS_PER_BURST;

        // ------------------------------------------
        // VIA
        // ------------------------------------------
        
        via_clock(&vi, TICKS_PER_BURST);
        cpuIrq = (vi.ifr & 0x80) != 0;
        // ------------------------------------------
        // IRQ CPU
        // ------------------------------------------

        *(vrEmu6502Int(cpu)) =
            cpuIrq
            ? IntRequested
            : IntCleared;

        // ------------------------------------------
        // ACIA
        // ------------------------------------------
        
        //acia_clock(&acia, TICKS_PER_BURST);

    // =====================================================
    // NMI (GPIO18 externe)
    // =====================================================
    // Ligne indépendante de l’IRQ (non masquable)
    *(vrEmu6502Nmi(cpu)) =
        nmi_request ? IntRequested : IntCleared;

    // Reset du latch NMI après prise en compte CPU
    nmi_request = false;

    
        
        // ------------------------------------------
        // TIMING
        // ------------------------------------------

        currentTime =
            delayed_by_us(
                currentTime,
                MICROSECONDS_PER_BURST
            );

        if (to_us_since_boot(currentTime)
            < to_us_since_boot(
                get_absolute_time()))
        {
            currentTime =
                get_absolute_time();
        }
        else
        {
            busy_wait_until(currentTime);
        }
    }
}

// --------------------------------------------------
// BUS WRITE
// --------------------------------------------------

void __not_in_flash_func(busWrite)(
    uint16_t addr,
    uint8_t val)
{
    if (addr < 0xC000) {
        // ------------------------------------------
        // VIA
        // ------------------------------------------

        if (addr >= 0x0300 &&
                addr < 0x0310) {

            via_write(
                    &vi,
                    addr & 0x0F,
                    val
                    );
            cpuIrq = (vi.ifr & 0x80) != 0;
        }
        //if (addr >= 0x031C &&     //031C-031F
        //        addr < 0x0320) {
        //    acia_write(&acia,addr & 0x03,val);
        //} 
       else {
            // --------------------------------------
            // RAM
            // --------------------------------------

            ram[addr] = val;
        }
    }
}

// --------------------------------------------------
// BUS READ
// --------------------------------------------------

uint8_t __not_in_flash_func(busRead)(
        uint16_t addr,
        bool isDbg) {
    // ------------------------------------------
    // ROM
    // ------------------------------------------

    if (addr >= 0xC000) {
        switch (stateCload) {
            case 0:
                if (addr == 0xE4AC) { //1re bande amorce au lieu de cload
                    cload();                    
                }
                break;
            case 1:
                if (addr == 0xE6C9) { //lecture de l'octet
                     oricRom[0xE6CA & (ORIC_ROM_SIZE - 1)]=buffer[startByteTap+cptByteTap];
                     cptByteTap++;
                     if (cptByteTap==sizeByteTap){
                         indexTap++;
                         //
                         if (indexTap==tapHeaders.nbHeader){
                           stateCload = 0;//retour au cload menu
                           //gpio_put(25, 1);
                           indexTap=0;
                           ram[0x2b1]=0;
                         }
                         else{ //il y a une autre partie
                           stateCload = 2;
                           ram[0x2b1]=0; //supprime les erreurs ??
                         }
                     }
                }
                break;
             case 2:
                if (addr == 0xE4AC) { //amorce
                    cptByteTap = 0; //reinit cpt
                    startByteTap = tapHeaders.bloc[indexTap].atSync; //nouvelle partie
                    sizeByteTap = tapHeaders.bloc[indexTap].dataFullSize;
                    stateCload = 1;                    
                }
                break;
        }
        if (addr == 0xE92C){
            csave();
        }
        return oricRom[
                addr &
                (ORIC_ROM_SIZE - 1)
                ];
    }

    // ------------------------------------------
    // VIA
    // ------------------------------------------

    if (addr >= 0x0300 &&
            addr < 0x0310) {
        uint8_t value =
                via_read(
                &vi,
                addr & 0x0F
                );
        cpuIrq = (vi.ifr & 0x80) != 0;
        return value;
    }
    if (addr == 0x02f5){
           
           gpio_put(25, gpio_get(25) ^ 1);
    }
    
    //if (addr >= 0x031C && //031C-031F
    //        addr < 0x0320) {
    //    uint8_t value = acia_read(&acia, addr & 0x03);
    //    return value;
    //} 

    // ------------------------------------------
    // RAM
    // ------------------------------------------

    return ram[addr];
}

void __not_in_flash_func(cload)() {
    int ret;
    if (indexTap == 0) {

        ret = list_tap_files(tap_filenames, MAX_TAP_FILES, &nb_files);
        menuColor(ram);

        print_files_ram(tap_filenames, nb_files, ram);

        deplace_curseur(0, nb_files, ram);
        
        uint8_t indexFile;
        uint8_t nesOldValue = 0xff;
        uint8_t kbdOldValue = ula_kbdScancode;

        /* historique clavier */
        uint8_t kbdHistory[3] = {0, 0, 0};

        /* état touches (anti-retrigger) */
        uint8_t rightPressed = 0;
        uint8_t leftPressed = 0;

        ula_kbdScancode=0;
        uint8_t nesValue;
        /* boucle principale */
        while (ula_nesValue != 0x7f && ula_kbdScancode != 0x5a) {
            
            /* ---------------- NES ---------------- */
            if (ula_nesSt != 0) {
                nesValue = ula_nesValue;
                if (nesValue != nesOldValue) {

                    if (nesValue == 0xfe) {
                        indexFile = deplace_curseur(1, nb_files, ram);
                    } else if (nesValue == 0xfd) {
                        indexFile = deplace_curseur(-1, nb_files, ram);
                    }

                    nesOldValue = nesValue;
                }
                ula_nesSt = 0;
            }

            /* ---------------- CLAVIER ---------------- */
            if (ula_kbdScancode != kbdOldValue) {

                /* mise à jour historique uniquement si changement réel */
                kbdHistory[0] = kbdHistory[1];
                kbdHistory[1] = kbdHistory[2];
                kbdHistory[2] = ula_kbdScancode;

                /* -------- droite (0x74) -------- */

                if (kbdHistory[2] == 0x74 && kbdHistory[1] != 0xF0) {

                    if (!rightPressed) {
                        rightPressed = 1;
                        indexFile = deplace_curseur(1, nb_files, ram);
                    }
                }

                if (kbdHistory[2] == 0x74 && kbdHistory[1] == 0xF0) {
                    rightPressed = 0;
                }

                /* -------- gauche (0x6B) -------- */

                if (kbdHistory[2] == 0x6B && kbdHistory[1] != 0xF0) {

                    if (!leftPressed) {
                        leftPressed = 1;
                        indexFile = deplace_curseur(-1, nb_files, ram);
                    }
                }

                if (kbdHistory[2] == 0x6B && kbdHistory[1] == 0xF0) {
                    leftPressed = 0;
                }

                kbdOldValue = ula_kbdScancode;
            }
        }
        ula_nesSt = 0;
        ula_kbdScancode = 0;
        oric_keyboard_process(0x66);
        //const char *filename = "mushmania";
        const char *filename = tap_filenames[indexFile];

        char fullpath[32];

        /* construit "mushmania.tap" */
        snprintf(fullpath,
                sizeof (fullpath),
                "%s.tap",
                filename);

        ret = read_binary_file(fullpath, buffer, sizeof (buffer), &bytes_read);
        if (!ret) {
            load_csv_u8(filename, tableNes);
            analyseHeader(buffer, bytes_read, &tapHeaders);
        }
        stateCload = 1;
        cptByteTap = 0;
        startByteTap = tapHeaders.bloc[indexTap].atSync;
        sizeByteTap = tapHeaders.bloc[indexTap].dataFullSize;
        clearKeys();
    }
}

void __not_in_flash_func(csave)() {

    tap_header_t *h = &tapHeaders.header[0];

    // 0x2A8 -> 0x2B0
    h->reserved = ram[0x2A8];
    h->start_addr_lo = ram[0x2A9];
    h->start_addr_hi = ram[0x2AA];
    h->end_addr_lo = ram[0x2AB];
    h->end_addr_hi = ram[0x2AC];
    h->autorun = ram[0x2AD];
    h->type = ram[0x2AE];
    h->flag_str = ram[0x2AF];
    h->flag_int = ram[0x2B0];

    memset(h->filename, 0, sizeof (h->filename));

    if (ram[0x27F] == 0) {
        strcpy((char*) h->filename, "save");
    } else {
        int i;

        for (i = 0; i < sizeof (h->filename) - 1; i++) {
            h->filename[i] = ram[0x27F + i];

            if (h->filename[i] == 0)
                break;
        }

        h->filename[sizeof (h->filename) - 1] = 0;
    }
    sd_write_binary(ram,&tapHeaders);
    clearKeys();
}

void moniDump() {
    memset(ramMoni, 0x20, ORIC_MONITOR_SIZE);
    for (int ligne = 0; ligne < 28; ligne++) {
        ramMoni[ligne * 40] = 20; // colonne 0
        ramMoni[ligne * 40 + 1] = 7; // colonne 1
    }
    ramMoni[1] = 3;
    memcpy(ramMoni + 3, "MONITEUR", 8);
    /* --- AJOUT PC CPU --- */
    uint16_t cpu_pc = vrEmu6502GetPC(cpu);

    char pcstr[10];
    sprintf(pcstr, "PC=%04X", cpu_pc);

    memcpy(ramMoni + 15, pcstr,7);
    
    //for (int j = 0; pcstr[j] && (col + j) < 40; j++)
    //    ramMoni[base + col + j] = pcstr[j];
    
    monitorEnable(true);

    CmdLine cmdline = {0};
    Command cmd;

    uint8_t sc;

    cmd_draw(&cmdline);

    do {
        sc = ula_kbdScancode;

        if (sc == 0)
            continue;

        ula_kbdScancode = 0;

        /* ⭐ efface ERROR dès qu’on touche le clavier */
        cmd_error_clear();

        if (cmd_feed(&cmdline, sc)) {
            /* ⭐ buffer temporaire sans espaces */
            char tmp[CMD_MAX];
            cmd_compact(tmp, cmdline.buf);

            if (cmd_parse(tmp, &cmd)) {
                switch (cmd.type) {
                    case CMD_DUMP:
                        dump_addr_state = cmd.addr;
                        dumpRam(); // ⭐ plus de cmd
                        witchDump=CMD_DUMP;
                        break;

                    case CMD_DISASM:              
                        disasm_addr_state = cmd.addr;
                        dumpDisasm();
                        witchDump=CMD_DISASM;
                        break;

                     case CMD_NEXT:
                        if (witchDump==CMD_DUMP) dumpRam();
                        if (witchDump==CMD_DISASM) dumpDisasm();
                        break;    
                        
                    case CMD_WRITEFILE:
                      sd_save_bloc(cmd.filename,cmd.addr,cmd.len,(uint8_t *)ram);
                        break;

                    case CMD_MODMEM:
                        ram[cmd.addr] = cmd.valeur;
                        break;

                    case CMD_SNAPLOAD:
                        snapLoad(cmd.filename);
                        break;

                    case CMD_SNAPSAVE:
                         snapSave(cmd.filename);
                        break;
                    case CMD_BMP:
                         bmpSave(cmd.filename);
                        break;

                    default:
                        break;
                }
            } else {
                cmd_error_draw();
                }

            memset(&cmdline, 0, sizeof (cmdline));
            cmd_draw(&cmdline);
        }

    } while (sc != 0x07);

    monitorEnable(false);
    videoSetMode(ram[0x21f]); //remettre le bon attribut pour l'ula
}



bool ps2_is_press(uint8_t sc)
{
    if (sc == 0xF0)
    {
        ps2_release = 1;
        return false;
    }

    if (ps2_release)
    {
        ps2_release = 0;
        return false;
    }

    return true;
}

void cmd_draw(const CmdLine *c)
{
    int i;

    for (i = 0; i < 40-3; i++)
        ramMoni[CMD_ROW_OFFSET + i] = ' ';

    for (i = 0; i < c->len; i++)
        ramMoni[CMD_ROW_OFFSET + i] = c->buf[i];

    /* curseur */
    if (c->cursor < 40)
        ramMoni[CMD_ROW_OFFSET + c->cursor] = 127;
}

void cmd_error_draw(void)
{
    const char *msg = "ERROR";

    for (int i = 0; i < 40-3; i++)
        ramMoni[CMD_ROW_OFFSET-40 + i] = ' ';

    for (int i = 0; msg[i]; i++)
        ramMoni[CMD_ROW_OFFSET-40 + i] = msg[i];
}

void cmd_error_clear(void)
{
    for (int i = 0; i < 40-3; i++)
        ramMoni[CMD_ROW_OFFSET - 40 + i] = ' ';
}

static const char scanToAscii[128] =
{
    [0x16] = '1',
    [0x1E] = '2',
    [0x26] = '3',
    [0x25] = '4',
    [0x2E] = '5',
    [0x36] = '6',
    [0x3D] = '7',
    [0x3E] = '8',
    [0x46] = '9',
    [0x45] = '0',

    [0x1C] = 'A',
    [0x32] = 'B',
    [0x21] = 'C',
    [0x23] = 'D',
    [0x24] = 'E',
    [0x2B] = 'F',
    [0x34] = 'G',
    [0x33] = 'H',
    [0x43] = 'I',
    [0x3B] = 'J',
    [0x42] = 'K',
    [0x4B] = 'L',
    [0x3A] = 'M',
    [0x31] = 'N',
    [0x44] = 'O',
    [0x4D] = 'P',
    [0x15] = 'Q',
    [0x2D] = 'R',
    [0x1B] = 'S',
    [0x2C] = 'T',
    [0x3C] = 'U',
    [0x2A] = 'V',
    [0x1D] = 'W',
    [0x22] = 'X',
    [0x35] = 'Y',
    [0x1A] = 'Z',

    [0x29] = ' '
};

char sc_to_ascii(uint8_t sc)
{
    return (sc < sizeof(scanToAscii)) ? scanToAscii[sc] : 0;
}


/*
char sc_to_ascii(uint8_t sc)
{
    switch(sc)
    {
        case 0x3A: return 'M';
        case 0x23: return 'D';
        case 0x2B: return 'F';
        case 0x1D: return 'W';

        case 0x16: return '1';
        case 0x1E: return '2';
        case 0x26: return '3';
        case 0x25: return '4';
        case 0x2E: return '5';
        case 0x36: return '6';
        case 0x3D: return '7';
        case 0x3E: return '8';
        case 0x46: return '9';
        case 0x45: return '0';

        case 0x21: return 'C';
        case 0x24: return 'E';
        case 0x32: return 'B';
        case 0x1B: return 'S';
        case 0x31: return 'N';
        case 0x4b: return 'L';
        case 0x2c: return 'T';

        case 0x29: return ' ';

        default: return 0;
    }
}
*/


bool cmd_feed(CmdLine *c, uint8_t sc)
{
    if (!ps2_is_press(sc))
        return false;

    /* ENTER */
    if (sc == 0x5A)
    {
        c->buf[c->len] = 0;
        return true;
    }

    /* BACKSPACE */
    if (sc == 0x66)
    {
        if (c->cursor > 0)
        {
            memmove(&c->buf[c->cursor - 1],
                    &c->buf[c->cursor],
                    c->len - c->cursor);

            c->cursor--;
            c->len--;
        }

        cmd_draw(c);
        return false;
    }

    /* LEFT */
    if (sc == 0x6B)
    {
        if (c->cursor > 0)
            c->cursor--;

        cmd_draw(c);
        return false;
    }

    /* RIGHT */
    if (sc == 0x74)
    {
        if (c->cursor < c->len)
            c->cursor++;

        cmd_draw(c);
        return false;
    }

    char ch = sc_to_ascii(sc);
    if (!ch)
        return false;

    if (c->len < CMD_MAX - 1)
    {
        memmove(&c->buf[c->cursor + 1],
                &c->buf[c->cursor],
                c->len - c->cursor);

        c->buf[c->cursor] = ch;

        c->cursor++;
        c->len++;

        cmd_draw(c);
    }

    return false;
}



bool hex4(const char *s, uint16_t *v)
{
    *v = 0;

    for (int i = 0; i < 4; i++)
    {
        char c = s[i];

        if (c >= '0' && c <= '9')
            *v = (*v << 4) | (c - '0');
        else if (c >= 'A' && c <= 'F')
            *v = (*v << 4) | (c - 'A' + 10);
        else
            return false;
    }

    return true;
}

bool hex2(const char *s, uint8_t *v)
{
    *v = 0;

    for (int i = 0; i < 2; i++)
    {
        char c = s[i];

        if (c >= '0' && c <= '9')
            *v = (*v << 4) | (c - '0');
        else if (c >= 'A' && c <= 'F')
            *v = (*v << 4) | (c - 'A' + 10);
        else
            return false;
    }

    return true;
}

bool cmd_parse(const char *txt, Command *cmd)
{
    memset(cmd, 0, sizeof(*cmd));

    /* Entrée seule */
    if (txt[0] == 0)
    {
        cmd->type = CMD_NEXT;
        return true;
    }

    /* -------------------- Dxxxx -------------------- */
    if (txt[0] == 'D')
    {
        uint16_t addr = 0;

        if (!hex4(txt + 1, &addr))
            return false;

        cmd->type = CMD_DISASM;
        cmd->addr = addr;
        return true;
    }

    /* -------------------- MMxxxxvv -------------------- */
    if (txt[0] == 'M' && txt[1] == 'M')
    {
        uint16_t addr = 0;
        uint8_t value = 0;

        if (!hex4(txt + 2, &addr))
            return false;

        if (!hex2(txt + 6, &value))
            return false;

        cmd->type   = CMD_MODMEM;
        cmd->addr   = addr;
        cmd->valeur = value;
        return true;
    }

    /* -------------------- Mxxxx (dump memory) -------------------- */
    if (txt[0] == 'M' && txt[1] != 'M')
    {
        uint16_t addr = 0;

        if (!hex4(txt + 1, &addr))
            return false;

        cmd->type = CMD_DUMP;
        cmd->addr = addr;
        return true;
    }

    /* -------------------- FWxxxxllllFILE -------------------- */
    if (txt[0] == 'F' && txt[1] == 'W')
    {
        const char *p = txt + 2;
        uint16_t addr = 0;
        uint16_t len  = 0;

        /* addr (4 hex) */
        for (int i = 0; i < 4; i++)
        {
            char c = *p++;
            if (c >= '0' && c <= '9')
                addr = (addr << 4) | (c - '0');
            else if (c >= 'A' && c <= 'F')
                addr = (addr << 4) | (c - 'A' + 10);
            else
                return false;
        }

        /* len (4 hex) */
        for (int i = 0; i < 4; i++)
        {
            char c = *p++;
            if (c >= '0' && c <= '9')
                len = (len << 4) | (c - '0');
            else if (c >= 'A' && c <= 'F')
                len = (len << 4) | (c - 'A' + 10);
            else
                return false;
        }

        cmd->type = CMD_WRITEFILE;
        cmd->addr = addr;
        cmd->len  = len;

        /* filename (reste de la chaîne) */
        strncpy(cmd->filename, p, sizeof(cmd->filename) - 1);
        cmd->filename[sizeof(cmd->filename) - 1] = 0;

        return true;
    }
        /* -------------------- SNLfilename -------------------- */
    if (txt[0] == 'S' && txt[1] == 'N' && txt[2] == 'L')
    {
        cmd->type = CMD_SNAPLOAD;

        strncpy(cmd->filename,
                txt + 3,
                sizeof(cmd->filename) - 1);

        cmd->filename[sizeof(cmd->filename) - 1] = 0;

        return true;
    }

    /* -------------------- SNSfilename -------------------- */
    if (txt[0] == 'S' && txt[1] == 'N' && txt[2] == 'S')
    {
        cmd->type = CMD_SNAPSAVE;

        strncpy(cmd->filename,
                txt + 3,
                sizeof(cmd->filename) - 1);

        cmd->filename[sizeof(cmd->filename) - 1] = 0;

        return true;
    }

    /* -------------------- BMPfilename -------------------- */
    if (txt[0] == 'B' && txt[1] == 'M' && txt[2] == 'P')
    {
        cmd->type = CMD_BMP;

        strncpy(cmd->filename,
                txt + 3,
                sizeof(cmd->filename) - 1);

        cmd->filename[sizeof(cmd->filename) - 1] = 0;

        return true;
    }
    
    return false;
}

void cmd_compact(char *dst, const char *src)
{
    while (*src)
    {
        if (*src != ' ')
            *dst++ = *src;
        src++;
    }
    *dst = 0;
}


void dumpRam()
{
    int16_t addr = dump_addr_state;

    for (int line = 0; line < 25; line++)
    {
        int base = (1 + line) * 40 + 2;  // ligne 1, col 2

        /* --- adresse --- */
        ramMoni[base + 0] = hexdigit((addr >> 12) & 0xF);
        ramMoni[base + 1] = hexdigit((addr >> 8) & 0xF);
        ramMoni[base + 2] = hexdigit((addr >> 4) & 0xF);
        ramMoni[base + 3] = hexdigit(addr & 0xF);

        ramMoni[base + 4] = ' ';

        /* --- hex bytes --- */
        int i;
        uint8_t v;

        for (i = 0; i < 8; i++)
        {
            uint16_t a = addr++;

            if (a >= 0xC000)
                v = oricRom[a & (ORIC_ROM_SIZE - 1)];
            else
                v = ram[a];

            ramMoni[base + 5 + i * 3 + 0] = hexdigit(v >> 4);
            ramMoni[base + 5 + i * 3 + 1] = hexdigit(v & 0xF);
            ramMoni[base + 5 + i * 3 + 2] = ' ';
        }

        /* --- ASCII --- */
        int ascii_off = base + 5 + 8 * 3;

        for (i = 0; i < 8; i++)
        {
            uint16_t a = addr - 8 + i;
            uint8_t v;

            if (a >= 0xC000)
                v = oricRom[a & (ORIC_ROM_SIZE - 1)];
            else
                v = ram[a];

            ramMoni[ascii_off + i] = (v >= 32 && v <= 126) ? v : '.';
        }
    }

    dump_addr_state = addr; // ⭐ seule source d'état
}

char hexdigit(uint8_t v)
{
    return (v < 10) ? ('0' + v) : ('A' + v - 10);
}

void dumpDisasm(void) {
    

    desassembleur_setCycleCounting(1);
    desassembleur_setHexOutput(1);
    desassembleur_setOricMode(1);
    desassembleur_setOrg(disasm_addr_state);
    desassembleur_setMaxNumBytes(48U * 1024U);
    desassembleur_setOffset(0);

    uint16_t pc = disasm_addr_state;
    char tmpstr[128];

    uint16_t org = desassembleur_getOrg(); // juste pour affichage logique si besoin

    for (int line = 0; line < 25; line++) {
        int base = (1 + line) * 40 + 2;

        /* efface la ligne */
        for (int i = 0; i < 38; i++)
            ramMoni[base + i] = ' ';

        /* désassemblage */
         if (disasm_addr_state<0xC000)
        desassembleur_disassemble(
                tmpstr,
                &ram[disasm_addr_state], // RAM Oric directe 0x0000–0xBFFF
                &pc
                );
         else
        desassembleur_disassemble(
                tmpstr,
                &oricRom[disasm_addr_state& (ORIC_ROM_SIZE - 1)], // ROM Oric
                &pc
                );
             
        pc++;
        /* copie dans l'écran */
        for (int i = 0; i < 38 && tmpstr[i]; i++)
            ramMoni[base + i] = tmpstr[i];
    }

    /* sauvegarde position (IMPORTANT : pc est déjà avancé par le désassembleur) */
    disasm_addr_state = pc;
}




void snapSave(const char* filename)
{
  uint8_t cpus=cpuSize();
  uint8_t vias=viaSize();
  uint8_t audios=audioSize();


    uint8_t snapBuffer[cpus+vias+audios+12];

    uint8_t* p = snapBuffer;
    memcpy(p,"CPU", 3);
    p+=3;
    memcpy(p,(uint8_t *)cpu,cpus);
    p+=cpus;
    memcpy(p,"VIA", 3);
    p+=3;
    memcpy(p, (uint8_t *)&vi, vias);
    p += vias;
    memcpy(p,"PSG", 3);
    p+=3;
    memcpy(p, audioSave(),  audios);
    p +=  audios;
    memcpy(p,"RAM", 3);
    p+=3;
    
   sd_save_snap(filename,snapBuffer,cpus+vias+audios+12,(uint8_t *)ram,ORIC_RAM_SIZE);
    
    
}

void snapLoad(const char* filename)
{
    uint8_t cpus = cpuSize();
    uint8_t vias = viaSize();
    uint8_t audios = audioSize();

    uint8_t snapBuffer[cpus + vias + audios + 12];

    sd_load_snap(filename,
                 snapBuffer,
                 cpus + vias + audios + 12,
                 ram,
                 ORIC_RAM_SIZE);

    uint8_t* p = snapBuffer;

    // ---- CPU ----
    p += 3; // skip "CPU"
    memcpy(cpu, p, cpus);
    p += cpus;

    // ---- VIA ----
    p += 3; // skip "VIA"
    memcpy(&vi, p, vias);
    p += vias;

    // ---- PSG ----
    p += 3; // skip "PSG"
    memcpy(audioSave(), p, audios);
    p += audios;
}

void bmpSave(const char* filename) {
    BMP_HANDLE h = bmp_open(filename);

    if (!h) return;
    uint8_t pattr_save = getpattr();
    for (int y = 0; y < BMP_H; y++)
    {
        renderScanline(h, y, ram, &pattr_save);
    }
    bmp_close(h);
    bmp_flip_vertical(filename);

}

