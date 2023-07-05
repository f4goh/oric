/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Oric.cpp
 * Author: ale
 * 
 * Created on 2 mars 2023, 19:18 
 * time measurement csave
 * 207 206 209 209 206 208 208 417 208 415 208 211 205 208 209 415 208 208 209 416 208 
 * 415 208 418 207 415 209 208 207 209 209 206 208 208 416 209 415 208 209 208 209 206 415 
 * 209 207 208 418 208 414 209 415 209 415 209 208 208 208 206 208 209 207 416 208 417 207 

 */

#include "Oric.h"

Oric::Oric()
{
    anchor = this;
    timer=NULL;
    telnetState=false;
}

Oric::Oric(const Oric& orig) {
}

Oric::~Oric() {
}

void Oric::begin(){
    pinMode(K7_OUT, OUTPUT);
    pinMode(LED, OUTPUT);
    pinMode(K7_IN, INPUT);
    pinMode(K7_REMOTE, INPUT);    
    interruptFlag = false;
    timerMux = portMUX_INITIALIZER_UNLOCKED;
    timer = timerBegin(0, 80, true);
    //timerAttachInterrupt(timer, Oric::marshallTimer, true);
    timerAlarmWrite(timer, TEMPO, true);   // 1s périodique
    timerAlarmEnable(timer);
    timerStart(timer);
    cptLed=0;
    //attachInterrupt(K7_IN, Oric::marshall, CHANGE);  //activé dans la sauvegarde
}

void Oric::marshallPin() {
    anchor->sensePinIsr();
}

void Oric::marshallTimer() {
    anchor->onTimer();
}


void IRAM_ATTR Oric::sensePinIsr() {    
    clignoteLed();
    switch (mesure) {
        case START: mesure = DEMI_BIT1;
            break;
        case DEMI_BIT1: duree = timerRead(timer);
            mesure = DEMI_BIT2; //manque la sortie FIN
            break;
        case DEMI_BIT2: duree += timerRead(timer);
            mesure = DEMI_BIT1;
            if (duree > 400 && duree < 450) {
                bitState = 1;
            } else if (duree > 600 && duree < 650) {
                bitState = 0;
            } else {
                bitState = -1;
            }
            interruptFlag = true;
            break;
    }
    timerWrite(timer, 0);    
}



void Oric::clignoteLed(){
    cptLed++;
    if (cptLed==CLIGNOTEMENT){
        digitalWrite(LED, digitalRead(LED) ^ 1);
    cptLed=0;
    } 
}

byte Oric::getBit() {
    while (interruptFlag == false) {
        if (timerRead(timer) > 2000) { //timeout fin de fichier : voir utiliser le commutateur relais
            sync_ok = 0;
            bitState = -1;
            interruptFlag = true;
        }
    }
    interruptFlag = false;
    return bitState;
}

byte Oric::getByte() {
    int decaleur = 0, byte = 0, i, bit, sum = 0;
    getBit();
    while ((bit = getBit()) == 1);
    if (bit == -1) {
        sync_ok = 0;
        return 0;
    }
    for (i = 0; i < 8; i++) decaleur = (decaleur << 1) | getBit();
    for (i = 0; i < 8; i++) {
        bit = decaleur & 1;
        decaleur = decaleur >> 1;
        byte = (byte << 1) | bit;
        sum += bit;
    }
    if (((sum & 1) == getBit()) && sync_ok) {
       Serial.println("parity error "); //offset
    }
    return byte;
}

void Oric::synchronize() {
    int val;
    int decaleur = 0;
    while (1) {
        while ((decaleur & 0xff) != 0x68) {
            decaleur = (decaleur << 1) | getBit();
        }
        if (getByte() != 0x16) continue;
        if (getByte() != 0x16) continue;
        if (getByte() != 0x16) continue;
        do {
            val = getByte();
        } while (val == 0x16);
        if (val == 0x24) break;
    }
    Serial.println("Synchro found, decoding bytes...\n");
    //printString("Synchro found, decoding bytes...\n",1); //telnet trop lent
}

