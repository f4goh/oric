/*==================================================
*                   ORIC SDCARD                    *
*       (c) 2018 Emmanuel de LAPPARENT (EdL)       *
*                   Version V2                     *
*==================================================*
* Ce code  est distribue gratuitement  mais  sans  *
* aucune garantie et sans engager la responsabilite*
* de l'auteur.                                     *
* Vous  pouvez  l'utiliser,  le  modifier  et  le  *
* diffuser librement, en conservant cette  licence *
* et les références de l'auteur dans  toutes  les  *
* copies. L'exploitation commerciale est interdite.*
*==================================================*/

/* SD card management is based on SD-LEP and SimpleSDAudio library */

/**************************************************\
*                  S D - L E P                     * 
*           (c) 2017 - Daniel Coulom               *  
*           http://dcmoto.free.fr/                 *
*           http://forum.system-cfg.com/           *
*--------------------------------------------------*
* Ce code est distribue gratuitement dans l'espoir *
* qu'il sera utile, mais sans aucune  garantie  et *
* sans  engager  la  responsabilité  de  l'auteur. *
* Vous  pouvez  l' utiliser,  le  modifier  et  le *
* diffuser librement, en conservant cette  licence *
* et les références de l'auteur dans   toutes  les *
* copies. L'exploitation commerciale est interdite.*
\**************************************************/

/*-----------------------------------------------------------------
* Visit SimpleSDAudio website for more information:
* http://www.hackerspace-ffm.de/wiki/index.php?title=SimpleSDAudio
*----------------------------------------------------------------*/

// SD card constants
#define SD_CARD_TYPE_SD1        1 /** Standard capacity V1 SD card */
#define SD_CARD_TYPE_SD2        2 /** Standard capacity V2 SD card */
#define SD_CARD_TYPE_SDHC       3 /** High Capacity SD card */
#define SD_PARTTYPE_UNKNOWN     0
#define SD_PARTTYPE_SUPERFLOPPY 1
#define SD_PARTTYPE_FAT16       2
#define SD_PARTTYPE_FAT32       3
#define SD_INIT_TIMEOUT      2000 /** temps maxi pour l'initialisation */
#define SD_READ_TIMEOUT       300 /** temps maxi pour le debut de la lecture d'un bloc */
#define SD_COMMAND_TIMEOUT    300 /** temps maxi pour repondre a une commande */
#define SD_READY_STATE       0x00 /** status for card in the ready state */
#define SD_IDLE_STATE        0x01 /** status for card in the idle state */
#define SD_ILLEGAL_COMMAND   0x04 /** status bit for illegal command */
#define SD_DATA_START_BLOCK  0xFE /** start data token for read or write single block*/

// SD card commands
#define SD_CMD0   0x00  /** GO_IDLE_STATE - init card in spi mode if CS low */
#define SD_CMD8   0x08  /** SEND_IF_COND - verify SD Memory Card interface operating condition.*/
#define SD_CMD9   0x09  /** SEND_CSD - read the Card Specific Data (CSD register), response R1 */
#define SD_CMD10  0x0A  /** SEND_CID - read the card identification information (CID register), response R1 */
#define SD_CMD12  0x0C  /** STOP_TRANSMISSION - end multiple block read sequence, response R1b */
#define SD_CMD13  0x0D  /** SEND_STATUS - read the card status register, response R2 */
#define SD_CMD16  0x10  /** SET_BLOCKLEN arg0[31:0]: block length, response R1 */
#define SD_CMD17  0x11  /** READ_SINGLE_BLOCK - read a single data block from the card, response R1 */
#define SD_CMD18  0x12  /** READ_MULTIPLE_BLOCK - read a multiple data blocks from the card, response R1 */
#define SD_CMD55  0x37  /** APP_CMD - escape for application specific command */
#define SD_CMD58  0x3A  /** READ_OCR - read the OCR register of a card */
#define SD_CMD59  0x3B  /** CRC_ON_OFF - Turns CRC option on or off, response R1 */
#define SD_ACMD41 0x29  /** SD_SEND_OP_COMD - Sends host capacity support information and activates the card's initialization process */

// SD card error codes
#define SD_ERROR_CMD0           0x01 /** timeout error for command CMD0 (initialize card in SPI mode), signal problem */
#define SD_ERROR_CMD8           0x02 /** CMD8 was not accepted - not a valid SD card */
#define SD_ERROR_ACMD41         0x03 /** ACMD41 initialization process timeout */
#define SD_ERROR_CMD58          0x04 /** card returned an error response for CMD58 (read OCR) */
#define SD_ERROR_CMD16          0x05 /** card returned an error response for CMD16 (set block len) */
#define SD_ERROR_VOLTMATCH      0x06 /** card operation voltage range doesn't match (2.7V - 3.6V) */
#define SD_ERROR_READ_TIMEOUT   0x07 /** timeout while waiting for start of read data */
#define SD_ERROR_READ           0x08 /** card returned error token when tried to read data */
#define SD_ERROR_CMD17          0x09 /** card returned an error response for CMD17 (read single block) */
#define SD_ERROR_CMD9           0x0e /** card returned an error response for CMD9  (read CSD) */
#define SD_ERROR_CMD10          0x0f /** card returned an error response for CMD10 (read CID) */
#define SD_ERROR_CMD18          0x10 /** card returned an error response for CMD18 (read multi block) */
#define SD_ERROR_INVAL_SECT0    0x30 /** No valid MBR/FAT-BS signature found in sector 0 */
#define SD_ERROR_INVAL_BS       0x31 /** Malformed FAT boot sector */
#define SD_ERROR_FAT12          0x32 /** FAT12 is not supported */
#define SD_ERROR_FAT_NOT_INIT   0x33 /** FAT not initialized properly */
#define SD_ERROR_DIR_EOC        0x34 /** End of cluster reached (not a real error, just information) */
#define SD_ERROR_FILE_NOT_FOUND 0x35 /** File not found after reaching end of directory */
#define SD_ERROR_EOF            0x38 /** End of file reached */

// SD card structures and variables
typedef struct {
  uint8_t     Attributes;
  uint32_t    Size;           // in bytes
  uint32_t    FirstCluster;   // First cluster
  uint32_t    ActSector;      // 0 to (SD_FAT.SecPerClus - 1)
  uint32_t    ActBytePos;     // 0 to Size
} SD_File_t;

SD_File_t SD_FileInfo;            // Structure fichier

