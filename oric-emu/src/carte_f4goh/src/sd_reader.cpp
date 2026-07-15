#include <Arduino.h>
#include <SD.h>
#include "ula.h"
#include <Wire.h>

#include "hardware/uart.h"
#include "hardware/gpio.h"

#include "sd_reader.h"

#define SD_CS 17
#define LED_PIN 25

// Variable globale
uint8_t indexFichier = 0;

static void led(bool on)
{
    digitalWrite(LED_PIN, on ? HIGH : LOW);
}

static void blink(int times, int d = 150)
{
    for (int i = 0; i < times; i++) {
        led(true);
        delay(d);
        led(false);
        delay(d);
    }
}
static inline void blink_error()
{
    for (int i = 0; i < 10; i++) {
        led(true);
        delay(80);
        led(false);
        delay(80);
    }
}
static inline void blink_short()
{
    led(true);
    delay(15);
    led(false);
}
int sd_init()
{
    pinMode(LED_PIN, OUTPUT);
    led(false);

    // init SD = LED slow blink start
    blink(2, 100);

    SPI.setRX(16);   // MISO
    SPI.setTX(19);   // MOSI
    SPI.setSCK(18);  // SCK
    SPI.setCS(17);   // CS
    //SD.begin(17, SPI, 4000000); // 4 MHz (debug)
    
    if (!SD.begin(SD_CS,SPI_FULL_SPEED, SPI)) { // SPI_HALF_SPEED
        // error = fast blink forever
        while (1) {
            blink(5, 100);
            delay(800);
        }
    }

    // OK = 3 blinks
    blink(3, 200);

    return 0;
}


int sd_read_binary(const char* filename)
{
    File f = SD.open(filename, FILE_READ);

    if (!f) {
        blink(10, 80);
        return -1;
    }

    uint8_t buffer[512];
    size_t total = 0;

    while (true) {

        int n = f.read(buffer, sizeof(buffer));
        if (n <= 0) break;

        total += n;

        if ((total % (512 * 20)) == 0) {
            led(true);
            delay(20);
            led(false);
        }
    }

    f.close();

    blink(6, 150);
   
    return 0;
}


uint16_t lireTap(const char *filename, uint8_t *buffer)
{
    File f = SD.open(filename, FILE_READ);

    if (!f) {
        Serial.println("open error");
        return 0;
    }

    uint16_t fileSize = f.size();

    if (fileSize > 48 * 1024) {
        Serial.println("file too big");
        f.close();
        return 0;
    }

    size_t br = f.read(buffer, fileSize);

    Serial.print("read ok : ");
    Serial.println(br);

    f.close();

    return fileSize;
}

bool parse_tap_header(const uint8_t *buffer,
                       uint32_t fileSize,
                       TAPHeader *header)
{
    memset(header, 0, sizeof(TAPHeader));

    uint32_t pos = 0;

    // sync
    while (buffer[pos] == 0x16) pos++;

    if (buffer[pos] != 0x24) return false;
    pos++;

    pos += 2; // reserved

    header->language = buffer[pos++];
    header->autostart = buffer[pos++];

    header->endAddress =
        (buffer[pos] << 8) | buffer[pos + 1];
    pos += 2;

    header->startAddress =
        (buffer[pos] << 8) | buffer[pos + 1];
    
    // ✔️ ASTUCE ULTRA FIABLE
    uint32_t codeSize =
        (header->endAddress - header->startAddress) + 1;

    header->offsetCode = fileSize - codeSize;

    header->sizeCode = codeSize;

    return true;
}

/*
 * Recherche la séquence :
 *   0x16 0x16 0x16 0x24
 *
 * Paramètres :
 *   buffer : buffer à analyser
 *   size   : taille du buffer
 *   start  : position de départ de la recherche
 *
 * Retour :
 *   position du premier octet après 0x24
 *   ou 0xFFFF si non trouvé
 */