bool Oric::saveFile(String fileName) {
    byte octet;
    int tailleBufUtilise;
    int ptr = 0;
    bool ret=waitRemote("Wait CSAVE\"");
    if (ret==false){
        return false;
    }    
    memset(buffer, 0, TAILLE_BUFFER);
    mesure = START;
    duree = 0;
    timerWrite(timer, 0);
    attachInterrupt(K7_IN, Oric::marshallPin, CHANGE);  //a vérifier à l'usage
    interruptFlag = false;
    synchronize();
    sync_ok = 1;
    buffer[ptr++] = 0x16;
    buffer[ptr++] = 0x16;
    buffer[ptr++] = 0x16;
    octet = 0x24;
    while (sync_ok) {
        //Serial.println(octet, HEX);  //debug
        buffer[ptr++] = octet;
        octet = getByte();
    }
    detachInterrupt(K7_IN);
    tailleBufUtilise = ptr - 1;
    printString("taille du fichier : ",0);
    printInt(tailleBufUtilise,1);
    fileName = "/" + fileName;
    File file = SPIFFS.open(fileName, "w");
    if (!file) {
        printString("erreur d'ouverture",1);
        return false;
    }
    size_t octetEcrit;
    octetEcrit = file.write(buffer, tailleBufUtilise); //idem buffer,size
    if (octetEcrit != tailleBufUtilise) {
        printString("erreur d'ecriture",1);
        return false;
    }
    file.close();
    return true;
}

bool Oric::waitRemote(String info){
      printString(info,1);
      long start=millis();
      while ((digitalRead(K7_REMOTE)==0) && (millis()-start<TIMEOUT_K7)){
      };
      if (digitalRead(K7_REMOTE)==1){
          return true;          
      }else{
          printString("Time out",1);
          return false;          
      }
}

void IRAM_ATTR Oric::onTimer() {
    portENTER_CRITICAL_ISR(&timerMux);  // Début de section critique
    interruptFlag=1;
    portEXIT_CRITICAL_ISR(&timerMux);   // Fin de section critique    
}

void Oric::waitIrqTimer() {
    while (interruptFlag == 0) {

    }
    portENTER_CRITICAL(&timerMux);
    interruptFlag = 0;
    portEXIT_CRITICAL(&timerMux);
}


void Oric::emitBit(int bit) {
    if (bit) {
        digitalWrite(K7_OUT, HIGH);
        waitIrqTimer();
        digitalWrite(K7_OUT, LOW);
        waitIrqTimer();
    } else {
        digitalWrite(K7_OUT, HIGH);
        waitIrqTimer();
        digitalWrite(K7_OUT, LOW);
        waitIrqTimer();
        waitIrqTimer();
    }
    clignoteLed();
}


// without irq
/*
void Oric::emitBit(int bit) {
    if (bit) {
        digitalWrite(K7_OUT, HIGH);
        delayMicroseconds(TEMPO);
        digitalWrite(K7_OUT, LOW);
        delayMicroseconds(TEMPO);
    } else {
        digitalWrite(K7_OUT, HIGH);
        delayMicroseconds(TEMPO);
        digitalWrite(K7_OUT, LOW);
        delayMicroseconds(TEMPO * 2);
    }
    clignoteLed();
}
 */

void Oric::emitByte(int val) {
    int i, parity = 1;
    emitBit(0);
    for (i = 0; i < 8; i++, val >>= 1) {
        parity += val & 1;
        emitBit(val & 1);
    }
    emitBit(parity & 1);
    emitBit(1);
    emitBit(1);
    emitBit(1);    
    // emitBit(1);
}

void Oric::emitGap() {
    int i;
    /* un paquet de bits stop pour laisser le temps d'afficher la ligne de statut */
    for (i = 0; i < 100; i++) emitBit(1);
}

