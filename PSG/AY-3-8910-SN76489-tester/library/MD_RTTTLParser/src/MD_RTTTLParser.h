#pragma once

#include <Arduino.h>

/**
 * \file
 * \brief Main header file for the MD_SN76489 library
 */

/**
\mainpage The RTTTTL Parser Library

Ring Tones Text Transfer Language (RTTTL) is a simple text-based format used to 
create ring tones for a mobile phones, developed by Nokia.

An example of a RTTTL string is

    Looney:d=4,o=5,b=140:32p,c6,8f6,8e6,8d6,8c6,a.,8c6,8f6,8e6,8d6,8d#6

Whilst it started as a mechanism for defining and distributing ring tone, RTTTL
is useful to more generally encode and playback simple tunes, and this library 
simplifies the parsing and playing of these tunes using either
- self management of all playing control
- library management with callback to user code for note generation

Topics
------
- \subpage pageRTTTLFormat
- \subpage pageLibrary
- \subpage pageCompileSwitch
- \subpage pageRevisionHistory
- \subpage pageCopyright
- \subpage pageDonation

\page pageRevisionHistory Revision History

Nov 2019 version 1.0.0
- Initial implementation.

\page pageRTTTLFormat RTTTL String format

A RTTTL tune can be divided into 3 parts, which are separated using a colon (':')

    Looney:d=4,o=5,b=140:32p,c6,8f6,8e6,8d6,8c6,a.,8c6,8f6,8e6,8d6,8d#6

- The title
- Default parameters (duration, pitch and bpm)
- Song Data

The Title
---------
The title of the song, which can be no more than 10 characters. The name of the example 
song (above) is 'Looney'.

Default parameters
------------------
The default value section is a set of values separated by commas, where each value contains
a key and a value separated by an equal ('=') character, which describes certain global defaults 
for the execution of the ring tone. Possible names are

|Id | Parameter                        |
|:-:|:---------------------------------|
| d	| Duration in milliseconds         |
| o	| Octave                           |
| b	| Beat or Tempo (beats per minute) |

If these parameters are not specified, the values "d=4,o=6,b=63" are used. 

The duration parameter is specified as a fraction of a full note duration.

| #  | Proportion of full note |
|---:|-------------------------|
|  1 | a full note             |
|  2 | a half note             |
|  4 | a quarter note          |
|  8 | an eighth note          |
| 16 | a sixteenth note        |
| 32 | a thirty-second note    |

The RTTTL format allows octaves starting from the A below middle C and going up four 
octaves. These octaves are numbered from lowest pitch to highest pitch from 4 to 7.

| Oct | Frequency         |
|----:|:------------------|
|   4 | Note A is 220Hz   |
|   5	| Note A is 440Hz   |
|   6	| Note A is 880Hz   |
|   7	| Note A is 1760Hz  |

BPM determines how much time each note or pause can take and is the number of
quarter notes in a minute. The BPM values can be one of 
25, 28, 31, 35, 40, 45, 50, 56, 63, 70, 80, 90, 100, 112, 125, 140,
160, 180, 200, 225, 250, 285, 320, 355, 400, 450, 500, 565, 635, 715, 
800 and 900.

Song data
---------
The last section of the string contains the song data as note specifications 
separated by a comma. A note is encoded by:

    [<duration>]<note>[<scale>][<special-duration>]

_Special-duration_ represents dotted rhythm patterns, formed by appending a period
('.') character to the end of duration-note-octave tuple in the song data. This
increases the duration of the note to 1.5 times its normal duration. Many RTTTL 
strings swap the _special-duration_ and _scale_ fields (ie, the octave value comes 
after the dot notation). The library recognizes and processes either form.

When the optional values _duration and/or _scale_ are omitted, the default
parameters from the default section of the RTTTL string are used instead. 

The following notes can be used in a RTTTL string:

| Id | Note Name          |
|:--:|--------------------|
| P  | Pause or rest      |
| C  | Note C             |
| C# | Note C sharp       |
| D	 | Note D             |
| D# | Note D sharp       |
| E	 | Note E             |
| F	 | Note F             |
| F# | Note F sharp       |
| G  | Note G             |
| G# | Note G sharp       |
| A  | Note A             |
| A# | Note A sharp       |
| H  | Note H (same as B) |

The octave is left out for a rest or pause.

Although not part of the original specification, these additional notes are 
also present in many RTTTL strings:

| Id | Note Name           |
|:--:|---------------------|
| B_ | B flat (same as A#) |
| C_ | C flat (same as B)  |
| D_ | D flat (same as C#) |
| E_ | E flat (same as D#) |
| F_ | F flat (same as E)  |
| G_ | G flat (same as F#) |
| A_ | A flat (same as G#) |
| H_ | H flat (same as B_) |

The library will translate these extensions into the equivalent note.

Formal specification
--------------------

    <RTTTL> := <title> ":" [<control-section>] ":" <tone-commands>

    <title> := <string:10>

    <control-section> := <def-note-duration> "," <def-note-scale> "," <def-beats>
    <def-duration> := "d=" <duration>
    <def-scale> := "o=" <scale>
    <def-beats> := "b=" <beats-per-minute>
    ; if not specified, defaults are duration=4, scale=6, beats-per-minute=63

    <tone-commands> := <note> ["," <tone-commands>]
    <note> := [<duration>] <note> [<scale>] [<special-duration>]

    <duration> := "1"|"2"|"4"|"8"|"16"|"32"
    <scale> := "5"|"6"|"7"|"8"
    <beats-per-minute> := "5"|"28"|"31"|"35"|"40"|"45"|"50"|"56"|"63"|"70"|"80"|"90"|"100"|"112"|
                        "125"|"140"|"160"|"180"|"200"|"225"|"250"|"285"|"20"|"355"|"400"|"450"|"500"|
                        "565"|"635"|"715"|"800"|"900"
    <note> := "P"|"C"|"C#"|"D"|"D#"|"E"|"F"|"F#"|"G"|"G#"|"A"|"A#"|"H"
    <special-duration> := "."

\page pageLibrary Using the Library
Defining the object
-------------------
The object definition requires no additional parameters.

setup()
-------
The setup() function must invoke the begin() method. All other library initialization 
takes place at this time.

Sound output
------------
To keep the parser library generic, sound output must be implemented 
in the application as the type of hardware used will dictate how the 
sound is produced.

The parser will translate the information from the RTTTL string into
parameters that the application can use:

+ __octave__ - generally in the valid range as is represented in the RTTTL 
string but no checking is done in the parser to enforce this.
+ __note identifier__ - the notes are numbered according to the ISO standard
numbering  (C=0, C#=1, ..., A#=11) and can be used directly with the 
MD_MusicTable library to obtain note frequency or midi note numbers. 
The library will also translate the non-standard flat notes into the 
correct equivalent. Pause values are passed back as note -1 and need 
to be explicitly checked.
+ __duration__ - the playing tempo and type of note, including dot notation, 
is converted into the number of milliseconds for the note or pause.

Application managed playback
----------------------------
The RTTLParser_Manual example demonstrates this use of the library.

In this mode the application has complete control over how the RTTTL 
file is processed.

A simplified flow for the application is:

+ setTune() is used initialize the library for the specified RTTTL
string.
+ nextNote() is used to retrieve the next note in the sequence. In 
the case of a playable note, the application invokes the sound 
generation hardware. The application implements the logic required 
for the note or pause timing before processing the next note.
+ nextNote() returns true when the end of the RTTTL string is 
reached. Alternatively, the isEoln() method can be used to test 
for end of line being reached.

Library managed playback
------------------------
The RTTLParser_Cback example demonstrates this use of the library.

In this mode the application provides the library with a callback 
that will manage the sound generation hardware. The library will 
manage duration timing for pauses (ie, the application is not 
aware of these) and will provide the callback with note on and note 
off events for the application to process.

The simplified flow for the application is:

+ setCallback() is used to register the callback function (can be 
done in setup()).
+ setTune() is used initialize the library for the specified RTTTL 
string.
+ run() is invoked every time through main program loop. This will 
run a Finite State Machine that manages the sequencing and timing 
of notes, and invokes the callback when required.
+ run() returns true when the end of the RTTTL string has been reached.

Parsing without playback
------------------------
Parsing without playback is identical to an application managed playback
without 'playing' the notes. The entire string can be scanned very fast 
when the note durations and pauses are ignored. Once the string has been 
parsed, it will need to be reset using setTune().

Parsing an RTTTL string is useful to gather information about the string, 
such as what octaves are being used, the range of notes, etc. These can 
then be used to modify subsequent playback (eg, shifting the octave range)
to suit a particular sound output device.

\page pageCompileSwitch Compiler Switches

LIBDEBUG
--------
Controls debugging output to the serial monitor from the library. If set to
1 debugging is enabled and the main program must open the Serial port for output

\page pageDonation Support the Library
If you like and use this library please consider making a small donation 
using [PayPal](https://paypal.me/MajicDesigns/4USD)

\page pageCopyright Copyright
Copyright (C) 2019 Marco Colli. All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

*/

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))  ///< Standard method to work out array size

