;https://osdk.org/index.php?page=documentation&subpage=oricutron
;https://en.wikipedia.org/wiki/Varicode
;https://www.masswerk.at/6502/6502_instruction_set.html
;copyright F4GOH Juin 2025 CC-NC-SA

;#define PRA_ADDRESS $380  ;->$38C
;#define DDRA_ADDRESS $381 ;->$38D
;#define PRB_ADDRESS $382  ;->$38E
;#define DDRB_ADDRESS $383 ;->$38F

#define PRA_ADDRESS $38C
#define DDRA_ADDRESS $38D
#define PRB_ADDRESS $38E
#define DDRB_ADDRESS $38F

#define NB_SYMB 162

;deplacer les flags en page 0

	.zero
	*= $50
;// Some two byte values
_zp_start_

;dds variables
regPhase
	.byt 0
_regDds
	.byt 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ; 4 derniers toujours à zero pour desactiver le DDS

;edge reg
regSync
	.dsb 1

signlett .byt 1		;flag pour passage lettres/chiffres

;psk, cw
charLen
	.byt 0;9 A
charMsb
	.byt 0
charLsb
	.byt 0;$5f
	
	
_zp_end_	

	.text    

 *=$8000

; Called when the assembler is loaded.
; Can be used to perfom initializations

;-------------------------
; INIT 6821 
;-------------------------


_EntryPoint
    lda #$0
    sta DDRA_ADDRESS
	sta DDRB_ADDRESS
    lda #$FF			;PORTA en sortie
    sta PRA_ADDRESS
    lda #$4
	sta DDRA_ADDRESS 
	lda #$F				;PORTB 4 bits de poids faible en sortie, 4 bits de poids fort en entrée
	sta PRB_ADDRESS
    lda #$4
	sta DDRB_ADDRESS
    lda #$0
    sta PRA_ADDRESS  ;clear PORT A et B
	sta PRB_ADDRESS
	sta regSync
	jmp pulseRst
	rts
;-------------------------
; DDS function
;-------------------------

pulseClk  
	lda PRB_ADDRESS   
	ora #$01          
	sta PRB_ADDRESS
	and #$FE  
	sta PRB_ADDRESS
    rts

pulseFq
	lda PRB_ADDRESS   
	ora #$02          
	sta PRB_ADDRESS
	and #$FD  
	sta PRB_ADDRESS
    rts

pulseRst
	lda PRB_ADDRESS   
	ora #$04
	sta PRB_ADDRESS
	and #$FB  
	sta PRB_ADDRESS
    rts

pttOn
	lda PRB_ADDRESS   
	ora #$08
	sta PRB_ADDRESS
    rts

pttOff
	lda PRB_ADDRESS   
	and #$F7  
	sta PRB_ADDRESS
    rts

_ddsOn ;calibation
.(
	sei
	jsr pttOn
	ldy #0
	jsr _writeDds
	cli
	rts
.)

_ddsOff ;calibation
.(
	sei
	jsr pttOff
	ldy #16
	jsr _writeDds
	cli
	rts
.)


_writeDds
.(
	;ldy #0 ;ici on peut changer l'index de début 0 4 8 12 
	ldx #4
	lda regPhase		;registre de phase et ctrl utilisé pour le psk31
	sta PRA_ADDRESS
	jsr pulseClk
loop
	lda _regDds,y
	sta PRA_ADDRESS
	jsr pulseClk
	iny
	dex	
	bne loop
	jsr pulseFq
	rts
.)	



;-------------------------
; WAIT AtTiny85 function
;-------------------------


_waitNext
.(	
loop:
    lda PRB_ADDRESS
    and #$10
    cmp regSync
    beq loop           ; Pas de changement → boucle
    sta regSync        ; Màj état précédent
    cmp #$10
    bne loop           ; Si pas passé à 1 → pas front montant
    rts
.)	

_waitPsk
.(	
loop:
    lda PRB_ADDRESS
    and #$80
    cmp regSync
    beq loop           ; Pas de changement → boucle
    sta regSync        ; Màj état précédent
    cmp #$80
    bne loop           ; Si pas passé à 1 → pas front montant
    rts
.)	

