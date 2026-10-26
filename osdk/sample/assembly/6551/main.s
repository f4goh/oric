

;6551



#define UART_TRX_BUF	$031c 
#define UART_STATE		$031d
#define UART_CDE		$031e 
#define UART_CTRL		$031f


	.zero

	*= $50

;// Some two byte values
_zp_start_

playerP1State
	.dsb 1
	
		
_zp_end_




	.text

_main
.(
	lda #11  ; 1011
	sta UART_CDE
	lda #30
	sta UART_CTRL
	ldy #10
loop
	ldx #65
	jsr sendChar
	ldx #66
	jsr sendChar
	dey
	;bne loop
	jmp loop


rts
.)

sendChar
.(
    lda UART_STATE
	and #$10
	beq sendChar ;0 pas vide attends
	stx UART_TRX_BUF
	rts
.)

getChar
.(
    lda UART_STATE
	and #$08
	beq getChar  ;1 plein , 0 vide ici bloquant
	ldx UART_TRX_BUF
	rts
.)




tableMasque
         ;0   1  2  3  4  5 
	.byt $20,$10,$8,$4,$2,$1
		
