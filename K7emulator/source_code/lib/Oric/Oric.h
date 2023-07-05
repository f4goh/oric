/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Oric.h
 * Author: ale
 *
 * Created on 2 mars 2023, 19:18
 */

#ifndef ORIC_H
#define ORIC_H
#include <Arduino.h>
#include "SPIFFS.h"
#include "ESPTelnet.h"
#include "ImageBmpMono.h"

#define TEMPO 204

#define K7_OUT 19 //k7 out
#define K7_IN 18  ///k7 int
#define K7_REMOTE 5  ///k7 int
#define LED 2 //blue led 

#define TAILLE_BUFFER 48*1024

#define CLIGNOTEMENT 200
#define TIMEOUT_K7 30*1000  //30 secondes


 typedef enum{
     START,
     DEMI_BIT1,
     DEMI_BIT2,
     FIN
 }typeMesure;
class Oric {
public:
    Oric();
    Oric(const Oric& orig);
    virtual ~Oric();
    void begin();
    void sendFile(String fileName);
    bool saveFile(String fileName);    
    void viewFile(String fileName);
    void listFile();
    void removeFile(String fileName);
    void setTelnetHandler(ESPTelnet *p);
    void setTelnetState(bool state);
    void formatSpiffs();
    void tap2Bas(String fileName);
    void conversion(String fileSource, String fileDest);
     
private:
    void emitBit(int bit);
    void emitByte(int val);
    void emitGap();

    void IRAM_ATTR sensePinIsr();
    static void marshallPin();
    static void marshallTimer();
    
    void IRAM_ATTR onTimer();
    void waitIrqTimer();
    
    byte getBit();
    byte getByte(); 
    void synchronize();
    void clignoteLed();
    bool waitRemote(String info);
    
    
    
    void printHex(byte value);
    void printHexAndAscii(byte* buffer, size_t length);
    void printSpaces(int count);
    void printString(String str,bool lf);
    void printInt(int value,bool lf);
    void printChar(char value);
    void crlf();
    
    portMUX_TYPE timerMux;
    
    
    hw_timer_t * timer;    
    volatile bool interruptFlag;

    volatile typeMesure mesure;
    volatile int duree;
    volatile byte bitState;
    int sync_ok;

    byte buffer[TAILLE_BUFFER];
    byte cptLed;
    ESPTelnet *telnet;
    bool telnetState;
    const char *keywords[119]={"END","EDIT","STORE","RECALL","TRON","TROFF","POP","PLOT", // 128-246: BASIC keywords
    
  "PULL","LORES","DOKE","REPEAT","UNTIL","FOR","LLIST","LPRINT","NEXT","DATA",
  "INPUT","DIM","CLS","READ","LET","GOTO","RUN","IF","RESTORE","GOSUB","RETURN",
  "REM","HIMEM","GRAB","RELEASE","TEXT","HIRES","SHOOT","EXPLODE","ZAP","PING",
  "SOUND","MUSIC","PLAY","CURSET","CURMOV","DRAW","CIRCLE","PATTERN","FILL",
  
  "CHAR","PAPER","INK","STOP","ON","WAIT","CLOAD","CSAVE","DEF","POKE","PRINT",
  "CONT","LIST","CLEAR","GET","CALL","!","NEW","TAB(","TO","FN","SPC(","@",
  "AUTO","ELSE","THEN","NOT","STEP","+","-","*","/","^","AND","OR",">","=","<",
  "SGN","INT","ABS","USR","FRE","POS","HEX$","&","SQR","RND","LN","EXP","COS",
  "SIN","TAN","ATN","PEEK","DEEK","LOG","LEN","STR$","VAL","ASC","CHR$","PI",
  "TRUE","FALSE","KEY$","SCRN","POINT","LEFT$","RIGHT$","MID$"}; // 247- : Error messages
  
   ImageBmpMono monImage;

    const byte header[52] = {0x16, 0x16, 0x16, 0x24, 0x00, 0xFF, 0x00, 0xC7, 0x05, 0x11, 0x05, 0x01, 0x00, 0x48, 0x49, 0x52,
        0x4C, 0x4F, 0x41, 0x44, 0x00, 0x07, 0x05, 0x0A, 0x00, 0xA2, 0x00, 0x0F, 0x05, 0x14, 0x00, 0xB6,
        0x22, 0x22, 0x00, 0x00, 0x00, 0x55, 0x16, 0x16, 0x16, 0x24, 0x00, 0x00, 0x80, 0x00, 0xBF, 0x3F,
        0xA0, 0x00, 0x00, 0x00};

   static Oric * anchor;

};

#endif /* ORIC_H */