void Oric::sendFile(String fileName) {
    int ptr, i, size, fileSize, cpt;
    byte header[9];
    fileName = "/" + fileName;
    File file = SPIFFS.open(fileName, "r");
    if (!file) {
        // File not found | le fichier de test n'existe pas
        printString("Failed to open ",0);
        printString(fileName,1);
        return;
    }
    memset(buffer, 0, TAILLE_BUFFER);
    fileSize = file.size();
    //Serial.print(fileSize);
    //buffer = (byte *) malloc(fileSize * (sizeof (byte))); //utiliser le buffer fixe
    file.read(buffer,fileSize);
    /*
    ptr = 0;
    while (file.available()) {
        buffer[ptr++] = file.read();
    }    
    */
    file.close();
    bool ret=waitRemote("Attente CLOAD\"");
    if (ret==false){
        return;
    }  
    // Envoie le contenu du fichier index.html sur le port série
    crlf();
    printString("Tap txing...Wait",1);
    ptr = 0;
    timerAttachInterrupt(timer, Oric::marshallTimer, true);
    while (ptr < fileSize) {
        while (buffer[ptr++] == 0x16) {
        }
        Serial.println("Sending sync bytes");
        //printString("Sending sync bytes",1);
        for (i = 0; i < 256; i++) emitByte(0x16);
        emitByte(0x24);
        //printString("Sending header",1);
        Serial.println("Sending header");
        for (i = 0; i < 9; i++) emitByte(header[i] = buffer[ptr++]);
        do {
            i = buffer[ptr++];
            emitByte(i);
            if (i != 0) {
                Serial.print((char) i);
            }
        } while (i != 0);
        size = (header[4]*256 + header[5])-(header[6]*256 + header[7]) + 1;
        //printString("\n\rSize: ",0);
        //printInt(size,1);
        Serial.print("\n\rSize: ");
        Serial.println(size);
        emitGap();
        cpt = 0;
        for (i = 0; i < size; i++) {
            emitByte(buffer[ptr++]);
            Serial.print(i);
            Serial.print("\r");
            //printInt(i,0);
            //printString("\r",0);

            // cpt++;
            // Serial.print('-');
            // if (cpt==32){
            //     Serial.println();
            //     cpt=0;
            // }

        }
        //printString("\n\rdone\n",1);
    }
    printString("\nfinished\n",1);
    timerDetachInterrupt(timer);
}

/*
 code from OSDK bas2tap.cpp
 */

void Oric::tap2Bas(String fileName) {
    unsigned int ptr, car;
    byte header[9];
    fileName = "/" + fileName;
    File file = SPIFFS.open(fileName, "r");
    if (!file) {
        // File not found | le fichier de test n'existe pas
        printString("Failed to open ", 0);
        printString(fileName, 1);
        return;
    }
    memset(buffer, 0, TAILLE_BUFFER);    
    ptr = 0;
    while (file.available()) {
        buffer[ptr++] = file.read();
    }
    file.close();
    if (buffer[0] != 0x16 || buffer[3] != 0x24) {
        printString("Not an Oric file",1);
        return;
    }
    if (buffer[6]) {
        printString("Not a BASIC file",1);
        return;
    }
    ptr = 13;
    while (buffer[ptr++]);    
    while (buffer[ptr] || buffer[ptr + 1]) {
        ptr += 2;
        //fprintf(out," %u ",ptr_buffer[i]+(ptr_buffer[i+1]<<8));        
        printString(" ",0);
        printInt(buffer[ptr]+(buffer[ptr + 1] << 8),0);
        printString(" ",0);
        
        ptr += 2;
        while ((car = buffer[ptr++])) {
            if (car < 128)
                //  fputc(car,out);
                printChar((char)car);
            else
                if (car < 247) {
                //fprintf(out,"%s", keywords[car - 128]);
                printString(keywords[car - 128],0);
            } else {
                // Probably corrupted listing
                // 247 : NEXT WITHOUT FOR
                //fprintf(out,"CORRUPTED_ERROR_CODE_%u", car);
                printString("CORRUPTED_ERROR_CODE",1);
            }
        }
        // fputc('\r',out);
        // fputc('\n', out);
        crlf();
    }    
}

