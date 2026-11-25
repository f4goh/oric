#include <Arduino.h>
#include "SPIFFS.h"
#include <WiFi.h>
#include <Preferences.h>
#include <GestionWifi.h>

#include "Oric.h"
#include "TelnetMenu.h"
#include "StandAlone.h"
#include "WebUi.h"

#define WIFI_LOCAL_PIN_DIS 26

Oric        *loric      = nullptr;
GestionWifi *gsWifi     = nullptr;
TelnetMenu  *monTelnet  = nullptr;
StandAlone  *standalone = nullptr;
WebUi       *webUi      = nullptr;
Preferences  configuration;

bool flagWifi = false;
bool isApMode = true;

void setup()
{
    Serial.begin(115200);
    pinMode(WIFI_LOCAL_PIN_DIS, INPUT_PULLUP);

    loric = new Oric();
    loric->begin();

    configuration.begin("oric", false);
    if (!configuration.getBool("config", false)) {
        configuration.putString("ssid", "");
        configuration.putString("pass", "");
        configuration.putBool("serial", true);   /* Pont Telnet <-> ACIA 6551 actif par defaut */
        configuration.putBool("internet", false);
        configuration.putBool("speed", false);
        configuration.putBool("config", true);
    }
    typeSpeed speed = configuration.getBool("speed", false) ? FAST16 : FAST;
    bool useClient  = configuration.getBool("internet", false);
    String ssid     = configuration.getString("ssid", "");
    String pass     = configuration.getString("pass", "");
    configuration.end();

    loric->setSpeed(speed);

    monTelnet  = new TelnetMenu(loric);
    standalone = new StandAlone(loric);
    gsWifi     = new GestionWifi();
    webUi      = new WebUi(loric);

    if (!SPIFFS.begin(true)) {
        Serial.println("Erreur montage/formatage SPIFFS");
    }

    uint8_t wifiCnx = digitalRead(WIFI_LOCAL_PIN_DIS); /* 1 = normal, 0 = forcer AP */

    if (!useClient || wifiCnx == LOW || ssid.length() == 0) {
        Serial.println("ORIC en mode Point d'Acces (ORIC_AP)");
        isApMode = true;
        flagWifi = gsWifi->setup("ORIC_AP", "oricatmos", ACCES_POINT);
    } else {
        Serial.printf("Connexion WiFi Client vers %s...\n", ssid.c_str());
        isApMode = false;
        flagWifi = gsWifi->setup(ssid.c_str(), pass.c_str(), CLIENT);
        if (!flagWifi) {
            Serial.println("Echec connexion WiFi -> Repli automatique en Point d'Acces (ORIC_AP)");
            isApMode = true;
            flagWifi = gsWifi->setup("ORIC_AP", "oricatmos", ACCES_POINT);
        }
    }

    String ipOled = flagWifi ? gsWifi->getIP() : String("StandAlone");
    String ipRaw  = isApMode ? WiFi.softAPIP().toString() : WiFi.localIP().toString();

    standalone->setIpAddress(ipOled);

    if (flagWifi) {
        monTelnet->setup();
        webUi->begin(isApMode, ipRaw);
        loric->setIdleCallback([]() {
            if (monTelnet) monTelnet->run();
            if (webUi)     webUi->run();
        });
    }
}

void loop()
{
    standalone->rotary_loop();

    WebPendingAction act;
    String fn;
    if (webUi && webUi->takePendingAction(act, fn)) {
        if (act == WEB_ACT_CLOAD) {
            loric->sendFile(fn);
            standalone->setIpAddress(flagWifi ? gsWifi->getIP() : String("StandAlone"));
        } else if (act == WEB_ACT_CSAVE) {
            loric->saveFile(fn);
            standalone->setIpAddress(flagWifi ? gsWifi->getIP() : String("StandAlone"));
        }
    }

    if (flagWifi) {
        monTelnet->run();
        if (webUi) {
            webUi->run();
        }
    }
}
