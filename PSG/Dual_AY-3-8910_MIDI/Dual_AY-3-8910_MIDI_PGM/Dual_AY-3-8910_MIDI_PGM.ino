/*
 * See https://dogemicrosystems.ca/wiki/Dual_AY-3-8910_MIDI_module
 */

// Uncomment to enable traiditonal serial midi
#define SERIALMIDI

// Uncomment to enable USB midi
//#define USBMIDI

// Uncomment to enable debugging output over USB serial
//#define DEBUG

//a voir https://github.com/jim3692/Arduino-LiquidCrystal-SoftI2C-library


#include <avr/io.h>

#ifdef USBMIDI
#include "MIDIUSB.h"
#endif

// We borrow one struct from MIDIUSB for traditional serial midi, so define it if were not using USBMIDI
#ifndef MIDIUSB_h
typedef struct
{
  uint8_t header;
  uint8_t byte1;
  uint8_t byte2;
  uint8_t byte3;
} midiEventPacket_t;
#endif

#ifdef SERIALMIDI
#include <MIDI.h>
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);//Serial1 on pro micro
#endif

typedef unsigned short int ushort;

typedef unsigned char note_t;
#define NOTE_OFF 0

typedef unsigned char midictrl_t;

// Pin driver ---------------------------------------------

static const int dbus[8] = { 2, 3, 4, 5, 6, 7, 8, 10 };

static const ushort
  BC2_A = 16,
  BDIR_A = 14,
  BC2_B = A0,
  BDIR_B = A1,
  nRESET = 15,
  clkOUT = 9;

static const ushort DIVISOR = 7; // Set for 1MHz clock

static void clockSetup() {
   // Timer 1 setup for Mega32U4 devices
   //
   // Use CTC mode: WGM13..0 = 0100
   // Toggle OC1A on Compare Match: COM1A1..0 = 01
   // Use ClkIO with no prescaling: CS12..0 = 001
   // Interrupts off: TIMSK0 = 0
   // OCR0A = interval value
   
   TCCR1A = (1 << COM1A0);
   TCCR1B = (1 << WGM12) | (1 << CS10);
   TCCR1C = 0;
   TIMSK1 = 0;
   OCR1AH = 0;
   OCR1AL = DIVISOR; // NB write high byte first
}

static void setData(unsigned char db) {
  unsigned char bit = 1;
  for (ushort i = 0; i < 8; i++) {
    digitalWrite(dbus[i], (db & bit) ? HIGH : LOW);
    bit <<= 1;
  }
}

static void writeReg_A(unsigned char reg, unsigned char db) {
  // This is a bit of an odd way to do it, BC1 is kept low and NACT, BAR, IAB, and DWS are used.
  // BC1 is kept low the entire time.
  
  // Inactive (BDIR BC2 BC1 0 0 0)
  digitalWrite(BDIR_A, LOW);
  digitalWrite(BC2_A, LOW);

  //Set register address
  setData(reg);

  // BAR (Latch) (BDIR BC2 BC1 1 0 0)
  digitalWrite(BDIR_A, HIGH);

  // Inactive (BDIR BC2 BC1 0 0 0)
  digitalWrite(BDIR_A, LOW);
  
  //Set register contents
  setData(db);

  // Write (BDIR BC2 BC1 1 1 0)
  digitalWrite(BC2_A, HIGH);
  digitalWrite(BDIR_A, HIGH);

  // Inactive (BDIR BC2 BC1 0 0 0)
  digitalWrite(BC2_A, LOW);
  digitalWrite(BDIR_A, LOW);
}

static void writeReg_B(unsigned char reg, unsigned char db) {
  // This is a bit of an odd way to do it, BC1 is kept low and NACT, BAR, IAB, and DWS are used.
  // BC1 is kept low the entire time.
  
  // Inactive (BDIR BC2 BC1 0 0 0)
  digitalWrite(BDIR_B, LOW);
  digitalWrite(BC2_B, LOW);

  //Set register address
  setData(reg);

  // BAR (Latch) (BDIR BC2 BC1 1 0 0)
  digitalWrite(BDIR_B, HIGH);

  // Inactive (BDIR BC2 BC1 0 0 0)
  digitalWrite(BDIR_B, LOW);
  
  //Set register contents
  setData(db);

  // Write (BDIR BC2 BC1 1 1 0)
  digitalWrite(BC2_B, HIGH);
  digitalWrite(BDIR_B, HIGH);

  // Inactive (BDIR BC2 BC1 0 0 0)
  digitalWrite(BC2_B, LOW);
  digitalWrite(BDIR_B, LOW);
}