typedef struct  {
  uint8_t     PartType;           // Use this to test whether it is FAT16 or FAT32 or not initialized
  // Stuff from FAT boot sector
  uint8_t     SecPerClus;
  uint16_t    RsvdSecCnt;
  uint8_t     NumFATs;
  uint16_t    RootEntryCount;
  uint32_t    TotalSec;
  uint32_t    SecPerFAT;
  uint32_t    RootClus;
  // For cluster calculations
  uint8_t     ClusterSizeShift;
  uint32_t    ClusterCount;
  // Start addresses (all in blocks / sector addresses)
  uint32_t    BootSectorStart;    // Address of boot sector from FAT
  uint32_t    FatStart;           // First file allocation table starts here
  uint32_t    RootDirStart;       // Root directory starts here
  uint32_t    DataStart;          // Cluster 0 starts here
  uint32_t    ClusterEndMarker;   // if Cluster >= this then end of file reached.
} SD_FAT_t;

SD_FAT_t SD_FAT;                  // Structure FAT

// ======= Definition VARIABLES GLOBALES =======
// Attention a la taille limite de la RAM de l'Arduino utilise !

// Lecture SD card
uint8_t  SD_type;                 // type de la carte SD
uint8_t  SD_CSPin;                // numero de la broche CS de la carte SD
uint8_t  SD_buffer[513];          // SD buffer must hold 512 bytes + 1
char     filename[13];            // nom fichier courant. Dernier caractere = 0
int      pFile;                   // pointeur fichier
boolean  bSDcardStatus;           // statut SD card. false = off, true = on
boolean  bSelectFile;             // boucle selection fichiers. false = arret, true = boucle
#define  MAXFILES 52              // WARNING ! Limité par la taille de la RAM de l'Arduino (2 Ko)!!
char     *directory[MAXFILES];    // directory de la SD card = tableau des noms des fichiers
int      numFile;                 // numero du fichier courant
int      maxFile;                 // nombre maxi de fichiers recuperes
#define  ERR_OUT_OF_MEMORY 0x90   // pas assez de memoire pour charge le directory ! trop de fichiers sur la carte SD
char     ligne1[17];              // ligne 1 afficheur LCD. Dernier caractere = 0
char     ligne2[17];              // ligne 2 afficheur LCD. Dernier caractere = 0

// Definition pins Arduino:

// Interface afficheur LCD 2 x 16
#define pinLCD_E  2               // Enable
#define pinLCD_RS 3               // Register Select
#define pinLCD_D4 4               // Data 4 (configuration en mode 4 bits)
#define pinLCD_D5 5               // Data 5 (configuration en mode 4 bits)
#define pinLCD_D6 6               // Data 6 (configuration en mode 4 bits)
#define pinLCD_D7 7               // Data 7 (configuration en mode 4 bits)

// Boutons de navigation
#define pinKEY_1 A6               // utilisation des entrees analogiques restantes pour les commandes de navigation. Valeurs: 0..1023. 3 etats: 0V, 2,5 V et 5V
#define pinKEY_2 A7               // utilisation des entrees analogiques restantes pour les commandes de navigation. Valeurs: 0..1023. 3 etats: 0V, 2,5 V et 5V
#define MIN_ANALOG 100            // Seuil maximal tolere equivalent a 0V
#define MAX_ANALOG 600            // Seuil minimal tolere aquivalent a 5V

// Interface SD card
#define pinCS 10                  // pin CS (SS) SD card (SPI)
// Pin 11                            pin MOSI pour SD card (SPI) par defaut
// Pin 12                            pin MISO pour SD card (SPI) par defaut
// Pin 13                            pin SCK pour SD card (SPI) par defaut

// Interface port parallele imprimante Oric
#define pinOUT0 A0                 // Data 0 (A0 est parametre en sortie digital)
#define pinOUT1 A1                 // Data 1 (A1 est parametre en sortie digital)
#define pinOUT2 A2                 // Data 2 (A2 est parametre en sortie digital)
#define pinOUT3 A3                 // Data 3 (A3 est parametre en sortie digital)
#define pinOUT4 A4                 // Data 4 (A4 est parametre en sortie digital)
#define pinOUT5 A5                 // Data 5 (A5 est parametre en sortie digital)
#define pinOUT6 8                  // Data 6 (on utilise D8 car A6 est en entrée analogique uniquement malheureusement)
#define pinOUT7 9                  // Data 7 (on utilise D9 car A7 est en entrée analogique uniquement malheureusement)
#define pinACK 1                   // "Ack" Oric = CA1. TX1 (OUT). Si = 0 alors donnees pretes a etre lues par l'Oric
#define pinSTROBE 0                // "Strobe" Oric = PB4. RX0 (IN). Si = 0 alors l'Oric est pret a recevoir

#define TIMEOUT 30000000           // Timeout = 2 min. WARNING ! Utile pour les jeux en plusieurs parties !

// Gestion ecran LCD
#include <LiquidCrystal.h>

