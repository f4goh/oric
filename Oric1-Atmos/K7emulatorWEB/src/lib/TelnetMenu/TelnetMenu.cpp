/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TelnetMenu.cpp
 * Author: ale
 * 
 * Created on 27 juin 2023, 18:36
 */

#include "TelnetMenu.h"

TelnetMenu::TelnetMenu(Oric *p) :
telnet(new ESPTelnet()),
configuration(new Preferences()),
loric(p) {
    serial = new HardwareSerial(SERIAL_HW_NUMBER);
    anchor = this;
    anchor->loric->setTelnetHandler(telnet);
}

TelnetMenu::TelnetMenu(const TelnetMenu& orig) {
}

TelnetMenu::~TelnetMenu() {
}

void TelnetMenu::setup() {
    serial->begin(SERIAL_SPEED, SERIAL_8N1, RXD2, TXD2);
    anchor->telnet->onConnect(onTelnetConnect);
    anchor->telnet->onConnectionAttempt(onTelnetConnectionAttempt);
    anchor->telnet->onReconnect(onTelnetReconnect);
    anchor->telnet->onDisconnect(onTelnetDisconnect);
    anchor->telnet->onInputReceived(onTelnetInput);
    Serial.print("- Telnet: ");
    if (anchor->telnet->begin(PORT)) {
        Serial.println("running");
    } else {
        Serial.println("error.");
        errorMsg("Will reboot...");
    }
    anchor->loric->setTelnetState(false);
    serialEnable = false;
}

void TelnetMenu::run() {
    anchor->telnet->loop();
    if (anchor->serialEnable == true) {             //redirection serie->telnet
        if (anchor->serial->available()>0){
            anchor->telnet->print((char)anchor->serial->read());
        }
    }    
}

void TelnetMenu::checkSerialOption() {
    configuration->begin("oric", false);
    serialEnable = anchor->configuration->getBool("serial");
    configuration->end();
    if (serialEnable == false) {
        loric->setTelnetState(true);
        telnet->println("\nWelcome " + anchor->telnet->getIP());
        telnet->print("\nOric> "); //ne pas envoyer en mode sérial !!!
    }
    else{
        loric->setTelnetState(false);
        anchor->serial->write('C');   //quand on se connecte C envoyé vers la liaison série de l'Oric
    }
}

void TelnetMenu::errorMsg(String error, bool restart) {
    Serial.println(error);
    if (restart) {
        Serial.println("Rebooting now...");
        delay(2000);
        ESP.restart();
        delay(2000);
    }
}
/* ------------------------------------------------- */

// (optional) callback functions for telnet events

void TelnetMenu::onTelnetConnect(String ip) {
    Serial.print("- Telnet: ");
    Serial.print(ip);
    Serial.println(" connected");   
    anchor->checkSerialOption();
    // anchor->telnet.println("(Use ^] + q  to disconnect.)");
}

void TelnetMenu::onTelnetDisconnect(String ip) {
    Serial.print("- Telnet: ");
    Serial.print(ip);
    Serial.println(" disconnected");
    anchor->loric->setTelnetState(false);
}

void TelnetMenu::onTelnetReconnect(String ip) {
    Serial.print("- Telnet: ");
    Serial.print(ip);
    Serial.println(" reconnected");
    anchor->checkSerialOption();
}

void TelnetMenu::onTelnetConnectionAttempt(String ip) {
    Serial.print("- Telnet: ");
    Serial.print(ip);
    Serial.println(" tried to connected");
    anchor->loric->setTelnetState(false);
}

void TelnetMenu::onTelnetInput(String str) {
    // Check for a certain command
    if (anchor->serialEnable == false) {
        //Serial.println(str);
        //String filename = str.substring(3);
        //Serial.println(filename);
        if (str == "help") {
            anchor->telnet->println(F("Available telnet commands"));
            anchor->telnet->println(F("ls                 : list tap files in SPIFFS"));
            anchor->telnet->println(F("free               : SPIFFS info space size"));
            anchor->telnet->println(F("rm filename.tap    : remove tap files in SPIFFS"));
            anchor->telnet->println(F("cload filename.tap : load fsk from esp32 to oric"));
            anchor->telnet->println(F("csave filename.tap : save fsk from oric to esp32"));
            anchor->telnet->println(F("view filename.tap  : view file content in HEX/ASCII format"));
            anchor->telnet->println(F("basic filename.tap : view file content in BASIC"));
            anchor->telnet->println(F("help               : this menu"));
            anchor->telnet->println(F("bye                : exit telnet"));
            anchor->telnet->println(F(VERSION));
        } else if (str == "bye") {
            anchor->telnet->println("> disconnecting you...");
            anchor->telnet->disconnectClient();
        } else if (str == "ls") {
            anchor->loric->listFile();
        } else if (str.startsWith("basic ")) {
            String filename = str.substring(6);
            anchor->loric->tap2Bas(filename);
        } else if (str.startsWith("rm ")) {
            String filename = str.substring(3);
            anchor->loric->removeFile(filename);
        } else if (str == "free") {
            unsigned int totalBytes = SPIFFS.totalBytes();
            unsigned int usedBytes = SPIFFS.usedBytes();
            anchor->telnet->println();
            anchor->telnet->println("===== Flash File System Info =====");
            anchor->telnet->print("Total space:      ");
            anchor->telnet->print(totalBytes);
            anchor->telnet->println(" octets");
            anchor->telnet->print("Total space used: ");
            anchor->telnet->print(usedBytes);
            anchor->telnet->println(" octets");
            anchor->telnet->println();
        } else if (str.startsWith("cload ")) {
            String filename = str.substring(6);
            anchor->loric->sendFile(filename);
        } else if (str.startsWith("csave ")) {
            String filename = str.substring(6);
            anchor->loric->saveFile(filename);
        } else if (str.startsWith("view ")) {
            String filename = str.substring(5);
            anchor->loric->viewFile(filename);
        } else if (str == ""){
             //anchor->telnet->println();  //nothing
        }else {
            anchor->telnet->println("\nError, see help command");
        }
        anchor->telnet->print("\nOric> ");
    } else {                        //redirection telnet->serie
        anchor->serial->print(str); //n'envoie pas le crlf !! a rajouter si nécéssaire
    }
}


TelnetMenu* TelnetMenu::anchor = NULL;