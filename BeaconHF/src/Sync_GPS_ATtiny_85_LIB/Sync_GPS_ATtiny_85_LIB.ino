//f4goh 7/2025
//attiny85 parse GPS and generate a pulse every even minutes

#include <SoftwareSerial.h>
#include <ATtinyGPS.h>
ATtinyGPS gps;

SoftwareSerial gpsSerial(4, 3); // RX, TX du module GPS

int pulsePin = 0;   // Impulsion chaque minute paire
int rttyPin = 1;    // Signal carré rtty
int squarePin = 2; // Signal carré permanent
int pskPin = 3;    // Signal carré rtty



// ⏱️ Signal carré
unsigned long lastToggleWspr = 0;
const unsigned long squarePeriodWspr = 682+11; // période totale en ms avec erreur compensée

unsigned long lastToggleRtty = 0;
const unsigned long squarePeriodRtty = 22; // période totale en ms

unsigned long lastTogglePsk = 0;
const unsigned long squarePeriodPsk = 32; // période totale en ms

static bool pulseActive = false;
static unsigned long pulseStart = 0;
static int lastPulseMinute = -1;


void setup() {

  pinMode(pulsePin, OUTPUT);
  pinMode(rttyPin, OUTPUT);
  pinMode(squarePin, OUTPUT);
  pinMode(pskPin, OUTPUT);

  digitalWrite(squarePin, LOW);
  digitalWrite(pulsePin, LOW);
  digitalWrite(rttyPin, LOW);
  digitalWrite(pskPin, LOW);

  gpsSerial.begin(9600);

}

void loop() {
  // 🔁 GÉNÉRATION DES SIGNAUX CARRÉS
   
  if (millis() - lastToggleWspr >= squarePeriodWspr / 2) { // 341 ms pour chaque état
    digitalWrite(squarePin, digitalRead(squarePin) ^ 1);
    lastToggleWspr = millis();
  }
  
  if (millis() - lastToggleRtty >= squarePeriodRtty / 2) { // 341 ms pour chaque état
    digitalWrite(rttyPin, digitalRead(rttyPin) ^ 1);
    lastToggleRtty = millis();
  }
  
  if (millis() - lastTogglePsk >= squarePeriodPsk / 2) { // 341 ms pour chaque état
    digitalWrite(pskPin, digitalRead(pskPin) ^ 1);
    lastTogglePsk = millis();
  }


  if (gpsSerial.available())
  {
    char c = gpsSerial.read();
    gps.parse(c);
  }
  if (gps.new_data())
  {
    if ((gps.mm % 2 == 0) && (gps.ss == 0) && (gps.mm != lastPulseMinute)) {
      digitalWrite(pulsePin, HIGH);
      pulseStart = millis();
      pulseActive = true;
      lastPulseMinute = gps.mm;
    }
/*
    if ((gps.mm % 2 == 0) && (gps.ss == 0)) {
      digitalWrite(pulsePin, HIGH);
    }
    if ((gps.mm % 2 == 0) && (gps.ss == 1)) {
      digitalWrite(pulsePin, LOW);
    }
    */

  }
  if (pulseActive && (millis() - pulseStart >= 200)) {
  digitalWrite(pulsePin, LOW);
  pulseActive = false;
}


}