const int rs = pinLCD_RS;
const int en = pinLCD_E;
const int d4 = pinLCD_D4;
const int d5 = pinLCD_D5;
const int d6 = pinLCD_D6;
const int d7 = pinLCD_D7;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//=================================================================
// Initialisation de l'Arduino et de la SD card et recuperation des 2 premiers fichiers
void setup()
{
 uint8_t ret;                   // Code erreur retour fontion SD card
 
 // Initialisation pins Arduino.
 // NB: les pins du LCD sont initialises par la fonction LiquidCrystal() et ceux de la SD card par SD_Init()
 pinMode(pinOUT0, OUTPUT);             // config Data 0
 pinMode(pinOUT1, OUTPUT);             // config Data 1
 pinMode(pinOUT2, OUTPUT);             // config Data 2
 pinMode(pinOUT3, OUTPUT);             // config Data 3
 pinMode(pinOUT4, OUTPUT);             // config Data 4
 pinMode(pinOUT5, OUTPUT);             // config Data 5
 pinMode(pinOUT6, OUTPUT);             // config Data 6
 pinMode(pinOUT7, OUTPUT);             // config Data 7
 pinMode(pinACK, OUTPUT);              // config "Ack" Oric en output pour l'Arduino
 pinMode(pinSTROBE, INPUT);            // config "Strobe" Oric en input pour l'Arduino

 // Init Ack a 1 par defaut
 digitalWrite(pinACK, HIGH);        // par defaut = 1

 // Initialisation LCD et message de demarrage
 lcd.begin(16, 2);
 lcd.clear();
 strcpy(ligne1, "Loading dir...");
 strcpy(ligne2, "");
 print_lignes();

 // Init SD card
 SD_CSPin = pinCS;                 // init pin CS SD card
 ret = SD_Init_FileSystem();       // init SD card
 if (ret != 0)
 {
  strcpy(ligne1, "Filesystem err!");
  sprintf(ligne2, "%X", ret);
  strcat(ligne2, " error code");
  print_lignes();
  
  // Desactive la SD card
  SD_DeInit();                     // Shut down SD card
  bSDcardStatus = false;           // SD card = off
    
  strcpy(ligne1, "SDcard shut down");
  strcpy(ligne2, "");
  print_lignes();
 }
 else
 {
  SD_SpiSetHighSpeed();            // init vitesse SD card

  // Lecture catalogue SD card niveau root et noms courts uniquement
  numFile = 0;
  ret = SD_Dir(0x00, 0x00, 0x18);        // Lecture directory SD card
  maxFile = numFile - 1;
 
  // Verifie si le directory a ete correctement lu
  if ((maxFile == -1) || (ret != 0))
  {
   if (ret != 0)                     // Erreur lecture directory
   {
    strcpy(ligne1, "Directory error!");
    strcpy(ligne2, "Error:0x");
    sprintf(ligne2, "%X", ret);
    strcat(ligne2, " error code");
    print_lignes();
   }
   else                             // Directory vide (pas de fichiers sur la SD card)
   {
    strcpy(ligne1, "Empty directory!");
    strcpy(ligne2, "");
    print_lignes();
   }
   
   // Desactive la SD card
   SD_DeInit();                     // Shut down SD card
   bSDcardStatus = false;           // SD card = off
     
   strcpy(ligne1, "SDcard shut down");
   strcpy(ligne2, "");
   print_lignes();
  }
  else
  {
   bSDcardStatus = true;       // statut SD card = on
   
   numFile = 0;
   
   // Affichage des 2 premiers noms de fichiers par defaut
   print_filenames();
   
   bSelectFile = true;               // Active boucle selection fichiers
  }
 }
}

//=================================================================
// Navigation haut / bas parmi les fichiers et selection du fichier a envoyer 
void loop(void)
{
 uint32_t SD_Bytes;                   // nb d'octets a lire
 char SD_Byte;                        // lecture d'un octet
 int i;                               // compteur boucle
 uint32_t t;                          // compteur timeout
 const int delai = 1600;              // delai en microsecondes. Max possible = 1683
 boolean bStrobe;                     // indicateur Strobe: false = 0 (ready), true = 1 (not ready)
 boolean bTimeout;                    // indicateur timeout: false = OK, true = timeout
 const int ACK_LENGTH = 100;          // Duree Ack = 100 micro-secondes
 boolean bKey1, bKey2, bKey3, bKey4;  // Etat des boutons
 uint8_t ret;                         // Code erreur retour fontion SD card
 
 if (bSDcardStatus)        // Si SD card active
 {
  if (bSelectFile)              // Si boucle selection fichiers
  {
   if (analogRead(pinKEY_1) < MIN_ANALOG) bKey1 = true; else bKey1 = false;      // Si bouton 1 appuye
   if (analogRead(pinKEY_1) > MAX_ANALOG) bKey2 = true; else bKey2 = false;      // Si bouton 2 appuye
   if (analogRead(pinKEY_2) < MIN_ANALOG) bKey3 = true; else bKey3 = false;      // Si bouton 3 appuye
   if (analogRead(pinKEY_2) > MAX_ANALOG) bKey4 = true; else bKey4 = false;      // Si button 4 appuye
   
/*   // Affiche valeurs analogiques pour test
   lcd.setCursor(0, 1);
   lcd.print(String(analogRead(pinKEY_1)) + " " + String(analogRead(pinKEY_2))+ "         ");
   bKey1 = false;
   bKey2 = false;
   bKey3 = false;
   bKey4 = false;
   delay(100);
*/
   
   // ============ Si arret ============
   if (bKey1)
   {
    // Desactive la SD card
    SD_DeInit();                     // Shut down SD card
    bSDcardStatus = false;           // SD card = off
    
    strcpy(ligne1, "SDcard shut down");
    strcpy(ligne2, "");
    print_lignes();
    bSelectFile = false;             // sortie de la boucle de selection des fichiers

    // Libere la memoire allouee
    free(directory);
   }
   
   // ============ Si fichier precedent ============
   if (bSelectFile and bKey2)
   {
    if (numFile > 0) numFile--;
    
    // Affichage fichiers precedents
    print_filenames(); 
    delay(200);
   }
   
   // ============ Si fichier suivant ============
   if (bSelectFile and bKey3)
   {
    if (numFile < maxFile) numFile++;

    // Affichage fichiers suivants
    print_filenames();
    delay(200);
   }
   
   // ============ Si selection fichier pour envoi ============
   if (bSelectFile and bKey4)
   {
    strcpy(filename, directory[numFile]);
    // Recherche filename sur SD card sur le repertoire racine uniquement (=cluster 0), shortname files only (0x00,0x18)
    pFile = SD_SearchFile((uint8_t *)filename, 0UL, 0x00, 0x18, &SD_FileInfo);
    
    if (pFile)
    {
     // Fichier non trouve !
     strcpy(ligne1, "File not found!");
     sprintf(ligne2, "%X", pFile);
     strcat(ligne2, " error code");
     print_lignes();
    }
    else
    {
     // Affiche nom fichier
     //strcpy(ligne2, "File found!");
     //print_lignes();
     
     noInterrupts();                                  // desactiver les interruptions 
     
     SD_Bytes = 0;
     
     // ==== Send CMD18 (multiblock read) ====
     uint32_t offset = SD_FileInfo.ActSector;         // offset = secteur
     if (SD_type != SD_CARD_TYPE_SDHC) offset <<= 9;  // si carte non SDHC alors offset = octet
     SD_CardCommand(18, offset);                      // lance CMD18
     
     strcpy(ligne1, "Sending...");
     sprintf(ligne2, "%u", SD_FileInfo.Size);
     strcat(ligne2, " bytes");
     print_lignes();
     
     bTimeout = false;
     
     // ==== Tant qu'il reste des octets et pas de timeout ====
     while((SD_Bytes < SD_FileInfo.Size) and !bTimeout)
     {
      // Attends octet 0xfe de debut de bloc
      while(SD_SpiReadByte() != 0xfe); 
      
      // ==== Traitement d'un bloc de 512 octets lus sur la carte SD ====
      i = 0;
      while((i < 512) and (SD_Bytes < SD_FileInfo.Size) and !bTimeout)
      {
       SD_Byte = SD_SpiReadByte();               // lecture d'un octet
       SD_Bytes++;                               // incremente nombre d'octets lus 
       
       // Attends que l'Oric soit pret (Strobe doit passer a 0)
       bStrobe = true;
       t = 0;                                    // Compteur pour timeout
       while(bStrobe and (t < TIMEOUT))          // Attends que Strobe passe à 0
       {
        delayMicroseconds(1);                        // temporisation environ 1 micro-seconde
        bStrobe = (digitalRead(pinSTROBE) == HIGH);  // lecture signal Strobe. Si = 1 alors attends
        t++;                                         // compteur pour timeout
       }
       if (t == TIMEOUT) bTimeout = true;
       
       if (!bTimeout)
       {
        // Envoi octet...
        digitalWrite(pinOUT0, bitRead(SD_Byte, 0));              // ecriture bit 0
        digitalWrite(pinOUT1, bitRead(SD_Byte, 1));              // ecriture bit 1
        digitalWrite(pinOUT2, bitRead(SD_Byte, 2));              // ecriture bit 2
        digitalWrite(pinOUT3, bitRead(SD_Byte, 3));              // ecriture bit 3
        digitalWrite(pinOUT4, bitRead(SD_Byte, 4));              // ecriture bit 4
        digitalWrite(pinOUT5, bitRead(SD_Byte, 5));              // ecriture bit 5
        digitalWrite(pinOUT6, bitRead(SD_Byte, 6));              // ecriture bit 6
        digitalWrite(pinOUT7, bitRead(SD_Byte, 7));              // ecriture bit 7

        // Envoi impulsion negative 100 micro-secondes sur Ack
        // WARNING ! Le VIA de l'Oric doit etre configure pour detecter sur le front DESCENDANT de CA1
        digitalWrite(pinACK, LOW);             // OK pour lecture par l'Oric. Mets Ack à 0 et active le buffer 74LS244
        delayMicroseconds(ACK_LENGTH);         // temporisation environ 100 micro-secondes
//        for(t = 0; t < 150 ; t++) delayMicroseconds(delai);       // temporisation plus longue pour test
        digitalWrite(pinACK, HIGH);            // Remet Ack a 1
        
/*        // Affiche compteur tous les 100 octets     
        if ((SD_Bytes % 100) == 0)
//        if (true)                            // Affiche a chaque octet pour test
        {
         lcd.setCursor(0, 1);
         lcd.print("Sending:" + String(SD_Bytes) + "       ");         // affiche nb d'octets envoyes
         // lcd.print("Sending:" + String((uint8_t) SD_Byte, HEX));    // affiche octet envoye
         // for(t = 0; t < 30 ; t++) delayMicroseconds(delai);         // temporisation pour test
        }
*/
       }
  
       i++;                            // on passe a l'octet suivant du bloc
      }

      if (i == 512)
      {
       // On saute les 2 octets de CRC a la fin du bloc et on passe au bloc suivant
       SD_SpiReadByte();                // lecture octet CRC1
       SD_SpiReadByte();                // lecture octet CRC2
      }
     }
      
     interrupts();                     //reactiver les interruptions
     
     if (!bTimeout)                    // Si pas de timeout alors fichier bien envoye sinon afficher timeout
     {
      print_filenames();
      strcpy(ligne2, "File sent!");
      print_lignes();
      print_filenames();
     }
     else
     {
      print_filenames();
      strcpy(ligne2, "Timeout!");
      print_lignes();
     }

     ret = SD_Init_FileSystem();       // reconstruit le filesystem car sinon KO. Un peu "brut de fonderie"... A affiner
     if (ret != 0)
     {
      strcpy(ligne1, "Filesystem err!");
      sprintf(ligne2, "%X", ret);
      strcat(ligne2, " error code");
      print_lignes();
     }
    }
   }
  }
 }
}