// AY-3-8910 driver ---------------------------------------

class PSGRegs {
public:
  enum {
    TONEALOW = 0,
    TONEAHIGH,
    TONEBLOW,
    TONEBHIGH,
    TONECLOW,
    TONECHIGH,
    NOISEGEN,
    MIXER,
    
    TONEAAMPL,
    TONEBAMPL,
    TONECAMPL,
    ENVLOW,
    ENVHIGH,
    ENVSHAPE,
    IOA,
    IOB
  };
  
  unsigned char regs_A[16];
  unsigned char regs_B[16];

  unsigned char lastregs_A[16];
  unsigned char lastregs_B[16];

  void init() {
    for (int i = 0; i < 16; i++) {
      regs_A[i] = lastregs_A[i] = 0xFF;
      writeReg_A(i, regs_A[i]);

      regs_B[i] = lastregs_B[i] = 0xFF;
      writeReg_B(i, regs_B[i]);
    }
  }
  
  void update() {
    for (int i = 0; i < 16; i++) {
      if (regs_A[i] != lastregs_A[i]) {
        writeReg_A(i, regs_A[i]);
        lastregs_A[i] = regs_A[i];
      }

      if (regs_B[i] != lastregs_B[i]) {
        writeReg_B(i, regs_B[i]);
        lastregs_B[i] = regs_B[i];
      }
    }
  }

  void setTone(ushort ch, ushort divisor, ushort ampl) {
    if (ch > 2) {
      //reduce channel to usable range
      ch = ch - 3;
      //use regs_B
      regs_B[TONEALOW  + (ch<<1)] = (divisor & 0xFF);
      regs_B[TONEAHIGH + (ch<<1)] = (divisor >> 8);
      regs_B[TONEAAMPL + ch] = ampl;
      
      ushort mask = (8+1) << ch;
      regs_B[MIXER] = (regs_B[MIXER] | mask) ^ (1 << ch);

      return;
    }
    
    regs_A[TONEALOW  + (ch<<1)] = (divisor & 0xFF);
    regs_A[TONEAHIGH + (ch<<1)] = (divisor >> 8);
    regs_A[TONEAAMPL + ch] = ampl;
    
    ushort mask = (8+1) << ch;
    regs_A[MIXER] = (regs_A[MIXER] | mask) ^ (1 << ch);
  }

  void setToneAndNoise(ushort ch, ushort divisor, ushort noisefreq, ushort ampl) {
    if (ch > 2) {
      //reduce channel to usable range
      ch = ch - 3;
      //use regs_B
      regs_B[TONEALOW  + (ch<<1)] = (divisor & 0xFF);
      regs_B[TONEAHIGH + (ch<<1)] = (divisor >> 8);
      regs_B[NOISEGEN] = noisefreq;
      regs_B[TONEAAMPL + ch] = ampl;
      
      ushort mask = (8+1) << ch;
      ushort bits = (noisefreq < 16 ? 8 : 0) + (divisor > 0 ? 1 : 0);
      regs_B[MIXER] = (regs_B[MIXER] | mask) ^ (bits << ch);

      return;
    }
    
    regs_A[TONEALOW  + (ch<<1)] = (divisor & 0xFF);
    regs_A[TONEAHIGH + (ch<<1)] = (divisor >> 8);
    regs_A[NOISEGEN] = noisefreq;
    regs_A[TONEAAMPL + ch] = ampl;
    
    ushort mask = (8+1) << ch;
    ushort bits = (noisefreq < 16 ? 8 : 0) + (divisor > 0 ? 1 : 0);
    regs_A[MIXER] = (regs_A[MIXER] | mask) ^ (bits << ch);
  }

