/*  Simple keyboard to serial port at 115200 baud

  The circuit:
     KBD Clock (PS2 pin 1) to an interrupt pin on Arduino ( this example pin 3 )
     KBD Data (PS2 pin 5) to a data pin ( this example pin 4 )
     +5V from Arduino to PS2 pin 4
     GND from Arduino to PS2 pin 3

   The connector to mate with PS2 keyboard is a 6 pin Female Mini-Din connector
   PS2 Pins to signal
    1       KBD Data
    3       GND
    4       +5V
    5       KBD Clock

   Keyboard has 5V and GND connected see plenty of examples and
   photos around on Arduino site and other sites about the PS2 Connector.

  Interrupts

   Clock pin from PS2 keyboard MUST be connected to an interrupt
   pin, these vary with the different types of Arduino

  Read method Returns an UNSIGNED INT containing
        Make/Break status
        Caps status
        Shift, CTRL, ALT, ALT GR, GUI keys
        Flag for function key not a displayable/printable character
        8 bit key code

  Code Ranges (bottom byte of unsigned int)
        0       invalid/error
        1-1F    Functions (Caps, Shift, ALT, Enter, DEL... )
        1A-1F   Functions with ASCII control code
                    (DEL, BS, TAB, ESC, ENTER, SPACE)
        20-61   Printable characters noting
                    0-9 = 0x30 to 0x39 as ASCII
                    A to Z = 0x41 to 0x5A as upper case ASCII type codes
                    8B Extra European key
        61-A0   Function keys and other special keys (plus F2 and F1)
                    61-78 F1 to F24
                    79-8A Multimedia
                    8B NOT included
                    8C-8E ACPI power
                    91-A0 and F2 and F1 - Special multilingual
        A8-FF   Keyboard communications commands (note F2 and F1 are special
                codes for special multi-lingual keyboards)

    By using these ranges it is possible to perform detection of any key and do
    easy translation to ASCII/UTF-8 avoiding keys that do not have a valid code.

    Top Byte is 8 bits denoting as follows with defines for bit code

        Define name bit     description
        PS2_BREAK   15      1 = Break key code
                   (MSB)    0 = Make Key code
        PS2_SHIFT   14      1 = Shift key pressed as well (either side)
                            0 = NO shift key
        PS2_CTRL    13      1 = Ctrl key pressed as well (either side)
                            0 = NO Ctrl key
        PS2_CAPS    12      1 = Caps Lock ON
                            0 = Caps lock OFF
        PS2_ALT     11      1 = Left Alt key pressed as well
                            0 = NO Left Alt key
        PS2_ALT_GR  10      1 = Right Alt (Alt GR) key pressed as well
                            0 = NO Right Alt key
        PS2_GUI      9      1 = GUI key pressed as well (either)
                            0 = NO GUI key
        PS2_FUNCTION 8      1 = FUNCTION key non-printable character (plus space, tab, enter)
                            0 = standard character key

  Error Codes
     Most functions return 0 or 0xFFFF as error, other codes to note and
     handle appropriately
        0xAA   keyboard has reset and passed power up tests
               will happen if keyboard plugged in after code start
        0xFC   Keyboard General error or power up fail
  https://github.com/twinearthsoftware/ArduinoSketchUploader
  https://github.com/dbuezas/arduino-web-uploader

*/

#include <PS2KeyAdvanced.h>
#include <digitalWriteFast.h>
#include <EEPROM.h>

//#define QWERTY
#define AZERTY

//#define CLEAR_JOY_BANK


//pinout
#define DATAPIN 4 //ps2 data
#define IRQPIN  3 //ps2 clk
#define KEY_EN 2 //autorisation d'appuie touche
#define COL_A 5  //entrées de sélection de colonne portD
#define COL_B 6
#define COL_C 7


#define MUX_A 8  // sorties de consigne colonne portB
#define MUX_B 9
#define MUX_C 10

#define CDE_ALIM 13  //sortie relais d'alimentation
#define POWER_SW 12  //entrée bp de mise en marche
#define RESET 11  //sortie pour faire un reset du 6502 ou NMI

#define ROM_TYPE A5 //sortie selection de la rom oric1 ou atmos


//proto sur atmos

#define PIN_HAUT A0  //joystick
#define PIN_BAS A1
#define PIN_GAUCHE A2
#define PIN_DROITE A3
#define PIN_FEU A4


//oric f4goh github
/*
  #define PIN_HAUT A3  //joystick
  #define PIN_BAS A2
  #define PIN_GAUCHE A1
  #define PIN_DROITE A4
  #define PIN_FEU A0
*/
//constantes
#define EXTERNAL_KBD_RESET_TIMEOUT 3000

#define TIME_PRESS 50
#define DEBOUNCE 2000  //debounce power up

#define EEP_ROM 0 //Adresse numéro de ROM dans l'EEPROM
#define EEP_JOY_BANK_SAVE 1 //Adresse JOY dans l'EEPROM
#define EEP_JOY_BASE 5 //Adresse JOY dans l'EEPROM
#define EEP_NB_VAL_JOY 5 //5 positions du joystic
#define EEP_NB_BANK_MAX 50 //nombre de bank Joy max =(256-1)/5=51


