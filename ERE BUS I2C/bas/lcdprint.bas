#import "build/symbols"

10 ' LCD 2x16 Exemple
55 HIMEM#7FFF:CLOAD"" ' Load module
60 PRINT "I2C LCD test"
70 PRINT "Plug I2C module and press any key."
80 GET A$
110 CALL lcdBegin
115 CALL lcdLightOn
120 B$="ceo.oric.org"
130 GOSUB 200
140 POKE lcdX,2
150 POKE lcdY,1
160 CALL lcdCursor
170 GOSUB 200
180 END
200 ADR=lcdTxt
210 FOR N=1 TO LEN(B$)
220 C$=MID$(B$,N,1)
230 POKE ADR,ASC(C$)
240 ADR=ADR+1
250 NEXT N
260 POKE ADR,0
270 CALL lcdPrint
280 RETURN