  void setEnvelope(ushort divisor, ushort shape) {
    regs_A[ENVLOW]  = (divisor & 0xFF);
    regs_A[ENVHIGH] = (divisor >> 8);
    regs_A[ENVSHAPE] = shape;

    regs_B[ENVLOW]  = (divisor & 0xFF);
    regs_B[ENVHIGH] = (divisor >> 8);
    regs_B[ENVSHAPE] = shape; 
  }
  
  void setOff(ushort ch) {
    if (ch > 2) {
      //reduce channel to usable range
      ch = ch - 3;
      //use regs_B
      ushort mask = (8+1) << ch;
      regs_B[MIXER] = (regs_A[MIXER] | mask);
      regs_B[TONEAAMPL + ch] = 0;
      if (regs_B[ENVSHAPE] != 0) {
        regs_B[ENVSHAPE] = 0;
        update(); // Force flush
      }

      return;
    }
    
    ushort mask = (8+1) << ch;
    regs_A[MIXER] = (regs_A[MIXER] | mask);
    regs_A[TONEAAMPL + ch] = 0;
    if (regs_A[ENVSHAPE] != 0) {
      regs_A[ENVSHAPE] = 0;
      update(); // Force flush
    }
  }
};

static PSGRegs psg;

// Voice generation ---------------------------------------

static const ushort
  MIDI_MIN = 24,
  MIDI_MAX = 96,
  N_NOTES = (MIDI_MAX+1-MIDI_MIN);

static const ushort note_table[N_NOTES] = {
   1911, // MIDI 24, 32.70 Hz
   1804, // MIDI 25, 34.65 Hz
   1703, // MIDI 26, 36.71 Hz
   1607, // MIDI 27, 38.89 Hz
   1517, // MIDI 28, 41.20 Hz
   1432, // MIDI 29, 43.65 Hz
   1351, // MIDI 30, 46.25 Hz
   1276, // MIDI 31, 49.00 Hz
   1204, // MIDI 32, 51.91 Hz
   1136, // MIDI 33, 55.00 Hz
   1073, // MIDI 34, 58.27 Hz
   1012, // MIDI 35, 61.74 Hz
   956, // MIDI 36, 65.41 Hz
   902, // MIDI 37, 69.30 Hz
   851, // MIDI 38, 73.42 Hz
   804, // MIDI 39, 77.78 Hz
   758, // MIDI 40, 82.41 Hz
   716, // MIDI 41, 87.31 Hz
   676, // MIDI 42, 92.50 Hz
   638, // MIDI 43, 98.00 Hz
   602, // MIDI 44, 103.83 Hz
   568, // MIDI 45, 110.00 Hz
   536, // MIDI 46, 116.54 Hz
   506, // MIDI 47, 123.47 Hz
   478, // MIDI 48, 130.81 Hz
   451, // MIDI 49, 138.59 Hz
   426, // MIDI 50, 146.83 Hz
   402, // MIDI 51, 155.56 Hz
   379, // MIDI 52, 164.81 Hz
   358, // MIDI 53, 174.61 Hz
   338, // MIDI 54, 185.00 Hz
   319, // MIDI 55, 196.00 Hz
   301, // MIDI 56, 207.65 Hz
   284, // MIDI 57, 220.00 Hz
   268, // MIDI 58, 233.08 Hz
   253, // MIDI 59, 246.94 Hz
   239, // MIDI 60, 261.63 Hz
   225, // MIDI 61, 277.18 Hz
   213, // MIDI 62, 293.66 Hz
   201, // MIDI 63, 311.13 Hz
   190, // MIDI 64, 329.63 Hz
   179, // MIDI 65, 349.23 Hz
   169, // MIDI 66, 369.99 Hz
   159, // MIDI 67, 392.00 Hz
   150, // MIDI 68, 415.30 Hz
   142, // MIDI 69, 440.00 Hz
   134, // MIDI 70, 466.16 Hz
   127, // MIDI 71, 493.88 Hz
   119, // MIDI 72, 523.25 Hz
   113, // MIDI 73, 554.37 Hz
   106, // MIDI 74, 587.33 Hz
   100, // MIDI 75, 622.25 Hz
   95, // MIDI 76, 659.26 Hz
   89, // MIDI 77, 698.46 Hz
   84, // MIDI 78, 739.99 Hz
   80, // MIDI 79, 783.99 Hz
   75, // MIDI 80, 830.61 Hz
   71, // MIDI 81, 880.00 Hz
   67, // MIDI 82, 932.33 Hz
   63, // MIDI 83, 987.77 Hz
   60, // MIDI 84, 1046.50 Hz
   56, // MIDI 85, 1108.73 Hz
   53, // MIDI 86, 1174.66 Hz
   50, // MIDI 87, 1244.51 Hz
   47, // MIDI 88, 1318.51 Hz
   45, // MIDI 89, 1396.91 Hz
   42, // MIDI 90, 1479.98 Hz
   40, // MIDI 91, 1567.98 Hz
   38, // MIDI 92, 1661.22 Hz
   36, // MIDI 93, 1760.00 Hz
   34, // MIDI 94, 1864.66 Hz
   32, // MIDI 95, 1975.53 Hz
   30, // MIDI 96, 2093.00 Hz
};