//==========================================================================

// ----------- Affichage des 2 premiers noms de fichiers par defaut ----------
void print_filenames()
{
   strcpy(ligne1, "->");
   strcat(ligne1, directory[numFile]);
   lcd.setCursor(0, 0);
   lcd.write("                ");
   lcd.setCursor(0, 0);
   lcd.write(ligne1);

   if (numFile < maxFile)
   {
     strcpy(ligne2, "  ");
     strcat(ligne2, directory[numFile+1]);
     lcd.setCursor(0, 1);
     lcd.write("                ");
     lcd.setCursor(0, 1);
     lcd.write(ligne2);
   }
   else
   {
     lcd.setCursor(0, 1);
     lcd.write("                ");
   }
}

// ------- Affichage 2 lignes sur ecran LCD ---------
void print_lignes()
{
   lcd.setCursor(0, 0);
   lcd.write("                ");
   lcd.setCursor(0, 0);
   lcd.write(ligne1);
   lcd.setCursor(0, 1);
   lcd.write("                ");
   lcd.setCursor(0, 1);
   lcd.write(ligne2);
   delay(3000);
}

/**************************************************************************\
* Set CS High 
* Sends also one dummy byte to ensure MISO goes high impedance
\**************************************************************************/
void SD_SetCSHigh()
{
 digitalWrite(SD_CSPin, HIGH);
 SD_SpiSendByte(0xff);
}

/**************************************************************************\
* Sends a raw byte to the SPI - \param[in] b The byte to sent.
\**************************************************************************/
void SD_SpiSendByte(uint8_t b)
{
 SPDR = b;
 while(!(SPSR & (1 << SPIF))); /* wait for byte to be shifted out */
 SPSR &= ~(1 << SPIF);
}

