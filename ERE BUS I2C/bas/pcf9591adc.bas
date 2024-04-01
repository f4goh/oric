#import "build/symbols"

10 ' PCF8591 Exemple
55 HIMEM#7FFF:CLOAD"" ' Load module
60 PRINT "I2C PCF8591 Read"
70 PRINT "Plug I2C module and press any key."
80 GET A$
100 POKE PCF8591ctrl,0
110 CALL PCF8591adc
120 V=PEEK(PCF8591value)
130 PRINT V
140 WAIT 100
150 GOTO 110
