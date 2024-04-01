#import "build/symbols"

10 ' PCF8574 Exemple
55 HIMEM#7FFF:CLOAD"" ' Load module
60 PRINT "I2C PCF8574 Read"
70 PRINT "Plug I2C module and press any key."
80 GET A$
100 CALL PCF8574read
110 V=PEEK(PCF8574value)
120 PRINT V
130 WAIT 100
140 GOTO 100
