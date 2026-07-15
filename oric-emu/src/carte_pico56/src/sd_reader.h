#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#define BMP_W 240
#define BMP_H 224
#define BMP_LINE_SIZE (BMP_W * 3)


typedef void* BMP_HANDLE;

typedef struct {
    uint8_t language;
    uint8_t autostart;
    uint16_t endAddress;
    uint16_t startAddress;
    char filename[20];
    uint16_t offsetCode;
    uint16_t sizeCode;
} TAPHeader;

/*
 * Structures
 */

#define TAP_NOT_FOUND 0xFFFF
#define MAX_HEADERS   4
#define MAX_ROWS 8
#define MAX_COLS 3


#define MAX_TAP_FILES   52
#define MAX_NAME_LEN    15
#define ORIC_SCREEN_ADDR   0xBB80

// Dimensions affichage
#define SCREEN_COLUMNS     40
#define SCREEN_LINES       28

// Colonnes d'affichage
#define COL1_X             5
#define COL2_X             25

#define CURSOR_CHAR        '#'

// Position curseur
#define CURSOR_COL1_X      4
#define CURSOR_COL2_X      24
#define FILES_FIRST_LINE   1


typedef struct {
    uint8_t flag_int;  // 0x2B0
    uint8_t flag_str;  // 0x2AF
    uint8_t type;  // 0x2AE
    uint8_t autorun;  // 0x2AD
    uint8_t end_addr_hi; // 0x2AC
    uint8_t end_addr_lo;  // 0x2AB
    uint8_t start_addr_hi; // 0x2AA
    uint8_t start_addr_lo; // 0x2A9
    uint8_t reserved;  // 0x2A8
    uint8_t filename[16]; //0x27F
} tap_header_t;

typedef struct {
    uint16_t atSync;
    uint16_t afterSync;
    uint16_t firstByte;
    uint16_t dataSize;
    uint16_t dataFullSize;
} tap_bloc_t;


typedef struct {
    uint8_t nbHeader;
    tap_bloc_t bloc[MAX_HEADERS];
    tap_header_t header[MAX_HEADERS];
} tap_headers_t;


    
int sd_init();
int sd_read_binary(const char* filename);
int sd_write_binary(uint8_t *ram,tap_headers_t *tapHeaders);
//int sd_read_binary_buf(const char* filename, uint8_t *buffer, size_t maxSize);
bool parse_tap_header(const uint8_t *buffer,uint32_t fileSize,TAPHeader *header);
uint16_t lireTap(const char *filename, uint8_t *buffer);

int read_binary_file(const char *filename,uint8_t *buffer,size_t max_size,size_t *bytes_read);
void analyseHeader(const uint8_t *buffer,size_t size,tap_headers_t *tapHeaders);
int load_csv_u8(const char *filename,uint8_t data[MAX_ROWS][MAX_COLS]);
int list_tap_files(char filenames[][MAX_NAME_LEN],size_t max_files,size_t *files_found);
void print_files_ram(char filenames[][MAX_NAME_LEN],size_t files_found,uint8_t *ram);
uint8_t deplace_curseur(int8_t sens, size_t max_files, uint8_t *ram);
void menuColor(uint8_t *ram);
void serialPrint(uint16_t value);
void serialBegin();
void serialPrintTxt(char * txt) ;
int sd_save_snap(const char *filename,
                 const uint8_t *snapBuffer,
                 uint32_t snapSize,
                 const uint8_t *ram,
                 uint32_t ramSize);
int sd_load_snap(const char *filename,
                 uint8_t *snapBuffer,
                 uint32_t snapSize,
                 uint8_t *ram,
                 uint32_t ramSize);

int sd_save_bloc(const char *filename,
                 uint16_t addr,
                 uint16_t len,
                 const uint8_t *ram);
BMP_HANDLE bmp_open(const char *filename);
void bmp_append_line(BMP_HANDLE h,
                     const uint16_t *pixels);
void bmp_close(BMP_HANDLE h);
void renderScanline(BMP_HANDLE h,uint16_t y, uint8_t* ram,uint8_t *pattr_save);
int bmp_flip_vertical(const char *filename);

        
#ifdef __cplusplus
}
#endif