/**
 * Base class for the MD_RTTTLParser library
 */
class MD_RTTTLParser
{
public:
  /**
   * Class Constructor.
   *
   * Instantiate a new instance of the class. Multiple instances may co-exist.
   */
   MD_RTTTLParser(void): _cbRTTTLHandler(nullptr) {}

  /**
   * Initialize the object.
   *
   * Initialize the object data. This needs to be called during setup() to initialize
   * new data for the class that cannot be done during the object creation.
   */
   void begin(void) {}

  //--------------------------------------------------------------
  /** \name Playing Control methods.
   * @{
   */
  /**
   * Set the current tune (dynamic RAM).
   *
   * An RTTTL tune is a nul terminated string of ASCII characters (C-style string)
   * that conform to the \ref pageRTTTLFormat. This method provides the library with
   * the address of the next string to be parsed. The string is not modified by the 
   * library. The string should not contain any spaces.
   * 
   * Following this call, the getTitle(), getOctaveDefault() and getBPMDefault() 
   * methods may be interrogated for information about the RTTTL string header.
   *
   * \sa getTitle(), getOctaveDefault(), getBPMDefault(), \ref pageRTTTLFormat
   *
   * \param p   a pointer to the RTTTL string stored in RAM to be processed.
   * \return true if the call was successful, false otherwise.
   */
   inline void setTune(const char* p) { _isPROGMEM = false;  newStringInit(p); }

