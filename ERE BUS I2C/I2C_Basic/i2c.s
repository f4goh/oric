; avec JSR clk 81µs 12Khz
; sans JSR clk 57µs 17Khz

#define EREBUS_ADDRESS $3F3
#define PCF8574_ADDRESS $20
#define PCF8591_ADDRESS $48
#define ARDUINO_ADDRESS $55
#define DS3231_ADDRESS $68
#define LCD_ADDRESS $27
#define LM75_ADDRESS $49

	.text    

 *=$8000

; Called when the assembler is loaded.
; Can be used to perfom initializations
_EntryPoint
	jmp init
	rts
;-------------------------
; I2C functions
;-------------------------

sclHi    
    lda #$20
    sta EREBUS_ADDRESS
    rts

sdaHi    
    lda #$10
    sta EREBUS_ADDRESS
    rts

sclLo
    lda #$02
    sta EREBUS_ADDRESS
    rts

sdaLo
    lda #$01
    sta EREBUS_ADDRESS
    rts

start
    jsr sdaHi
    jsr sclHi
    jsr sdaLo
    jsr sclLo
    rts

stop
    jsr sdaLo
    jsr sclHi
    jsr sdaHi
    rts

clockPulse
    jsr sclHi
    jsr sclLo
    rts

init
    jsr sdaHi
    jsr sclHi
    rts



writeByte
.(
    ldx #0
loop
    bit ByteBuf
    bpl bit_clear
    jsr sdaHi
    jmp bit_done
bit_clear:
    jsr sdaLo
bit_done:
    jsr clockPulse
    asl ByteBuf
    inx
    cpx #8
    bne loop
    rts
.)	

readBit
    jsr sclHi
    lda EREBUS_ADDRESS
    and #$01
    pha
    jsr sclLo
	pla
    rts

readByte
.(
    ldx #0
	stx ByteBuf   
    jsr sdaHi
loop
    jsr readBit
    bne bit_set
    clc
    jmp bit_done
bit_set
    sec
bit_done
    rol ByteBuf
    inx
    cpx #8
    bne loop
    rts
.)

;ack maitre :
;pulse ack=0 octet suivant
;pulse ack=1 non stop après

setAck
	jsr sdaLo
	jsr clockPulse
	rts

setNack
	jsr sdaHi
	jsr clockPulse
	rts

;ack slave :
readAck           ;return readBit() == 0 ? 0 : 1;
    jsr sdaHi
    jsr readBit
    cmp #$00
    beq ack_received
    lda #1
    rts
ack_received
    lda #0
    rts

readNack		;return readBit() == 1 ? 0 : 1;
    jsr sdaHi
    jsr readBit
    cmp #$01
    beq nack_received
    lda #1
    rts
nack_received
    lda #0
    rts


;envoi un octet et un seul	
;octet a envoyer : WriteByte
;Error : 0 envoi ok; 1 envoi echoué
busWriteByte
.(
    jsr start			
    jsr writeByte
    jsr readAck
    bne stop_failed
    jsr stop
    lda #0
	sta Error
    rts
stop_failed
    lda #1
	sta Error
    rts
.)


;envoi de plusieurs octets
;buffer : busWriteBytes 
;nb octets: Length 
;Error : 0 envoi ok; 1 envoi echoué

busWriteBytes 
.(
    ldy #0
    jsr start
loop
    lda BytesBuf,y
	sta ByteBuf
    jsr writeByte  ;envoi octet
    jsr readAck
    bne stop_failed
    iny
    cpy LengthBuf
    bne loop
    jsr stop
    lda #0
	sta Error
    rts
stop_failed
    lda #1
	sta Error
    rts
.)

;reception un octet et un seul	
;octet d'adresse : ByteBuf
;octet lecture : ByteBuf
;Error : 0 envoi ok  1 envoi echoué

busReadByte
.(
	jsr start			
    jsr writeByte
    jsr readAck
    bne stop_failed
	jsr readByte
	jsr setNack
    jsr stop
    lda #0
	sta Error
    rts
stop_failed
    lda #1
	sta Error
    rts
.)

busReadBytes  ;A tester
.(    
    lda BytesBuf
	sta ByteBuf
	jsr start
    jsr writeByte  ;envoi octet
	jsr readAck
	bne stop_failed
    	
	ldy #0
    jmp read	
loop
	jsr setAck
read
	jsr readByte
	lda ByteBuf
	sta BytesBuf,y
    iny
	cpy LengthBuf
	bne loop
    jsr setNack
    jsr stop
    lda #0
	sta Error
    rts
stop_failed
    lda #1
	sta Error
    rts
.)


