#import "build/symbols"

10 ' LM75 Exemple
55 HIMEM#7FFF:CLOAD"" ' Load module
60 PRINT "I2C LM75 1"
70 PRINT "Plug I2C module and press any key."
80 GET A$
100 CALL LM75read
110 V=DEEK(LM75value)
120 PRINT V/256,"DEG"
130 WAIT 100
140 GOTO 100



