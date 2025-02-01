/*
MD_RTTTLPArser - Library for parsing and playing RTTTL string

See header file for copyright and licensing comments.
*/
#include "MD_RTTTLParser.h"

/**
* \file
* \brief Implements class definition and general methods
*/

#ifndef LIBDEBUG
#define LIBDEBUG 0    ///< Control debugging output. See \ref pageCompileSwitch
#endif

#if LIBDEBUG
#define DEBUG(s,v)  { Serial.print(F(s)); Serial.print(v); }                          ///< Print a string followed by a value (decimal)
#define DEBUGX(s,v) { Serial.print(F(s)); Serial.print("0x"); Serial.print(v, HEX); } ///< Print a string followed by a value (hex)
#define DEBUGS(s)   { Serial.print(F(s)); }                                           ///< Print a string
#define DEBUGC(c)   { Serial.print(c); }                                              ///< Print a single character
#else
#define DEBUG(s,v)  ///< Print a string followed by a value (decimal)
#define DEBUGX(s,v) ///< Print a string followed by a value (hex)
#define DEBUGS(s)   ///< Print a string
#define DEBUGC(c)   ///< Print a single character
#endif

// Note translation table.
// Translates the note string to the noteId. Flat notes (extension to the specification)
// are remapped to theire recognised equivalents.
//
const MD_RTTTLParser::noteTable_t PROGMEM MD_RTTTLParser::_noteTable[MAX_NOTES] =
{
  { "C", 0 },  
  { "C#", 1 }, { "D_", 1 },
  { "D", 2 },  
  { "D#", 3 }, { "E_", 3 },
  { "E", 4 },  { "F_", 4 },
  { "F", 5 },  
  { "F#", 6 }, { "G_", 6 },
  { "G", 7 },  
  { "G#", 8 }, { "A_", 8 },
  { "A", 9 },  
  { "A#", 10 },{ "B_", 10 }, { "H_", 10 },
  { "B", 11 }, { "H", 11, }, { "C_", 11 }
};

// Class methods
int8_t MD_RTTTLParser::findNoteId(const char* note)
// First get the index for the note by matching the name
// then return the value of that note from the table.
// Return -1 if not found.
{
  int8_t id = -1;

  for (uint8_t i = 0; i < ARRAY_SIZE(_noteTable); i++)
  {
    if (strcmp_P(note, _noteTable[i].name) == 0)
    {
      id = pgm_read_byte(&_noteTable[i].noteId);
      break;
    }
  }

  return(id);
}


void MD_RTTTLParser::newStringInit(const char* p)
{
  // set up defaults
  _dDuration = DEFAULT_DURATION;
  _dOctave = DEFAULT_OCTAVE;
  _dBPM = DEFAULT_BPM;
  _pTune = p;
  _curIdx = 0;
  _eoln = false;
  processHeader();
  _timeNote = calcNoteTime(_dBPM);
  _runState = PLAYING;    // start the FSM if it is ever invoked
}

char MD_RTTTLParser::getCh(void)
// return the next char, skipping whitespace
{
  char c = '\0';

  do
  {
    if (_isPROGMEM)
      c = pgm_read_byte(&_pTune[_curIdx]);
    else
      c = _pTune[_curIdx];
    //DEBUGC(c);
    _eoln = (c == '\0');
    if (!_eoln) _curIdx++;
  } while (!_eoln && isspace(c));

  return(c);
}

uint16_t MD_RTTTLParser::getNum(void)
// get the next number
{
  uint16_t num = 0;
  char c = getCh();

  while (isdigit(c))
  {
    num = (num * 10) + (c - '0');
    c = getCh();
  }
  ungetCh();

  return(num);
}

void MD_RTTTLParser::synch(char cSync)
// resynch to past this character in the stream
{
  char c;

  do
  {
    c = getCh();
  } while (c != '\0' && c != cSync);
}

uint32_t MD_RTTTLParser::getTimeToEnd(void)
{
  uint32_t t = 0;
  uint8_t octave;
  int8_t noteId;
  uint16_t duration;
  uint16_t idx = _curIdx; // remember where we are in the string

  while (nextNote(octave, noteId, duration))
    t += duration;

  _curIdx = idx;    // restore where we were
  return(t);
}

