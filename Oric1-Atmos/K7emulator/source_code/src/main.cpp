/* 
 * File:   main.cpp
 * Author: ale
 *
 * Created on 2 mars 2023, 19:18
 * pio lib install https://github.com/dplasa/FTPClientServer
 * pio lib install https://github.com/lennarthennigs/espTelnet
 * pio lib install akoro/Console@^1.2.1
 * https://github.com/MaffooClock/ESP32RotaryEncoder
 */
#include <Arduino.h>
#include "SPIFFS.h"
#include <WiFi.h>
#include <FTPServer.h>
#include <Preferences.h>
#include <Menu.h>               // Classe commandes via la console
#include <GestionWifi.h>

#include "Oric.h"
#include "TelnetMenu.h"

#include "StandAlone.h"

#define WIFI_LOCAL_PIN_DIS 26
//const char *ssid = "SFR_6C4F";
//const char *password = "x19p24bunnjzc8vqnx9n";

FTPServer ftpSrv(SPIFFS);

Menu *leMenu;
Oric *loric; 
GestionWifi *gsWifi;
TelnetMenu *monTelnet;
Preferences configuration;

StandAlone *standalone;



bool flagWifi=false;

void erreur(String msg);


void erreur(String msg){
    Serial.print("Sartup Error on ");
    Serial.println(msg);
    Serial.println("Check configuration ");    
    while(1){
        leMenu->run();
    }    
}

void setup() {
    //nvs_flash_erase(); // erase the NVS partition and...
    //nvs_flash_init(); // initialize the NVS partition.
    Serial.begin(115200);
    while (!Serial)
        yield(); // yield() Passe le contrôle à d'autres tâches
    
    pinMode(WIFI_LOCAL_PIN_DIS,INPUT_PULLUP);
   

    loric = new Oric();
    loric->begin();

    leMenu = new Menu(loric); // Menu de configuration
    leMenu->setup();

    monTelnet = new TelnetMenu(loric);  
    
    standalone = new StandAlone(loric);
        
    gsWifi = new GestionWifi;
     
    
    // Démarre le système de fichier SPIFFS
    if (!SPIFFS.begin()) {
        Serial.println("\n\nFormatting .... wait about 10 sec");
        bool formatted = SPIFFS.format();
        if (formatted) {
            Serial.println("\n\nSuccess formatting");
            if (SPIFFS.begin()) {
                Serial.println("Success mounting SPIFFS");                
            }
            else{
                erreur("\n\nError mounting SPIFFS");     
            }
        } else {
            erreur("\n\nError formatting");            
        }
    }
     
    uint8_t wifiCnx=digitalRead(WIFI_LOCAL_PIN_DIS);  //1 wifi connected, 0 :access point
    
    Serial.println(configuration.getBool("internet"));
    Serial.println(wifiCnx);
    
    
    configuration.begin("oric", false);    
    if (!configuration.getBool("internet") || !wifiCnx) {  //access point, internet disable
        Serial.println("ORIC in Access point");
        flagWifi = gsWifi->setup("ORIC_AP", "oricatmos", ACCES_POINT);
        if (flagWifi) {
            Serial.println("Access point WIFI enable");
        } else erreur("Access point WIFI error");
    } else { //tcp/ip on aprs.is internet enable
        Serial.println("ORIC connected to any local access point"); //local access point, internet enable
        Serial.println(configuration.getString("ssid").c_str());
        Serial.println(configuration.getString("pass").c_str());
        flagWifi = gsWifi->setup(configuration.getString("ssid").c_str(), configuration.getString("pass").c_str(), CLIENT);
        if (flagWifi) {
            Serial.println("Wifi Enable");
        } else {
            erreur("Wifi error");
        }
    }
    configuration.end();   
   //show ip dans le menu
   // Serial.printf_P(PSTR("\nConnected to %s, IP address is %s\n"), ssid, WiFi.localIP().toString().c_str());
    
    if (flagWifi == true) {
        ftpSrv.begin(F("ftp"), F("ftp"));
        monTelnet->setup();     
    }

    standalone->setIpAddress(gsWifi->getIP());
    
      
}


void loop() {
    standalone->rotary_loop();
    leMenu->run();

    if (flagWifi == true) { //pas de wifi console seulement
       ftpSrv.handleFTP();  
       monTelnet->run();
        }    
}