_waitRtty  ;; problème attiny85 modulo minutes
.(
loop:
    lda PRB_ADDRESS
    and #$40
    cmp regSync
    beq loop           ; Pas de changement → boucle
    sta regSync        ; Màj état précédent
    cmp #$40
    bne loop           ; Si pas passé à 1 → pas front montant
    rts
.)	
	
_waitRttyBis
.(
loop
	lda PRB_ADDRESS
	and #$40
	ora regSync
	cmp #$40
	beq front
	asl
	and #$C0
	sta regSync
	jmp loop
front
	asl
	and #$C0
	sta regSync
	rts
.)	

_waitSync  ;; problème attiny85 modulo minutes
.(
loop:
    lda PRB_ADDRESS
    and #$20
    cmp regSync
    beq loop           ; Pas de changement → boucle
    sta regSync        ; Màj état précédent
    cmp #$20
    bne loop           ; Si pas passé à 1 → pas front montant
    rts
.)	



;-------------------------
; WSPR function
;-------------------------


_sendWspr
.(
	sei
	jsr _waitSync
	jsr pttOn
	ldy #0
	ldx #NB_SYMB
loop:
	tya
	pha
	txa
	pha
	lda wsprSymb,y
	tay
	lda offsetReg,y
	tay
	jsr _writeDds
	;jsr _waitNext		;la tempo delivrée par l'attiny85 est variable à revoir
	jsr waitTempoWspr
	pla
	tax
	pla
	tay
	iny
	dex
	bne loop
	ldy #16			;dds off
	jsr _writeDds
	jsr pttOff
	cli
	rts
.)	

waitTempoWspr		;tempo de 682ms
.(
    lda #$03        ; 3 itérations externes
    sta outer       ; compteur externe
outer_loop:
    ldx #$B1
loop_x:
    ldy #$FF        ; 255
loop_y:
    dey             ; 2 cycles
    bne loop_y      ; 3 cycles
    dex             ; 2 cycles
    bne loop_x      ; 3 cycles
    dec outer         ; 5 cycles
    bne outer_loop  ; 3 cycles
    rts             ; 6 cycles
.)

outer
	.byt 0

offsetReg
	.byt 0,4,8,12

wsprSymb
	.byt 3, 3, 0, 0, 0, 2, 0, 2, 1, 2, 0, 2, 3, 3, 1, 0, 2, 0, 3, 0, 0, 1, 2, 1, 1, 3, 1, 0, 2, 0, 0, 2, 0, 2, 3, 2, 0, 1, 2, 3, 0, 0, 0, 0
    .byt 2, 2, 3, 0, 1, 3, 2, 0, 3, 3, 2, 3, 2, 2, 2, 1, 3, 0, 1, 0, 2, 0, 2, 1, 1, 2, 1, 0, 3, 2, 1, 2, 3, 0, 0, 1, 2, 0, 1, 0, 1, 3, 0, 0
    .byt 0, 1, 3, 2, 1, 2, 1, 2, 2, 2, 3, 0, 0, 2, 2, 2, 3, 2, 0, 1, 2, 0, 3, 3, 1, 2, 3, 3, 0, 2, 1, 3, 0, 3, 2, 2, 0, 3, 3, 1, 2, 0, 0, 0
    .byt 2, 1, 0, 1, 2, 0, 3, 3, 2, 2, 0, 2, 2, 2, 2, 1, 3, 2, 1, 0, 1, 1, 2, 0, 0, 3, 1, 2, 2, 2

;-------------------------
; RTTY functions
;-------------------------


