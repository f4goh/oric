/* 
 * File:   StandAlone.cpp
 * Author: alecren
 * 
 * Created on 2 mars 2025, 21:24
 */

#include "StandAlone.h"

StandAlone::StandAlone(Oric *p) :
loric(p) {
    anchor = this;
    state=CLOAD_CSAVE_MENU;
    afficheur = new Afficheur;
    rotaryEncoder = new AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS,ROTARY_PULLDOWN);

    numberSelector = new AiEsp32RotaryEncoderNumberSelector();
    numberSelector->attachEncoder(rotaryEncoder);
    rotaryEncoder->begin();
    rotaryEncoder->setup(StandAlone::marshallreadEncoderISR);
    //set boundaries and if values should cycle or not
    //in this example we will set possible values between 0 and 1000;

    rotaryEncoder->setBoundaries(0, 1000, false); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)

    /*Rotary acceleration introduced 25.2.2021.
     * in case range to select is huge, for example - select a value between 0 and 1000 and we want 785
     * without accelerateion you need long time to get to that number
     * Using acceleration, faster you turn, faster will the value raise.
     * For fine tuning slow down.
     */
    rotaryEncoder->disableAcceleration(); //acceleration is now enabled by default - disable if you dont need it
    //rotaryEncoder->setAcceleration(250);  //or set the value - larger number = more accelearation; 0 or 1 means disabled acceleration
}



StandAlone::StandAlone(const StandAlone& orig) {
}

StandAlone::~StandAlone() {
     anchor = NULL;
}

void StandAlone::marshallreadEncoderISR(){
    anchor->readEncoderISR();
}

void IRAM_ATTR StandAlone::readEncoderISR(){
     rotaryEncoder->readEncoder_ISR();
}

void StandAlone::setIpAddress(String ip){
    ipAddress=ip;
    afficheur->select(ipAddress);
}


void StandAlone::rotary_onButtonClick() {
     char car;
    static unsigned long lastTimePressed = 0;
    //ignore multiple press in that time milliseconds
    if (millis() - lastTimePressed < 500) {
        return;
    }
    lastTimePressed = millis();
    //Serial.print("button pressed ");
    //Serial.print(millis());
    //Serial.println(" milliseconds after restart");
    switch (state) {
        //csave ou cload
        case CLOAD_CSAVE_MENU: if (afficheur->getMode() == CLOAD) {
                state = FILE_LIST_MENU;
                afficheur->updatelisteFiles();
                afficheur->afficherFichiersOLED(0, 0);
                numberSelector->setRange(0.0, (float) afficheur->getMaxFiles() - 1, 1, false, 0);
                numberSelector->setValue(0);
            } else {
                state = FILE_NAME_MENU;
                numberSelector->setRange((float) 'A', (float) '[', 1, false, 0);
                numberSelector->setValue('A');
                saveName = "";
                afficheur->afficherAlphabet('A', saveName);
                state = FILE_CHAR_MENU;
            }
            break;
            //compose le nom du fichier a ecrire
        case FILE_CHAR_MENU:
            car = (char) numberSelector->getValue();
            if (car != '[') {
                saveName = saveName + car;
                numberSelector->setValue('A');
                afficheur->afficherAlphabet('A', saveName);
            } else {
                saveName = saveName + ".tap";
                afficheur->afficher("CSAVE", saveName);
               //fin de séquence
                loric->saveFile(saveName);
                state = CLOAD_CSAVE_MENU;
                afficheur->select(ipAddress); 
            }
            break;
            //selectionne le fichier a lire
        case FILE_LIST_MENU:
            String fileName;
            fileName = afficheur->getFileName((int) numberSelector->getValue());
            //Serial.println(fileName);
            afficheur->afficher("CLOAD", fileName);

            //fin de séquence
            loric->sendFile(fileName);
            state = CLOAD_CSAVE_MENU;
            afficheur->select(ipAddress);
            break;
    }

    
}

void StandAlone::rotary_loop() {
    //dont print anything unless value changed
    if (rotaryEncoder->encoderChanged()) {
        //Serial.print("Value: ");
        //Serial.println(rotaryEncoder->readEncoder());
        switch (state) {
            case CLOAD_CSAVE_MENU: afficheur->select(ipAddress);
                break;
            case FILE_LIST_MENU: afficheur->afficherFichiersOLED(numberSelector->getValue(), 0);
                break;
            case  FILE_CHAR_MENU: afficheur->afficherAlphabet(numberSelector->getValue(),saveName);
                break;          
        }
    }
    if (rotaryEncoder->isEncoderButtonClicked()) {
        rotary_onButtonClick();

    }
}


StandAlone* StandAlone::anchor = NULL;
