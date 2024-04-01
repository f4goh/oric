#import "build/symbols"

10 ' PCF8591 Exemple
55 HIMEM#7FFF:CLOAD"" ' Load module
60 PRINT "I2C PCF8591 dac"
70 PRINT "Plug I2C module and press any key."
80 GET A$
90 PRINT "Sawtooth Generator"
100 FOR N=0 TO 255
110 POKE PCF8591value,N
120 CALL PCF8591dac
130 NEXT N
140 GOTO 100
