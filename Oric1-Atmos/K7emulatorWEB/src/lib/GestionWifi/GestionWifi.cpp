/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   GestionWifi.cpp
 * Author: ale
 * 
 * Created on 9 avril 2022, 09:04
 */

#include "GestionWifi.h"
#include <esp_wifi.h>

GestionWifi::GestionWifi() {
     anchor = this;
}

GestionWifi::GestionWifi(const GestionWifi& orig) {
}

GestionWifi::~GestionWifi() {
}

bool GestionWifi::setup(const char* _ssid, const char* _password, wifiMode _wMode) {
    ssid = strdup(_ssid);
    password = strdup(_password);
    wMode = _wMode;
    bool retCnx;
    if (wMode == CLIENT) {
        Serial.println("Client Wifi");
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        esp_wifi_set_ps(WIFI_PS_NONE);
        esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G);
        WiFi.setTxPower(WIFI_POWER_11dBm);
        WiFi.setAutoReconnect(true);
        WiFi.disconnect(false);
        delay(100);
        WiFi.onEvent(GestionWifi::WiFiEvent);
        WiFi.begin(ssid, password);
        retCnx = connexion();
        if (retCnx) {
            WiFi.setSleep(false);
            esp_wifi_set_ps(WIFI_PS_NONE);
            wifi_ps_type_t ps;
            esp_wifi_get_ps(&ps);
            Serial.printf("WiFi PS mode: %d (0=NONE)\n", (int)ps);
            WiFiClient testCli;
            bool gwOk = testCli.connect(WiFi.gatewayIP(), 80, 1500);
            Serial.printf("Test Gateway %s:80 -> %s\n", WiFi.gatewayIP().toString().c_str(), gwOk ? "OK" : "FAIL");
            testCli.stop();
        }
    } else {
        Serial.println("Access point Wifi");
        WiFi.mode(WIFI_AP);
        WiFi.setSleep(false);
        esp_wifi_set_ps(WIFI_PS_NONE);
        WiFi.setTxPower(WIFI_POWER_11dBm);
        retCnx = WiFi.softAP(ssid, password);
        if (retCnx) {
            Serial.print("[+] AP Created with IP Gateway ");
            Serial.println(WiFi.softAPIP());
            Serial.println("\n[*] Creating AP");
        }
    }
    return retCnx;
}
/*
 * WiFi Events
0  SYSTEM_EVENT_WIFI_READY               < ESP32 WiFi ready
1  SYSTEM_EVENT_SCAN_DONE                < ESP32 finish scanning AP
2  SYSTEM_EVENT_STA_START                < ESP32 station start
3  SYSTEM_EVENT_STA_STOP                 < ESP32 station stop
4  SYSTEM_EVENT_STA_CONNECTED            < ESP32 station connected to AP
5  SYSTEM_EVENT_STA_DISCONNECTED         < ESP32 station disconnected from AP
6  SYSTEM_EVENT_STA_AUTHMODE_CHANGE      < the auth mode of AP connected by ESP32 station changed
7  SYSTEM_EVENT_STA_GOT_IP               < ESP32 station got IP from connected AP
8  SYSTEM_EVENT_STA_LOST_IP              < ESP32 station lost IP and the IP is reset to 0
9  SYSTEM_EVENT_STA_WPS_ER_SUCCESS       < ESP32 station wps succeeds in enrollee mode
10 SYSTEM_EVENT_STA_WPS_ER_FAILED        < ESP32 station wps fails in enrollee mode
11 SYSTEM_EVENT_STA_WPS_ER_TIMEOUT       < ESP32 station wps timeout in enrollee mode
12 SYSTEM_EVENT_STA_WPS_ER_PIN           < ESP32 station wps pin code in enrollee mode
13 SYSTEM_EVENT_AP_START                 < ESP32 soft-AP start
14 SYSTEM_EVENT_AP_STOP                  < ESP32 soft-AP stop
15 SYSTEM_EVENT_AP_STACONNECTED          < a station connected to ESP32 soft-AP
16 SYSTEM_EVENT_AP_STADISCONNECTED       < a station disconnected from ESP32 soft-AP
17 SYSTEM_EVENT_AP_STAIPASSIGNED         < ESP32 soft-AP assign an IP to a connected station
18 SYSTEM_EVENT_AP_PROBEREQRECVED        < Receive probe request packet in soft-AP interface
19 SYSTEM_EVENT_GOT_IP6                  < ESP32 station or ap or ethernet interface v6IP addr is preferred
20 SYSTEM_EVENT_ETH_START                < ESP32 ethernet start
21 SYSTEM_EVENT_ETH_STOP                 < ESP32 ethernet stop
22 SYSTEM_EVENT_ETH_CONNECTED            < ESP32 ethernet phy link up
23 SYSTEM_EVENT_ETH_DISCONNECTED         < ESP32 ethernet phy link down
24 SYSTEM_EVENT_ETH_GOT_IP               < ESP32 ethernet got IP from connected AP
25 SYSTEM_EVENT_MAX
 */