/**************************************************************************\
* Send a command to the memory card which responses with a R1 response 
* (and possibly others).
* \param[in] command The command to send.
* \param[in] arg The argument for command.
* \returns The command answer.
\**************************************************************************/
uint8_t SD_CardCommand(uint8_t cmd, uint32_t arg) 
{
 uint8_t response;
 uint8_t crc;
 uint16_t t0;
      
 // select card
 digitalWrite(SD_CSPin, LOW);

 // wait up to timeout if busy
 t0 = ((uint16_t)millis());
 while (SD_SpiReadByte() != 0xFF)
  if ((((uint16_t)millis()) - t0) >= SD_COMMAND_TIMEOUT) break;

 // send command
 SD_SpiSendByte(cmd | 0x40);

 // send argument
 SD_SpiSendByte((arg >> 24) & 0xff);
 SD_SpiSendByte((arg >> 16) & 0xff);
 SD_SpiSendByte((arg >>  8) & 0xff);
 SD_SpiSendByte((arg >>  0) & 0xff);
  
 // send CRC, only required for commands 0 and 8
 crc = 0xFF;
 if (cmd == SD_CMD0) crc = 0x95;  // correct crc for CMD0 with arg 0
 if (cmd == SD_CMD8) crc = 0x87;  // correct crc for CMD8 with arg 0X1AA
 SD_SpiSendByte(crc);

 // skip stuff byte for stop read
 if (cmd == SD_CMD12) SD_SpiReadByte();

 // wait for response
 for(uint8_t i = 0; i < 100; ++i)
 {
  response = SD_SpiReadByte();
  if(response != 0xff) break;
 }
 return response;
}

/**************************************************************************\
* Send an application specific command which responses with a R1 response 
* (and possibly others).
* \param[in] command The command to send.
* \param[in] arg The argument for command.
* \returns The command answer.
\**************************************************************************/
uint8_t SD_CardACommand(uint8_t cmd, uint32_t arg)
{
 SD_CardCommand(SD_CMD55, 0);
 return SD_CardCommand(cmd, arg);
}

/**************************************************************************\
 * Initialize the SD memory card.
 * Power up the card, set SPI mode.
 * Detects the card version (V1, V2, SDHC), sets sector length to 512.
 * \return Zero if successfull, errorcode otherwise
\**************************************************************************/
uint8_t SD_Init()
{
 uint8_t status;
 uint16_t t0 = ((uint16_t)millis());
 uint32_t arg;

 /* Setup ports */
 pinMode(SD_CSPin, OUTPUT);
 digitalWrite(SD_CSPin, HIGH);
 pinMode(MISO, INPUT);
 pinMode(SCK, OUTPUT);
 pinMode(MOSI, OUTPUT);
 pinMode(SS, OUTPUT);
  
 digitalWrite(SCK, LOW);
 digitalWrite(MOSI, LOW);
 digitalWrite(SS, HIGH);

 /*
 * SPI configuration: 
 * - enable uC for SPI master
 * - typical no interrupts are used for SPI
 * - data order: MSB is transmitted first
 * - clock polarity: CLK is low when idle
 * - clock phase: 1-0 > Sample, 0-1 > Setup
 * - clock frequency: less than 400kHz 
 *   (will be switched to higher value after initialization)
 */
 /* initialize SPI with lowest frequency; max. 400kHz during identification mode of card */
 SPCR = (0 << SPIE) | /* SPI Interrupt Enable */
        (1 << SPE)  | /* SPI Enable */
        (0 << DORD) | /* Data Order: MSB first */
        (1 << MSTR) | /* Master mode */
        (0 << CPOL) | /* Clock Polarity: SCK low when idle */
        (0 << CPHA) | /* Clock Phase: sample on rising SCK edge */
        (1 << SPR1) | /* Clock Frequency: f_OSC / 128 */
        (1 << SPR0);
 SPSR &= ~(1 << SPI2X); /* No doubled clock frequency */
  
 // must supply min of 74 clock cycles with CS high.
 SD_SetCSHigh();
 for (uint8_t i = 0; i < 10; i++) SD_SpiSendByte(0xFF);

 // command to go idle in SPI mode
 while ((SD_CardCommand(SD_CMD0, 0)) != SD_IDLE_STATE)
  if ((((uint16_t)millis()) - t0) > SD_INIT_TIMEOUT) {SD_SetCSHigh(); return(SD_ERROR_CMD0);}
  
 // check SD version ( 2.7V - 3.6V + test pattern )
 SD_type = 0;
 if ((SD_CardCommand(SD_CMD8, 0x1AA) & SD_ILLEGAL_COMMAND)) SD_type = SD_CARD_TYPE_SD1;
    // Not done here: Test if SD or MMC card here using CMD55 + CMD1
 else
 {
  // only need last byte of r7 response
  SD_SpiReadByte();
  SD_SpiReadByte();
  status = SD_SpiReadByte();
  if ((status & 0x01) == 0) // card operation voltage range doesn't match
  {SD_SetCSHigh(); return(SD_ERROR_VOLTMATCH);}
  if (SD_SpiReadByte() != 0xAA) {SD_SetCSHigh(); return(SD_ERROR_CMD8);}
  SD_type = SD_CARD_TYPE_SD2;
 }
  
 // Turn CRC option off
 SD_CardCommand(SD_CMD59, 0);
  
 // initialize card and send host supports SDHC if SD2
 arg = (SD_type == SD_CARD_TYPE_SD2) ? 0X40000000 : 0;
 while ((SD_CardACommand(SD_ACMD41, arg)) != SD_READY_STATE) // check for timeout
  if ((((uint16_t)millis()) - t0) > SD_INIT_TIMEOUT) {SD_SetCSHigh(); return(SD_ERROR_ACMD41);}
  
 // if SD2 read OCR register to check for SDHC card
 if (SD_type == SD_CARD_TYPE_SD2)
 {
  if (SD_CardCommand(SD_CMD58, 0)) {SD_SetCSHigh(); return(SD_ERROR_CMD58);}
  // other implementation test only against 0x40 for SDHC detection...
  if ((SD_SpiReadByte() & 0xC0) == 0xC0) SD_type = SD_CARD_TYPE_SDHC;
  // discard rest of ocr - contains allowed voltage range
  SD_SpiReadByte();
  SD_SpiReadByte();
  SD_SpiReadByte();
 }

 // set block size to 512 bytes
 if(SD_CardCommand(SD_CMD16, 512)) {SD_SetCSHigh(); return(SD_ERROR_CMD16);}
 SD_SetCSHigh();
 SD_SpiSetHighSpeed();
 return 0;
}

/**************************************************************************\
* DeInitialize the file system (for safe power down mode)
\**************************************************************************/
void SD_DeInit()
{
  /* Get status (initialized or not) by reading SD_FAT.PartType */
  SD_FAT.PartType = SD_PARTTYPE_UNKNOWN;

  /* Shutdown SD card safely */
  SD_type = 0;
  
  // must supply min of 74 clock cycles with CS high.
  SD_SetCSHigh();
  for (uint8_t i = 0; i < 10; i++) SD_SpiSendByte(0xFF);
  
  /* Setup pins and SPI for minimum power consumption. Deactivate SPI and power from card */
  SPCR &= ~(1 << SPE);
  
  /* Setup ports for safe eject */
  pinMode(SD_CSPin, INPUT);
  pinMode(MISO, INPUT);
  pinMode(MOSI, INPUT);
  pinMode(SCK, INPUT);
  digitalWrite(SD_CSPin, HIGH); // Set CS high
  digitalWrite(SCK, HIGH); 
}

