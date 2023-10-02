# oric-1 PS2 Keyboard and joystick

The GAL 22V10 U15 contains a 3-bit comparator with a specific input (enable) which simulates the pressing of a key.

I compare the 3 bits A, B, C initially intended for the 4051 of the original keyboard with a 3-bit instruction given by the Arduino Nano. The active output at NL0 controls the feedback transistor to the 6522.

As far as column selection is concerned, the GAL 'reduces' the 8 wires to 3 bits ColA, ColB, ColC (operation identical to an inverted 74LS138); this allows me to use one of the Nano's ports directly, to have the state of the selected column (0 to 7) in one instruction and thus reduce the number of wires to manage the joystick.

The trick with the code in the nano is to place the right instruction (line) in the right column depending on the character pressed.

I produced two versions of code for the Arduino Nano:

- Without Timer interrupt: (no repetition if a key is pressed for a long time)
- With Timer interrupt: (repetition if a key is pressed for a long time)

Both versions can handle an AZERTY and QWERTY keyboard, but the source code will have to be recompiled.

The combination of the ctrl+alt+delete keys to manage a reset is not activated in these 2 versions of code, but it is possible to add it.

The arduino Nano also manages a classic 5-switch joystick via a male DB9 connector.

Before loading a game, you'll need to learn how to use it, in the same way as the famous 'protek' interface of the time.

Press F7 to switch to joystick recording mode. Use the F9, F10 and F11 keys to select one of the 50 memory banks.

Press a key on the keyboard, then move the joystick in the desired direction. Repeat for the other keys.
To exit recording mode, press F8. I was able to test Ghost Gobbler successfully.

Keyboard shortcuts

- F1 : ZAP
- F2 : EXPLODE
- F3: SHOOT
- F4: PING
- F5:LOAD
- F6: CSAVE
- F7: Joystick recording mode 
- F8: Exit joystick recording mode 
- F9: Decrements memory bank number by x1
- F10: Increments memory bank number by x1
- F11: Increments memory bank number by 10
- F12 : Swap ROM Oric-1 <-> Oric Atmos, then reset.

#note

The Arduino nano bootloader has been reprogrammed to disable flashing of the LED connected to GPIO 13.

GPIO13 is used to control the oric power-up relay. It is essential that this GPIO13 output is level throughout the initialisation of the Arduino Nano.

to reprogram the arduino nano bootloader, use optiboot flash and minicore addon

https://github.com/mcudude/optiboot_flash

the commande
```console
make atmega328p AVR_FREQ=16000000L BAUD_RATE=115200 LED=B5 LED_START_FLASHES=0 UART=0
```
generate this file 

optiboot_flash_atmega328p_UART0_115200_16000000L_B5.hex

to program the hex file into the Arduino nano follow this [tutorial](https://electropeak.com/learn/use-arduino-as-isp-to-burn-bootloader-on-avr-microcontrollers/) 






