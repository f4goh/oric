#import "build/symbols"

10 ' LCD 2x16 & LM75
55 HIMEM#7FFF:CLOAD"" ' Load module
60 PRINT "I2C LCD & LM75"
70 PRINT "Plug I2C module and press any key."
80 GET A$
100 CALL lcdBegin
110 CALL lcdLightOn
120 B$="ceo.oric.org"
130 GOSUB 400
140 POKE lcdX,0
150 POKE lcdY,1
160 CALL lcdCursor
170 B$="Temperature LM75"
180 GOSUB 400
190 POKE lcdX,5
200 POKE lcdY,2
210 CALL lcdCursor
220 CALL LM75read
230 V=DEEK(LM75value)
240 V=V/256
250 PRINT V,"DEG"
260 B$=STR$(V)+ " Deg  "
270 GOSUB 400
280 WAIT 100
290 GOTO 190
400 ADR=lcdTxt
410 FOR N=1 TO LEN(B$)
420 C$=MID$(B$,N,1)
430 POKE ADR,ASC(C$)
440 ADR=ADR+1
450 NEXT N
460 POKE ADR,0
470 CALL lcdPrint
480 RETURN