struct FXParams {
  ushort noisefreq;
  ushort tonefreq;
  ushort envdecay;
  ushort freqdecay;
  ushort timer;
};

struct ToneParams {
  ushort decay;
  ushort sustain; // Values 0..32
  ushort release;
};

static const ushort MAX_TONES = 4;
static const ToneParams tones[MAX_TONES] = {
  { 30, 24, 10 },
  { 30, 12, 8 },
  { 5,  8,  7 },
  { 10, 31, 30 }
};

class Voice {
public:
  ushort m_chan;  // Index to psg channel 
  ushort m_pitch;
  int m_ampl, m_decay, m_sustain, m_release;
  static const int AMPL_MAX = 1023;
  ushort m_adsr;

  void init (ushort chan) {
    m_chan = chan;
    m_ampl = m_sustain = 0;
    kill();
  }
  
  void start(note_t note, midictrl_t vel, midictrl_t chan) {
    const ToneParams *tp = &tones[chan % MAX_TONES];
    
    m_pitch = note_table[note - MIDI_MIN];
    if (vel > 127) {
      m_ampl = AMPL_MAX;
    }
    else {
      m_ampl = 768 + (vel << 1);
    }
    m_decay = tp->decay;
    m_sustain = (m_ampl * tp->sustain) >> 5;
    m_release = tp->release;
    m_adsr = 'D';
    psg.setTone(m_chan, m_pitch, m_ampl >> 6);
  }

  struct FXParams m_fxp;
  
  void startFX(const struct FXParams &fxp) {
    m_fxp = fxp;
  
    if (m_ampl > 0) {
      psg.setOff(m_chan);
    }
    m_ampl = AMPL_MAX;
    m_adsr = 'X';
    m_decay = fxp.timer;

    psg.setEnvelope(fxp.envdecay, 9); 
    psg.setToneAndNoise(m_chan, fxp.tonefreq, fxp.noisefreq, 31);
  }
  

  void stop() {
    if (m_adsr == 'X') {
      return; // Will finish when ready...
    }
      
    if (m_ampl > 0) {
      m_adsr = 'R';
    }
    else {
      psg.setOff(m_chan);
    }
  }
  
  void update100Hz() {
    if (m_ampl == 0) {
      return;
    }
      
    switch(m_adsr) {
      case 'D':
        m_ampl -= m_decay;
        if (m_ampl <= m_sustain) {
          m_adsr = 'S';
          m_ampl = m_sustain;
        }
        break;

      case 'S':
        break;

      case 'R':
        if ( m_ampl < m_release ) {
          m_ampl = 0;
        }
        else {
          m_ampl -= m_release;
        }
        break;

      case 'X':
        // FX is playing.         
        if (m_fxp.freqdecay > 0) { 
          m_fxp.tonefreq += m_fxp.freqdecay;
          psg.setToneAndNoise(m_chan, m_fxp.tonefreq, m_fxp.noisefreq, 31);
        }
        
        m_ampl -= m_decay;
        if (m_ampl <= 0) {
          psg.setOff(m_chan);
          m_ampl = 0;
        }
        return;
        
      default:
        break;
    }  

    if (m_ampl > 0) {
      psg.setTone(m_chan, m_pitch, m_ampl >> 6);
    }
    else {
      psg.setOff(m_chan);    
    }
  }
  