// shift+col, ctrl+line
//                        Sp     !    "    #      $      %    &    '     (     )       *    +     ,     -     .   /
byte oricAsciiCode[] = {0x04, 0xD0, 0xF3, 0xF0, 0xB2, 0xA0, 0x80, 0x73, 0x93, 0xA7, 0x87, 0xF7, 0x14, 0x33, 0x24, 0x37,
                        //0    1      2     3    4      5     6      7    8    9     :       ;    <    =      >    ?    @
                        0x27, 0x50, 0x62, 0x70, 0x32, 0x20, 0x12, 0x00, 0x07, 0x13, 0xA3, 0x23, 0x94, 0x77, 0xA4, 0xB7, 0xE2,
                        //A     B    C      D     E     F    G      H     I    J      K    L     M      N    O      P    Q      R     S     T    U     V     W     X      Y     Z
                        0x56, 0x22, 0x72, 0x71, 0x36, 0x31, 0x26, 0x16, 0x15, 0x01, 0x03, 0x17, 0x02, 0x10, 0x25, 0x35, 0x61, 0x21, 0x66, 0x11, 0x05, 0x30, 0x76, 0x60, 0x06, 0x52,
                        //[    \     ]   ^    _    `
                        0x75, 0x63, 0x65, 0x92, 0xff, 0xff,
                        //a      b     c   d       e     f    g    h      i    j      k    l     m      n    o      p    q    r       s    t      u    v      w    x     y      z
                        //0xD6, 0xA2, 0xF2, 0xF1, 0xB6, 0xB1, 0xA6, 0x96, 0x95, 0x81, 0x83, 0x97, 0x82, 0x90, 0xA5, 0xB5, 0xE1, 0xA1, 0xE6, 0x91, 0x85, 0xB0, 0xF6, 0xE0, 0x86, 0xD2,
                        0x56, 0x22, 0x72, 0x71, 0x36, 0x31, 0x26, 0x16, 0x15, 0x01, 0x03, 0x17, 0x02, 0x10, 0x25, 0x35, 0x61, 0x21, 0x66, 0x11, 0x05, 0x30, 0x76, 0x60, 0x06, 0x52,
                        //{      |    }
                        0xF5, 0xE3, 0xE5
                       };
//                    ESC  DEL   FCT    SFTG SFTD   RET  CTRL  HAUT  BAS  GAUCHE DROITE SPACE
byte oricRedKey[] = {0x51, 0x55, 0x45, 0x44, 0x47, 0x57, 0x42, 0x34, 0x64, 0x54, 0x74, 0x04};

typedef enum {
  HAUT = 7,
  BAS,
  GAUCHE,
  DROITE,
  FEU
} curseur;

typedef struct {
  byte haut;
  byte bas;
  byte gauche;
  byte droite;
  byte feu;
} joyKey;


//gestion alim
byte bpPower = 0;
byte powerState = 0;
unsigned long tempPrec = 0;

//gestion clavier
volatile bool enable = false;  //validation du comparateur des lignes
volatile byte code;            //code colonne,ligne a scanner

//gestion joystick avec 50 bank mémoire
volatile joyKey jk;
bool joyRecordFlag = false;
byte adrBank;

PS2KeyAdvanced keyboard;


void setup( )
{

  Serial.begin( 115200 );
  Serial.println( "PS2 key for Oric-1 & Atmos Original ROM" );

  pinMode(CDE_ALIM, OUTPUT);
  pinMode(POWER_SW, INPUT_PULLUP);
  pinMode(RESET, OUTPUT);
  pinMode(ROM_TYPE, OUTPUT);

  pinMode(MUX_A, OUTPUT);
  pinMode(MUX_B, OUTPUT);
  pinMode(MUX_C, OUTPUT);
  pinMode(KEY_EN, OUTPUT);
  pinMode(COL_A, INPUT);
  pinMode(COL_B, INPUT);
  pinMode(COL_C, INPUT);

  pinMode(PIN_HAUT, INPUT_PULLUP);
  pinMode(PIN_BAS, INPUT_PULLUP);
  pinMode(PIN_GAUCHE, INPUT_PULLUP);
  pinMode(PIN_DROITE, INPUT_PULLUP);
  pinMode(PIN_FEU, INPUT_PULLUP);


  digitalWrite(CDE_ALIM, LOW);
  digitalWrite(RESET, LOW);
  //digitalWrite(ROM_TYPE, LOW); //oric-1
  //digitalWrite(ROM_TYPE, HIGH); //atmos
  digitalWrite(KEY_EN, LOW);


  //init ROM
  if (EEPROM.read(EEP_ROM) == 0xff) {
    EEPROM.write(EEP_ROM, 0);
  }
  byte rom = EEPROM.read(EEP_ROM);
  digitalWrite(ROM_TYPE, rom);
  if (rom) {
    Serial.println("ROM ORIC ATMOS");
  }
  else {
    Serial.println("ROM ORIC-1");
  }


  //init Joy
#ifdef CLEAR_JOY_BANK
  EEPROM.write(EEP_JOY_BANK_SAVE, 0xff);  //force l'init
#endif
  if (EEPROM.read(EEP_JOY_BANK_SAVE) == 0xff) {
    EEPROM.write(EEP_JOY_BANK_SAVE, EEP_JOY_BASE);
    int ptr = EEP_JOY_BASE;
    for (int i = 0; i < EEP_NB_BANK_MAX; i++) {
      EEPROM.write(ptr, oricRedKey[HAUT]);
      EEPROM.write(ptr + 1, oricRedKey[BAS]);
      EEPROM.write(ptr + 2, oricRedKey[GAUCHE]);
      EEPROM.write(ptr + 3, oricRedKey[DROITE]);
      EEPROM.write(ptr + 4, oricRedKey[FEU]);
      ptr += EEP_NB_VAL_JOY;
    }
  }
  adrBank = EEPROM.read(EEP_JOY_BANK_SAVE);
  jk.haut = EEPROM.read(adrBank);
  jk.bas = EEPROM.read(adrBank + 1);
  jk.gauche = EEPROM.read(adrBank + 2);
  jk.droite = EEPROM.read(adrBank + 3);
  jk.feu = EEPROM.read(adrBank + 4);


  keyboard.begin( DATAPIN, IRQPIN );

  unsigned long start = millis();
  keyboard.resetKey();

  bool keyOk = false;

  while ((millis() - start) < EXTERNAL_KBD_RESET_TIMEOUT && keyOk == false) { //a refaire
    if (keyboard.available()) {
      switch (keyboard.read()) {
        case 0xaa:
          keyOk = true;
          keyboard.setLock(PS2_LOCK_NUM);
          keyboard.setNoRepeat(1);
          break;
      }
    }
    delay(250);
  }
  if (keyOk == true) {
    Serial.println("keyboard ok");
  }
  else {
    Serial.println("no keyboard");
  }

  //    keyboard.setNoBreak( 1 );         // No break codes for keys (when key released)
  //  keyboard.setNoRepeat( 1 );        // Don't repeat shift ctrl etc

}


