/* -----------------------------------------------------------------
   AY3891x Library Example Sketch 3: Simple Tone Generator
   https://github.com/Andy4495/AY3891x

   This code will play through the middle-octave notes C4 to C5.
   It assumes a 1 MHz clock input to the AY-3-891x chip.

   There is hardware-specific code to have the microcontroller
   generate the 1 MHz clock. If you are not using an Atmega328 or 32U4
   device, then you will need to provide your own code or clock signal
   to generate the required clock, and comment out
   // #define HARDWARE_GENERATED_CLOCK

   12/21/20 - A.T. - Original
   08/24/21 - A.T. - Support PROGMEM for Notes[] array

*/

#include "AY3891x.h"
#include "AY3891x_sounds.h"  // Contains the divisor values for the musical notes

// Be sure to use the correct pin numbers for your setup.
//          DA7, DA6, DA5, DA4, DA3, DA2, DA1, DA0, BDIR, BC2, BC1
//AY3891x psg( A3,   8,   7,   6,   5,   4,   2,   3,   A2,  A1,  A0);

const byte notUsed = 255;    // the library considers 255 to mean pin not used when configuring AY3891x pins

//          these are the Nano pins used in the schematic to interface with the AY-3-8910
//          DA7, DA6, DA5, DA4, DA3, DA2, DA1, DA0, BDIR, BC2, BC1, A9, A8, Reset, Clock
AY3891x psg( A3,   8,   7,   6,   5,   4,   3,   2,   A2,  A1,  A0, notUsed, notUsed, A4, notUsed);

#ifdef ARDUINO_ARCH_AVR
#define HARDWARE_GENERATED_CLOCK  // Comment this line if not using supported microcontroller
#ifdef HARDWARE_GENERATED_CLOCK
// The following code generates a 1 MHz 50% duty cycle output to be used
// as the clock signal for the AY-3-891x chip.
// Note that the following code is hardware-specific. It works on certain Atmega
// chips (including Arduino UNO), but will not work on all microcontrollers
// without modification
static const byte clkOUT = 9;
const byte DIVISOR = 7; // Set for 1MHz clock
static void clockSetup()
{
  TCCR1A = (1 << COM1A0);
  TCCR1B = (1 << WGM12) | (1 << CS10);
  TCCR1C = 0;
  TIMSK1 = 0;
  OCR1AH = 0;
  OCR1AL = DIVISOR;
}
#endif
#endif

const int notes_to_play[] = {
  C_4, D_4, E_4, F_4, G_4, A_4, B_4, C_5,
  C_4, E_4, G_4, C_5, G_4, E_4, C_4
};

void setup() {
#ifdef HARDWARE_GENERATED_CLOCK
  // Hardware-specific microcontroller code to generate a clock signal for the AY-3-891x chip
  pinMode(clkOUT, OUTPUT);
  digitalWrite(clkOUT, LOW);
  clockSetup();
#endif
  Serial.begin(38400);
  Serial.println("");
  Serial.println("AY-3-891x Sound Chip Library Example 3: Simple Tone Generator.");

  psg.begin();
  // psg.setAddress(TheChipsAddress);   // Only need this for special-ordered chips with non-default address.

  // Use less than max amplitude, in case external amp can't handle the higher level (start low and increase after testing)
  psg.write(AY3891x::ChA_Amplitude, 0x04); // Lower amplitude
  psg.write(AY3891x::ChB_Amplitude, 0x08); // Mid amplitude
  psg.write(AY3891x::Enable_Reg, ~(MIXER_TONE_A_DISABLE | MIXER_TONE_B_DISABLE));   // Enable Channel A and B tone generator output

  for (byte i = 0; i < sizeof(notes_to_play) / sizeof(notes_to_play[0]); i++) {
    Serial.print("Playing note freq: ");
    Serial.println(1000000UL/16/pgm_read_word(&Notes[notes_to_play[i]]));
    psg.write(AY3891x::ChA_Tone_Period_Coarse_Reg, pgm_read_word(&Notes[notes_to_play[i]]) >> 8);
    psg.write(AY3891x::ChA_Tone_Period_Fine_Reg, pgm_read_word(&Notes[notes_to_play[i]]) & TONE_GENERATOR_FINE);
    psg.write(AY3891x::ChB_Tone_Period_Coarse_Reg, pgm_read_word(&Notes[notes_to_play[i]]) >> 8);
    psg.write(AY3891x::ChB_Tone_Period_Fine_Reg, pgm_read_word(&Notes[notes_to_play[i]]) & TONE_GENERATOR_FINE);
    delay(1000);
  }

  psg.write(AY3891x::Enable_Reg, MIXER_ALL_DISABLED);   // Disable Tone Generators
  
  playDemoSounds();  // demo the tones and noise at startup
  
}