  bool isPlaying() {
    return (m_ampl > 0);
  }
  
  void kill() {
    psg.setOff(m_chan);
    m_ampl = 0;
  }
};


const ushort MAX_VOICES = 6;

static Voice voices[MAX_VOICES];

// MIDI synthesiser ---------------------------------------

// Deals with assigning note on/note off to voices

static const uint8_t PERC_CHANNEL = 9;

static const note_t
  PERC_MIN = 35,
  PERC_MAX = 50;
  
static const struct FXParams perc_params[PERC_MAX-PERC_MIN+1] = {
  // Mappings are from the General MIDI spec at https://www.midi.org/specifications-old/item/gm-level-1-sound-set
  
  // Params are: noisefreq, tonefreq, envdecay, freqdecay, timer
  
  { 9, 900, 800, 40, 50 },   // 35 Acoustic bass drum
  { 8, 1000, 700, 40, 50 },  // 36 (C) Bass Drum 1
  { 4, 0, 300, 0, 80 },      // 37 Side Stick
  { 6, 0, 1200, 0, 30  },    // 38 Acoustic snare
  
  { 5, 0, 1500, 0, 90 },     // 39 (D#) Hand clap
  { 6, 400, 1200, 11, 30  }, // 40 Electric snare
  { 16, 700, 800, 20, 30 },  // 41 Low floor tom
  { 0, 0, 300, 0, 80 },      // 42 Closed Hi Hat
  
  { 16, 400, 800, 13, 30 },   // 43 (G) High Floor Tom
  { 0, 0, 600, 0, 50 },      // 44 Pedal Hi-Hat
  { 16, 800, 1400, 30, 25 }, // 45 Low Tom
  { 0, 0, 800, 0, 40 },      // 46 Open Hi-Hat
  
  { 16, 600, 1400, 20, 25 }, // 47 (B) Low-Mid Tom
  { 16, 450, 1500, 15, 22 }, // 48 Hi-Mid Tom
  { 1, 0, 1800, 0, 25 },     // 49 Crash Cymbal 1
  { 16, 300, 1500, 10, 22 }, // 50 High Tom
};
  
  

static const int REQ_MAP_SIZE = (N_NOTES+7) / 8;
static uint8_t m_requestMap[REQ_MAP_SIZE];
  // Bit is set for each note being requested
static  midictrl_t m_velocity[N_NOTES];
  // Requested velocity for each note
static  midictrl_t m_chan[N_NOTES];
  // Requested MIDI channel for each note
static uint8_t m_highest, m_lowest;
  // Highest and lowest requested notes

static const uint8_t NO_NOTE = 0xFF;
static const uint8_t PERC_NOTE = 0xFE;
static uint8_t m_playing[MAX_VOICES];
  // Which note each voice is playing

static const uint8_t NO_VOICE = 0xFF;
static uint8_t m_voiceNo[N_NOTES];
  // Which voice is playing each note
  

static bool startNote(ushort idx) {
  for (ushort i = 0; i < MAX_VOICES; i++) {
    if (m_playing[i] == NO_NOTE) {
      voices[i].start(MIDI_MIN + idx, m_velocity[idx], m_chan[idx]);
      m_playing[i] = idx;
      m_voiceNo[idx] = i;
      return true;
    }
  }
  return false;
}
  
static bool startPercussion(note_t note) {
  ushort i;
  for (i = 0; i < MAX_VOICES; i++) {
    if (m_playing[i] == NO_NOTE || m_playing[i] == PERC_NOTE) {
      if (note >= PERC_MIN && note <= PERC_MAX) {
        voices[i].startFX(perc_params[note-PERC_MIN]);
        m_playing[i] = PERC_NOTE;
      }
      return true;
    }        
  }
  return false;
}
    