uint16_t tap_seek(const uint8_t *buffer, size_t size, uint16_t start)
{
    /* Vérification minimale */
    if (buffer == NULL || start >= size)
    {
        return TAP_NOT_FOUND;
    }

    /* Il faut au moins 4 octets à comparer */
    for (size_t i = start; i + 3 < size; i++)
    {
        if (buffer[i]     == 0x16 &&
            buffer[i + 1] == 0x16 &&
            buffer[i + 2] == 0x16 &&
            buffer[i + 3] == 0x24)
        {
            /* Retourne l'octet suivant 0x24 */
            if (i + 4 < size)
            {
                return (uint16_t)(i + 4);
            }
            else
            {
                return TAP_NOT_FOUND;
            }
        }
    }
    return TAP_NOT_FOUND;
}


/*
 * Analyse des headers TAP
 */
void analyseHeader(const uint8_t *buffer,
                   size_t size,
                   tap_headers_t *tapHeaders)
{
    uint16_t pos = 0;
    uint8_t index = 0;

    if (tapHeaders == NULL || buffer == NULL)
    {
        return;
    }

    /* Reset structure */
    memset(tapHeaders, 0, sizeof(tap_headers_t));

    while (index < MAX_HEADERS)
    {
        pos = tap_seek(buffer, size, pos);

        if (pos == TAP_NOT_FOUND)
        {
            break;
        }

        /* sécurité taille header */
        if (pos + sizeof(tap_header_t) > size)
        {
            break;
        }

        /* copie header */
        memcpy(&tapHeaders->header[index],
               &buffer[pos],
               sizeof(tap_header_t));

        /* afterSync */
        tapHeaders->bloc[index].afterSync = pos;
        tapHeaders->bloc[index].atSync = pos-1;

        /* calcul start/end */
        uint16_t start_addr =
            ((uint16_t)tapHeaders->header[index].start_addr_hi << 8) |
             (uint16_t)tapHeaders->header[index].start_addr_lo;

        uint16_t end_addr =
            ((uint16_t)tapHeaders->header[index].end_addr_hi << 8) |
             (uint16_t)tapHeaders->header[index].end_addr_lo;

        tapHeaders->bloc[index].dataSize =
            (uint16_t)(end_addr - start_addr + 1);

       
        
        index++;

        pos += sizeof(tap_header_t);
    }

    tapHeaders->nbHeader = index;

    /*
     * Calcul firstByte
     */
    for (uint8_t i = 0; i < tapHeaders->nbHeader; i++)
    {
        if (i < (tapHeaders->nbHeader - 1))
        {
            tapHeaders->bloc[i].firstByte =
                (uint16_t)(
                    (tapHeaders->bloc[i + 1].afterSync - 4) -
                    tapHeaders->bloc[i].dataSize
                );
            
            tapHeaders->bloc[i].dataFullSize =
            (uint16_t)(
                    (tapHeaders->bloc[i + 1].atSync - 3) -
                    tapHeaders->bloc[i].atSync
                );
        }
        else
        {
            tapHeaders->bloc[i].firstByte =
                (uint16_t)(size - tapHeaders->bloc[i].dataSize);
             tapHeaders->bloc[i].dataFullSize =
                (uint16_t)(size - tapHeaders->bloc[i].atSync);
        }
    }

    /*
     * Sécurise filename (garantit string propre)
     */
    for (uint8_t i = 0; i < tapHeaders->nbHeader; i++)
    {
        tapHeaders->header[i].filename[15] = '\0';
    }
}

int read_binary_file(const char *filename,
                     uint8_t *buffer,
                     size_t max_size,
                     size_t *bytes_read)
{
    if (bytes_read != nullptr) {
        *bytes_read = 0;
    }

    
    File f = SD.open(filename, FILE_READ);

    if (!f) {
        return -1;
    }

    size_t file_size = f.size();

    if (file_size > max_size) {
        f.close();
        return -2;
    }

    size_t br = f.read(buffer, file_size);

    if (bytes_read != nullptr) {
        *bytes_read = br;
    }

    f.close();

    return 0;
}

