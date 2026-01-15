#utilitaires PSG AY-3-8910

## ORIC [deinterleave](https://f4goh.github.io/oric/PSG/utils/deinterleave) 

## ORIC [delta_compression](https://f4goh.github.io/oric/PSG/utils/delta_compression) 

## ORIC [generate_header](https://f4goh.github.io/oric/PSG/utils/generate_header) 

## ORIC [lecteur_brut](https://f4goh.github.io/oric/PSG/utils/lecteur_brut) 

## ORIC [lecteur_compresse](https://f4goh.github.io/oric/PSG/utils/lecteur_compresse) 

## ORIC [lecteur_compresseV2](https://f4goh.github.io/oric/PSG/utils/lecteur_compresseV2) 

## ORIC [player](https://f4goh.github.io/oric/PSG/utils/player) 

## ORIC [clavier midi avec google chrome](https://aym-js.emaxilde.net/synth/) 


# Tableau des registres

| Register \ bit | Description                       | B7 | B6 | B5 | B4 | B3 | B2 | B1 | B0 |
|----------------|-----------------------------------|----|----|----|----|----|----|----|----|
| R0             | Channel A Tone Period             | 8-Bit Fine Tune A (8 bits) |
| R1             | Channel A Tone Period             | —  | —  | —  | —  | 4-Bit Coarse Tune A (4 bits) |
| R2             | Channel B Tone Period             | 8-Bit Fine Tune B (8 bits) |
| R3             | Channel B Tone Period             | —  | —  | —  | —  | 4-Bit Coarse Tune B (4 bits) |
| R4             | Channel C Tone Period             | 8-Bit Fine Tune C (8 bits) |
| R5             | Channel C Tone Period             | —  | —  | —  | —  | 4-Bit Coarse Tune C (4 bits) |
| R6             | Noise Period                      | —  | —  | —  | 5-Bit Period Control |
| R7             | Enable (0 = on, 1 = off)          | IN/OUT IOB | IN/OUT IOA | Noise C | Noise B | Noise A | Tone C | Tone B | Tone A |
| R8             | Channel A Envelope / Volume       | — | — | Env | Volume (5 bits) |
| R9             | Channel B Envelope / Volume       | — | — | Env | Volume (5 bits) |
| R10            | Channel C Envelope / Volume       | — | — | Env | Volume (5 bits) |
| R11            | Envelope Period                   | 8-Bit Fine Tune Envelope |
| R12            | Envelope Period                   | — | — | — | — | 4-Bit Coarse Tune Envelope |
| R13            | Envelope Shape / Cycle            | — | — | — | — | CONT | ATT | ALT | HOLD |
| R14            | I/O Port A Data Store             | 8-Bit Parallel I/O Port A |
| R15            | I/O Port B Data Store             | 8-Bit Parallel I/O Port B |













