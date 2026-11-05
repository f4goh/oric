/*
 10 A=0
 20 LPRINT A;
 30 LPRINT A+2;
 50 A=A+1
 60 IF A=256 THEN A=0
 70 GOTO 20
*/


#include <Arduino.h>

// Définition des broches de contrôle (Signaux Centronics)
#define STROBE_PIN  2   // PD2 (INT0) - Entrée
#define ACK_PIN     4   // PD4 - Sortie

// Les broches de données (D0 à D7) sont documentées ci-dessous pour le câblage :
// D0 -> Broche 8  (PB0)
// D1 -> Broche 9  (PB1)
// D2 -> Broche 10 (PB2)
// D3 -> Broche 11 (PB3)
// D4 -> Broche 12 (PB4)
// D5 -> Broche 13 (PB5)
// D6 -> Broche 6  (PD6)
// D7 -> Broche 7  (PD7)

// Variable volatile car modifiée dans l'interruption
volatile uint8_t receivedByte = 0;
volatile bool dataReady = false;

// Routine d'interruption (ISR) sur front descendant de STROBE (INT0)
void instanceStrobeISR() {
  // Lecture instantanée des ports pour reconstituer l'octet
  uint8_t portB_bits = PINB; // Lit PB0 à PB5 (Broches 8 à 13)
  uint8_t portD_bits = PIND; // Lit PD6 et PD7 (Broches 6 et 7)

  // Reconstruction du mot de 8 bits :
  // Les 6 bits de poids faible (D0-D5) viennent du bas de PORTB (masque 0x3F)
  // Les 2 bits de poids fort (D6-D7) viennent du haut de PORTD (D6 sur bit 6, D7 sur bit 7)
  receivedByte = (portB_bits & 0x3F) | (portD_bits & 0xC0);
  
  dataReady = true;

  // Envoi de l'impulsion négative sur ACK (Actif à l'état bas)
  // Une impulsion ultra-rapide en écriture de registre pour respecter les timings Centronics
  PORTD &= ~(1 << PD4); // ACK passe à 0
  delayMicroseconds(5); // Durée de l'impulsion ACK (généralement ~5µs)
  PORTD |= (1 << PD4);  // ACK repasse à 1
}

void setup() {
  // Initialisation de la liaison série à 115200 bauds
  Serial.begin(115200);

  // Configuration des lignes de données en ENTRÉE
  // Broches 8 à 13 (PB0 à PB5)
  DDRB &= ~0x3F; 
  // Broches 6 et 7 (PD6 et PD7)
  DDRD &= ~0xC0; 

  // Configuration des lignes de contrôle
  pinMode(STROBE_PIN, INPUT_PULLUP); // STROBE en entrée (PD2)
  pinMode(ACK_PIN, OUTPUT);                        // ACK en sortie (PD4)

  // État initial de ACK : Repos à l'état HAUT (Signal inversé)
  digitalWrite(ACK_PIN, HIGH);

  // Attachement de l'interruption INT0 (Broche 2) sur front descendant (FALLING)
  attachInterrupt(digitalPinToInterrupt(STROBE_PIN), instanceStrobeISR, FALLING);
}

void loop() {
  // Le traitement lourd (envoi série) est fait dans le loop pour ne pas bloquer l'interruption
  if (dataReady) {
    Serial.write(receivedByte); 
    dataReady = false;
  }
}
