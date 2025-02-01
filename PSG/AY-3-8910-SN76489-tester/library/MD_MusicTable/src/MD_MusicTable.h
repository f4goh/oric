#pragma once

#include <Arduino.h>

/*
Musical Notes Lookup table
 
This class contains a lookup table for musical note information that
can be searched by name or note/octave (find*()) functions. Elements of
the table can then be retrieved (get*()). These include the note name, 
frequency octave and ANSI note designation.

Use this class by first finding the table entry and then getting the 
data one field at a time.

Note that Piano notes are a subset of the entire MIDI note range, starting 
at note 21 (A0) and ending at note 108 (C8), for a total of 88 notes.

   # # | # # | # # # | # # | # # # | # # | # #......# # # | # # | # # # | #
   # # | # # | # # # | # # | # # # | # # | # #......# # # | # # | # # # | #
   |_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|......|_|_|_|_|_|_|_|_|_|_|_|
    A B C D E F G A B C D E F G A B C D E F G        G A B C D E F G A B C 
    0 0 1 1 1 1 1 1 1 2 2 2 2 2 2 2 3 3 3 3 3        6 6 6 7 7 7 7 7 7 7 8 
    
The basis of the data for this table is taken from 
http://www.sengpielaudio.com/calculator-notenames.htm and the note frequency
recalculated using the formulae given there, where the relationship between 
frequency (f) and MIDI note number (m) is

f = 27.5 * 2^((m - 21)/12)
m = (12/ln(2)) * ln(f/27.5) + 21

Version History
---------------
Nov 2019 - v1.1.0
- Added findNoteOctave() method

Nov 2019 - v1.0.3
- Minor updates

Sep 2019 - v1.0.2
- More efficient search
- Added note id lookup 
- Adjusted scope of NOTES_IN_OCTAVE constant

Sep 2019 - v1.0.1
- Adjusted scope of NOTE_COUNT constant

Sep 2019 - v1.0.0
- First creation
*/

class MD_MusicTable
{
  public:
    static const uint8_t NOTES_COUNT = 128;    /// number of MIDI notes
    static const uint8_t NOTES_IN_OCTAVE = 12; /// number of notes

    MD_MusicTable(void): _curItem(-1) {}
    ~MD_MusicTable(void) {}

    // Searching functions set the current item selection index 
    // if the MIDI note item is found in the table
    
    // Search by MIDI note number
    bool findId(uint8_t midiId);

    // Search by note name (string eg "A" or "A#") and octave. 
    // Octave defaults to before Middle C if not specified
    bool findName(const char *note, int8_t octave = 3);

    // Search by NoteId and octave
    bool findNoteOctave(uint8_t noteId, int8_t octave = 3);

    // return the note index into the note table for a simple note
    // string (eg, "A" or "F#")
    int8_t lookupNoteId(const char* note);

    // Return values from the table
    float getFrequency(void);               // frequency in Hz
    char *getName(char *buf, uint8_t len);  // ISO name (note and octave eg A4)
    char *getNote(char *buf, uint8_t len);  // note name only (eg A)
    int8_t getOctave(void);                 // octave number
    int8_t getId(void);                     // MIDI note number
    int8_t getNoteId(void);                 // Note number [0..NOTES_IN_OCTAVE-1]
    
  private:
    static const uint8_t NOTE_NAME_SIZE = 3;   // number of characters
    
    // Structure to hold the data for one note
    struct noteItem_t
    {
      uint8_t id;     /// MIDI note number
      int8_t nameId;  /// ANSI note name id (noteName table index)
      int8_t octave;  /// the octave for this note
      float freq;     /// sound frequency for this note in Hz
    };

     int8_t _curItem; // currently selected item

    // Static data in data.cpp file
    static const char noteName[NOTES_IN_OCTAVE][NOTE_NAME_SIZE];
    static const noteItem_t notes[NOTES_COUNT];
};