_sendRtty:
.(
    sei                  ; Désactive les interruptions (optionnel)
	jsr pttOn
    LDY #0               ; Initialise l'index
    LDA #1
    STA signlett         ; Mode lettre par défaut
	LDA #31              ; Switch vers mode lettre une seule fois au démarrage ?
    JSR rttyTxByte
	;LDA #31              ; Switch vers mode lettre
    ;JSR rttyTxByte
	
nextChar:
    LDA _beaconText,Y    ; Charge le prochain caractère    
    BEQ endLoop          ; Fin de chaîne détectée (terminateur nul)

    CMP #10
    BEQ sendLF
    CMP #13
    BEQ sendCR
    CMP #32
    BEQ sendSpace

    CMP #33
    BCC increment         ; Caractère < 33, non imprimable → ignorer
    CMP #91
    BCS increment         ; Caractère ≥ 91 → hors plage → ignorer

    SEC
    SBC #32              ; Ajuste l’index pour la table
    TAX                  ; Index RTTY dans X

    CPX #33
    BCS isLetter
    JMP isSymbol

isLetter:
    LDA signlett
    BNE sendCode         ; Déjà en mode lettre
    LDA #1
    STA signlett
    LDA #31              ; Switch vers mode lettre
    JSR rttyTxByte
    JMP sendCode

isSymbol:
    LDA signlett
    BEQ sendCode         ; Déjà en mode symbole
    LDA #0
    STA signlett
    LDA #27              ; Switch vers mode symbole
    JSR rttyTxByte

sendCode:
    LDA tableRtty,X      ; Code RTTY depuis la table
    JSR rttyTxByte
    JMP increment

sendLF:
    LDA #8               ; Code RTTY pour Line Feed (à valider selon table)
    JSR rttyTxByte
    JMP increment

sendCR:
    LDA #2               ; Code RTTY pour Carriage Return (à valider)
    JSR rttyTxByte
    JMP increment

sendSpace:
    LDA #4               ; Code RTTY pour espace (à valider)
    JSR rttyTxByte

increment:
    INY
    JMP nextChar

endLoop:
	ldy #16			;dds off
	jsr _writeDds
	jsr pttOff
    cli                  ; Réactive les interruptions
    RTS
.)	




rttyTxByte 
.(
	stx regx ;sauve
	sty regy
	asl			;bit de start
	ora #$c0	;bit de stop
    ldy #8        ; Nombre de bits à envoyer
    ;lda #11       ; Valeur source
loop:
    lsr           ; Décale le bit dans Carry
	pha			  ;sauve a et y
	tya
	pha	
    bcc zeroBit   ; Si Carry = 0, bit vaut 0
	ldy #4		  ; Bit = 1
    jsr _writeDds    
    jmp nextBit
zeroBit:
	ldy #0		  ; Bit = 0
    jsr _writeDds
nextBit:
	;jsr _waitRtty
	jsr waitTempoRtty
	;jsr pulsePtt
    pla			  ;restore y et a
	tay
	pla
    dey
    bne loop
	ldx regx  ;restore
	ldy regy
    rts
.)

regx .dsb 1
regy .dsb 1

waitTempoRtty
.(
    ldx #$CE        ; 206
outer_loop:
    ldy #$15        ; 21
inner_loop:
    dey             ; 2 cycles
    bne inner_loop  ; 3 (taken) / 2 (not taken) cycles
    dex             ; 2 cycles
    bne outer_loop  ; idem
    rts
.)	

tableRtty
;      SPACE  !   "    #  $   %   &   '   (    )   *   +   ,  -    .   /  0   1    2   3   4   5  6   7   8   9   :   ;  >
	.byt $04,$0D,$11,$14,$09,$00,$1A,$0B,$0F,$12,$00,$00,$0C,$03,$1C,$1D,$16,$17,$13,$01,$0A,$10,$15,$07,$06,$18,$0E,$1E,$00
;         =   >   ?   @    A   B  C   D   E   F   G   H   I   J   K   L   M   N   O   P   Q   R   S   T   U   V   W   X  Y   Z 
	.byt $00,$00,$19,$00,$03,$19,$0E,$09,$01,$0D,$1A,$14,$06,$0B,$0F,$12,$1C,$0C,$18,$16,$17,$0A,$05,$10,$07,$1E,$13,$1D,$15,$11

;-------------------------
; PSK function
;-------------------------

_sendPsk
.(
	sei
	jsr pttOn
	lda #0
	sta regPhase
	jsr pskIdle
	LDY #0               ; Initialise l'index
loop
    LDA _beaconText,Y    ; Charge le prochain caractère    
    beq fin				; Fin de chaîne détectée (terminateur nul)
	jsr pskTxByte
	iny
	jmp loop
fin
	jsr pskIdle
	ldy #16			;dds off
	jsr _writeDds
	jsr pttOff
	cli
	rts
.)


