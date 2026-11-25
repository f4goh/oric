/* 
 * File:   StandAlone.h
 * Author: alecren
 *
 * Created on 2 mars 2025, 21:24
 */

#ifndef STANDALONE_H
#define STANDALONE_H

#include <Arduino.h>
#include "Oric.h"
#include "AiEsp32RotaryEncoder.h"
#include "AiEsp32RotaryEncoderNumberSelector.h"
#include <Afficheur.h>          // Afficheur SSD1306


#define ROTARY_ENCODER_A_PIN 14
#define ROTARY_ENCODER_B_PIN 27
#define ROTARY_ENCODER_BUTTON_PIN 25//4
#define ROTARY_ENCODER_VCC_PIN -1 /* 27 put -1 of Rotary encoder Vcc is connected directly to 3,3V; else you can use declared output pin for powering rotary encoder */
//depending on your encoder - try 1,2 or 4 to get expected behaviour
#define ROTARY_ENCODER_STEPS 4
#define ROTARY_PULLDOWN false

 typedef enum{
     CLOAD_CSAVE_MENU,
     FILE_NAME_MENU,
     FILE_LIST_MENU,
     FILE_CHAR_MENU        
 }stateRotary;
 

class StandAlone {
public:
    StandAlone(Oric *p);
    StandAlone(const StandAlone& orig);
    virtual ~StandAlone();
    void rotary_loop();
    void setIpAddress(String ip);
    
     
private:

    void IRAM_ATTR readEncoderISR();

    static void marshallreadEncoderISR();

    void rotary_onButtonClick();


    //unsigned long shortPressAfterMiliseconds = 50; //how long short press shoud be. Do not set too low to avoid bouncing (false press events).
    //unsigned long longPressAfterMiliseconds = 1000; //how long čong press shoud be.

    Oric *loric;
    AiEsp32RotaryEncoder *rotaryEncoder;
    Afficheur *afficheur;
    stateRotary state;
    AiEsp32RotaryEncoderNumberSelector *numberSelector;
    String saveName;
    String ipAddress;
    static StandAlone * anchor;

};

#endif /* STANDALONE_H */