int sd_write_binary(uint8_t *ram,
                    tap_headers_t *tapHeaders)
{
    tap_header_t *h = &tapHeaders->header[0];

    char fullFilename[32];

    snprintf(fullFilename,
             sizeof(fullFilename),
             "%s.tap",
             (char *)h->filename);

    File f = SD.open(fullFilename, FILE_WRITE);

    if (!f) {
        blink(10, 80);
        return -1;
    }

    // En-tête obligatoire
    const uint8_t header[4] = {0x16, 0x16, 0x16, 0x24};

    size_t written = f.write(header, sizeof (header));



    // écrire les champs avant filename
    written += f.write((uint8_t *) & h->flag_int, 9);

    // écrire filename jusqu'au premier 0 inclus
    for (int i = 0; i < sizeof (h->filename); i++) {
        written += f.write(&h->filename[i], 1);

        if (h->filename[i] == 0) {
            break;
        }
    }

   // données RAM
    uint16_t startAddr =
        ((uint16_t)h->start_addr_hi << 8) | h->start_addr_lo;

    uint16_t endAddr =
        ((uint16_t)h->end_addr_hi << 8) | h->end_addr_lo;

    if (endAddr >= startAddr) {
        uint32_t dataSize = (uint32_t)(endAddr - startAddr + 1);
        written += f.write(&ram[startAddr], dataSize);
    }


    f.flush();
    f.close();

    return (int) written;
}



#define CSV_BUFFER_SIZE 128

/*
 * Lit un fichier CSV depuis SD
 *
 * Exemple fichier :
 * 1,7,4
 * 2,5,4
 *
 * Le paramètre filename doit être sans extension :
 * "test" -> ouvre "test.csv"
 *
 * Retour :
 * >=0  : nombre de lignes chargées
 * -1   : erreur ouverture
 * -2   : fichier trop gros
 */
int load_csv_u8(const char *filename,
                uint8_t data[MAX_ROWS][MAX_COLS])
{
    char fullpath[32];

    /* construit "test.csv" */
    snprintf(fullpath,
             sizeof(fullpath),
             "%s.csv",
             filename);

    uint8_t buffer[CSV_BUFFER_SIZE];
    size_t bytes_read = 0;

    /*
     * Réutilise ta fonction existante
     */
    int ret = read_binary_file(fullpath,
                               buffer,
                               sizeof(buffer) - 1,
                               &bytes_read);

    if (ret != 0)
    {
        return ret;
    }

    /*
     * Important :
     * transforme buffer en string C
     */
    buffer[bytes_read] = '\0';

    int row = 0;

    char *line = strtok((char *)buffer, "\r\n");

    while (line != NULL && row < MAX_ROWS)
    {
        unsigned int v0, v1, v2;

        int n = sscanf(line,
                       "%u,%u,%u",
                       &v0,
                       &v1,
                       &v2);

        if (n == 3)
        {
            data[row][0] = (uint8_t)v0;
            data[row][1] = (uint8_t)v1;
            data[row][2] = (uint8_t)v2;

            row++;
        }

        line = strtok(NULL, "\r\n");
    }

    return row;
}

int list_tap_files(char filenames[][MAX_NAME_LEN],
                   size_t max_files,
                   size_t *files_found)
{
    if (files_found != nullptr) {
        *files_found = 0;
    }

    File root = SD.open("/");

    if (!root || !root.isDirectory()) {
        return -1;
    }

    size_t count = 0;

    while (true) {

        File entry = root.openNextFile();

        if (!entry) {
            break;
        }

        // Ignorer les dossiers
        if (entry.isDirectory()) {
            entry.close();
            continue;
        }

        const char *name = entry.name();

        size_t len = strlen(name);

        // Vérifie extension ".tap"
        if (len >= 4) {

            const char *ext = &name[len - 4];

            if (strcasecmp(ext, ".tap") == 0) {

                if (count >= max_files) {
                    entry.close();
                    root.close();
                    return -2;
                }

                // Copie sans extension
                size_t copy_len = len - 4;

                if (copy_len >= MAX_NAME_LEN) {
                    copy_len = MAX_NAME_LEN - 1;
                }

                strncpy(filenames[count], name, copy_len);
                filenames[count][copy_len] = '\0';

                count++;
            }
        }

        entry.close();
    }

    root.close();

    if (files_found != nullptr) {
        *files_found = count;
    }

    return 0;
}