Error      	;0 if all steps were executed, else 1
		.dsb 1
ByteBuf
		.dsb 1
LengthBuf
		.dsb 1
BytesBuf
		.dsb 10

		
;-------------------------
; TM1637 driver
;-------------------------

_TM1637 
.(
	sei
	lda #3
	sta BytesBuf
	ldy #0
loopConvert
	lda _TM1637value3,y
	tax	
	lda TM1637segments,x
	iny
	sta BytesBuf,y
	cpy #4
	bne loopConvert
	lda _TM1637point
	and #1
	beq	suite
	ora BytesBuf+2
	sta BytesBuf+2
suite
	lda #$02
	sta ByteBuf
	jsr busWriteByte
	lda #$05
	sta LengthBuf
	jsr busWriteBytes
	lda #$f1
	sta ByteBuf
	jsr busWriteByte
	cli
	rts
.)

TM1637segments
	.byt $FC,$60,$DA,$F2,$66,$B6,$BE,$E0,$FE,$F6,$EE,$3E,$9C,$7A,$9E,$8E

_TM1637value3
	.dsb 1
_TM1637value2
	.dsb 1
_TM1637value1
	.dsb 1
_TM1637value0
	.dsb 1
_TM1637point
	.dsb 1

;-------------------------
; PCF8574 driver
;-------------------------

_PCF8574write
	sei
	lda #PCF8574_ADDRESS
	asl
	sta BytesBuf
	lda _PCF8574value
	sta BytesBuf+1
	lda #2
	sta LengthBuf
	jsr busWriteBytes
	cli
	rts

_PCF8574read
.(
	sei
	lda #PCF8574_ADDRESS
	asl
    ora #1
	sta ByteBuf
	jsr busReadByte
	lda ByteBuf
	sta _PCF8574value
	cli
    rts
.)	
	
_PCF8574value
	.dsb 1

;-------------------------
; PCF8591 driver
;-------------------------


_PCF8591adc
.(
	sei
	lda #PCF8591_ADDRESS  
	asl
	sta BytesBuf
	lda _PCF8591ctrl	;send ctrl byte
	sta BytesBuf+1
	lda #2
	sta LengthBuf
	jsr busWriteBytes
	lda #PCF8591_ADDRESS
	asl
    ora #1
	sta ByteBuf
	jsr busReadByte		;read adc
	lda ByteBuf
	sta _PCF8591value
	cli
    rts
.)	

_PCF8591dac
.(
	sei
	lda #PCF8591_ADDRESS  ;send ctrl byte
	asl
	sta BytesBuf
	lda #$40
	sta BytesBuf+1
	lda _PCF8591value
	sta BytesBuf+2		;value
	lda #3
	sta LengthBuf
	jsr busWriteBytes
	cli
    rts
.)	

_PCF8591ctrl
	.dsb 1
_PCF8591value
	.dsb 1


;-------------------------
; Arduino Uno Slave driver
;-------------------------

_ARDUINOread ;envoi un octet et lit un octet
.(
	sei
	lda #ARDUINO_ADDRESS  
	asl
	sta BytesBuf
	lda _ARDUINOctrl	;send ctrl byte
	sta BytesBuf+1
	lda #2
	sta LengthBuf
	jsr busWriteBytes
	lda #ARDUINO_ADDRESS
	asl
    ora #1
	sta ByteBuf
	jsr busReadByte		;read uno
	lda ByteBuf
	sta _ARDUINOvalue
	cli
    rts
.)	

_ARDUINOwrite  ;envoi un octet
.(
	sei
	lda #ARDUINO_ADDRESS  ;send ctrl byte
	asl
	sta BytesBuf
	lda _ARDUINOctrl
	sta BytesBuf+1		;value
	lda #2
	sta LengthBuf
	jsr busWriteBytes
	cli
    rts
.)	

_ARDUINOctrl
	.dsb 1
_ARDUINOvalue
	.dsb 1


;-------------------------
; DS3231
;-------------------------

_DS3231write  ;mise à l'heure
.(
	sei
	lda #DS3231_ADDRESS  ;send ctrl byte
	asl
	sta BytesBuf	
	lda #$0				
	sta BytesBuf+1		;word adr
	ldy #0
loop
	lda _DS3231sec,y
	sta BytesBuf+2,y
	iny
	cpy #7
	bne loop
	lda #9
	sta LengthBuf
	jsr busWriteBytes
	cli
    rts
.)	