/**************************************************************************\
* Initialize the file system .
* Does the lower level initialization and * tries to find the boot sector 
* of the first FAT16 or FAT32 partition and parse it.
* Workbuf must hold at least 512 bytes.
* Workbuf will be used later also for following functions:
* - SD_SearchFile
* - SD_Dir
* \return Zero if successful, error code otherwise
\**************************************************************************/
uint8_t SD_Init_FileSystem()
{
 uint8_t  retval;
 uint8_t  PartType;
 uint16_t temp16;
 uint32_t temp32;
    
 SD_FAT.PartType = SD_PARTTYPE_UNKNOWN;
    
 // Try init SD-Card
 retval = SD_Init();
 if(retval) return(retval);
    
 // ==== MBR (partition table) access here =====
    
 // Read sector 0 
 retval = SD_ReadBlock(0);
 if(retval) return(retval);

 // Test for signature (valid not only for MBR, but FAT Boot Sector as well!)
 if((SD_buffer[0x1fe] != 0x55) || (SD_buffer[0x1ff] != 0xaa)) return(SD_ERROR_INVAL_SECT0);
    
 // Store most important MBR values for first partition
 PartType = SD_buffer[0x1be + 0x04];
 SD_FAT.BootSectorStart =  (uint32_t)SD_buffer[0x1be + 0x08] 
                        | ((uint32_t)SD_buffer[0x1be + 0x09] << 8UL)
                        | ((uint32_t)SD_buffer[0x1be + 0x0a] << 16UL)
                        | ((uint32_t)SD_buffer[0x1be + 0x0b] << 24UL);
    
    // Check MBR values for plausibility
    if(((SD_buffer[0x1be] & 0x7f) == 0)
      && ((PartType == 0x04) || (PartType == 0x06) || (PartType == 0x0B) 
           || (PartType == 0x0C) || (PartType == 0x0E)) )  
    {
        // MBR seems to contain valid FAT16/FAT32 partition entry
        SD_FAT.PartType = ((PartType == 0x0B) || (PartType == 0x0C)) ? SD_PARTTYPE_FAT32 : SD_PARTTYPE_FAT16;
    }
    else
    {
        // MBR seems to contain not an valid entry, so try for super-floppy now
        SD_FAT.BootSectorStart = 0UL;
        SD_FAT.PartType = SD_PARTTYPE_SUPERFLOPPY;
    }
    
    // ====== FAT access here ======
    
    // Read Boot-Sector and test for signature
    retval = SD_ReadBlock(SD_FAT.BootSectorStart);
    if(retval) return(retval);  

    // Test for signature (valid not only for MBR, but FAT Boot Sector as well!)
    if((SD_buffer[0x1fe] != 0x55) || (SD_buffer[0x1ff] != 0xaa)) return(SD_ERROR_INVAL_BS);
    
    // Plausibility checks for FAT
    if((SD_buffer[0x0b] != 0x00) || (SD_buffer[0x0c] != 0x02) || (SD_buffer[0x15] != 0xf8)) return(SD_ERROR_INVAL_BS);

    // Read fields that are same for FAT16 and FAT32
    SD_FAT.SecPerClus = SD_buffer[0x0d];
    SD_FAT.RsvdSecCnt = (uint16_t)SD_buffer[0x0e] | ((uint16_t)SD_buffer[0x0f]<<8U);
    if((SD_FAT.SecPerClus == 0) || (SD_FAT.RsvdSecCnt == 0)) return(SD_ERROR_INVAL_BS);
    SD_FAT.NumFATs = SD_buffer[0x10];
    SD_FAT.RootEntryCount = (uint16_t)SD_buffer[0x11] | ((uint16_t)SD_buffer[0x12]<<8U);
    
    temp16 = (uint16_t)SD_buffer[0x13] | ((uint16_t)SD_buffer[0x14]<<8U);
    temp32 = (uint32_t)SD_buffer[0x20] | ((uint32_t)SD_buffer[0x21]<<8U) | ((uint32_t)SD_buffer[0x22]<<16U) | ((uint32_t)SD_buffer[0x23]<<24U);
    SD_FAT.TotalSec  = temp16 ? temp16 : temp32;
    
    temp16 = (uint16_t)SD_buffer[0x16] | ((uint16_t)SD_buffer[0x17]<<8U);
    temp32 = (uint32_t)SD_buffer[0x24] | ((uint32_t)SD_buffer[0x25]<<8U) | ((uint32_t)SD_buffer[0x26]<<16U) | ((uint32_t)SD_buffer[0x27]<<24U);
    SD_FAT.SecPerFAT  = temp16 ? temp16 : temp32;
    
    // Calculate start sectors
    SD_FAT.FatStart = SD_FAT.BootSectorStart + (uint32_t)SD_FAT.RsvdSecCnt;
    SD_FAT.RootDirStart = SD_FAT.FatStart + SD_FAT.NumFATs * (uint32_t)SD_FAT.SecPerFAT;
    
    // Data area starts at cluster #2
    SD_FAT.DataStart = SD_FAT.RootDirStart+ ((32 * (uint32_t)SD_FAT.RootEntryCount + 511)/512) - (2 * SD_FAT.SecPerClus);
    
    // determine shift that is same as multiply by SD_FAT.SecPerClus
    SD_FAT.ClusterSizeShift = 0;
    while (SD_FAT.SecPerClus != (1 << SD_FAT.ClusterSizeShift))
    {
      // error if not power of 2
      if (SD_FAT.ClusterSizeShift++ > 7) return(SD_ERROR_INVAL_BS);
    }  
    
     // Calculate number and shifting of clusters
    // total data blocks
    SD_FAT.ClusterCount = SD_FAT.TotalSec - (SD_FAT.DataStart - SD_FAT.BootSectorStart);
    // divide by cluster size to get cluster count
    SD_FAT.ClusterCount >>= SD_FAT.ClusterSizeShift;  
    
    // determine if FAT16 or FAT32 (only by cluster count as done by M$)
    if (SD_FAT.ClusterCount < 4085)
    {
        // this would be FAT12, which is not supported
        SD_FAT.PartType = SD_PARTTYPE_UNKNOWN;
        return(SD_ERROR_FAT12);
    }
    else
    if (SD_FAT.ClusterCount < 65525)
    {
        SD_FAT.PartType = SD_PARTTYPE_FAT16;
        SD_FAT.ClusterEndMarker = 0xfff8UL;
    }
    else
    {
        temp32 = (uint32_t)SD_buffer[0x2c] | ((uint32_t)SD_buffer[0x2d]<<8U) | ((uint32_t)SD_buffer[0x2e]<<16U) | ((uint32_t)SD_buffer[0x2f]<<24U);
        SD_FAT.RootDirStart = SD_Cluster2Sector(temp32);
        SD_FAT.PartType = SD_PARTTYPE_FAT32;
        SD_FAT.ClusterEndMarker = 0xffffff8UL;
    }
    return 0;
}

