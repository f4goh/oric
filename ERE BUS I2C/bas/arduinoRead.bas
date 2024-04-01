#import "build/symbols"

10 ' ARDUINO Exemple
55 HIMEM#7FFF:CLOAD"" ' Load module
60 PRINT "I2C ARDUINO Read"
70 PRINT "Plug I2C module and press any key."
80 GET A$
100 POKE ARDUINOctrl,10
110 CALL ARDUINOread
120 V=PEEK(ARDUINOvalue)
130 PRINT V
140 WAIT 100
150 GOTO 110
