// 
// 
// 

#include "MD_MusicTable.h"

bool MD_MusicTable::findId(uint8_t midiId)
{
  if (midiId < NOTES_COUNT)
    _curItem = midiId;
  else
    _curItem = -1;

  return(_curItem != -1);
}

bool MD_MusicTable::findName(const char *note, int8_t octave)
{
  int8_t idxName = -1;

  _curItem = -1;

  // first get the index for the note name by matching the name
  idxName = lookupNoteId(note);

  if (idxName != -1)
  {
    // if that passed, then work out the index based on the octave
    _curItem = (octave * NOTES_IN_OCTAVE) + idxName;
    if (_curItem >= NOTES_COUNT)
      _curItem = -1; 
  }

  return(_curItem != -1);
}

bool MD_MusicTable::findNoteOctave(uint8_t noteId, int8_t octave)
{
  _curItem = -1;

  // first get the index for the note name is within bounds
  if (noteId <= NOTES_IN_OCTAVE)
  {
    // if that passed, then work out the index based on the octave
    _curItem = (octave * NOTES_IN_OCTAVE) + noteId;
    if (_curItem >= NOTES_COUNT)
      _curItem = -1;
  }

  return(_curItem != -1);
}

int8_t MD_MusicTable::lookupNoteId(const char* note)
// first get the index for the note name by matching the name
{
  int8_t idxName = -1;

  for (uint8_t i = 0; i < NOTES_IN_OCTAVE; i++)
  {
    if (strcmp_P(note, noteName[i]) == 0)
    {
      idxName = i;
      break;
    }
  }

  return(idxName);
}

float MD_MusicTable::getFrequency(void)
{
  float f = 0;

  if (_curItem != -1)
    f = pgm_read_float(&notes[_curItem].freq);

  return(f);
}

char *MD_MusicTable::getName(char *buf, uint8_t len)
// return the ANSI name
{
  *buf = '\0';

  if (_curItem != -1)
  {
    uint8_t nameIdx = pgm_read_byte(&notes[_curItem].nameId);
    int8_t octave = pgm_read_byte(&notes[_curItem].octave);
    char sz[3];

    itoa(octave, sz, 10);
    strncpy_P(buf, noteName[nameIdx], len);
    strncat(buf, sz, len-1);
  }
  return(buf);
}

char *MD_MusicTable::getNote(char *buf, uint8_t len)
{
  *buf = '\0';
  if (_curItem != -1)
  {
    uint8_t nameIdx = pgm_read_byte(&notes[_curItem].nameId);
    strncpy_P(buf, noteName[nameIdx], len);
  }
  return(buf);
}

int8_t MD_MusicTable::getOctave(void)
{
  int8_t octave = -99;

  if (_curItem != -1)
    octave = pgm_read_byte(&notes[_curItem].octave);

  return(octave);
}

int8_t MD_MusicTable::getId(void)
{
  uint8_t id = 255;

  if (_curItem != -1)
    id = pgm_read_byte(&notes[_curItem].id);

  return(id);
};

int8_t MD_MusicTable::getNoteId(void)
{
  uint8_t id = 255;

  if (_curItem != -1)
    id = pgm_read_byte(&notes[_curItem].nameId);

  return(id);
};