/**************************************************************************\
* Set SPI for full operation speed (up to 25 MHz).
* Will be called after first part of card 
* initialization was successful.
\**************************************************************************/
void SD_SpiSetHighSpeed(void)
{
 SPCR &= ~((1 << SPR1) | (1 << SPR0)); /* Clock Frequency: f_OSC / 4 */
 SPSR |= (1 << SPI2X);         /* Doubled Clock Frequency: f_OSC / 2 */
}

/**************************************************************************\
* Receives a raw byte from the SPI.
* \returns The byte which should be read.
\**************************************************************************/
uint8_t SD_SpiReadByte()
{
 SPDR = 0xff; /* send dummy data for receiving some */
 while(!(SPSR & (1 << SPIF)));
 SPSR &= ~(1 << SPIF);
 return SPDR;
}

/**************************************************************************\
* Read a 512 byte block from an SD card.
* \param[in] blockNumber Logical block to be read.
* \param[out] dst Pointer to the location that will receive the data.
* \return 0 is returned for success, error code otherwise
\**************************************************************************/
uint8_t SD_ReadBlock(uint32_t blockNumber) 
{
  uint8_t status;
  uint16_t t0;

 // use address if not SDHC card
 if (SD_type != SD_CARD_TYPE_SDHC) blockNumber <<= 9;
 if (SD_CardCommand(SD_CMD17, blockNumber)) {SD_SetCSHigh(); return(SD_ERROR_CMD17);}

 // wait for start block token
 t0 = ((uint16_t)millis());
 while ((status = SD_SpiReadByte()) == 0xFF)
  if ((((uint16_t)millis()) - t0) > SD_READ_TIMEOUT) {SD_SetCSHigh(); return(SD_ERROR_READ_TIMEOUT);}
 if (status != SD_DATA_START_BLOCK) {SD_SetCSHigh(); return(SD_ERROR_READ);}
  
 // transfer data
 SPDR = 0xFF;
 for (uint16_t i = 0; i < 512; i++)
 {
  while (!(SPSR & (1 << SPIF)));
  SD_buffer[i] = SPDR;
  SPDR = 0xFF;
 }
 while (!(SPSR & (1 << SPIF)));
 SD_buffer[512] = SPDR;

 // discard CRC
 SD_SpiReadByte();
 SD_SpiReadByte();
 SD_SetCSHigh();

 return 0;
}

/**************************************************************************\
* Returns the first sector of a given cluster
\**************************************************************************/
uint32_t SD_Cluster2Sector(uint32_t cluster)
{
 return((cluster << SD_FAT.ClusterSizeShift) + SD_FAT.DataStart);
}

/**************************************************************************\
* Search a file in the directory.
* Filename must be 8.3 format, terminated by \0 (can not access ".." now...)
* Works only over one cluster of directory information. 
* If SD_ERROR_DIR_EOC is returned call function again with next cluster number. 
* Set cluster to 0 to access root directory.
* Deleted files and long name entries are not shown generally.
* Only files are printed that has their attributes set/unset regarding maskSet/maskUnset.
* Examples for maskSet, maskUnset:
*  Ouput everything:           0x00, 0x00
*  Shortname files only:       0x00, 0x18
*  Shortname files and dirs:   0x00, 0x08
*  Shortname dirs:             0x10, 0x08
*  Volume name:                0x08, 0x10
* Mask bits: B7 = 0, B6 = 0, B5 = archive, B4 = directory, 
*            B3 = volume name, B2 = system, B1 = hidden, B0 = read only
* If file is found, fileinfo gets filled with file attributes, 
* file size in bytes and first cluster.
*  return Zero if successfully, error code otherwise
\**************************************************************************/
uint8_t SD_SearchFile(uint8_t *filename, const uint32_t cluster, const uint8_t maskSet, const uint8_t maskUnset, SD_File_t *fileinfo)
{
 uint16_t maxsect = SD_FAT.SecPerClus;
 uint32_t startsect = SD_Cluster2Sector(cluster);
 char     fnentry[12];   // file entry, ie filename without the '.'

 if((SD_FAT.PartType != SD_PARTTYPE_FAT16) && (SD_FAT.PartType != SD_PARTTYPE_FAT32)) return(SD_ERROR_FAT_NOT_INIT);
 
 if(cluster == 0)
 {
  startsect = SD_FAT.RootDirStart; // Set root dir sector
  if(SD_FAT.PartType == SD_PARTTYPE_FAT16) maxsect = (uint16_t)((32 * (uint32_t)SD_FAT.RootEntryCount + 511)/512);
 }
 
 // convert filename to space-filled uppercase format
 for(uint8_t i = 0; i < 11; i++) fnentry[i] = ' '; // init file entry with spaces
 
 for(uint8_t i = 0; i < 9; i++)                    // process the filename without the extension (8 char)
 {
  uint8_t c = *filename++;
  if((c < 0x20) || (c == '.')) break;              // stop at extension delimiter ('.')
  if((c>='a') && (c<='z')) c -= 0x20;              // to upper case
  fnentry[i] = c;
 }
 
 for(uint8_t i = 8; i < 11; i++)                   // process the extension (3 char)
 {
  uint8_t c = *filename++;
  if(c < 0x20) break;
  if((c>='a') && (c<='z')) c -= 0x20;              // to upper case
  fnentry[i] = c;
 }
 
 fnentry[11] = 0;    // end the file entry
 
 // go through sectors
 for(uint16_t i = 0; i < maxsect; i++)
 {
  uint8_t retval = SD_ReadBlock(startsect + i);             // read a block
  
  if(retval) return(retval);
  
  for(uint16_t j = 0; j < 512; j+=32)
  {
   uint8_t attrib;
   if(SD_buffer[j] == 0) return(SD_ERROR_FILE_NOT_FOUND);    // Last entry when first character of filename == 0
   if(SD_buffer[j] == 0xe5) continue;  // Skip deleted files
   if(SD_buffer[j] == 0x05) SD_buffer[j] = 0xE5;
   
   attrib = SD_buffer[j+0x0b];
   
   // Test masks (skip long file name entries also)
   if(((attrib & maskSet) == maskSet) && ((attrib & maskUnset) == 0) && (attrib != 0x0f))
   {
    uint16_t k;
    
    // compare filename
    for(k = 0; k < 11; k++) if(SD_buffer[j+k] != fnentry[k]) break;
    if(k >= 11)
    {
     // found it
     fileinfo->Attributes = attrib;
     fileinfo->Size = (uint32_t)SD_buffer[j+0x1c] | (((uint32_t)SD_buffer[j+0x1d])<<8) 
                  | (((uint32_t)SD_buffer[j+0x1e])<<16) | (((uint32_t)SD_buffer[j+0x1f])<<24);
     if(SD_FAT.PartType == SD_PARTTYPE_FAT16)
      {fileinfo->FirstCluster = (uint32_t)SD_buffer[j+0x1a] | (((uint32_t)SD_buffer[j+0x1b])<<8);}
     else
      {fileinfo->FirstCluster = (uint32_t)SD_buffer[j+0x1a] | (((uint32_t)SD_buffer[j+0x1b])<<8) 
                            | (((uint32_t)SD_buffer[j+0x14])<<16) | (((uint32_t)SD_buffer[j+0x15])<<24);} 
     
     // Initialize some things
     fileinfo->ActSector = SD_Cluster2Sector(fileinfo->FirstCluster);
     fileinfo->ActBytePos = 0;
     return(0);
    }
   }
  }
 }
 if(SD_FAT.PartType == SD_PARTTYPE_FAT16) return(SD_ERROR_FILE_NOT_FOUND);
 return(SD_ERROR_DIR_EOC);
}

