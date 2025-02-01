// 
// Example sketch for the MD_RTTTLParser library 
//
// Implements sketch based management of the parsing/playing process.
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
Tone player;
MD_MusicTable T;
MD_RTTTLParser Tune;

void processRTTTL(void)
{
  // Note on/off FSM variables
  const uint16_t PAUSE_TIME = 5000;  // pause time between melodies

  static enum { NEXT, START, PLAYING, PAUSE, WAIT_BETWEEN } state = START; // current state
  static uint32_t timeStart = 0;    // millis() timing marker
  static uint8_t octave;            // next octave note to play
  static int8_t noteId;             // index number of the note to play
  static uint16_t playDuration;     // note duration
  static uint8_t idxTable = 0;      // index of next song to play

  // Manage reading and playing the note
  switch (state)
  {
  case NEXT:      // set up for next song
    idxTable++;
    if (idxTable == ARRAY_SIZE(songTable))
      idxTable = 0;
    state = START;
    break;

  case START: // starting a new melody
    {
      // now reset to start of song and start playing
      Tune.setTune_P(songTable[idxTable]);
      Serial.print("\n");
      Serial.print(Tune.getTitle());
      state = PLAYING;
    }
    break;

  case PLAYING:     // playing a melody - get next note
    if (Tune.nextNote(octave, noteId, playDuration))
    {
      // look up the frequency to play
      if (T.findNoteOctave(noteId, octave))
        player.play(T.getFrequency(), playDuration);

      timeStart = millis();
      state = PAUSE;
    }
    else
      state = WAIT_BETWEEN;   // could not get a note, end playing
    break;

  case PAUSE:  // pause while note/pause is executing
    if (millis() - timeStart >= playDuration)
    {
      if (!player.isPlaying())
        state = Tune.isEnded() ? WAIT_BETWEEN : PLAYING;
    }
    break;

  case WAIT_BETWEEN:  // wait at the end of a melody
    if (millis() - timeStart >= PAUSE_TIME)
      state = NEXT;     // start the next melody
    break;
  }
}

void setup(void)
{
  player.begin(PIN_TONE);
  Tune.begin();
}

void loop(void)
{
  processRTTTL();
}