void print_files_ram(char filenames[][MAX_NAME_LEN],
                     size_t files_found,
                     uint8_t *ram)
{
    //1120 bytes for display (28 lines of 40 columns) (from #BB80 to #BFDF)
    // Pointeur vers mémoire écran ORIC
    uint8_t __aligned(4) *screen = &ram[ORIC_SCREEN_ADDR];

    // Efface uniquement les zones texte
    // en conservant les 2 premiers octets de chaque ligne
    for (size_t y = FILES_FIRST_LINE; y < SCREEN_LINES; y++) {

        uint8_t *line_ptr = screen + (y * SCREEN_COLUMNS);

        for (size_t x = 2; x < SCREEN_COLUMNS; x++) {
            line_ptr[x] = 0x20;
        }
    }

    // Affichage fichiers
    for (size_t i = 0; i < files_found; i++) {

        size_t row = FILES_FIRST_LINE + (i / 2);

        if (row >= SCREEN_LINES) {
            break;
        }

        size_t col = ((i & 1) == 0) ? COL1_X : COL2_X;

        uint8_t *line_ptr = screen + (row * SCREEN_COLUMNS);

        char *name = filenames[i];

        size_t len = strlen(name);

        if (len > MAX_NAME_LEN) {
            len = MAX_NAME_LEN;
        }

        // Alignement à gauche
        for (size_t j = 0; j < len; j++) {
            if ((col + j) < SCREEN_COLUMNS) {
                line_ptr[col + j] = name[j];
            }
        }
    }
}


uint8_t deplace_curseur(int8_t sens,
                        size_t max_files,
                        uint8_t *ram)
{
    static uint8_t old_index = 0;

    // Pointeur vers mémoire écran ORIC
    uint8_t __aligned(4) *screen = &ram[ORIC_SCREEN_ADDR];

    // Aucun fichier
    if (max_files == 0) {
        return 0;
    }

    // Efface ancien curseur
    {
        size_t old_row = FILES_FIRST_LINE + (old_index / 2);

        size_t old_col;

        if ((old_index & 1) == 0) {
            old_col = CURSOR_COL1_X;
        } else {
            old_col = CURSOR_COL2_X;
        }

        screen[(old_row * SCREEN_COLUMNS) + old_col] = ' ';
    }

    // Déplacement
    if (sens > 0) {

        // Avance seulement si un fichier existe
        if ((indexFichier + 1) < max_files) {
            indexFichier++;
        }

    } else if (sens < 0) {

        // Recule sans passer sous 0
        if (indexFichier > 0) {
            indexFichier--;
        }
    }

    // Nouvelle position curseur
    {
        size_t row = FILES_FIRST_LINE + (indexFichier / 2);

        size_t col;

        if ((indexFichier & 1) == 0) {
            col = CURSOR_COL1_X;
        } else {
            col = CURSOR_COL2_X;
        }

        screen[(row * SCREEN_COLUMNS) + col] = CURSOR_CHAR;
    }

    old_index = indexFichier;

    return indexFichier;
}

void menuColor(uint8_t *ram)
{
    uint8_t *screen = &ram[ORIC_SCREEN_ADDR];

    // Colonne 0 : PAPER 0x10 sur toutes les lignes
    for (uint8_t y = 0; y < SCREEN_LINES; y++) {
        screen[y * SCREEN_COLUMNS + 0] = 0x10;
    }

    // Colonne 1 : INK 0x03 sur toutes les lignes
    for (uint8_t y = 0; y < SCREEN_LINES; y++) {
        screen[y * SCREEN_COLUMNS + 1] = 0x03;
    }

    // Exception ligne 0 colonne 1 : INK 0x14
    screen[0] = 0x14;
    screen[1] = 0x07;

    // Texte "F4GOH 2026" ligne 0 colonne 30
    const char *txt = "F4GOH 2026";

    uint8_t *dst = &screen[30];

    while (*txt) {
        *dst++ = *txt++;
    }
}
  