void loop() {

}
void playDemoSounds() {

  Serial.println("Playing demo tones...");

  // turn off tones on all channels
  psg.write(AY3891x::Enable_Reg, (MIXER_TONE_A_DISABLE | MIXER_TONE_B_DISABLE | MIXER_TONE_C_DISABLE));

  psg.write(AY3891x::Enable_Reg, ~(MIXER_TONE_A_DISABLE));   // enable tone Ch A
  psg.write(AY3891x::ChA_Amplitude, 0x08);                   // set Ch A volume

  // play tones on Ch A
  psg.write(AY3891x::ChA_Tone_Period_Coarse_Reg, pgm_read_word(&Notes[C_3]) >> 8);
  psg.write(AY3891x::ChA_Tone_Period_Fine_Reg, pgm_read_word(&Notes[C_3]) & TONE_GENERATOR_FINE);
  delay(500);
  psg.write(AY3891x::ChA_Tone_Period_Coarse_Reg, pgm_read_word(&Notes[D_3S]) >> 8);
  psg.write(AY3891x::ChA_Tone_Period_Fine_Reg, pgm_read_word(&Notes[D_3S]) & TONE_GENERATOR_FINE);
  delay(500);

  // keep playing final tone from Ch A and add Ch B tones
  psg.write(AY3891x::Enable_Reg, ~(MIXER_TONE_A_DISABLE | MIXER_TONE_B_DISABLE));   // enable tone Ch A + B
  psg.write(AY3891x::ChB_Amplitude, 0x08);                                          // set Ch B volume

  // play tones on Ch B
  psg.write(AY3891x::ChB_Tone_Period_Coarse_Reg, pgm_read_word(&Notes[F_3S]) >> 8);
  psg.write(AY3891x::ChB_Tone_Period_Fine_Reg, pgm_read_word(&Notes[F_3S]) & TONE_GENERATOR_FINE);
  delay(500);
  psg.write(AY3891x::ChB_Tone_Period_Coarse_Reg, pgm_read_word(&Notes[A_3]) >> 8);
  psg.write(AY3891x::ChB_Tone_Period_Fine_Reg, pgm_read_word(&Notes[A_3]) & TONE_GENERATOR_FINE);
  delay(500);

  // keep playing final tone from Ch B, add Ch C tones
  psg.write(AY3891x::Enable_Reg, ~(MIXER_TONE_B_DISABLE | MIXER_TONE_C_DISABLE));   // enable tone Ch B + C
  psg.write(AY3891x::ChC_Amplitude, 0x08);                                          // set Ch C volume

  // play tones on Ch C
  psg.write(AY3891x::ChC_Tone_Period_Coarse_Reg, pgm_read_word(&Notes[C_4]) >> 8);
  psg.write(AY3891x::ChC_Tone_Period_Fine_Reg, pgm_read_word(&Notes[C_4]) & TONE_GENERATOR_FINE);
  delay(500);
  psg.write(AY3891x::ChC_Tone_Period_Coarse_Reg, pgm_read_word(&Notes[D_4S]) >> 8);
  psg.write(AY3891x::ChC_Tone_Period_Fine_Reg, pgm_read_word(&Notes[D_4S]) & TONE_GENERATOR_FINE);
  delay(1000);

  // disable tones
  psg.write(AY3891x::Enable_Reg, (MIXER_TONE_A_DISABLE | MIXER_TONE_B_DISABLE | MIXER_TONE_C_DISABLE));
  psg.write(AY3891x::ChA_Amplitude, 0x00);
  psg.write(AY3891x::ChB_Amplitude, 0x00);
  psg.write(AY3891x::ChC_Amplitude, 0x00);
  delay(250);

  // noise with envelope control
  Serial.println("Playing demo noise...");

  // lower pitched noise fade in
  psg.write(AY3891x::Enable_Reg, ~(MIXER_NOISES_DISABLE));        // enable all noise channels
  psg.write(AY3891x::Noise_Period_Reg, 0x1f);                     // configure noise pitch (lower values = higher pitch)
  psg.write(AY3891x::Env_Period_Coarse_Reg, 0x15);                // envelope time control - higher values = longer duration of effect eg slow volume fade up
  psg.write(AY3891x::Env_Shape_Cycle, ENVELOPE_CONTROL_ATTACK);   // attack envelope fades noise in

  psg.write(AY3891x::ChA_Amplitude, 0x10);                        // set noise channels to have volume controlled by envelope instead of fixed amplitudes (0x10)
  psg.write(AY3891x::ChB_Amplitude, 0x10);
  psg.write(AY3891x::ChC_Amplitude, 0x10);
  delay(1000);
  psg.write(AY3891x::Env_Period_Coarse_Reg, 0x00);                // reset envelope to shortest time period (disable effect?)
  delay(250);

  // higher pitched noise fade out
  psg.write(AY3891x::Enable_Reg, ~(MIXER_NOISES_DISABLE));        // enable all noise channels
  psg.write(AY3891x::Noise_Period_Reg, 0x05);                     // configure noise pitch (lower values = higher pitch)
  psg.write(AY3891x::Env_Period_Coarse_Reg, 0xf);                 // envelope time control - higher values = longer duration of effect eg slow volume fade up
  psg.write(AY3891x::Env_Shape_Cycle, ENVELOPE_CONTROL_HOLD);     // hold envelope fades noise out

  psg.write(AY3891x::ChA_Amplitude, 0x10);                        // set noise channels to have volume controlled by envelope instead of fixed amplitudes (0x10)
  psg.write(AY3891x::ChB_Amplitude, 0x10);
  psg.write(AY3891x::ChC_Amplitude, 0x10);
  delay(1000);
  psg.write(AY3891x::Env_Period_Coarse_Reg, 0x00);                // reset envelope to shortest time period (disable effect?)


  // high pitched tapping on metal object or 8 bit video game "walking" sound
  psg.write(AY3891x::Enable_Reg, ~(MIXER_NOISES_DISABLE));        // enable all noise channels
  psg.write(AY3891x::Noise_Period_Reg, 0x02);                     // configure noise pitch (lower values = higher pitch)
  psg.write(AY3891x::Env_Period_Coarse_Reg, 0x04);                // envelope time control - higher values = longer duration of effect eg slow volume fade up
  psg.write(AY3891x::Env_Shape_Cycle, ENVELOPE_CONTROL_CONTINUE); // continue envelope does what...repeat?

  psg.write(AY3891x::ChA_Amplitude, 0x10);                        // set noise channels to have volume controlled by envelope instead of fixed amplitudes (0x10)
  psg.write(AY3891x::ChB_Amplitude, 0x10);
  psg.write(AY3891x::ChC_Amplitude, 0x10);
  delay(1000);
  psg.write(AY3891x::Env_Period_Coarse_Reg, 0x00);                // reset envelope to shortest time period (disable effect?)


  // lower pitched noise - explosion
  psg.write(AY3891x::Enable_Reg, ~(MIXER_NOISES_DISABLE));         // enable all noise channels
  psg.write(AY3891x::Noise_Period_Reg, 0x1f);                      // configure noise pitch (lower values = higher pitch)
  psg.write(AY3891x::Env_Period_Coarse_Reg, 0x13);                 // envelope time control - higher values = longer duration of effect eg slow volume fade up
  psg.write(AY3891x::Env_Shape_Cycle, ENVELOPE_CONTROL_ALTERNATE); // alternate envelope does what...?

  psg.write(AY3891x::ChA_Amplitude, 0x10);                         // set noise channels to have volume controlled by envelope instead of fixed amplitudes (0x10)
  psg.write(AY3891x::ChB_Amplitude, 0x10);
  psg.write(AY3891x::ChC_Amplitude, 0x10);
  delay(1000);
  psg.write(AY3891x::Env_Period_Coarse_Reg, 0x00);                 // reset envelope to shortest time period (disable effect?)

  // just flat noise
  psg.write(AY3891x::Enable_Reg, ~(MIXER_NOISES_DISABLE));         // enable all noise channels
  psg.write(AY3891x::Noise_Period_Reg, 0x1a);                      // configure noise pitch (lower values = higher pitch)

  psg.write(AY3891x::ChA_Amplitude, 0x08);                         // set noise channels to have fixed volume level 0x08
  psg.write(AY3891x::ChB_Amplitude, 0x08);
  psg.write(AY3891x::ChC_Amplitude, 0x08);
  delay(1500);

  // disable noise
  psg.write(AY3891x::Enable_Reg, (MIXER_NOISES_DISABLE));

  // turn down channel volume, no more envelope control
  psg.write(AY3891x::ChA_Amplitude, 0x00);
  psg.write(AY3891x::ChB_Amplitude, 0x00);
  psg.write(AY3891x::ChC_Amplitude, 0x00);
}