static bool stopNote(ushort idx) {
  uint8_t v = m_voiceNo[idx];
  if (v != NO_VOICE) {
    voices[v].stop();
    m_playing[v] = NO_NOTE;
    m_voiceNo[idx] = NO_VOICE;
    return true;
  }
  return false;
}

static void stopOneNote() {
  uint8_t v, chosen = NO_NOTE;

  // At this point we have run out of voices.
  // Pick a voice and stop it. We leave a voice alone
  // if it's playing the highest requested note. If it's
  // playing the lowest requested note we look for a 'better'
  // note, but stop it if none found.

  for (v = 0; v < MAX_VOICES; v++) {
    uint8_t idx = m_playing[v];
    if (idx == NO_NOTE) {// Uh? Perhaps called by mistake.
      return;
    }

    if (idx == m_highest) {
      continue;
    }

    if (idx == PERC_NOTE) {
      continue;
    }
      
    chosen = idx;
    if (idx != m_lowest) {
      break;
    }
    // else keep going, we may find a better one
  }

  if (chosen != NO_NOTE) {
    stopNote(chosen);
  }
}

static void updateRequestedNotes() {
  m_highest = m_lowest = NO_NOTE;
  ushort i,j;
    
  // Check highest requested note is playing
  // Return true if note was restarted; false if already playing 
  for (i = 0; i < REQ_MAP_SIZE; i++) {
    uint8_t req = m_requestMap[i];
    if (req == 0) {
      continue;
    }

    for (j = 0; j < 8; j++) {
      if (req & (1 << j)) {
        ushort idx = i*8 + j;
        if (m_lowest == NO_NOTE || m_lowest > idx) {
          m_lowest = idx;
        }
        if (m_highest==NO_NOTE || m_highest < idx)  {
          m_highest = idx;
        }
      }
    }
  }
}

static bool restartANote() {
  if (m_highest != NO_NOTE && m_voiceNo[m_highest] == NO_VOICE) {
    return startNote(m_highest);
  }

  if (m_lowest != NO_NOTE && m_voiceNo[m_lowest] == NO_VOICE) {
    return startNote(m_lowest);
  }

  return false;
}
  
static void synth_init () {
  ushort i;

  for (i = 0; i < REQ_MAP_SIZE; i++) {
    m_requestMap[i] = 0;
  }

  for (i = 0; i < N_NOTES; i++) {
    m_velocity[i] = 0;
    m_voiceNo[i] = NO_VOICE;
  }
    
  for (i = 0; i < MAX_VOICES; i++) {
    m_playing[i] = NO_NOTE;
  }
    
  m_highest = m_lowest = NO_NOTE;
}

static void noteOff(midictrl_t chan, note_t note, midictrl_t vel) {
  if (chan == PERC_CHANNEL || note < MIDI_MIN || note > MIDI_MAX) {
    return; // Just ignore it
  }

  ushort idx = note - MIDI_MIN;

  m_requestMap[idx/8] &= ~(1 << (idx & 7));
  m_velocity[idx] = 0;
  updateRequestedNotes();
    
  if (stopNote(idx)) {
    restartANote();
  }
}

static void noteOn(midictrl_t chan, note_t note, midictrl_t vel) {
  if (vel == 0) {
    noteOff(chan, note, 0);
    return;
  }

  if (chan == PERC_CHANNEL) {
    if (!startPercussion(note)) {
      stopOneNote();
      startPercussion(note);
    }
    return;
  }
    
  // Regular note processing now
    
  if (note < MIDI_MIN || note > MIDI_MAX) {
    return; // Just ignore it
  }

  ushort idx = note - MIDI_MIN;
    
  if (m_voiceNo[idx] != NO_VOICE) {
    return; // Already playing. Ignore this request.
  }

  m_requestMap[idx/8] |= 1 << (idx & 7);
  m_velocity[idx] = vel;
  m_chan[idx] = chan;
  updateRequestedNotes();
    
  if (!startNote(idx)) {
     stopOneNote();
     startNote(idx);
  }
}
  
  
static void update100Hz() {
  for (ushort i = 0; i < MAX_VOICES; i++) {
    voices[i].update100Hz();
    if (m_playing[i] == PERC_NOTE && ! (voices[i].isPlaying())) {
      m_playing[i] = NO_NOTE;
      restartANote();
    }        
  }
}

