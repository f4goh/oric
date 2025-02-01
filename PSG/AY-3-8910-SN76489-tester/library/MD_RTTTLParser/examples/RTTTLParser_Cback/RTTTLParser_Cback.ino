// 
// Example sketch for the MD_RTTTLParser library 
//
// Implements library based management of the parsing/playing process with callbacks 
// for sound output.
// RTTTL output is played using the Arduino Tone library for minimal hardware requirements.
//
// Dependencies
// MD_MusicTable available at https://github.com/MajocDesigns/MD_MusicTable
//

#include <Tone.h>
#include <MD_MusicTable.h>
#include <MD_RTTTLParser.h>
#include "RTTTLParser_Data.h"

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

// Tone output pin
const uint8_t PIN_TONE = 3;

//Global objects
Tone Player;
MD_MusicTable T;
MD_RTTTLParser Tune;

void RTTTLhandler(uint8_t octave, uint8_t noteId, uint32_t duration, bool activate)
// If activate is true, play a note (octave, noteId) for the duration (ms) specified.
// If activate is false, the note should be turned off (other parameters are ignored).
//
// A deactivate call always follow an activate. If the music output can work with duration, 
// the handler needs to ignore the deactivate callback.
{
  if (activate)
  {
    if (T.findNoteOctave(noteId, octave))
      Player.play(T.getFrequency(), duration);
  }
  else
    Player.stop();
}

void setup(void)
{
  Serial.begin(57600);

  Player.begin(PIN_TONE);

  Tune.begin();
  Tune.setCallback(RTTTLhandler);
}

void loop(void)
{
  const uint16_t PAUSE_TIME = 5000;  // pause time between melodies

  static enum { START, PLAYING, WAIT_BETWEEN } state = START; // current state
  static uint32_t timeStart = 0;    // millis() timing marker
  static uint8_t idxTable = 0;      // index of next song to play
  
  // Manage feeding songs to the RTTTL library
  switch (state)
  {
  case START: // starting a new melody
    {
      // set to start of song and start playing
      Tune.setTune_P(songTable[idxTable]);
      Serial.print(F("\n"));
      Serial.print(Tune.getTitle());
      Serial.print(F("  "));
      Serial.print(Tune.getTimeToEnd());
      Serial.print(F(" ms"));

      // set up for next song
      idxTable++;
      if (idxTable == ARRAY_SIZE(songTable))
        idxTable = 0;

      state = PLAYING;
    }
    break;

  case PLAYING:     // playing the melody
    if (Tune.run())
    {
      timeStart = millis();
      state = WAIT_BETWEEN;   // could not get a note, end playing
    }
    break;

  case WAIT_BETWEEN:  // wait at the end of a melody
    if (millis() - timeStart >= PAUSE_TIME)
      state = START;     // start the next melody
    break;
  }
}