int sd_save_snap(const char *filename,
                 const uint8_t *snapBuffer,
                 uint32_t snapSize,
                 const uint8_t *ram,
                 uint32_t ramSize)
{
    char fullFilename[30];

    snprintf(fullFilename,
             sizeof(fullFilename),
             "%s.sna",
             filename);
    //SD.remove(filename);
    File f = SD.open(fullFilename,
                     O_WRONLY | O_CREAT | O_TRUNC);

    if (!f) {
        blink(10, 80);
        return -1;
    }

    size_t written = 0;

    // Écriture du snapshot (CPU + VIA + PSG)
    written += f.write(snapBuffer, snapSize);

    // Écriture de la RAM
    written += f.write(ram, ramSize);

    f.flush();
    f.close();

    return (int)written;
}

int sd_load_snap(const char *filename,
                 uint8_t *snapBuffer,
                 uint32_t snapSize,
                 uint8_t *ram,
                 uint32_t ramSize)
{
    char fullFilename[30];

    snprintf(fullFilename,
             sizeof(fullFilename),
             "%s.sna",
             filename);

    File f = SD.open(fullFilename, O_RDONLY);

    if (!f) {
        blink(10, 80);
        return -1;
    }

    size_t read = 0;

    // lecture snapshot (CPU + VIA + PSG)
    read += f.read(snapBuffer, snapSize);

    // lecture RAM
    read += f.read(ram, ramSize);

    f.close();

    // sécurité : vérifie cohérence
    if (read != (snapSize + ramSize)) {
        blink(20, 50);
        return -2;
    }

    return (int)read;
}

int sd_save_bloc(const char *filename,
                 uint16_t addr,
                 uint16_t len,
                 const uint8_t *ram)
{
    char fullFilename[30];

    snprintf(fullFilename,
             sizeof(fullFilename),
             "%s.bin",
             filename);

    File f = SD.open(fullFilename,
                     O_WRONLY | O_CREAT | O_TRUNC);

    if (!f) {
        blink(10, 80);
        return -1;
    }

    size_t written = 0;

    /* 1. sauvegarde du bloc mémoire RAM */
    written += f.write(ram + addr, len);

    f.flush();
    f.close();

    return (int)written;
}

BMP_HANDLE bmp_open(const char *filename)
{
    char fullFilename[32];

    snprintf(fullFilename,
             sizeof(fullFilename),
             "%s_tmp.bmp",
             filename);

    File *f = new File(
        SD.open(fullFilename,
                O_WRONLY | O_CREAT | O_TRUNC));

    if (!(*f))
    {
        delete f;
        return nullptr;
    }

    const uint32_t width     = BMP_W;
    const uint32_t height    = BMP_H;
    const uint32_t imageSize = width * height * 3;
    const uint32_t fileSize  = 54 + imageSize;

    uint8_t header[54];

    memset(header, 0, sizeof(header));

    // Signature BMP
    header[0] = 'B';
    header[1] = 'M';

    // Taille fichier
    memcpy(&header[2], &fileSize, 4);

    // Offset des pixels
    const uint32_t dataOffset = 54;
    memcpy(&header[10], &dataOffset, 4);

    // Taille du DIB header
    const uint32_t dibSize = 40;
    memcpy(&header[14], &dibSize, 4);

    // Dimensions
    memcpy(&header[18], &width, 4);
    memcpy(&header[22], &height, 4);

    // Nombre de plans
    const uint16_t planes = 1;
    memcpy(&header[26], &planes, 2);

    // 24 bits/pixel
    const uint16_t bpp = 24;
    memcpy(&header[28], &bpp, 2);

    // Compression = aucune
    const uint32_t compression = 0;
    memcpy(&header[30], &compression, 4);

    // Taille image
    memcpy(&header[34], &imageSize, 4);

    f->write(header, sizeof(header));

    return (BMP_HANDLE)f;
}