// Main code ----------------------------------------------

static unsigned long lastUpdate = 0;

void setup() {
  // Hold in reset while we set up the reset
  pinMode(nRESET, OUTPUT);
  digitalWrite(nRESET, LOW);

  pinMode(clkOUT, OUTPUT);
  digitalWrite(clkOUT, LOW);
  clockSetup();

  pinMode(BC2_A, OUTPUT);
  digitalWrite(BC2_A, LOW); // BC2 low
  pinMode(BDIR_A, OUTPUT);
  digitalWrite(BDIR_A, LOW); // BDIR low

  pinMode(BC2_B, OUTPUT);
  digitalWrite(BC2_B, LOW); // BC2 low
  pinMode(BDIR_B, OUTPUT);
  digitalWrite(BDIR_B, LOW); // BDIR low

  for (ushort i = 0; i < 8; i++) {
    pinMode(dbus[i], OUTPUT);
    digitalWrite(dbus[i], LOW); // Set bus low
  }

  delay(100);
  digitalWrite(nRESET, HIGH); // Release Reset
  delay(10);

  lastUpdate = millis();
  
  psg.init();
  for (ushort i = 0; i < MAX_VOICES; i++) {
    voices[i].init(i);
  }
  synth_init();

#ifdef DEBUG
    Serial.begin(115200);
#endif

#ifdef SERIALMIDI
  // Initiate MIDI communications, listen to all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);
#endif
}

void handleMidiMessage(midiEventPacket_t &rx) {
  if (rx.header==0x9) {// Note on
    noteOn(rx.byte1 & 0xF, rx.byte2, rx.byte3);
  }
  else if (rx.header==0x8) {// Note off
    noteOff(rx.byte1 & 0xF, rx.byte2, rx.byte3);
  }
  else if (rx.header==0xB) {// Control Change
    if (rx.byte2 == 0x78 || rx.byte2 == 0x79 || rx.byte2 == 0x7B) {// AllSoundOff, ResetAllControllers, or AllNotesOff
      // Kill Voices
      for (ushort i = 0; i < MAX_VOICES; i++) {
        voices[i].kill();
      }
    }
  }
}

void loop() {
  midiEventPacket_t rx;

#ifdef USBMIDI
  rx = MidiUSB.read();

#ifdef DEBUG
  //MIDI debugging
  if (rx.header != 0) {
    Serial.print("Received USB: ");
    Serial.print(rx.header, HEX);
    Serial.print("-");
    Serial.print(rx.byte1, HEX);
    Serial.print("-");
    Serial.print(rx.byte2, HEX);
    Serial.print("-");
    Serial.println(rx.byte3, HEX);
  }
#endif

  handleMidiMessage(rx);
#endif

#ifdef SERIALMIDI
  //Check for serial MIDI messages
  //MIDI.read();
  while (MIDI.read()) {
    // Create midiEventPacket_t
    rx = 
      {
        byte(MIDI.getType() >>4), 
        byte(MIDI.getType() | ((MIDI.getChannel()-1) & 0x0f)), /* getChannel() returns values from 1 to 16 */
        MIDI.getData1(), 
        MIDI.getData2()
      };

#ifdef DEBUG
    //MIDI debugging
    if (rx.header != 0) {
      Serial.print("Received MIDI: ");
      Serial.print(rx.header, HEX);
      Serial.print("-");
      Serial.print(rx.byte1, HEX);
      Serial.print("-");
      Serial.print(rx.byte2, HEX);
      Serial.print("-");
      Serial.println(rx.byte3, HEX);
    }
#endif

    handleMidiMessage(rx);
  }
#endif

  unsigned long now = millis();
  if ((now - lastUpdate) > 10) {
    update100Hz();
    lastUpdate += 10;
  }
  
  psg.update();
}