   /**
    * Set the current tune (PROGMEM flash RAM).
    *
    * This works like the setTune() method but the string is in PROGMEM flash RAM.
    * 
    * Where PROGMEM is supported, the library expects this string to be stored in PROGMEM
    * and processes it accordingly.
    *
    * For architectures that don't need or don't support PROGMEM, this should resolve 
    * itself to working just like setTune().
    *
    * \sa setTune()
    *
    * \param p   a pointer to the RTTTL string stored in PROGMEM to be processed.
    * \return true if the call was successful, false otherwise.
    */
    inline void setTune_P(const char* p) { _isPROGMEM = true;  newStringInit(p); }
   
   /**
   * Parse the next note in the RTTTL string.
   *
   * Parses the next note in the RTTL string and passes back the parameters to the 
   * calling application for sound production. All parameters are passed by reference 
   * and will be filled in with the note attributes if the note was correctly processed.
   *
   * - The _octave_ parameter receives the scale parameter for the note. The octave 
   * specified in the RTTTL string is passed back directly to the caller. Any scale 
   * boundary checking should be done by the in the calling application. This allows 
   * applications controlling hardware with broader capability to decide whether they 
   * should play this note.
   *
   * - The _noteId_ is the number of the note within the octave. A pause (RTTTL note P) 
   * is represented by a -1 noteID, the remaining notes are numbered according to the 
   * ISO standard: 
   *
   *     C=0, C#=1, D=2, D#=3, E=4, F=5, F#=6, G=7, G#=8, A=9, A#=10, B=11
   * 
   * - The _duration_ is the how many milliseconds to play the note or pause.
   * 
   * If a callback function is set, its function parameters contain the same
   * information as these parameters.
   *
   * \sa run(), setCallback(), \ref pageRTTTLFormat, \ref pageLibrary
   *
   * \param octave    reference to return the scale (octave) for this note.
   * \param noteId    reference to return the note number within the octave.
   * \param duration  reference to return the duration in milliseconds for the note.
   * \return true if the note was validly parsed and variables are set correctly, false otherwise.
   */
   bool nextNote(uint8_t& octave, int8_t& noteId, uint16_t& duration);