/**
 * Gets a directory.
 *
 * Works only over one cluster of directory information. If 
 * SD_ERROR_DIR_EOC is returned call function again with next
 * cluster number. 
 *
 * Set cluster to 0 to access root directory.
 *
 * Deleted files and long name entries are not shown generally.
 *
 * Only files are printed that has their attributes set/unset regarding maskSet/maskUnset.
 * Examples for maskSet, maskUnset:
 *  Ouput everything:           0x00, 0x00
 *  Shortname files only:       0x00, 0x18
 *  Shortname files and dirs:   0x00, 0x08
 *  Shortname dirs:             0x10, 0x08
 *  Volume name:                0x08, 0x10
 *
 * Mask bits: B7 = 0, B6 = 0, B5 = archive, B4 = directory, 
 *            B3 = volume name, B2 = system, B1 = hidden, B0 = read only
 */
uint8_t SD_Dir(const uint32_t cluster, const uint8_t maskSet, const uint8_t maskUnset)
{
    uint16_t maxsect = SD_FAT.SecPerClus;
    uint32_t startsect = SD_Cluster2Sector(cluster);
    
    if((SD_FAT.PartType != SD_PARTTYPE_FAT16) && (SD_FAT.PartType != SD_PARTTYPE_FAT32)) return(SD_ERROR_FAT_NOT_INIT);
    
    if(cluster == 0)
    {
        // Set root dir sector
        startsect = SD_FAT.RootDirStart;
        if(SD_FAT.PartType == SD_PARTTYPE_FAT16) maxsect = (uint16_t)((32 * (uint32_t)SD_FAT.RootEntryCount + 511)/512);
    }
    
    for(uint16_t i = 0; i<maxsect; i++)         // browse sectors
    {
        uint8_t retval = SD_ReadBlock(startsect + i);    // read a bloc
        
        if(retval) return(retval);
        
        for(uint16_t j = 0; j < 512; j+=32)       // browse sector bytes
        {
            uint8_t attrib;
            if(SD_buffer[j] == 0) return(0);    // Last entry when first character of filename == 0
            if(SD_buffer[j] == 0xe5) continue;  // Skip deleted files
            if(SD_buffer[j] == 0x05) SD_buffer[j] = 0xE5;
            
            attrib = SD_buffer[j+0x0b];
            
            // Test masks (skip long file name entries also)
            if(((attrib & maskSet) == maskSet) && ((attrib & maskUnset) == 0) && (attrib != 0x0f))
            {
                uint8_t z=0;
                // Prepare output
                for(uint16_t k = 0; k < 12; k++) filename[k] = ' ';    // init filename with spaces
                
                for(uint16_t k = 0; k < 8; k++)                        // add the filename without the extension (8 char)
                {
                  filename[z] = SD_buffer[j+k];
                  if(filename[z] > ' ') z++;                           // Remove space
                }
                
                filename[z++] = '.';                                   // add the extension delimiter ('.')
                
                for(uint16_t k = 0; k < 3; k++)                        // add the extension (3 char)
                {
                  filename[z++] = SD_buffer[j+k+0x08];
                }
                
                filename[z++] = 0;                                     // end of filename
                
                directory[numFile] = (char *)malloc(sizeof(filename) * sizeof(char));      // Alloue de la place pour un nom supplementaire
                if(directory[numFile] == NULL)                        // manque de memoire
                {
                 strcpy(ligne1, "Lack of memory!");
                 sprintf(ligne2, "%u", numFile);
                 strcat(ligne2, " files");
                 print_lignes();

                 return(ERR_OUT_OF_MEMORY);
                }

                strcpy(directory[numFile], filename);           // Sauve le nom de fichier trouve dans le tableau du directory

                // Affichage des fichiers recuperes
                //lcd.setCursor(0,1);
                //lcd.print(String(numFile+1) + ":" + String(directory[numFile]) + "           ");
                //delay(50);

                numFile++;
                
                if (numFile > MAXFILES)             // Nb max de fichiers atteint
                {
                 strcpy(ligne1, "Partial dir!");
                 sprintf(ligne2, "%u", MAXFILES);
                 strcat(ligne2, " files");
                 print_lignes();

                 return(0);
                }
            }
        }
    }
    if(SD_FAT.PartType == SD_PARTTYPE_FAT16)
      return(0);
    return(SD_ERROR_DIR_EOC);
}