void MD_RTTTLParser::processHeader(void)
{
  char c;
  uint8_t idx = 0;

  // skip name
  // format: title:
  DEBUGS("\n>SKIP NAME ");
  do {
    c = getCh();
    if (idx < ARRAY_SIZE(_title)-1) _title[idx++] = c;
  } while (c != SECTION_DELIMITER);
  idx--;
  _title[idx] = '\0';
  DEBUG("\n", _title);

  // Now handle defaults section
  // format: d=N,o=N,b=NNN:
  DEBUGS("\n>DEFAULTS ");
  do
  {
    c = getCh();
    switch (c)
    {
    case 'd':    // get default duration
      synch(FIELD_EQUATE);
      _dDuration = getNum();
      synch(FIELD_DELIMITER);
      DEBUG("\ndDur: ", _dDuration);
      break;

    case 'o':   // get default octave
      synch(FIELD_EQUATE);
      _dOctave = getNum();
      synch(FIELD_DELIMITER);
      DEBUG("\ndOct: ", _dOctave);
      break;

    case 'b':   // get BPM
      synch(FIELD_EQUATE);
      _dBPM = getNum();
      // synch(RTTL_SECTION_DELIMITER);
      DEBUG("\ndBPM: ", _dBPM);
      break;
    }
  } while (c != SECTION_DELIMITER && !_eoln);
}

bool MD_RTTTLParser::nextNote(uint8_t& octave, int8_t& noteId, uint16_t& duration)
{
  char n[3];
  uint16_t num;
  bool gotDot = false;

  // first, get note playDuration, if available
  // we will need to check if we are a dotted note after  
  num = getNum();
  duration = (num != 0) ? (_timeNote / num) : (_timeNote / _dDuration);

  // this catches RTTTL string that end in a comma (actually illegal!)
  if (_eoln)
  {
    DEBUGS("eoln reached");
    return(false);
  }

  // now get the note into a string (with sharp if present)
  n[0] = toupper(getCh());
  n[1] = getCh();    // check for optional sharp/flat
  n[2] = '\0';
  if (n[1] != NOTE_SHARP && n[1] != NOTE_FLAT)
  {
    ungetCh();
    n[1] = '\0';
  }
  if (n[0] != 'P')
    noteId = findNoteId(n);
  else
    noteId = -1;

  // check for optional dotted  (sometimes before octave)
  char c = getCh();
  if (c == NOTE_DOTTED)
  {
    duration += duration / 2;
    c = getCh();  // for reversal later
    gotDot = true;
  }

  // get the octave
  ungetCh();
  octave = (isdigit(c)) ? getNum() : _dOctave;

  // check for optional dotted (sometimes after octave)
  if (!gotDot)
  {
    c = getCh();
    if (c == NOTE_DOTTED)
      duration += duration / 2;
    else
      ungetCh();
  }

  synch(FIELD_DELIMITER);   // skip to the next delimiter

  return(true);
}

bool MD_RTTTLParser::run(void)
// Manage reading and playing the note with callback
{
  switch (_runState)
  {
    case IDLE: // waiting to start a new tune
      // do nothing. 
      // This state is exited from the setBuffer() method.
    break;

    case PLAYING:     // playing a melody - get next note
    {
      if (nextNote(_octave, _noteId, _duration))
      {
        if (_noteId == -1)     // this is just a pause
        {
          DEBUG("\nPAUSE for ", _duration);
          DEBUGS("ms");
        }
        else
        {
          DEBUG("\nNOTE ", _noteId);
          DEBUG(" ON for ", _dDuration);
          DEBUGS("ms");
          if (_cbRTTTLHandler != nullptr)
            _cbRTTTLHandler(_octave, _noteId, _duration, true);
        }
        _timeStart = millis();
        _runState = WAIT;
      }
      else
        _runState = IDLE;   // could not get a note, end playing
    }
    break;

    case WAIT:  // pause while note/pause is executing
      if (millis() - _timeStart >= _duration)
      {
        _runState = _eoln ? IDLE : PLAYING;
        if ((_noteId != -1) && (_cbRTTTLHandler != nullptr))
          _cbRTTTLHandler(_octave, _noteId, _duration, false);
      }
      break;
  }

  return(_runState == IDLE);
}