  /**
   * Run the RTTTL player.
   *
   * The RTTTL library can be used to manage tune playback within the library. 
   * This is done in a non-blocking manner within this run() method.
   * 
   * For the library to be able to manage playback, run() must be invoked every 
   * each time through the main loop() after the song is set up using setTune().
   * Once the method returns true, it no longer need to be invoked until the 
   * next setTune().
   *
   * The callback function must defined prior to calling run() or the application will be 
   * unable to receive note parameters for sound output.
   *
   * run() should never be invoked if the application is managing playback.
   *
   * \sa setTune(), setCallback(), \ref pageLibrary
   *
   * \return true if the end of the RTTTL string has been reached, false otherwise.
   */
   bool run(void);

  /**
   * Check if the RTTTL string has reach end of line.
   *
   * Checks if the currently defined RTTTL string has been completely processed (ie,
   * the end of the string has been reached).
   *
   * \sa setTune()
   *
   * \return true if the end of line has been reached, false otherwise.
   */
   inline bool isEnded(void) { return(_eoln); }

  /**
   * Set the callback function.
   *
   * When operating in callback mode the library will call the specified function
   * when it needs to turn a note on or off. The callback is specifically designed
   * for use when the library is managing the timing of note using the run() method.
   *
   * The function prototype for the callback function is
   * 
   *     void RTTTLhandler(uint8_t octave, uint8_t noteId, uint32_t duration, bool activate)
   *
   * The current note parameters passed in the callback are identical to those described 
   * for the nextNote() method. The additional parameter _activate_ is used as follows:
   * - If activate is true, play a note (octave, noteId) for the duration (ms) specified.
   * - If activate is false (deactivate), the note should be turned off. The other 
   *   parameters will match the those in the previous activate call but can can be
   *   ignored.
   * 
   * A deactivate callback always follows an activate. If the music output device is 
   * managing the duration, the callback function should ignore the deactivate portion 
   * of the callback cycle.
   *
   * Passing a null pointer (_nullptr_) disables the callback. The callback is disabled 
   * by default.
   *
   * \sa nextNote(), \ref pageLibrary
   * 
   * \param cb the address of the callback function.
   */
   void setCallback(void(*cb)(uint8_t octave, uint8_t noteId, uint32_t duration, bool activate)) { _cbRTTTLHandler = cb; }
  
  /** @} */

  //--------------------------------------------------------------
  /** \name Informational methods.
   * @{
   */
  /**
   * Get the tune title.
   *
   * Once the tune has been set, the song name/title from the RTTTL
   * string is available for interrogation.
   *
   * \sa setTune()
   *
   * \return a pointer to the title.
   */
   const char* getTitle(void) { return(_title); }