void GestionWifi::WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
    (void)info;
    switch (event) {
        case ARDUINO_EVENT_WIFI_READY:
            Serial.println("WiFi interface ready");
            break;
        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.println("WiFi client started");
            WiFi.setHostname("oric-k7");
            break;
        case ARDUINO_EVENT_WIFI_STA_STOP:
            Serial.println("WiFi clients stopped");
            anchor->wifi_connected = false;
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("Connected to access point");
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("Disconnected from WiFi access point");
            anchor->wifi_connected = false;
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            WiFi.setSleep(false);
            esp_wifi_set_ps(WIFI_PS_NONE);
            Serial.print("Obtained IP address: ");
            Serial.println(WiFi.localIP());
            Serial.printf("RSSI: %ld dBm, Canal: %ld\n", (long)WiFi.RSSI(), (long)WiFi.channel());
            anchor->wifi_connected = true;
            break;
        case ARDUINO_EVENT_WIFI_AP_START:
            Serial.println("WiFi access point started");
            break;
        case ARDUINO_EVENT_WIFI_AP_STOP:
            Serial.println("WiFi access point stopped");
            break;
        default: break;
    }
}


bool GestionWifi::connected(){
    return (WiFi.status() == WL_CONNECTED);
}

String GestionWifi::getIP() {
    if (wMode == CLIENT) {
        return "C:"+WiFi.localIP().toString();
    } else {
        return "A:"+WiFi.softAPIP().toString();
        
    }
}

// est-ce nécéssaire ?

bool GestionWifi::connexion() {
    unsigned long startTime = millis();
    bool ret=false;

    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < TIMEOUT)
    {
        delay(500);
        Serial.print(".");
        //pas de reset en boucle
        /*
        if ((millis() - startTime) >= TIMEOUT) {
            Serial.println("Resetting due to Wifi not connecting...");
            ESP.restart();
        }
         */
    }

    if (WiFi.status() == WL_CONNECTED) {
        //while (!MDNS.begin(HOSTNAME)) {
        //    Serial.println("Starting mDNS...");
        //    delay(1000);
        //}
        //Serial.println("MDNS started");
        ip = WiFi.localIP();
        subnet = WiFi.subnetMask();
        gateway = WiFi.gatewayIP();
        Serial.println();
        Serial.println("WiFi connecté");
        Serial.print("Adresse IP : ");
        Serial.println(ip);
        Serial.print("Adresse IP de la gateway : ");
        Serial.println(gateway);
        ret = true;
    }
    return ret;
}
/*

void GestionWifi::marshall(void * parametres) {
    anchor->checkWifiTask();
}

void GestionWifi::checkWifiTask() {
    Serial.println("Tache Wifi en fonctionnement");
    while (1) {
        vTaskDelay(1);
        //cnxState = false;
        if (WiFi.status() != WL_CONNECTED) {            
            Serial.println("Reconnecting to WiFi...");
            WiFi.reconnect();
            connexion();
        }
    }
}
*/

GestionWifi* GestionWifi::anchor = NULL;


