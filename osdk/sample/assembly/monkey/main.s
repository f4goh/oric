
;https://osdk.org/index.php?page=articles&ref=ART9

;240 x 200  240/6=40
;8000 bytes for the HIRES part of the screen (from #A000 to #BF3F)
;120 bytes for the TEXT part of the screen screen (from #BF68 to #BFDF)
;42 bytes lost between the HIRES and TEXT parts of the screen (from #BF40 to #BF67)
;1024 bytes for the default character set (from #9800 to #9BFF)

;each 0 bit will be drawn using the PAPER (background) color
;each 1 bit will be drawn using the INK (foreground) color

;r pc $xxxx 
;m $xxxx  dump


;//
;// ROM system defines
;// (Works only on ROM 1.1)
;//
;#define ORIC_CTRL
#define PC_CTRL


#define _hires		$ec33
//#define _rom_text		$ec21

#define _rom_ink		$f210
#define _rom_paper		$f204  ; paramètre #$2e1 

#define _rom_ping		$fa9f
#define _rom_shoot		$fab5
#define _rom_zap		$fae1
#define _rom_explode	$facb

#define _rom_kbdclick1	$fb14
#define _rom_kbdclick2	$fb2a

//#define _rom_cls		$ccce
//#define _rom_lores0		$d9ed
//#define _rom_lores1		$d9ea

#define _rom_curset		$f0c8
#define _rom_curmov		$f0fd
#define _rom_draw		$f110
#define _rom_fill		$f268
#define _rom_circle		$f37f

#define _rom_redef_chars	$f8d0

#define	VIA_PORTB		$0300  ; Input/Output register B
#define	VIA_PORTAH		$0301  ; Input/Output register A (with handshake)
#define	VIA_DDRB		$0302  ; Data Direction Register B
#define	VIA_DDRA		$0303  ; Data Direction Register A
#define	VIA_T1CL		$0304  ; Timer 1 low-order latches/counter
#define	VIA_T1CH       	$0305  ; Timer 1 high-order counter
#define	VIA_T1LL       	$0306  ; Timer 1 low-order latches
#define	VIA_T1LH       	$0307  ; Timer 1 high-order latches
#define	VIA_T2LL       	$0308  ; Timer 2 low-order latches/counter
#define	VIA_T2CH       	$0309  ; Timer 2 high-order counter
#define	VIA_SR        	$030A  ; Shift Register (Buggy on many Oric, do not use) 
#define VIA_ACR         $030B  ; Auxiliary Control Register
#define	VIA_PCR        	$030C  ; Peripheral Control Register
#define	VIA_IFR			$030D  ; Interupt Flag Register
#define	VIA_IER			$030E  ; Interupt Enable Register
#define	VIA_PORTA      	$030F  ; Input/Output register A (without handshake)


#define	KEY_NONE	$38
#define KEY_UP		11	//$9c
#define	KEY_DOWN	10	//$b4
#define KEY_LEFT	8	//$ac
#define KEY_RIGHT	9	//$bc
#define KEY_SPACE	32	//$84
#define KEY_P		80	//$9d
#define KEY_DELETE	127
#define KEY_ESCAPE	27	//$a9
#define KEY_ENTER	13

#define SCREEN_SRC $a000

#define LOGO_ADR $A000+(40*80)+12

#define frameLOW  $1d	;4637 $121d
#define frameHIGH $12



;//
;// Zero page definition
;//

	.zero

	*= $50

;// Some two byte values
_zp_start_

psgReg       .dsb 16
ptrReg	     .dsb 1


bitMaskLow   .dsb 1
bitMaskHigh  .dsb 1

valIncBuf   .dsb 1
frameCountLow  .byt 0
frameCountHigh  .byt 0

regNumber   .byt 0
regValue    .byt 0 

_zp_end_




	.text

_main
.(
	;
	; Switch to hires
	;

	lda #$0  ;paper noir
	sta $02e1
	sta $02e2	
	jsr _rom_paper
	
	jsr _hires
	
	lda #8+2	;// NOKEYCLICK+SCREEN no cursor
	sta $26a
	
	jsr afficheImage
	jsr InitMusic
	
	jsr StartMusic
	;jsr EndMusic
	
loop
	jmp loop
.)



StartMusic
	php
	pha
	sei

    clc
	lda $fffe
	adc #1
	sta __auto_1+1
	sta __auto_3+1
	sta __auto_5+1
	lda $ffff
	adc #0
	sta __auto_1+2
	sta __auto_3+2
	sta __auto_5+2

    clc
	lda $fffe
	adc #2
	sta __auto_2+1
	sta __auto_4+1
	sta __auto_6+1
	lda $ffff
	adc #0
	sta __auto_2+2
	sta __auto_4+2
	sta __auto_6+2

	; Save the old handler value
__auto_1
	lda $245
	sta jmp_old_handler+1
__auto_2
	lda $246
	sta jmp_old_handler+2

	; Install our own handler
	lda #<irq_handler
__auto_3
	sta $245
	lda #>irq_handler
__auto_4
	sta $246

	pla
	plp
	rts


EndMusic
	php
	pha
	sei

	; Restore the old handler value
	lda jmp_old_handler+1
__auto_5
	sta $245
	lda jmp_old_handler+2
__auto_6
	sta $246

	; Stop the sound
	lda #8
	ldx #0
	jsr WriteRegister

	lda #9
	ldx #0
	jsr WriteRegister

	lda #10
	ldx #0
	jsr WriteRegister

	pla
	plp
	rts	


_50hzFlipFlop	.byt 0
_1hzFlipFlop	.byt 0
_cpt	.byt 0


irq_handler
	pha
	txa
	pha
	tya
	pha

	; This handler runs at 100hz if comming from the BASIC, 
	; but the music should play at 50hz, so we need to call the playing code
	; only every second frame
	lda _50hzFlipFlop
	eor #1
	sta _50hzFlipFlop
	beq skipFrame

	jsr playMusicFrame
	
	;lda _cpt
	;clc
	;adc #1
	;sta _cpt
	;cmp #10
	;bne skipFrame
	;lda #0
	;sta _cpt
	;lda _1hzFlipFlop
	;eor #1
	;sta _1hzFlipFlop
	;ora #$40
	;sta $A010
	

skipFrame
	pla
	tay
	pla
	tax
	pla

jmp_old_handler
	jmp 0000


InitMusic 
   lda #<_musicData
   sta getValMusic+1
   lda #>_musicData
   sta getValMusic+2
	ldx #0
loopInitPsg
	jsr getValMusic
	sta psgReg,x
	inx
	cpx #14
	bne loopInitPsg
	lda #14
	sta valIncBuf
	jsr incPtrBuffer
	lda #0
	sta frameCountLow
	sta frameCountHigh
	rts

getValMusic
	lda $0123,x
	rts

;-----------------------------------
; Avancer le pointeur de valIncBuf octets
;-----------------------------------	
incPtrBuffer
.(
    clc
    lda getValMusic+1
    adc valIncBuf
    sta getValMusic+1
    bcc noCarry
    inc getValMusic+2
noCarry
	rts
.)


;---------------------------------------
; Routine appelée toutes les 20 ms
;---------------------------------------
playMusicFrame
.(
	ldx #0				;recupere les 2 octets de mask bits
	jsr getValMusic
	sta bitMaskLow
	inx
	jsr getValMusic
	sta bitMaskHigh
	lda #2
	sta valIncBuf
	jsr incPtrBuffer
		
	lda #0		;pointeur
	sta ptrReg

	ldy #0
suiteLow	
	lsr bitMaskLow
	bcc	updateLow ;zero pas update psg
	ldx #0
	jsr getValMusic
	ldx ptrReg 
	sta psgReg,x
	lda #1
	sta valIncBuf
	jsr incPtrBuffer
updateLow
	inc ptrReg
	iny 
	cpy #8
	bne suiteLow

	ldy #0
suiteHigh	
	lsr bitMaskHigh
	bcc	updateHigh ;zero pas update psg
	ldx #0
	jsr getValMusic
	ldx ptrReg 
	sta psgReg,x
	lda #1
	sta valIncBuf
	jsr incPtrBuffer
updateHigh
	inc ptrReg
	iny 
	cpy #4
	bne suiteHigh
	jsr sendPsg
	
	inc frameCountLow
	bne skipHigh
	inc frameCountHigh
	skipHigh

	lda frameCountHigh
	cmp #frameHIGH
	bne exit

	lda frameCountLow
	cmp #frameLOW
	bne exit
	jsr InitMusic
exit
	rts

.)

#define frameLOW  $1d	;4637 $121d
#define frameHIGH $12


;-----------------------------------
; Boucle d’envoi des 14 registres moins 2 plus tard
;-----------------------------------
sendPsg
.(
    ldx #0              ; numéro de registre
nextReg		
	lda psgReg,x
	sta regValue
	txa
	sta regNumber
	pha
    ;jsr WriteRegister
	jsr psgSetRegister

    pla
    tax
    ; Incrémenter compteurs
    inx
    cpx #14
    bne nextReg
	rts
.)




WriteRegister
.(
	lda regValue
	tax
	lda regNumber
	
	PHP  			; WRITE X TO REGISTER A 0F 8912.
	SEI
	STA $030F  		; Send A to port A of 6522.
	TAY
	TXA
	CPY #$07  		; If writing to register 7, set
	BNE skip  		; 1/0 port to output.
	ORA #$40
skip	
	PHA
	LDA $030C  		; Set CA2 (BC1 of 8912) to 1,
	ORA #$EE  		; set CB2 (BDIR of 8912) to 1.
	STA $030C  		; 8912 latches the address.
	AND #$11  		; Set CA2 and CB2 to 0, BC1 and
	ORA #$CC  		; BDIR in inactive state.
	STA $030C
	TAX
	PLA
	STA $030F  		; Send data to 8912 register.
	TXA
	ORA #$EC  		; Set CA2 to 0 and CB2 to 1,
	STA $030C  		; 8912 latches data.
	AND #$11  		; Set CA2 and CB2 to 0, BC1 and
	ORA #$CC 		; BDIR in inactive state.
	STA $030C
	PLP
	RTS
.)


#define via_porta               $030f 
#define via_pcr                 $030c 

; y=register number
; x=value to write
psgSetRegister
.(
	lda regValue
	tax
	lda regNumber
    tay
    sty	via_porta
    txa
    pha
    lda	via_pcr
    ora	#$EE		; $EE	238	11101110
    sta	via_pcr

    and	#$11		; $11	17	00010001
    ora	#$CC		; $CC	204	11001100
    sta	via_pcr

    tax
    pla
    sta	via_porta
    txa
    ora	#$EC		; $EC	236	11101100
    sta	via_pcr

    and	#$11		; $11	17	00010001
    ora	#$CC		; $CC	204	11001100
    sta	via_pcr
    rts
.)

;-----------------------------------
; Gestion graphique
;-----------------------------------

afficheImage
.(
	;x et y varient automatiquement
	lda #0
	sta coordX
	sta coordY
	lda #<_imageData
    sta getValImage+1
    lda #>_imageData
    sta getValImage+2
loop
	ldx #0
	jsr getValImage
	beq exit		;fin de la decompression
	sta gfxCounter	
	BMI is_repeat   ;
	lda gfxCounter  ;masque le compteur
    and #$7F        
    sta gfxCounter
	lda #1			;recale le pointeur
	sta valIncImage
	jsr incPtrImage
is_recopie
	txa
	pha
	jsr getValImage
	sta gfxByte
	jsr afficheOctet
	pla
	tax
	inx
	cpx gfxCounter
	bne is_recopie
	lda gfxCounter	;avancer le pointeur de gfxCounter
	sta valIncImage
	jsr incPtrImage
	jmp loop

is_repeat
	ldx #1
	jsr getValImage ;recup la valeur a copier0
	sta gfxByte
	lda gfxCounter  ;masque le compteur
    and #$7F        
    sta gfxCounter
loopRepeat			;recopie
	jsr afficheOctet
	dec gfxCounter
	bne loopRepeat
	lda #2			;avancer le pointeur de 2
	sta valIncImage
	jsr incPtrImage
	jmp loop
exit	
	rts
.)

getValImage
	lda $0123,x
	rts

;-----------------------------------
; Avancer le pointeur de valIncBuf octets
;-----------------------------------	
incPtrImage
.(
    clc
    lda getValImage+1
    adc valIncImage
    sta getValImage+1
    bcc noCarry
    inc getValImage+2
noCarry
	rts
.)


afficheOctet
.(
	ldx coordX
	ldy coordY
	lda hiresLow,y
	sta setGfxByte+1
	lda hiresHigh,y
	sta setGfxByte+2
	lda gfxByte
setGfxByte
	sta $0123,x
	;traitement prochain octet
	inc coordY
	lda coordY
	cmp #200
	bne exit
	lda #0
	sta coordY
	inc coordX ;pas de test sur x necessaire
exit
	rts
.)

gfxByte
	.byt $4f 
gfxCounter
	.byt 0
coordX
	.byt 0
coordY
	.byt 0
valIncImage
	.byt 0

;A) Bloc RAW (brut)
;   Format :
;       [count] [b0 b1 b2 ... b(count-1)]
;   - count = 1 à 127
;   - bit 7 = 0
;   - utilisé quand il n’y a pas de répétition suffisante
;B) Bloc REPEAT (répétition)
;   Format :
;       [0x80 | count] [value]
;   - count = 3 à 127
;   - bit 7 = 1
;   - utilisé quand au moins 3 octets identiques se suivent



hiresLow
.byt $00,$28,$50,$78,$A0,$C8,$F0,$18,$40,$68,$90,$B8,$E0,$08,$30,$58,$80,$A8,$D0,$F8
.byt $20,$48,$70,$98,$C0,$E8,$10,$38,$60,$88,$B0,$D8,$00,$28,$50,$78,$A0,$C8,$F0,$18
.byt $40,$68,$90,$B8,$E0,$08,$30,$58,$80,$A8,$D0,$F8,$20,$48,$70,$98,$C0,$E8,$10,$38
.byt $60,$88,$B0,$D8,$00,$28,$50,$78,$A0,$C8,$F0,$18,$40,$68,$90,$B8,$E0,$08,$30,$58
.byt $80,$A8,$D0,$F8,$20,$48,$70,$98,$C0,$E8,$10,$38,$60,$88,$B0,$D8,$00,$28,$50,$78
.byt $A0,$C8,$F0,$18,$40,$68,$90,$B8,$E0,$08,$30,$58,$80,$A8,$D0,$F8,$20,$48,$70,$98
.byt $C0,$E8,$10,$38,$60,$88,$B0,$D8,$00,$28,$50,$78,$A0,$C8,$F0,$18,$40,$68,$90,$B8
.byt $E0,$08,$30,$58,$80,$A8,$D0,$F8,$20,$48,$70,$98,$C0,$E8,$10,$38,$60,$88,$B0,$D8
.byt $00,$28,$50,$78,$A0,$C8,$F0,$18,$40,$68,$90,$B8,$E0,$08,$30,$58,$80,$A8,$D0,$F8
.byt $20,$48,$70,$98,$C0,$E8,$10,$38,$60,$88,$B0,$D8,$00,$28,$50,$78,$A0,$C8,$F0,$18
hiresHigh
.byt $A0,$A0,$A0,$A0,$A0,$A0,$A0,$A1,$A1,$A1,$A1,$A1,$A1,$A2,$A2,$A2,$A2,$A2,$A2,$A2
.byt $A3,$A3,$A3,$A3,$A3,$A3,$A4,$A4,$A4,$A4,$A4,$A4,$A5,$A5,$A5,$A5,$A5,$A5,$A5,$A6
.byt $A6,$A6,$A6,$A6,$A6,$A7,$A7,$A7,$A7,$A7,$A7,$A7,$A8,$A8,$A8,$A8,$A8,$A8,$A9,$A9
.byt $A9,$A9,$A9,$A9,$AA,$AA,$AA,$AA,$AA,$AA,$AA,$AB,$AB,$AB,$AB,$AB,$AB,$AC,$AC,$AC
.byt $AC,$AC,$AC,$AC,$AD,$AD,$AD,$AD,$AD,$AD,$AE,$AE,$AE,$AE,$AE,$AE,$AF,$AF,$AF,$AF
.byt $AF,$AF,$AF,$B0,$B0,$B0,$B0,$B0,$B0,$B1,$B1,$B1,$B1,$B1,$B1,$B1,$B2,$B2,$B2,$B2
.byt $B2,$B2,$B3,$B3,$B3,$B3,$B3,$B3,$B4,$B4,$B4,$B4,$B4,$B4,$B4,$B5,$B5,$B5,$B5,$B5
.byt $B5,$B6,$B6,$B6,$B6,$B6,$B6,$B6,$B7,$B7,$B7,$B7,$B7,$B7,$B8,$B8,$B8,$B8,$B8,$B8
.byt $B9,$B9,$B9,$B9,$B9,$B9,$B9,$BA,$BA,$BA,$BA,$BA,$BA,$BB,$BB,$BB,$BB,$BB,$BB,$BB
.byt $BC,$BC,$BC,$BC,$BC,$BC,$BD,$BD,$BD,$BD,$BD,$BD,$BE,$BE,$BE,$BE,$BE,$BE,$BE,$BF