void Oric::conversion(String fileSource, String fileDest) {
    Serial.println("conversion");
    if (monImage.loadImage(fileSource)) {
        uint8_t pixel, buffer;
        fileDest = "/" + fileDest;
        File file = SPIFFS.open(fileDest, "w");
        if (!file) {
            printString("erreur d'ouverture", 1);
        } else {
            size_t octetEcrit;
            octetEcrit = file.write(header, sizeof (header)); //idem buffer,size
            if (octetEcrit != sizeof (header)) {
                printString("erreur d'ecriture", 1);

            } else {
                for (int y = 0; y < 200; y++) {
                    for (int x = 0; x < 240; x += 6) {
                        buffer = 0;
                        for (int i = 0; i < 6; i++) {
                            pixel = monImage.getPixel(x + i, y);
                            buffer |= (pixel << (5 - i));
                        }
                        buffer |= 0x40; //masque pour l'oric
                        //Serial.print(buffer,HEX);
                        //Serial.print(" ");
                        file.write(buffer);

                    }
                    //Serial.println();
                }
            }
            file.close();
        }
    }
}

void Oric::printHex(byte value) {
    if (value < 0x10) {
        printString("0",0);
    }
    char buf[5];
    sprintf(buf,"%X",value);
    //Serial.print(value, HEX);
    printString(buf,0);
}

void Oric::printHexAndAscii(byte* buffer, size_t length) {
    for (size_t i = 0; i < length; i++) {
        printHex(buffer[i]);
        printString(" ",0);
    }

    // Ajouter de l'espace pour l'alignement si nécessaire
    if (length < 16) {
        size_t padding = 16 - length;
        for (size_t i = 0; i < padding; i++) {
            printString("   ",0);
        }
    }

    printString("   ",0);

    // Afficher la représentation ASCII des caractères
    for (size_t i = 0; i < length; i++) {
        if (buffer[i] >= 0x20 && buffer[i] <= 0x7E) {
            //Serial.write(buffer[i]);
            printChar(buffer[i]);
        } else {
            printString(".",0);
        }
    }
}

void Oric::viewFile(String fileName) {
    fileName = "/" + fileName;
    File file = SPIFFS.open(fileName, "r");
    if (!file) {
        printString("Erreur lors de l'ouverture du fichier",1);
        return;
    }
    byte buffer[16];
    size_t bytesRead = 0;

    while ((bytesRead = file.read(buffer, sizeof (buffer))) > 0) {
        printHexAndAscii(buffer, bytesRead);
        crlf();
    }

    file.close();
}

void Oric::listFile(){
     File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file) {
        printString(file.name(),0);
        printSpaces(23-strlen(file.name()));
        printInt(file.size(),1);
        file = root.openNextFile();
    }
}

void Oric::removeFile(String fileName) {
    fileName = "/" + fileName;
    if (SPIFFS.exists(fileName)){
        SPIFFS.remove(fileName);
        printString("File removed",1);
    }
    else
    {
        printString("Unknown file",1);
    }
}

void Oric::printSpaces(int count) {
    for (int i = 0; i < count; i++) {
        printString(" ",0);
    }
}

//pour redirection vers telnet et console série

void Oric::printString(String str, bool lf) {
    if (lf) {
        Serial.println(str);
    } else {
        Serial.print(str);
    }
    if (telnetState == true) {
        if (lf) {
            telnet->println(str);
        } else {
            telnet->print(str);
        }
    }

}

void Oric::printInt(int value, bool lf) {
    if (lf) {
        Serial.println(value);
    } else {
        Serial.print(value);
    }
    if (telnetState == true) {
        if (lf) {
            telnet->println(value);
        } else {
            telnet->print(value);
        }
    }
}

void Oric::crlf() {
    Serial.println();
    if (telnetState == true) {
        telnet->println();
    }
}

void Oric::printChar(char value) {
    Serial.write(value);
    if (telnetState == true) {
        telnet->print(value);
    }
}

void Oric::setTelnetHandler(ESPTelnet *p){
 telnet=p;   
}

void Oric::setTelnetState(bool state){
 telnetState=state;   
}

void Oric::formatSpiffs() {
    printString("\n\nFormatting .... wait about 10 sec",1);
    bool formatted = SPIFFS.format();
    if (formatted) {
        printString("\nSuccess formatting",1);
    } else {
        printString("\nError formatting",1);
    }
}

Oric* Oric::anchor = NULL;
