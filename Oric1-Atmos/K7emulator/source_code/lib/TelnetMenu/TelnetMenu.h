/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TelnetMenu.h
 * Author: ale
 *
 * Created on 27 juin 2023, 18:36
 */

#ifndef TELNETMENU_H
#define TELNETMENU_H

#include <Arduino.h>
#include <Preferences.h>        // Classe pour écrire et lire la configuration
#include <HardwareSerial.h>
#include "ESPTelnet.h"
#include "Oric.h"
#include "SPIFFS.h"
#include "Menu.h"

#define PORT 23

#define RXD2 GPIO_NUM_16
#define TXD2 GPIO_NUM_17

#define SERIAL_HW_NUMBER 2  //attention a cette valeur 1 ou 2 suivant la liaison série
#define SERIAL_SPEED        9600


class TelnetMenu {
public:
    TelnetMenu(Oric *p);
    TelnetMenu(const TelnetMenu& orig);
    virtual ~TelnetMenu();
    void run();
    void setup();
    void checkSerialOption();
    

    static void errorMsg(String error, bool restart = true);
    static void onTelnetConnect(String ip);
    static void onTelnetDisconnect(String ip);
    static void onTelnetReconnect(String ip);
    static void onTelnetConnectionAttempt(String ip);
    static void onTelnetInput(String str);

    
private:
    ESPTelnet *telnet;
    Preferences *configuration;
    Oric *loric;
    HardwareSerial *serial;
    static TelnetMenu* anchor;
    bool serialEnable;
};

#endif /* TELNETMENU_H */

