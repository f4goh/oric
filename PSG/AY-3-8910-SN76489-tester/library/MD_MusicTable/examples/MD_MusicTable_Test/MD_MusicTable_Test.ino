#include "MD_MusicTable.h"

MD_MusicTable T;

void setup(void)
{
  Serial.begin(57600);
  Serial.print("\n[MD_MusicTable Tester]");
  Serial.print("\nSet monitor to end with newline.");
  Serial.print("\nEnter a midi note to search.");
}

int16_t readInput(void)
{
  static int16_t n = 0;
  int16_t r = -1;
  char c;

  while (Serial.available())
  {
    c = Serial.read();

    // check for end of line
    if (c == '\n')
    {
      r = n;
      n = 0;
      break;
    }

    // build up the number
    if (c >= '0' && c <= '9')
      n = (10 * n) + c - '0';
    else
      n = 0;
  }

  return(r);
}

void loop(void)
{
  int16_t idx = readInput();

  if (idx != -1)
  {
    Serial.print(F("\n\nSearching by ID for "));
    Serial.print(idx);

    Serial.print(F(" - "));
    if (!T.findId(idx))
      Serial.print(F("not "));
    Serial.print(F("found."));

    char buf[10];
    int8_t octave = T.getOctave();

    Serial.print(F("\nName: "));   Serial.print(T.getName(buf, sizeof(buf)));
    Serial.print(F("\nFreq: "));   Serial.print((uint16_t)(T.getFrequency() + 0.5));  // round it up
    Serial.print(F("\nId: "));     Serial.print(T.getId());
    Serial.print(F("\nNote: "));   Serial.print(T.getNote(buf, sizeof(buf)));
    Serial.print(F("\nOctave: ")); Serial.print(octave);
    Serial.print(F("\n"));

    Serial.print(F("\nSearching by NAME for '"));
    Serial.print(buf);
    Serial.print("'");

    Serial.print(F(" - "));
    if (!T.findName(buf, octave))
      Serial.print(F("not "));
    Serial.print(F("found."));

    Serial.print(F("\nName: "));   Serial.print(T.getName(buf, sizeof(buf)));
    Serial.print(F("\nFreq: "));   Serial.print((uint16_t)(T.getFrequency() + 0.5));  // round it up
    Serial.print(F("\nId: "));     Serial.print(T.getId());
    Serial.print(F("\nNote: "));   Serial.print(T.getNote(buf, sizeof(buf)));
    Serial.print(F("\nOctave: ")); Serial.print(T.getOctave());
    Serial.print(F("\n"));
  }
}