void bmp_append_line(BMP_HANDLE h,
                     const uint16_t *pixels)
{
    if (!h)
        return;

    File *f = (File *)h;

    uint8_t line[BMP_W * 3];
    uint8_t *dst = line;

 for (int x = 0; x < BMP_W; x++)
{
    uint16_t c = pixels[x];

    // format réel : B V R (B MSB, R LSB)
    uint8_t b4 = (c >> 8) & 0x0F;
    uint8_t v4 = (c >> 4) & 0x0F;
    uint8_t r4 = (c >> 0) & 0x0F;

    uint8_t r8 = (r4 << 4) | r4;
    uint8_t g8 = (v4 << 4) | v4;
    uint8_t b8 = (b4 << 4) | b4;

    *dst++ = b8;   // BMP = BGR
    *dst++ = g8;
    *dst++ = r8;
}

    f->write(line, sizeof(line));
}

void bmp_close(BMP_HANDLE h)
{
    if (!h)
        return;

    File *f = (File *)h;

    f->flush();
    f->close();

    delete f;
   
}


int bmp_flip_vertical(const char *filename)
{
    char inName[40];
    char outName[40];

    snprintf(inName, sizeof(inName), "%s_tmp.bmp", filename);
    snprintf(outName, sizeof(outName), "%s.bmp", filename);

    File in = SD.open(inName, FILE_READ);
    if (!in)
        return -1;

    File out = SD.open(outName, FILE_WRITE);
    if (!out)
    {
        in.close();
        return -2;
    }

    // 1. copier header (54 bytes)
    uint8_t header[54];
    in.read(header, 54);
    out.write(header, 54);

    // 2. buffer UNE ligne
    uint8_t line[BMP_LINE_SIZE];

    // 3. inversion verticale
    for (int y = BMP_H - 1; y >= 0; y--)
    {
        uint32_t offset = 54 + (uint32_t)y * BMP_LINE_SIZE;

        in.seek(offset);
        in.read(line, BMP_LINE_SIZE);

        out.write(line, BMP_LINE_SIZE);
    }
    out.flush();
    in.close();
    out.close();

    // 4. remplacement fichier
    SD.remove(inName);
    //SD.rename(outName, inName);

    return 0;
}