pskTxByte	;dans a la valeur ascii
.(
	sty regy
	tay
	lda PskVaricodeLen,Y
	sta charLen
	tya			;remet la valeur
	asl			;fois 2 pour 16 bits
	tay
	lda PskVaricode,y
	sta charLsb
	iny
	lda PskVaricode,y
	sta charMsb
	
	ldy charLen
loop
	lsr charMsb
	ror charLsb
	bcs noChange   ; Si Carry = 1, pas de changement de phase
	lda regPhase
	beq	reverse
	lda #0
	jmp suite
reverse
	lda #$80
suite
	sta regPhase
noChange
	tya
	pha
	ldy #0
    jsr _writeDds
	;jsr _waitPsk
	jsr waitTempoPsk
	pla
	tay
	dey
	bne loop
	ldy regy
	rts
.)





pskIdle
.(
	ldy #50
loop
	lda regPhase
	beq	reverse
	lda #0
	jmp suite
reverse
	lda #$80
suite
	sta regPhase
	tya
	pha
	ldy #0
    jsr _writeDds
	;jsr _waitPsk
	jsr waitTempoPsk
	pla
	tay
	dey
	bne loop
	rts
.)


waitTempoPsk
.(
    ldx #$B6        ; 182
outer_loop:
    ldy #$22        ; 34
inner_loop:
    dey             ; 2 cycles
    bne inner_loop  ; 3 cycles (branch taken)
    dex             ; 2 cycles
    bne outer_loop  ; 3 cycles (branch taken)
    rts
.)

PskVaricodeLen
.byt 12,12,12,12,12,12,12,12,12,10,7,12,12,7,12,12
.byt 12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12
.byt 3,11,11,11,11,12,12,11,10,10,11,11,9,8,9,11
.byt 10,10,10,10,11,11,11,11,11,11,10,11,11,9,11,12
.byt 12,9,10,10,10,9,10,10,11,9,11,11,10,10,10,10
.byt 10,11,10,9,9,11,11,11,11,11,12,11,11,11,12,11
.byt 12,6,9,8,8,4,8,9,8,6,11,10,7,8,6,5
.byt 8,11,7,7,5,8,9,9,10,9,11,12,11,12,12,12

PskVaricode ;Msb du varicode placé en LSB pour decallage vers la droite et récupérer le carry
.word 853,877,733,955,861,1003,989,765,1021,247,23,987,749,31,699,855
.word 957,701,727,983,875,859,731,939,891,763,951,683,747,887,893,1019
.word 1,511,501,351,439,685,885,509,223,239,493,503,87,43,117,491
.word 237,189,183,255,477,437,429,363,427,475,175,379,367,85,471,981
.word 757,95,215,181,173,119,219,191,341,127,383,381,235,221,187,213
.word 171,375,245,123,91,469,347,373,349,445,725,479,495,447,1013,365
.word 1005,13,125,61,45,3,47,109,53,11,431,253,27,55,15,7
.word 63,507,21,29,5,59,111,107,251,93,343,949,443,693,941,695



_beaconText
	;.dsb 100
	;.asc "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-$',!:()#?&/;....",0 
	.asc "  BEACON RTTY BEACON F4GOH ORIC ATMOS BEACON JN07DV33 PSE REPORT F4GOH AT ORANGE.FR.....",0 

;-------------------------
; CW function
;-------------------------
;https://github.com/f4goh/MODULATION/blob/master/MODULATION.cpp


_sendCw
.(
	sei
	jsr pttOn
	lda #0
	sta regPhase	
	LDY #0               ; Initialise l'index
loop
    LDA _cwText,Y    ; Charge le prochain caractère    
    beq fin				; Fin de chaîne détectée (terminateur nul)
    SEC
    SBC #32              ; Ajuste l’index pour la table
	beq espace	;c'est un espace
	jsr cwTxLetter
	iny
	jmp loop
fin	
	jsr pttOff
	cli
	rts
espace
	;ajout tempo espace
	tya
	pha
	jsr tempoCW
	jsr tempoCW
	jsr tempoCW
	jsr tempoCW
	jsr tempoCW
	pla
	tay
	iny
	jmp loop	
.)