   /**
   * Get the tune default octave/scale.
   *
   * Once the tune has been set, the song's default scale
   * from the RTTTL defaults section is available for interrogation.
   *
   * \sa setTune()
   *
   * \return the octave number. 
   */
  uint8_t getOctaveDefault(void) { return(_dOctave); }

  /**
   * Get the tune default BPM.
   *
   * Once the tune has been set, the song's default beats per minute 
   * from the RTTTL defaults section is available for interrogation.
   *
   * \sa setTune()
   *
   * \return the number of beats per minute.
   */
  uint16_t getBPMDefault(void) { return(_dBPM); }

  /**
   * Get the total tune playing time.
   *
   * Once the tune has been set, the total remaining time can be interrogated 
   * at any stage of the playing process. The time includes all notes and pauses 
   * defined in the RTTTL string.
   *
   * If this is invoked before the tune has started playing it will return the 
   * total tune playing time.
   *
   * \sa setTune()
   *
   * \return the remaining playing time in milliseconds.
   */
  uint32_t getTimeToEnd(void);

  /** @} */

private:
  // Constants
  // RTTTL parsing tokens
  static const char SECTION_DELIMITER = ':';
  static const char FIELD_DELIMITER = ',';
  static const char FIELD_EQUATE = '=';
  static const char NOTE_SHARP = '#';
  static const char NOTE_FLAT = '_';
  static const char NOTE_DOTTED = '.';

  // Array sizing
  static const uint8_t MAX_NOTES = 21;        // max entries in the noteTable
  static const uint8_t NOTE_NAME_SIZE = 3;    // in characters including nul terminator

  // Default values per tune
  static const uint8_t DEFAULT_DURATION = 4;  // default ms per note
  static const uint8_t DEFAULT_OCTAVE = 5;    // default octave
  static const uint16_t DEFAULT_BPM = 63;     // default BPM

  // Structures & enums
  typedef struct
  {
    const char name[NOTE_NAME_SIZE];    // note names in the octave
    const uint8_t noteId;               // the identifier for this note
  } noteTable_t;

  typedef enum { IDLE, PLAYING, WAIT } state_t; // enumerated states for the FSM
  
  // Control variables
  bool _isPROGMEM;            // current string being parsed is in PROGMEM
  const char* _pTune;         // ppinmter to the tune memory, set by setTune()
  uint16_t _curIdx;           // current index into _pTune
  bool _eoln;                 // flag set true when the string end of line is reached
  state_t _runState = IDLE;   // state for run() FSM
  uint32_t _timeStart;        // millis() timing marker for FSM

  static const noteTable_t _noteTable[MAX_NOTES]; // note translation table (static data)

  // values for the current RTTL string
  void (*_cbRTTTLHandler)(uint8_t octave, uint8_t noteId, uint32_t duration, bool activate); // callback function
  char _title[11];     // buffer for tune title including nul terminator
  uint8_t _dDuration;  // default duration in ms for one note
  uint8_t _dOctave;    // default octave scale
  uint32_t _timeNote;  // calculated time in ms for a whole note

  uint16_t _dBPM;      // BPM usually expresses the number of quarter notes per minute
  uint16_t _duration;  // current note length in ms
  uint8_t _octave;     // current octave to play
  int8_t _noteId;      // current index number of note to play

  // Methods
  inline bool isdigit(char c) { return(c >= '0' && c <= '9'); }     // check is char is a digit
  inline void ungetCh() { if (_curIdx != 0) _curIdx--; }            // reverse one char back in the string
  inline uint32_t calcNoteTime(uint16_t bpm) { return((60 * 1000L / bpm) * 4); }  // time for whole note in ms

  void newStringInit(const char *p);    // set up defaults for a new string
  char getCh(void);                     // get the next character from the RTTTL string
  uint16_t getNum(void);                // get the next number in the RTTTL string
  void synch(char cSync);               // synch the position in the string to after cSync
  int8_t findNoteId(const char* note);  // translate note string into noteId
  void processHeader(void);             // Process the RTTTL header (name and defaults) information
};