void touche(byte val, byte cde) {
  byte col, line;
  col = (cde & 0x70) >> 4;
  line = (cde & 0x07);
  if (val == col) {                   //droite 7   xenon inv 6 <-> 7
    PORTB = (PORTB & 0x38) | line;
    digitalWriteFast(KEY_EN, HIGH);
  }
  else {
    digitalWriteFast(KEY_EN, LOW);
  }
}


void arrowGame() {
  byte val, droite, gauche, haut, bas, feu;
  droite = digitalReadFast(PIN_DROITE);
  gauche = digitalReadFast(PIN_GAUCHE);
  haut = digitalReadFast(PIN_HAUT);
  bas = digitalReadFast(PIN_BAS);
  feu = digitalReadFast(PIN_FEU);
  val = (PIND >> 5);
  if (droite == 0) {
    touche(val, jk.droite);               //0x24
  }
  if (gauche == 0) {            // 0x14
    touche(val, jk.gauche);
  }
  if (haut == 0) {          //0x56
    touche(val, jk.haut);
  }
  if (bas == 0) {       //0x52
    touche(val, jk.bas);
  }
  if (feu == 0) {
    touche(val, jk.feu); //0x04
  }
  if (droite & gauche & haut & bas & feu) {
    digitalWriteFast(KEY_EN, LOW);
  }
}



void powerCtrl2()
{

  bpPower = digitalRead(POWER_SW);
  //filter out any noise by setting a time buffer
  if ( (millis() - tempPrec) > DEBOUNCE) {
    //if the button has been pressed, lets toggle the POWER from "off to on" or "on to off"
    if ( (bpPower == LOW) && (powerState == 0) ) {
      digitalWrite(CDE_ALIM, HIGH); //turn LED on
      powerState = 1; //now the LED is on, we need to change the state
      tempPrec = millis(); //set the current time
    }
    else if ( (bpPower == LOW) && (powerState == 1) ) {
      digitalWrite(CDE_ALIM, LOW); //turn LED off
      powerState = 0; //now the LED is off, we need to change the state
      tempPrec = millis(); //set the current time
    }

  }
}


void onlyCar(byte code) {
  byte val, col, line;
  col = (code & 0x70) >> 4;
  line = (code & 0x07);
  long start = millis();
  do {
    val = (PIND >> 5);
    if (val == col) {
      PORTB = (PORTB & 0x38) | line;
      digitalWrite(KEY_EN, HIGH);
    }
    else {
      digitalWrite(KEY_EN, LOW);
    }
  }
  while (millis() - start < TIME_PRESS);
  digitalWrite(KEY_EN, LOW);
}


//pour plus tard
void shortPress(byte code) {
  byte val, col, line, shift, ctrl;
  shift = code >> 7;
  ctrl = (code >> 3) & 1;
  col = (code & 0x70) >> 4;
  line = (code & 0x07);
  Serial.println(ctrl);
  Serial.println(shift);
  Serial.println(col);
  Serial.println(line);
  long start = millis();
  do {
    val = (PIND >> 5);
    if (val == col) {
      PORTB = (PORTB & 0x38) | line;
      digitalWrite(KEY_EN, HIGH);
    }
    else if (val == 4)  {
      if (shift == 1) {
        PORTB = (PORTB & 0x38) | 4;
        digitalWrite(KEY_EN, HIGH);
      }
      if (ctrl == 1) {
        PORTB = (PORTB & 0x38) | 2;
        digitalWrite(KEY_EN, HIGH);
      }
    }
    else {
      digitalWrite(KEY_EN, LOW);
    }
  }
  while (millis() - start < TIME_PRESS);
  digitalWrite(KEY_EN, LOW);
  delay(TIME_PRESS);
  //Serial.println("*");
}


