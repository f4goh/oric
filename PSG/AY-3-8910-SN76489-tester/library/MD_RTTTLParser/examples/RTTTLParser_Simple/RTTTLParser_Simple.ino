// 
// Example sketch for the MD_RTTTLParser library 
//
// Implements library based management of the parsing/playing process with callbacks 
// for sound output in the most bare-bones application to play the same song on repeat.
// RTTTL output is played using the Arduino Tone library for minimal hardware requirements.
//
// Dependencies
// MD_MusicTable available at https://github.com/MajocDesigns/MD_MusicTable
//

#include <Tone.h>
#include <MD_MusicTable.h>
#include <MD_RTTTLParser.h>

// Tone output pin
const uint8_t PIN_TONE = 3;

//Global objects
Tone player;
MD_MusicTable T;
MD_RTTTLParser Tune;

const char song[] = "Flinstones:d=4,o=5,b=40:32p,16f6,16a#,16a#6,32g6,16f6,16a#.,16f6,32d#6,32d6,32d6,32d#6,32f6,16a#,16c6,d6,16f6,16a#.,16a#6,32g6,16f6,16a#.,32f6,32f6,32d#6,32d6,32d6,32d#6,32f6,16a#,16c6,a#,16a6,16d.6,16a#6,32a6,32a6,32g6,32f#6,32a6,8g6,16g6,16c.6,32a6,32a6,32g6,32g6,32f6,32e6,32g6,8f6,16f6,16a#.,16a#6,32g6,16f6,16a#.,16f6,32d#6,32d6,32d6,32d#6,32f6,16a#,16c.6,32d6,32d#6,32f6,16a#,16c.6,32d6,32d#6,32f6,16a#6,16c7,8a#.6";

void RTTTLhandler(uint8_t octave, uint8_t noteId, uint32_t duration, bool activate)
// If activate is true, play a note (octave, noteId) for the duration (ms) specified.
// If activate is false, the note should be turned off (other parameters are ignored).
//
// A deactivate call always follows an activate. If the music output can work with duration, 
// the handler needs to ignore the deactivate callback.
{
  if (activate)
  {
    if (T.findNoteOctave(noteId, octave))
      player.play(T.getFrequency());
  }
  else
    player.stop();
}

void setup(void)
{
  Serial.begin(57600);
  player.begin(PIN_TONE);

  Tune.begin();
  Tune.setCallback(RTTTLhandler);
}

void loop(void)
{
  if (Tune.run())
  {
    delay(2000);
    Tune.setTune(song);
  }    
}