void renderScanline(BMP_HANDLE h,uint16_t y, uint8_t* ram,uint8_t *pattr_save) {

    static const uint16_t pal[8] ={
        0x000, // black
        0x00F, // red
        0x0F0, // green
        0x0FF, // yellow
        0xF00, // blue
        0xF0F, // magenta
        0xFF0, // cyan
        0xFFF // white
    };
    uint16_t out[ORIC_FRAMEBUFFER_SIZE*2];
    uint8_t fb[ORIC_FRAMEBUFFER_SIZE];

    uint8_t pattr = *pattr_save;

    uint8_t lattr = 0;
    uint8_t fgcol = 7;
    uint8_t bgcol = 0;

    uint8_t* p= fb;

    for (int x = 0; x < 40; x++) {
        // Lookup the byte and, if needed, the pattern data
        uint8_t ch, pat;
        if ((pattr & PATTR_HIRES) && y < 200)
            ch = pat = ram[0xA000 + y * 40 + x];

        else {
              //ch = ram[0xBB80 + (y >> 3) * 40 + x];  //oui
              ch = ramMoni[(y >> 3) * 40 + x];  
    
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
               base = ram + 0xB400;  //charset text
                 
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
        if (lattr & LATTR_BLINK) c_fgcol = c_bgcol;

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
    
    
    for (int i = 0; i < 120; i++) {
        uint8_t v = fb[i];

        // pixel gauche (nibble haut)
        uint8_t color = (v >> 4) & 0x07;
        out[x++] = pal[color];

        // pixel droit (nibble bas)
        color = v & 0x07;
        out[x++] = pal[color];
    }
    bmp_append_line(h,out);
    *pattr_save=pattr;
}



/*
void serialBegin(){
    Serial1.setTX(12);
    Serial1.setRX(13);

    Serial1.begin(115200);  // UART sur GPIO12/13
}

void serialPrint(uint16_t value) {
    //while(Serial1.available() > 0){
    //if (Serial1.available() > 0) {
    //    char car = Serial1.read();
    //    Serial1.println(car, HEX);
    //}
    //Serial1.println(value, HEX);
}

void serialPrintTxt(char * txt) {
    //while(Serial1.available() > 0){
    //if (Serial1.available() > 0) {
    //    char car = Serial1.read();
    //    Serial1.println(car, HEX);
    //}
    //Serial1.println(txt);
}
*/

void i2cBegin() {
    Wire.setSDA(20);
    Wire.setSCL(21);
    Wire.setClock(400000);
    Wire.begin();
}

char hexDigit(uint8_t v)
{
    return (v < 10) ? ('0' + v) : ('A' + v - 10);
}

void i2cDetect(uint8_t *ramMoni)
{
    /* ---------- Entête ---------- */
    int header = 1 * 40 + 2;   // ligne 1, colonne 2

    ramMoni[header + 0] = ' ';
    ramMoni[header + 1] = ' ';
    ramMoni[header + 2] = ' ';
    ramMoni[header + 3] = ' ';

    for (int col = 0; col < 8; col++)
    {
        ramMoni[header + 4 + col * 3 + 0] = '0';
        ramMoni[header + 4 + col * 3 + 1] = hexDigit(col);
        ramMoni[header + 4 + col * 3 + 2] = ' ';
    }

    /* ---------- Scan ---------- */
    for (int line = 0; line < 15; line++)
    {
        uint8_t baseAddr = line * 8;
        int base = (2 + line) * 40 + 2;   // ligne 1..15, colonne 2

        /* Adresse de début de ligne */
        ramMoni[base + 0] = hexDigit(baseAddr >> 4);
        ramMoni[base + 1] = hexDigit(baseAddr & 0x0F);
        ramMoni[base + 2] = ':';
        ramMoni[base + 3] = ' ';

        for (int col = 0; col < 8; col++)
        {
            uint8_t addr = baseAddr + col;
            int pos = base + 4 + col * 3;

            if (addr < 0x78)
            {
                Wire.beginTransmission(addr);
                bool found = (Wire.endTransmission() == 0);

                if (found)
                {
                    ramMoni[pos + 0] = hexDigit(addr >> 4);
                    ramMoni[pos + 1] = hexDigit(addr & 0x0F);
                }
                else
                {
                    ramMoni[pos + 0] = '-';
                    ramMoni[pos + 1] = '-';
                }

                ramMoni[pos + 2] = ' ';
            }
            else
            {
                ramMoni[pos + 0] = ' ';
                ramMoni[pos + 1] = ' ';
                ramMoni[pos + 2] = ' ';
            }
        }
    }
}

/*
int sd_read_binary_buf(const char* filename, uint8_t *buffer, size_t maxSize)
{
    File f = SD.open(filename, FILE_READ);

    if (!f) {
        blink_error();
        return -1;
    }

    size_t total = 0;
    size_t lastBlink = 0;
    
    size_t sz = f.size();

    if (sz == 0) {
        // fichier vu mais vide côté FAT
        while (1) {
            led(true);
            delay(100);
            led(false);
            delay(100);
        }
    }


    while (f.available() && total < maxSize)
    {
        int n = f.read(buffer + total, maxSize - total);

        if (n <= 0) break;

        total += n;

        // 🔥 blink rapide pour activité continue
        led(true);

        // petit délai visible
        delayMicroseconds(200);

        led(false);

        // 🔥 blink périodique (toutes ~8KB)
        if ((total - lastBlink) >= 8192)
        {
            blink_short();
            lastBlink = total;
        }
    }

    f.close();
    if (total==0) led(true);
    
    
    return total;
}*/