void releaseKey() {
  uint16_t c;
  bool relache = false;
  while (relache == false) {
    if ( keyboard.available( ) )
    {
      // read the next key
      c = keyboard.read( );
      //Serial.print("---> ");
      //Serial.println(c & PS2_BREAK, HEX );
      if (c & PS2_BREAK) {
        relache = true;
        enable = false;
        //Serial.print("ok");
      }
    }
  }
}


//---------------------------------------------------------------------QWERTY
#ifdef QWERTY
void touchesSimples(byte key) {
  bool notSupported = false;
  bool noDouble = false;
  if ((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9')) { //problème avec le caps lock et le shift
    code = oricAsciiCode[key - ' '];
  }
  else {
    //touches directes
    switch (key) {
      case PS2_KEY_COMMA:   code = oricAsciiCode[',' - ' '];
        break;
      case PS2_KEY_DOT:   code = oricAsciiCode['.' - ' '];
        break;
      case PS2_KEY_DIV:   code = oricAsciiCode['/' - ' '];
        break;
      case PS2_KEY_SEMI:   code = oricAsciiCode[';' - ' '];
        break;
      case PS2_KEY_APOS:   code = oricAsciiCode['\'' - ' '];
        break;
      case PS2_KEY_OPEN_SQ :   code = oricAsciiCode['[' - ' '];
        break;
      case PS2_KEY_CLOSE_SQ :   code = oricAsciiCode[']' - ' '];
        break;
      case PS2_KEY_MINUS:   code = oricAsciiCode['-' - ' '];
        break;
      case PS2_KEY_EQUAL :   code = oricAsciiCode['=' - ' '];
        break;
      case PS2_KEY_EUROPE2:    printCmd("POKE #", 0);
        noDouble = true;
      case PS2_KEY_BACK:   code = oricAsciiCode['\\' - ' '];
        break;
      //pave numérique
      case PS2_KEY_KP_DOT:   code = oricAsciiCode['.' - ' '];
        break;
      case PS2_KEY_KP_ENTER:  code = oricRedKey[5];
        break;
      case PS2_KEY_KP_PLUS:   code = oricAsciiCode['+' - ' '];
        notSupported = true;
        break;
      case PS2_KEY_KP_MINUS:   code = oricAsciiCode['-' - ' '];
        break;
      case PS2_KEY_KP_TIMES:   code = oricAsciiCode['*' - ' '];
        notSupported = true;
        break;
      case PS2_KEY_KP_DIV:   code = oricAsciiCode['/' - ' '];
        break;
      case PS2_KEY_KP0:   code = oricAsciiCode['0' - ' '];
        break;
      case PS2_KEY_KP1:   code = oricAsciiCode['1' - ' '];
        break;
      case PS2_KEY_KP2:   code = oricAsciiCode['2' - ' '];
        break;
      case PS2_KEY_KP3:   code = oricAsciiCode['3' - ' '];
        break;
      case PS2_KEY_KP4:   code = oricAsciiCode['4' - ' '];
        break;
      case PS2_KEY_KP5:   code = oricAsciiCode['5' - ' '];
        break;
      case PS2_KEY_KP6:   code = oricAsciiCode['6' - ' '];
        break;
      case PS2_KEY_KP7:   code = oricAsciiCode['7' - ' '];
        break;
      case PS2_KEY_KP8:   code = oricAsciiCode['8' - ' '];
        break;
      case PS2_KEY_KP9:   code = oricAsciiCode['9' - ' '];
        break;

    }
  }
  if (!noDouble) {
    if (notSupported == true) {
      shortPress(code);
    } else
    {
      onlyCar(code);
    }
  }
}
#endif

//---------------------------------------------------------------------AZERTY
#ifdef AZERTY
void touchesSimples(byte key) {
  byte notSupported = false;
  if (key >= 'A' && key <= 'Z') { //problème avec le caps lock et le shift
    switch (key) {
      case 'A' :  code = oricAsciiCode['Q' - ' '];
        break;
      case 'Q' :  code = oricAsciiCode['A' - ' '];
        break;
      case 'Z' :  code = oricAsciiCode['W' - ' '];
        break;
      case 'W' :  code = oricAsciiCode['Z' - ' '];
        break;
      case 'M' :  code = oricAsciiCode[',' - ' '];
        break;
      default:
        code = oricAsciiCode[key - ' '];
        break;
    }
  }
  else {
    //touches directes
    switch (key) {
      case PS2_KEY_COMMA:   code = oricAsciiCode[';' - ' '];
        break;
      case PS2_KEY_DOT:   code = oricAsciiCode[':' - ' '];
        notSupported = true;
        break;
      case PS2_KEY_DIV:   code = oricAsciiCode['!' - ' '];
        notSupported = true;
        break;
      case PS2_KEY_SEMI:   code = oricAsciiCode['M' - ' '];
        break;
      case PS2_KEY_APOS:   code = 0x40;
        break;
      case PS2_KEY_OPEN_SQ :   code = oricAsciiCode['^' - ' '];
        notSupported = true;
        break;
      case PS2_KEY_CLOSE_SQ :   code = oricAsciiCode['$' - ' '];
        notSupported = true;
        break;
      case PS2_KEY_MINUS:   code = oricAsciiCode[')' - ' '];
        notSupported = true;
        break;
      case PS2_KEY_EQUAL :   code = oricAsciiCode['=' - ' '];
        break;
      case PS2_KEY_EUROPE2:   code = oricAsciiCode['<' - ' '];
        notSupported = true;
        break;
      case PS2_KEY_BACK:   code = oricAsciiCode['*' - ' '];
        notSupported = true;
        break;

      case PS2_KEY_1:   code = oricAsciiCode['&' - ' '];
        notSupported = true;
        break;
      case PS2_KEY_2:   code = 0x40;
        break;
      case PS2_KEY_3:   code = oricAsciiCode['"' - ' '];
        notSupported = true;
        break;
      case PS2_KEY_4:   code = oricAsciiCode['\'' - ' '];
        break;
      case PS2_KEY_5:   code = oricAsciiCode['(' - ' '];
        notSupported = true;
        break;
      case PS2_KEY_6:   code = oricAsciiCode['-' - ' '];
        break;
      case PS2_KEY_7:   code = 0x40;
        break;
      case PS2_KEY_8:   code = 0x40;
        break;
      case PS2_KEY_9:   code = 0x40;
        break;
      case PS2_KEY_0:   code = 0x40;
        break;

      //pave numérique

      case PS2_KEY_KP_DOT:   code = oricAsciiCode['.' - ' '];
        break;
      case PS2_KEY_KP_ENTER:  code = oricRedKey[5];
        break;
      case PS2_KEY_KP_PLUS:   code = oricAsciiCode['+' - ' '];
        notSupported = true;
        break;
      case PS2_KEY_KP_MINUS:   code = oricAsciiCode['-' - ' '];
        break;
      case PS2_KEY_KP_TIMES:   code = oricAsciiCode['*' - ' '];
        notSupported = true;
        break;
      case PS2_KEY_KP_DIV:   code = oricAsciiCode['/' - ' '];
        break;
      case PS2_KEY_KP0:   code = oricAsciiCode['0' - ' '];
        break;
      case PS2_KEY_KP1:   code = oricAsciiCode['1' - ' '];
        break;
      case PS2_KEY_KP2:   code = oricAsciiCode['2' - ' '];
        break;
      case PS2_KEY_KP3:   code = oricAsciiCode['3' - ' '];
        break;
      case PS2_KEY_KP4:   code = oricAsciiCode['4' - ' '];
        break;
      case PS2_KEY_KP5:   code = oricAsciiCode['5' - ' '];
        break;
      case PS2_KEY_KP6:   code = oricAsciiCode['6' - ' '];
        break;
      case PS2_KEY_KP7:   code = oricAsciiCode['7' - ' '];
        break;
      case PS2_KEY_KP8:   code = oricAsciiCode['8' - ' '];
        break;
      case PS2_KEY_KP9:   code = oricAsciiCode['9' - ' '];
        break;
    }
  }
  if (notSupported == true) {
    shortPress(code);
  } else
  {
    onlyCar(code);
  }
}
#endif


void printCmd(char * chaine, bool CrLf)
{
  int i = 0;
  while (chaine[i] != '\0') {
    //Serial.println(chaine[i]);
    if (chaine[i] == 'Z' || chaine[i] == 'z') {
      code = oricAsciiCode[chaine[i] - ' '] | 0x80; //force le shift
    }
    else {
      code = oricAsciiCode[chaine[i] - ' '];
    }
    shortPress(code);
    i++;
  }
  if (CrLf) {
    code = oricRedKey[5]; //enter
    shortPress(code);
  }
}


void readEpromJoy() {
  Serial.println(adrBank / 5);
  jk.haut = EEPROM.read(adrBank);
  jk.bas = EEPROM.read(adrBank + 1);
  jk.gauche = EEPROM.read(adrBank + 2);
  jk.droite = EEPROM.read(adrBank + 3);
  jk.feu = EEPROM.read(adrBank + 4);
  char s[10];
  sprintf(s, "Bk %d ", adrBank / 5);
  printCmd(s, 0);
}


void traitementSpecial(byte key) {
  switch (key) {
    case PS2_KEY_F1: printCmd("ZAP", 1);
      break;
    case PS2_KEY_F2: printCmd("EXPLODE", 1);
      break;
    case PS2_KEY_F3: printCmd("SHOOT", 1);
      break;
    case PS2_KEY_F4: printCmd("PING", 1);
      break;
    case PS2_KEY_F5: printCmd("CLOAD\"", 1);
      break;
    case PS2_KEY_F6: printCmd("CSAVE\"", 1);
      break;
    case PS2_KEY_F7: // F7   start record flag ou flip flop
      Serial.println("Enregistrement joystic");
      joyRecordFlag = true;
      printCmd("REC JOY ", 0);
      break;
    case PS2_KEY_F8:  // F8  stop record flag
      Serial.println("Fin enregistrement joystic");
      EEPROM.update(EEP_JOY_BANK_SAVE, adrBank);
      joyRecordFlag = false;
      printCmd("EXIT JOY ", 0);
      break;
    case PS2_KEY_F9:
      Serial.print("Dec Bank N°");
      adrBank -= 5;
      if (adrBank == 0) adrBank = EEP_JOY_BASE;
      readEpromJoy();
      break;
    case PS2_KEY_F10:
      Serial.print("Inc Bank N° ");
      adrBank += 5;
      if (adrBank == 255) adrBank = EEP_JOY_BASE;
      readEpromJoy();
      break;
    case PS2_KEY_F11:
      Serial.print("Inc x10 Bank N° ");
      adrBank += 50;
      if (adrBank >= 0 && adrBank < EEP_JOY_BASE) adrBank = EEP_JOY_BASE;
      readEpromJoy();
      break;
    case PS2_KEY_F12:        //F12     permute la rom entre oric-1 et atmos
      byte rom = EEPROM.read(EEP_ROM);
      rom = rom ^ 1;
      EEPROM.write(EEP_ROM, rom);
      Serial.println("ROM RESET");
      if (rom) {
        Serial.println("ROM ORIC ATMOS");
      }
      else {
        Serial.println("ROM ORIC-1");
      }
      digitalWriteFast(RESET, HIGH);
      delay(100);
      digitalWriteFast(ROM_TYPE, rom);
      digitalWriteFast(RESET, LOW);
      break;
    //avec touche FN (pas tous les claviers)
    case PS2_KEY_SLEEP:
      break;
    case PS2_KEY_MUTE:
      break;
    case PS2_KEY_VOL_DN:
      break;
    case PS2_KEY_VOL_UP:   //Fn F9
      break;
  }
}

void touchesFonctions(byte key) {
  bool notSupported = false;
  switch (key) {
    case PS2_KEY_ENTER:   code = oricRedKey[5];
      break;
    case PS2_KEY_UP_ARROW:   code = oricRedKey[7];
      break;
    case PS2_KEY_DN_ARROW:   code = oricRedKey[8];
      break;
    case PS2_KEY_L_ARROW:   code = oricRedKey[9];
      break;
    case PS2_KEY_R_ARROW:   code = oricRedKey[10];
      break;


    //not supported
    case PS2_KEY_HOME:  notSupported = true;
      break;
    case PS2_KEY_END:   notSupported = true;
      break;
    case PS2_KEY_PGUP:  notSupported = true;
      break;
    case PS2_KEY_PGDN:  notSupported = true;
      break;
    case PS2_KEY_INSERT: notSupported = true;
      break;
    case PS2_KEY_DELETE: notSupported = true;
      break;
    case PS2_KEY_TAB: notSupported = true;
      break;

    case PS2_KEY_ESC:    code = oricRedKey[0];
      break;
    case PS2_KEY_BS:    code = oricRedKey[1];
      break;
    case PS2_KEY_SPACE:   code = oricAsciiCode[' ' - ' '];
      break;
    case PS2_KEY_L_SHIFT:    code = oricRedKey[3];
      break;
    case PS2_KEY_R_SHIFT:   code = oricRedKey[4];
      break;
    case PS2_KEY_L_CTRL:    code = oricRedKey[6];
      break;
    case PS2_KEY_R_CTRL:   code = oricRedKey[6];
      break;
    case PS2_KEY_L_GUI: code = oricRedKey[2];  //Funct touche windows
      break;
  }
  if (key >= PS2_KEY_F1 && key <= PS2_KEY_WAKE) {
    traitementSpecial(key);
    notSupported = true;
  }
  if (notSupported == false) {
    onlyCar(code);
  }
}

void touchesCTRL(byte key) {
  bool notSupported = true;
  switch (key) {
    case 'A' : code = oricAsciiCode[key - ' '] | 0x8;  //edit
      notSupported = false;
      break;
    case 'C' : code = oricAsciiCode[key - ' '] | 0x8;  //break
      notSupported = false;
      break;
    case 'G' : code = oricAsciiCode[key - ' '] | 0x8;  //ping
      notSupported = false;
      break;
    case 'T' : code = oricAsciiCode[key - ' '] | 0x8;  //caps lock
      notSupported = false;
      break;
    case 'P' : code = oricAsciiCode[key - ' '] | 0x8;  //printer
      notSupported = false;
      break;
    case 'F' : code = oricAsciiCode[key - ' '] | 0x8;  //keyclick
      notSupported = false;
      break;
    case 'D' : code = oricAsciiCode[key - ' '] | 0x8;  //Auto  double  height
      notSupported = false;
      break;
    case 'Q' : code = oricAsciiCode[key - ' '] | 0x8;  //Cursor
      notSupported = false;
      break;
    case 'S' : code = oricAsciiCode[key - ' '] | 0x8;  //V.D.U.
      notSupported = false;
      break;
    case 'I' : code = oricAsciiCode[key - ' '] | 0x8;  //Protected  column  (far  left)
      notSupported = false;
      break;
    case 'J' : code = oricAsciiCode[key - ' '] | 0x8;  // Line  feed
      notSupported = false;
      break;
    case 'L' : code = oricAsciiCode[key - ' '] | 0x8;  //Clear return
      notSupported = false;
      break;
    case 'M' : code = oricAsciiCode[key - ' '] | 0x8;  //Carriage  return
      notSupported = false;
      break;
    case 'N' : code = oricAsciiCode[key - ' '] | 0x8;  //Clear row
      notSupported = false;
      break;

  }
  if (notSupported == false) {
    shortPress(code);
  }
}
//---------------------------------------------------------------------QWERTY
#ifdef QWERTY
void touchesShift(byte key) {
  bool notSupported = false;
  if ((key >= 'A' && key <= 'Z')) { //Passage en majuscules quand caps desactivé
    code = oricAsciiCode[key - ' '] | 0x80; //+Shift pour les caractères
  }
  else {
    //touches directes
    switch (key) {
      case PS2_KEY_1:   code = oricAsciiCode['!' - ' '];
        break;
      case PS2_KEY_2:   code = oricAsciiCode['@' - ' '];
        break;
      case PS2_KEY_3:   code = oricAsciiCode['#' - ' '];
        break;
      case PS2_KEY_4:   code = oricAsciiCode['$' - ' '];
        break;
      case PS2_KEY_5:   code = oricAsciiCode['%' - ' '];
        break;
      case PS2_KEY_6:   code = oricAsciiCode['^' - ' '];
        break;
      case PS2_KEY_7:   code = oricAsciiCode['&' - ' '];
        break;
      case PS2_KEY_8:   code = oricAsciiCode['*' - ' '];
        break;
      case PS2_KEY_9:   code = oricAsciiCode['(' - ' '];
        break;
      case PS2_KEY_0:   code = oricAsciiCode[')' - ' '];
        break;


      case PS2_KEY_COMMA:   code = oricAsciiCode['<' - ' '];
        break;
      case PS2_KEY_DOT:   code = oricAsciiCode['>' - ' '];
        break;
      case PS2_KEY_DIV:   code = oricAsciiCode['?' - ' '];
        break;
      case PS2_KEY_SEMI:   code = oricAsciiCode[':' - ' '];
        break;
      case PS2_KEY_APOS:   code = oricAsciiCode['"' - ' '];
        break;
      case PS2_KEY_OPEN_SQ :   code = oricAsciiCode['{' - ' '];
        break;
      case PS2_KEY_CLOSE_SQ :   code = oricAsciiCode['}' - ' '];
        break;
      case PS2_KEY_MINUS:   code = oricAsciiCode['-' - ' '] | 0x80;
        break;
      case PS2_KEY_EQUAL :   code = oricAsciiCode['+' - ' '];
        break;
      case PS2_KEY_EUROPE2:    printCmd("PEEK #", 0);
        notSupported = true;
        //code = oricAsciiCode['|' - ' '];    //facultatif
        break;
      case PS2_KEY_BACK:   code = oricAsciiCode['|' - ' '];
        break;
    }
  }
  if (!notSupported) {
    shortPress(code);
  }
}
#endif
//---------------------------------------------------------------------AZERTY

#ifdef AZERTY
void touchesShift(byte key) {
  bool notSupported = false;
  if ((key >= 'A' && key <= 'Z')) { //+Shift pour les caractères
    switch (key) {
      case 'A' :  code = oricAsciiCode['Q' - ' '] | 0x80;
        break;
      case 'Q' :  code = oricAsciiCode['A' - ' '] | 0x80;
        break;
      case 'Z' :  code = oricAsciiCode['W' - ' '] | 0x80;
        break;
      case 'W' :  code = oricAsciiCode['Z' - ' '] | 0x80;
        break;
      case 'M' :  code = oricAsciiCode['?' - ' '];
        break;
      default:
        code = oricAsciiCode[key - ' '] | 0x80;
        break;
    }
  }
  else {
    //touches directes
    switch (key) {
      case PS2_KEY_1:   code = oricAsciiCode['1' - ' '];
        break;
      case PS2_KEY_2:   code = oricAsciiCode['2' - ' '];
        break;
      case PS2_KEY_3:   code = oricAsciiCode['3' - ' '];
        break;
      case PS2_KEY_4:   code = oricAsciiCode['4' - ' '];
        break;
      case PS2_KEY_5:   code = oricAsciiCode['5' - ' '];
        break;
      case PS2_KEY_6:   code = oricAsciiCode['6' - ' '];
        break;
      case PS2_KEY_7:   code = oricAsciiCode['7' - ' '];
        break;
      case PS2_KEY_8:   code = oricAsciiCode['8' - ' '];
        break;
      case PS2_KEY_9:   code = oricAsciiCode['9' - ' '];
        break;
      case PS2_KEY_0:   code = oricAsciiCode['0' - ' '];
        break;


      case PS2_KEY_COMMA:   code = oricAsciiCode['.' - ' '];
        break;
      case PS2_KEY_DOT:   code = oricAsciiCode['/' - ' '];
        break;
      case PS2_KEY_DIV:   code = 0x40;
        break;
      case PS2_KEY_SEMI:   code = oricAsciiCode['M' - ' '];
        break;
      case PS2_KEY_APOS:   code = oricAsciiCode['%' - ' '];
        break;
      case PS2_KEY_OPEN_SQ :   printCmd("PEEK #", 0);
        notSupported = true;
        break;
      case PS2_KEY_CLOSE_SQ :   code = oricAsciiCode['-' - ' '] | 0x80;
        break;
      case PS2_KEY_MINUS:   code = oricAsciiCode['-' - ' '];
        break;
      case PS2_KEY_EQUAL :   code = oricAsciiCode['+' - ' '];
        break;
      case PS2_KEY_EUROPE2:   code = oricAsciiCode['>' - ' '];
        break;
      case PS2_KEY_BACK:    printCmd("POKE #", 0);
        notSupported = true;
        //code = oricAsciiCode['|' - ' '];
        break;
    }
  }
  if (!notSupported) {
    shortPress(code);
  }
}
#endif


void touchesAltGr(byte key) {
  //touches directes
  switch (key) {
    case PS2_KEY_1:   code = 0x40;
      break;
    case PS2_KEY_2:   code = 0x40;
      break;
    case PS2_KEY_3:   code = oricAsciiCode['#' - ' '];
      break;
    case PS2_KEY_4:   code = oricAsciiCode['{' - ' '];
      break;
    case PS2_KEY_5:   code = oricAsciiCode['[' - ' '];
      break;
    case PS2_KEY_6:   code = oricAsciiCode['|' - ' '];
      break;
    case PS2_KEY_7:   code = 0x40;
      break;
    case PS2_KEY_8:   code = oricAsciiCode['\\' - ' '];
      break;
    case PS2_KEY_9:   code = oricAsciiCode['^' - ' '];
      break;
    case PS2_KEY_0:   code = oricAsciiCode['@' - ' '];
      break;
    case PS2_KEY_MINUS:   code = oricAsciiCode[')' - ' '];
      break;
    case PS2_KEY_EQUAL :   code = oricAsciiCode['}' - ' '];
      break;
    default: code = 0x40;
  }
  shortPress(code);
}


void processKey()
{
  uint16_t c;
  byte status, key;
  bool notSupported = false;
  if ( keyboard.available( ) )
  {
    // read the next key
    c = keyboard.read( );
    if ( c > 0 )
    {
      status = c >> 8;
      key = c & 0xFF;
      Serial.print("Status Bits ");
      Serial.print( status, HEX );
      Serial.print( " Code " );
      Serial.println( key, HEX );

      if (c == 0x291A) {  //ctrl+alt+suppr
        Serial.println("RESET");            //RESET TOTAL
        digitalWriteFast(RESET, HIGH);
        delay(100);
        digitalWriteFast(RESET, LOW);
      }
      else {
        //spécial capslock
        if (c == 0x011D) {
          code = oricAsciiCode['T' - ' '] | 0x8;      //CTRL+T  mets le bazar conflit avec numlock
          shortPress(code);
        } else {
          switch (status) {
            case 0x0: Serial.println("touches simples");
              if (key != 0xFA) {
                touchesSimples(key);
              }
              break;
            case 0x1: Serial.println("touches fonctions");
              touchesFonctions(key);
              break;
            case 0x20: Serial.println("touche ctrl");
              touchesCTRL(key);
              break;
            case 0x40: Serial.println("touche shift");
              touchesShift(key);
              break;
            case 0x4: Serial.println("touche AltGr"); //pour azerty
              touchesAltGr(key);
              break;

          }
        }
      }
    }
  }
}

void joyRecord() {
  if (joyRecordFlag) {
    byte droite, gauche, haut, bas, feu;
    droite = digitalReadFast(PIN_DROITE);
    gauche = digitalReadFast(PIN_GAUCHE);
    haut = digitalReadFast(PIN_HAUT);
    bas = digitalReadFast(PIN_BAS);
    feu = digitalReadFast(PIN_FEU);
    if (droite == 0) {
      Serial.println("joy droit");  //update EEPROM
      jk.droite = code;
      EEPROM.update(adrBank + 3, jk.droite);
      onlyCar(code);
    }
    if (gauche == 0) {
      Serial.println("joy gauche");
      jk.gauche = code;
      EEPROM.update(adrBank + 2, jk.gauche);
      onlyCar(code);
    }
    if (haut == 0) {
      Serial.println("joy haut");
      jk.haut = code;
      EEPROM.update(adrBank, jk.haut);
      onlyCar(code);
    }
    if (bas == 0) {
      Serial.println("joy bas");
      jk.bas = code;
      EEPROM.update(adrBank + 1, jk.bas);
      onlyCar(code);
    }
    if (feu == 0) {
      Serial.println("joy feu");
      jk.feu = code;
      EEPROM.update(adrBank + 4, jk.feu);
      onlyCar(code);
    }
  }
  else {
    arrowGame();
  }
}



void loop( )
{
  char car;
  String str;
  char *buf;


  powerCtrl2();

  joyRecord();


  if (Serial.available() > 0)
  {
    str = Serial.readStringUntil('\n');
    buf = malloc(str.length() + 1);
    str.toCharArray(buf, str.length() + 1);
    Serial.println(buf);
    printCmd(buf, 1);
    free(buf);
  }
  processKey();

}
