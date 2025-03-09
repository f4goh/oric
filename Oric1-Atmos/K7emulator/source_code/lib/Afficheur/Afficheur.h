/* 
 * File:   Afficheur.cpp
 * Author: F4GOH
 * 
 * spécialisation de SSD1306Wire
 */

#ifndef AFFICHEUR_H
#define AFFICHEUR_H

#include <Arduino.h>
#include <SSD1306Wire.h>
#include <SPIFFS.h>
#include <vector>


#define MaxFiles 100

   const uint8_t okSymbol[] PROGMEM = {
    B01111110,
    B10000001,
    B01111110,
    B00000000,
    B11111111,
    B00011000,
    B00100100,
    B11000011
};


enum modeTap { CLOAD, CSAVE };


class Afficheur : public SSD1306Wire
{
public:
    
    Afficheur();
    virtual ~Afficheur();
    
    void afficher(String action,String fileName);
    void afficherAlphabet(char caractereInverse,String fileName);
    
    void updatelisteFiles();
    void select(String ipAddress);
    void afficherFichiersOLED(int index, int sel);
    modeTap getMode();
    int getMaxFiles();
    String getFileName(int index);
    
    
private:
    std::vector<String> fileNames;
    modeTap mode;
    
};

#endif /* AFFICHEUR_H */