cwTxLetter
.(
	sty regy
	tay
	lda lenCW,Y
	sta charLen
	lda cwTable,y
	sta charLsb
	tya 
	
	ldy charLen
	beq fin	;c'est un zero prochaine lettre fin	
loop
	lsr charLsb	
	bcs dash   ; Si Carry = 1, dash
dot	tya
	pha
	ldy #0
	jsr _writeDds
	jsr tempoCW
	ldy #16
	jsr _writeDds
	jsr tempoCW
	jmp suite
dash
	tya
	pha
	ldy #0
	jsr _writeDds
	jsr tempoCW
	jsr tempoCW
	jsr tempoCW
	ldy #16
	jsr _writeDds
	jsr tempoCW
	jmp suite
suite
	pla
	tay
	dey
	bne loop
	;tempo entre deux lettre
	ldy #16
	jsr _writeDds
	jsr tempoCW
	jsr tempoCW
	jsr tempoCW
fin
	ldy regy
	rts
.)

; --------------------------------------------------------
; tempoCW : attend la durée d’un point Morse selon WPM
; Entrée : WPM (valeur entre 10 et 20) dans la variable WPM
; Durée dot calculée comme 24000 / WPM (en cycles)
; --------------------------------------------------------

tempoCW:
.(
    lda _WPM            ; Charge la valeur WPM (10–20)
    sec
    sbc #10            ; Index 0–10
    tay
    lda dotTable,y     ; Charge le nombre de boucles à effectuer
    tax                ; Place dans X (boucle externe)
outerLoop:
    ldy #$FF           ; Boucle interne : 255 × 5 cycles
innerLoop:
    dey                ; 2 cycles
    bne innerLoop      ; 3 cycles
    dex                ; Décrémente boucle externe
    bne outerLoop      ; Continue si X ≠ 0
    rts                ; Fin de la temporisation
.)

_WPM
	.byt 15

;tempo=1200/WPM
dotTable:
    .byt 94,86,78,72,67,63,59,55,52,49,47  ; WPM 10 à 20


; 1 trait 0 point
lenCW
.byt 0,6,5,0,4,0,4,6,5,6,0,5,6,6,6,5
.byt 5,5,5,5,5,5,5,5,5,5,6,6,0,5,0,6
.byt 6,2,4,4,3,1,4,3,4,2,4,3,4,2,2,3
.byt 4,4,3,3,1,3,4,3,4,4,4

cwTable ;Msb du code CW placé en LSB pour decallage vers la droite et récupérer le carry
.byt 0,43,18,0,9,0,1,30,13,45,0,10,51,33,42,9
.byt 31,30,28,24,16,0,1,3,7,15,7,21,0,17,0,12
.byt 22,2,1,5,1,0,4,3,0,0,14,5,2,3,1,7
.byt 6,11,2,0,1,4,8,6,9,13,3

_cwText
	.asc " F4GOHBEACON JN07DV",0
	

;   0 0b0
; ! 6 0b101011
; " 5 0b10010
; # 0 0b0
; $ 4 0b1001
; % 0 0b0
; & 4 0b1
; ' 6 0b11110
; ( 5 0b1101
; ) 6 0b101101
; * 0 0b0
; + 5 0b1010
; , 6 0b110011
; - 6 0b100001
; . 6 0b101010
; / 5 0b1001
; 0 5 0b11111
; 1 5 0b11110
; 2 5 0b11100
; 3 5 0b11000
; 4 5 0b10000
; 5 5 0b0
; 6 5 0b1
; 7 5 0b11
; 8 5 0b111
; 9 5 0b1111
; : 6 0b111
; ; 6 0b10101
; < 0 0b0
; = 5 0b10001
; > 0 0b0
; ? 6 0b1100
; @ 6 0b10110
; A 2 0b10
; B 4 0b1
; C 4 0b101
; D 3 0b1
; E 1 0b0
; F 4 0b100
; G 3 0b11
; H 4 0b0
; I 2 0b0
; J 4 0b1110
; K 3 0b101
; L 4 0b10
; M 2 0b11
; N 2 0b1
; O 3 0b111
; P 4 0b110
; Q 4 0b1011
; R 3 0b10
; S 3 0b0
; T 1 0b1
; U 3 0b100
; V 4 0b1000
; W 3 0b110
; X 4 0b1001
; Y 4 0b1101
; Z 4 0b11