_DS3231read  ;a tester
.(  
	sei
	lda #DS3231_ADDRESS 
	asl
	sta BytesBuf
	lda #0			;send ctrl byte
	sta BytesBuf+1
	lda #2
	sta LengthBuf
	jsr busWriteBytes ; stop inclus
	lda #DS3231_ADDRESS
	asl
    ora #1
	sta BytesBuf
	lda #7
	sta LengthBuf
	jsr busReadBytes
	ldy #0
loop
	lda BytesBuf,y
	sta _DS3231sec,y
	iny
	cpy #7
	bne loop
	cli
    rts
.)

_DS3231sec
	.dsb 1
_DS3231min
	.dsb 1
_DS3231hour
	.dsb 1
_DS3231wd
	.dsb 1
_DS3231day
	.dsb 1
_DS3231month
	.dsb 1
_DS3231year
	.dsb 1

;-------------------------
; Lcd 2x16 I2C
;-------------------------

_lcdBegin
	lda #0
    sta lcdXfer
	jsr lcdI2C
	lda #$30 ;4bits mode
	sta lcdCurByte
	jsr lcdWrite4bits ;retry pour l'oric ?
	jsr lcdWrite4bits
	jsr lcdWrite4bits
	lda #$20 ;4bits mode
	sta lcdCurByte
	jsr lcdWrite4bits
	lda #$28
	sta lcdByte
	jsr lcdCommand
	lda #$0C
	sta lcdByte
	jsr lcdCommand
	jsr	_lcdClear
	lda #$06
	sta lcdByte
	jsr lcdCommand
	rts
	
_lcdPrint
.(  
	ldy #0
	jmp load
loop
	sta lcdByte
	tya
	pha
	jsr lcdWrite
	pla
	tay
	iny
load
	lda _lcdTxt,y
	bne loop
	rts
.)	

_lcdClear
	lda #$01
	sta lcdByte
	jsr lcdCommand
	rts
	
_lcdCursor
	ldx _lcdY
	lda lcdRows,x
	clc
	adc _lcdX
	ora #$80
	sta lcdByte
	jsr lcdCommand
	rts

_lcdLightOn
	lda #$08
	sta _backLight
	sta lcdXfer
	jsr lcdI2C
	rts

_lcdLightOff
	lda #$00
	sta _backLight
	sta lcdXfer
	jsr lcdI2C
	rts


lcdRows
	.byt $00,$40,$14,$54

lcdWrite
	lda #$1  ;RS
	sta lcdMode
	jmp lcdSend
	
lcdCommand
	lda #$0
	sta lcdMode
	jmp lcdSend


lcdI2C  ;envoi un octet
.(
	sei
	lda #LCD_ADDRESS  ;send ctrl byte
	asl
	sta BytesBuf
	lda lcdXfer
	ora _backLight  ;ajout backlight
	sta BytesBuf+1		;value
	lda #2
	sta LengthBuf
	jsr busWriteBytes
	cli
    rts
.)	

lcdWrite4bits
	lda lcdCurByte
	sta lcdXfer
	jsr lcdI2C
	jsr lcdPulse
	rts

lcdPulse
	lda lcdCurByte
	and #$FB		;enb 1111 1011 FB
	sta lcdXfer
	jsr lcdI2C
	lda lcdCurByte
	ora #$04		;EN  0000 0100 04
	sta lcdXfer
	jsr lcdI2C
	lda lcdCurByte
	and #$FB
	sta lcdXfer
	jsr lcdI2C
	rts

lcdSend
	lda lcdByte
	and #$F0
	ora lcdMode
	sta lcdCurByte
	jsr lcdWrite4bits
	lda lcdByte
	ASL
	ASL
	ASL
	ASL
	and #$F0
	ora lcdMode
	sta lcdCurByte
	jsr lcdWrite4bits
	rts
	
;low level buffer
lcdXfer
	.dsb 1
lcdCurByte
	.dsb 1
;mid level buffer	
lcdByte
	.dsb 1
lcdMode
	.dsb 1
_lcdTxt
	.dsb 25
_lcdX
	.dsb 1
_lcdY
	.dsb 1
_backLight
	.dsb 1
	
;-------------------------
; LM75
;-------------------------


_LM75read  ;a tester
.(  
	sei
	lda #LM75_ADDRESS
	asl
	sta BytesBuf
	lda #0			;send ctrl byte
	sta BytesBuf+1
	lda #2
	sta LengthBuf
	jsr busWriteBytes ; stop inclus
	lda #LM75_ADDRESS
	asl
    ora #1
	sta BytesBuf
	lda #2
	sta LengthBuf
	jsr busReadBytes
	lda BytesBuf
	sta _LM75value+1
	lda BytesBuf+1
	and #$80
	sta _LM75value
	cli
    rts
.)

_LM75value
	.dsb 2
