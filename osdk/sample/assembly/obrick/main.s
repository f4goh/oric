;https://ceo.oric.org/manuel-du-debogueur-doricutron/


#define	INK_BLACK   0	
#define	INK_RED	    1
#define	INK_GREEN   2
#define	INK_YELLOW  3
#define	INK_BLUE    4	
#define INK_MAGENTA	5
#define INK_CYAN	6
#define INK_WHITE	7

#define	PAPER_BLACK   $10	
#define	PAPER_RED	  $11
#define	PAPER_GREEN   $12
#define	PAPER_YELLOW  $13
#define	PAPER_BLUE    $14	
#define PAPER_MAGENTA $15
#define PAPER_CYAN	  $16
#define PAPER_WHITE	  $17

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

#define DISPLAY_ADRESS $BB80
#define END_DISPLAY_ADRESS $BFE0
#define _lores0		$d9ed
#define _ping		$fa9f
#define _kbdclick1	$fb14
#define _kbdclick2	$fb2a


#define HEIGHT 28
#define WIDTH 40
#define START_POS 2
#define TETRIS_WIDTH 10
#define STOP_POS START_POS+TETRIS_WIDTH+1
#define BLOC_MOVE_COL 1
#define COUL_TETRIS PAPER_RED
#define COUL_BLOC_MOVE INK_GREEN
#define BLOC_SIZE 16
#define TIMER_BLOC_MIN 5
#define TIMER_BLOC_MAX 75
#define TIMER_SCROLL 20

	.zero

	*= $50

;// Some two byte values
_zp_start_
	

posBlocX    ; position du bloc en X
	.dsb 1
posBlocY    ; position du bloc en Y
	.dsb 1

checkPosBlocX  ; position du bloc en X pour un test de collision
	.dsb 1
checkPosBlocY  ; position du bloc en Y pour un test de collision
	.dsb 1
collisionState ; etat de la collision,  1 : collision , 0 pas de collision 
	.dsb 1
posx         ; position x d'un carré élémentaire
    .dsb 1 
posy
    .dsb 1   ; position Y d'un carré élémentaire
setCoul
	.dsb 1  ; attribut du carré élémentaire en ecriture
getCoul
	.dsb 1  ; attribut du carré élémentaire en lecture

offsetBloc
	.dsb 1		;index de table pour recopier le bloc dans la memoire écran
blocSelect		; numéro du bloc de 0 à 7
	.dsb 1
	
blocBuffer	   ; buffer temporaire pour la rotation d'un bloc
	.dsb 16
		
timerLow
	.dsb 1  ;compteur 16 bits pour la gestion de la vitesse
timerHighGame
	.dsb 1
timerHighScroll
	.dsb 1

ligneEnCours	; variables pour le décallage des lignes
	.dsb 1
lignePtrDest   ;recopie de la ligne du bas vers le haut	
	.dsb 1
lignePtrSource	
	.dsb 1

gameState	; etat du jeu 0:en attente de jouer
	.dsb 1  ;             1:en cours de partie               
			;			  2:en pause
charPtrLow	       
	.dsb 1
charPtrHigh	       
	.dsb 1

cptScore	
	.dsb 1

vitesseJeu
	.dsb 1

	
;// Some one byte temporaries recup de Dbug
rand_low		.dsb 1		;// Random number generator, low part
rand_high		.dsb 1		;// Random number generator, high part
b_tmp1			.dsb 1
	
last_key_press	.dsb 1

ptr_dst
pl_dst			.dsb 1
ph_dst			.dsb 1

joy_Left	
;OsdkJoystick_0
	.dsb 1
joy_Left_back
	.dsb 1
joy_Right	
;OsdkJoystick_1
	.dsb 1	
joy_Right_back
	.dsb 1
;joy_ijk_left_present
;	.dsb 1	
;joy_ijk_right_present
;	.dsb 1	

jreg0
	.dsb 1
jreg1
	.dsb 1
jreg2
	.dsb 1
jreg3
	.dsb 1
jreg4
	.dsb 1  ;bit 5 detect
jreg5
	.dsb 1


	
_zp_end_	



.text

.(
_main
	lda #8+2	;// NOKEYCLICK+SCREEN no cursor
	sta $26a
	;jsr _lores0
	
;	jsr joystick_detect_ijk
;	lda jreg4
;	and #%00100000 
;	bne suite ;on efface pas joy non present
	
;	ldy #0
;	ldx #12
;again
;	lda joyFound,y
;	sta joyNotFound,x
;	inx
;	iny
;	cpy #6
;	bne again
	
suite
	jsr initTeris
	jsr printReturn
	
loop	
	jsr timer
    jmp loop   
.)


timer
.(
  dec timerLow
  bne fin
  lda #255
  sta timerLow 
  jsr handle_keyboard
  ;lda jreg4
  ;and #%00100000 
  ;bne suite ;joy non present
  ;jsr gestionJoystick
 suite 
  dec timerHighGame
  bne scroll_only
  lda vitesseJeu  ;doit etre une variable pour acc
  sta timerHighGame
  lda gameState
  cmp #1
  bne scroll_only
  jsr deplaceVersLeBas
scroll_only
  dec timerHighScroll
  bne fin
  lda #TIMER_SCROLL
  sta timerHighScroll
  jsr scrollText
fin
  rts
.)

scrollText
.(
  jsr decaleLigne
  jsr placeCar
  rts
.)

decaleLigne
.(
   ldx #2
   ldy #3
loop
   lda $bfb8,y
   sta $bfb8,x
   inx
   iny
   cpy #WIDTH
   bne loop
   rts
.)

placeCar
.(
 lda charPtrLow
 sta read+1
 lda charPtrHigh
 sta read+2
read
 lda $0123  ;lit le caractère
 bne suite  ;0 la fin du scroll ?
 lda #<scrTxt : reinit ptr
 sta charPtrLow
 lda #>scrTxt
 sta charPtrHigh
 jmp placeCar 
 suite
 cmp #32
 bne notSpace
 jsr GetRand
 and  #7
 bne notSpace
 ora  #1 
 notSpace
 sta $BFDF    ; place le car tjs au meme endroit
 inc charPtrLow ; incrémente le ptr sur 16 bits car scroll length > 255
 bne cont
 inc charPtrHigh
 cont
 rts
.)
	

	

; deplace le bloc mobile vers le bas a chaque intervalle de temps
deplaceVersLeBas
.(
  ldx posBlocY
  inx
  stx checkPosBlocY
  ldx posBlocX
  stx checkPosBlocX
  jsr checkCollision
  lda collisionState
  cmp #1
  bne fin
  ;test game over
  jmp verifieGameOver
fin
  rts
.)

; deplace le bloc mobile vers le bas quand on appuie sur le barre d'espace
deplaceVersLeBasDirect
.(
  ldx posBlocY
  stx checkPosBlocY
again
  inc checkPosBlocY
  ldx posBlocX
  stx checkPosBlocX
  jsr checkCollision
  lda collisionState
  cmp #1
  bne again
  jmp verifieGameOver
  ;test game over
  rts
.)

; verifie fin de partie sinon retire un nouveau bloc
verifieGameOver
.(
  jsr gameOver
  jsr GetRand   ;recopie et autre bloc
  lda  rand_low
  and  #7		;couleur entre 2 et 7
  cmp  #2
  bpl suite
  ora  #2
suite
  ;sta $bfb8+1   ;changement de couleur du scolling
  ora  #PAPER_BLACK
  sta setCoul
  jsr copyBloc 
  jsr checkLines
  jsr tirage
  rts  
.)


gameOver
.(
  ldx posBlocY  ;le bloc n'a pas bougé
  bne fin
  lda #0	  ;game over
  sta gameState
  jsr printGameOver
fin  
  rts
.)



tirage
.(
  ;essai de couleur
  jsr GetRand
  lda  rand_low
  and  #7
  cmp  #0
  bne suite
  ora  #1
suite
  ldx #BLOC_MOVE_COL
  jsr traceLigneVerticale
  ;fin test
  jsr GetRand
  lda  rand_low
  and  #7
  ;lda  #0    ;test bloc 0 continu
  sta blocSelect  ; 0 à 7
  lda #127
  sta setCoul
  ldx #4
  stx posBlocX
  ldx #0
  stx posBlocY
  lda #COUL_BLOC_MOVE
  jsr copyBloc
  rts
.)


;en partant du bas vers le haut compteur de HEIGHT-3 à 4
checkLines
.(
  ldx #HEIGHT-3
  stx ligneEnCours
loop
  jsr checkOneLine
  cmp #0
  bne loop  ;peut etre encore une autre ligne a traiter
  ldx ligneEnCours  ; passe à la suivante
  dex
  stx ligneEnCours
  cpx #3
  bne loop  
  rts
.)


;verifie une ligne pleine
checkOneLine
.(
    ldx ligneEnCours
	stx posy
	ldx #START_POS+1
	stx posx
loop
	jsr getPosXY
	cmp #PAPER_BLACK
	beq fin
	inc posx
	ldx posx
	cpx #STOP_POS
	bne loop
    jsr decallageTotale
	jsr afficheScore
	jsr updateVitesse
    lda #1  ;le decallage a eu lieu ici incrémemtation du score
   rts	
fin
    lda #0  ;pas de decallage  pour le retour
    rts
.)

updateVitesse
.(
	lda cptScore
	lsr
	lsr
	lsr
	lsr
	tax
	lda tableVitesse,x
	sta vitesseJeu 
	rts
.)	

afficheScore
.(
	sed ;decimal
	clc
	lda cptScore
	adc #1
	sta cptScore
	cld ;binaire
    lda score_adr_low
	sta writescore+1
    lda score_adr_high
	sta writescore+2
    ;dizaines
	ldx #0
	lda cptScore
	lsr
	lsr
	lsr
	lsr
	clc
	adc #48
	jsr writescore
    ldx #40
	jsr writescore
	;unites
	ldx #1
	lda cptScore
	and #$0f
	clc
	adc #48
	jsr writescore
    ldx #41
	jsr writescore
	rts

writescore
	sta $0123,x
	rts

.)


;de la ligne en cours vers le haut
decallageTotale
.(
	ldx ligneEnCours
	stx lignePtrDest
	stx lignePtrSource
	dec lignePtrSource
loop
  jsr decallageUneLigne
  dec lignePtrDest
  dec lignePtrSource
  ldx lignePtrDest
  cpx #4
  bne loop  
  rts
.)

;recopie une ligne
decallageUneLigne
.(
	ldx #START_POS+1
	stx posx
loop
    lda lignePtrSource
	sta posy
	jsr getPosXY ;recupère l'attribut
	lda getCoul 
	sta setCoul
    lda lignePtrDest
	sta posy
	jsr setPosXY ;replace l'attribut
	inc posx
	ldx posx
	cpx #STOP_POS
	bne loop	
    rts
.)

reinitTimer
	lda #255
	sta timerLow
	lda #TIMER_BLOC_MAX
	sta vitesseJeu
	sta timerHighGame
	rts


; dessine le conteneur rouge de tetris
initTeris
	lda #23
	sta rand_low
	lda #35
	sta rand_high	
	
	jsr reinitTimer

	lda #TIMER_SCROLL
	sta timerHighScroll

	lda #0	  ;game state
	sta gameState
	
	lda #<scrTxt  ;init ptr for 16 bits add scroll
	sta charPtrLow
	lda #>scrTxt
	sta charPtrHigh

	jsr efface
	ldx #START_POS
	lda #COUL_TETRIS
	jsr traceLigneVerticale
	ldx #STOP_POS
	lda #COUL_TETRIS
	jsr traceLigneVerticale
	ldx #BLOC_MOVE_COL
	lda #COUL_BLOC_MOVE
	jsr traceLigneVerticale
	ldy #HEIGHT-2
	lda #COUL_TETRIS
	jsr traceLigneHorizontale
	jsr init_logo
	jsr init_rules

	lda #<cornerUP_LEFT ;[5b b6d8
	sta readCharDef+1
	lda #>cornerUP_LEFT
	sta readCharDef+2
	lda #<$b6d8
	sta writeCharDef+1
	lda #>$b6d8
	sta writeCharDef+2
	jsr charDef

	lda #<cornerUP_RIGHT ;]5d b6e8 
	sta readCharDef+1
	lda #>cornerUP_RIGHT
	sta readCharDef+2
	lda #<$b6e8
	sta writeCharDef+1
	lda #>$b6e8
	sta writeCharDef+2
	jsr charDef

	lda #<cornerDOWN_LEFT ;{7b b7d8
	sta readCharDef+1
	lda #>cornerDOWN_LEFT
	sta readCharDef+2
	lda #<$b7d8
	sta writeCharDef+1
	lda #>$b7d8
	sta writeCharDef+2
	jsr charDef

	lda #<cornerDOWN_RIGHT  ;}7d b7e8
	sta readCharDef+1
	lda #>cornerDOWN_RIGHT
	sta readCharDef+2
	lda #<$b7e8
	sta writeCharDef+1
	lda #>$b7e8
	sta writeCharDef+2
	jsr charDef

	lda #<intersection ;@64 b600
	sta readCharDef+1
	lda #>intersection
	sta readCharDef+2
	lda #<$b600
	sta writeCharDef+1
	lda #>$b600
	sta writeCharDef+2
	jsr charDef

	lda #<smiley ;£5f b6f8
	sta readCharDef+1
	lda #>smiley
	sta readCharDef+2
	lda #<$b6f8
	sta writeCharDef+1
	lda #>$b6f8
	sta writeCharDef+2
	jsr charDef
	
	rts

charDef
	ldx	#0
readCharDef
	lda $0123,x
writeCharDef
	sta $0123,x
	inx
	cpx #8
	bne readCharDef
	rts	
			
init_logo
.(
	ldx #0
	ldy #0
lignes	
	lda obrick_logo_adr_low,x
	sta write+1
	lda obrick_logo_adr_high,x
	sta write+2
	txa
	pha
	ldx #0
loop
	lda obrick_logo,y
write 
	sta $0123,x
	inx
	iny
	cpx #22
	bne loop
	pla
	tax
	inx
	cpx #6
	bne lignes
	rts
.)	



init_rules
.(
 ldy #0
 ldx #0
 loop
 lda rulesCmd_adr_low,x
 sta writeRules+1
 lda rulesCmd_adr_high,x
 sta writeRules+2
 jsr cpy_rules
 inx
 cpx #10
 bne loop
 rts
.)	

cpy_rules
    txa
	pha
	ldx #0
cpyAgain
	lda rulesCmdTxt,y
writeRules
	sta $0123,x
	inx
	iny
	cmp #0
	bne cpyAgain
	pla
	tax
	rts
	
effaceGamePlay
.(  
	lda #$10
    sta setCoul
    ldy #0
loopy
    sty posy
	ldx #START_POS+1
loopx
    stx posx
	txa
	pha
	tya
	pha
	jsr setPosXY
	pla
	tay
	pla
	tax	
	inx
	cpx #STOP_POS
	bne loopx
	iny
	cpy #HEIGHT-2
	bne loopy
	rts 
.)


printGameOver
.(  
   lda #<gameOverTxt
   sta readAll+1
   lda #>gameOverTxt
   sta readAll+2
   jsr prepareMessage
   rts
.)

printPaused
.(  
   lda #<gamePausedTxt
   sta readAll+1
   lda #>gamePausedTxt
   sta readAll+2
   jsr prepareMessage
   rts
.)


printReturn
.(  
   lda #<pressReturnTxt
   sta readAll+1
   lda #>pressReturnTxt
   sta readAll+2
   jsr prepareMessage
   rts
.)

printSpaces
.(  
   lda #<cleanTxt
   sta readAll+1
   lda #>cleanTxt
   sta readAll+2
   jsr prepareMessage
   rts
.)


prepareMessage
.(  
   lda msg_adr_low
   sta writeAll+1
   lda msg_adr_high
   sta writeAll+2
   jsr printAll
   lda msg_adr_low+1
   sta writeAll+1
   lda msg_adr_high+1
   sta writeAll+2
   jsr printAll
.)



printAll
	ldx #0
readAll	
	lda $0123,x
writeAll
	sta $0123,x
	inx
	cpx #22
	bne readAll
	rts

	
traceLigneVerticale
.(
	stx posx
	sta setCoul
	ldy #0
loop
	sty posy
	jsr setPosXY
	ldy posy
	iny
	cpy #HEIGHT-1
	bne loop
	rts
.)	
	
traceLigneHorizontale
.(
	sty posy
	sta setCoul
	ldx #START_POS
loop
	stx posx
	jsr setPosXY
	ldx posx
	inx
	cpx #STOP_POS
	bne loop
	rts
.)	

; tourne le bloc dans le sens horaire
rotateBloc
.(
	ldx blocSelect   ;num bloc
    lda bloc_AddrLow,x
	sta read+1
	sta write+1
    lda bloc_AddrHigh,x
	sta read+2
	sta write+2
	ldx #0   ;index
read
	lda $0123,x
	ldy rotSequence,x
	sta blocBuffer,y
	inx
	cpx #BLOC_SIZE
	bne read
	;il faut tester si rotation possible avant de recopier
	jsr checkCollisionRotation
	lda collisionState
	cmp #1
	beq fin   ;collision pas de rotation
	
	jsr deleteBloc  ; efface bloc
	
	ldx #0   ;index  recopie le buffer car pas de collision détecté
loop					
	lda blocBuffer,x
write
	sta $0123,x
	inx
	cpx #BLOC_SIZE
	bne loop
	lda #127
	sta setCoul
	jsr copyBloc  ;affiche le bloc
fin
	rts
.)	

;efface le bloc sur l'écran avant un déplacement
deleteBloc
.(
	lda posBlocX
	sta posx
	lda posBlocY
	sta posy
	ldx #0   ;index
	stx offsetBloc
loop
	jsr getPosXY
	cmp #127
	bne suite
	lda #$10
	sta setCoul
	jsr setPosXY
suite
	ldx offsetBloc
	lda posx
	clc
    adc blocIncXSeq,x
	sta posx
	lda posy
	clc
    adc blocIncYSeq,x
	sta posy
	inx
	stx offsetBloc
	cpx #BLOC_SIZE
	bne loop
	rts
.)	

; vérifie si il y a une collision lors de la rotation
checkCollisionRotation
.(
	lda posBlocX
	sta posx
	lda posBlocY
	sta posy
	ldx #0   ;index
	stx offsetBloc
	stx collisionState
read
	lda blocBuffer,x
	jsr detect
	lda collisionState
	cmp #1
	beq fin  ; il y a collision
	inc offsetBloc
	ldx offsetBloc
	cpx #BLOC_SIZE
	bne read
	;pas de collision
fin 
	rts
.)

; vérifie si il y a une collision lors d'un déplacement latéral ou vertical
checkCollision
.(
    ldx blocSelect   ;num bloc
    lda bloc_AddrLow,x
	sta read+1
    lda bloc_AddrHigh,x
	sta read+2
	lda checkPosBlocX
	sta posx
	lda checkPosBlocY
	sta posy
	ldx #0   ;index
	stx offsetBloc
	stx collisionState
read
	lda $0123,x
	jsr detect
	lda collisionState
	cmp #1
	beq fin ; il y a collision
	ldx offsetBloc
	inx
	stx offsetBloc
	cpx #BLOC_SIZE
	bne read
	;recopie coord pas de collision
	jsr deleteBloc
	lda checkPosBlocX
	sta posBlocX
	lda checkPosBlocY
	sta posBlocY
	lda #127
	sta setCoul
	jsr copyBloc
fin 
	rts
.)	

; compare l'attribut élémentaire de l'écran avec celui du bloc
detect
.(
	cmp #0
	beq fin  ;pas de collision
	jsr getPosXY ;recup coul fond
	lda getCoul
	cmp #PAPER_RED  ; collision entre $11 et $17 inclus
	bmi fin 		;inférieur à $11
	cmp #PAPER_WHITE+1  	
	bpl fin 		;supérieur à $17
	lda #1
	sta collisionState
	rts
fin
	ldx offsetBloc
	lda posx
	clc
    adc blocIncXSeq,x
	sta posx
	lda posy
	clc
    adc blocIncYSeq,x
	sta posy
	rts
.)	


; copie le bloc dans l'écran
copyBloc
.(
	ldx blocSelect   ;num bloc
    lda bloc_AddrLow,x
	sta read+1
    lda bloc_AddrHigh,x
	sta read+2
	lda posBlocX
	sta posx
	lda posBlocY
	sta posy
	ldx #0   ;index
	stx offsetBloc
read
	lda $0123,x
	jsr pasteBlocXY
	inc offsetBloc
	ldx offsetBloc
	cpx #BLOC_SIZE
	bne read
	rts
.)	

;positionne un bloc élémentaire sur l'écran et passe au suivant
pasteBlocXY
.(
	cmp #1
	bne fin
	jsr setPosXY	
fin
	ldx offsetBloc
	lda posx
	clc
    adc blocIncXSeq,x
	sta posx
	lda posy
	clc
    adc blocIncYSeq,x
	sta posy
	rts
.)	

	
efface
	lda #$10  
writeEfface:
	sta DISPLAY_ADRESS
	lda writeEfface+1
	clc
	adc #1
	sta writeEfface+1
	lda writeEfface+2
	adc #0
	sta writeEfface+2
	cmp	#>END_DISPLAY_ADRESS  ;$bf
	bne efface
	lda writeEfface+1
	cmp #<END_DISPLAY_ADRESS  ;$e0
	bne efface
	rts
	

setPosXY
.(
	ldy posy
	lda ScreenAdressLow,Y
	clc	
	adc posx
	sta wsetPosXY+1
	lda ScreenAdressHigh,Y
	adc #0			
	sta wsetPosXY+2
	lda setCoul
wsetPosXY
	sta $0123 ; adresse bidon
	rts
.)	
	
getPosXY
.(
	ldy posy
	lda ScreenAdressLow,Y
	clc	
	adc posx
	sta wgetPosXY+1
	lda ScreenAdressHigh,Y
	adc #0			
	sta wgetPosXY+2
wgetPosXY
	lda $0123 ; adresse bidon
	sta getCoul
	rts
.)	


handle_keyboard
	;//
	;// Handle keyboard
	;//  y contains the position of hero during all code, do not alterate

	ldx $208
	cpx #56
	bne key_pressed
	stx last_key_press
	jmp end_keyboard
key_pressed
	cpx	last_key_press
	beq end_keyboard
	stx last_key_press
	jsr HandleKeys
end_keyboard
	rts
	
HandleKeys
.(
	ldx #0
loop_scan
	lda KeyboardRouter_ScanCode,x
	beq end_of_scan
	cmp last_key_press
	beq execute_key
	inx
	jmp loop_scan

execute_key
	lda KeyboardRouter_AddrLow,x
	sta pl_dst
	lda KeyboardRouter_AddrHigh,x
	sta ph_dst
	jmp (ptr_dst)
end_of_scan
	rts
.)

gestionJoystick
.(
  ;jsr joystick_read_ijk
  jsr joystick_detect_ijk
 
  lda joy_Left
  cmp joy_Left_back
  beq test_joy_right
  sta joy_Left_back
  cmp #1
  beq moveLeft
  cmp #2
  beq moveRight
  cmp #8
  beq moveSpace ;down
  cmp #16
  beq pauseKey ;up
  cmp #32
  beq moveUpRotate ;fire
  jmp fin
test_joy_right
  lda joy_Right
  cmp joy_Right_back
  beq fin
  sta joy_Right_back
  cmp #1
  beq moveLeft
  cmp #2
  beq moveRight
  cmp #8
  beq moveSpace ;down
  cmp #16
  beq pauseKey ;up
  cmp #32
  beq moveUpRotate ;fire
fin  
  rts
.)

moveLeft
.(
	lda gameState
	cmp #2
	beq fin
	ldx posBlocX
	dex
	stx checkPosBlocX
	ldx posBlocY
    stx checkPosBlocY
	jsr checkCollision
	jsr _kbdclick1
fin
	rts
.)


moveRight
.(
	lda gameState
	cmp #2
	beq fin
	ldx posBlocX
	inx
	stx checkPosBlocX
	ldx posBlocY
    stx checkPosBlocY
	jsr checkCollision
	jsr _kbdclick1
fin
	rts
.)


moveUpRotate
.(
	lda gameState
	cmp #2
	beq fin
    lda rand_high
	eor posBlocY
    sta rand_low
	jsr rotateBloc
	jsr _kbdclick1
fin
	rts
.)

moveDown ; not used testing
	;jsr _ping
	rts

moveSpace
	jsr deplaceVersLeBasDirect
	jsr _kbdclick2
	rts

	
pauseKey
.(
	lda gameState
    cmp #1
	bne suite
	jsr printPaused
	lda #2
	jmp fin
suite
	jsr printSpaces
    lda #1
fin
	sta gameState
	;jsr _ping
	rts
.)

returnKey
.(
	lda gameState
	bne fin
	lda #1
	sta gameState
	lda #0
	sta cptScore
	jsr init_rules
	jsr reinitTimer
	jsr effaceGamePlay
	jsr tirage
	jsr printSpaces
fin
	;jsr _ping
	rts
.)
	
		
GetRand
	lda rand_high
	sta b_tmp1
	lda rand_low
	asl 
	rol b_tmp1
	asl 
	rol b_tmp1
	asl
	rol b_tmp1
	asl
	rol b_tmp1
	clc
	adc rand_low
	pha
	lda b_tmp1
	adc rand_high
	sta rand_high
	pla
	adc #$11
	sta rand_low
	lda rand_high
	adc #$36
	sta rand_high
	rts

joystick_detect_ijk
.(
    SEI              
    PHA              
    LDA VIA_PORTA    
    PHA              
    LDA VIA_DDRA     
    PHA              
    LDA VIA_PORTB     
    PHA              
    LDA VIA_DDRB     
    PHA              
    LDA #$C0   ; Set top two bits of Port A to output and rest as input  
    STA VIA_DDRA     
    LDA #$7F         ;0111 1111 selectionne le joy left
    STA VIA_PORTA    
    LDA VIA_PORTA    
    STA jreg0          ;memorise l'etat du joyleft
    LDA #$BF         ;1011 1111  selectionne le joy right
    STA VIA_PORTA    
    LDA VIA_PORTA    
    STA jreg1          ;memorise l'etet du joy right
    LDA jreg0        
    CMP #$7F         ;test si joy left present : 7f bien la
    BNE suite        ;sortie
    LDA jreg1          ;
    CMP #$BF         ;test si joy right present : BF bien la
    BNE suite        ;sortie
    LDA VIA_DDRB     ; Ensure printer strobe is set to output
    ORA #$10         
    STA VIA_DDRB     
    LDA #$00         
    STA VIA_PORTB     
    LDA #$7F 			;left        
    STA VIA_PORTA    
    LDA VIA_PORTA    
    STA jreg2
	
	and #%00011111        ; mask out unused bits              
	eor #%00011111        ; invert bits
    tax
	lda GenericIJKBits,X
    sta joy_Left
	
    LDA #$BF         
    STA VIA_PORTA    
    LDA VIA_PORTA    
    STA jreg3          

	and #%00011111        ; mask out unused bits              
	eor #%00011111        ; invert bits
    tax
	lda GenericIJKBits,X
    sta joy_Right

	
    LDA #$3F         
    STA VIA_PORTA    
    LDA VIA_PORTA    
    STA jreg4          
    LDA #$FF         
    STA VIA_PORTA    
    LDA VIA_PORTA    
    STA jreg5          
    JMP fin
suite			  
    LDA #$FF         
    STA jreg2          
    STA jreg3        
    STA jreg4        
    STA jreg5        
fin
    PLA              
    STA VIA_DDRB     
    PLA              
    STA VIA_PORTB     
    PLA              
    STA VIA_DDRA     
    PLA              
    STA VIA_PORTA    
    PLA              
    CLI              
    RTS              
.)
				
		
GenericIJKBits
      .byt 0,2,1,3,32,34,33,0,8,10,9,0,40,42,41,0
      .byt 16,18,17,0,48,50,49,0,0,0,0,0,0,0,0,0

;Bit	Value	Joystick Movement
;0	1	Left
;1	2	Right
;2	4	
;3	8	Down
;4	16	Up
;5	32	Fire

	  
ScreenAdressLow 
	.byt $80,$a8,$d0,$f8,$20,$48,$70,$98,$c0,$e8,$10,$38,$60,$88,$b0,$d8
    .byt $00,$28,$50,$78,$a0,$c8,$f0,$18,$40,$68,$90,$b8
ScreenAdressHigh 
	.byt  $bb,$bb,$bb,$bb,$bc,$bc,$bc,$bc,$bc,$bc,$bd,$bd,$bd,$bd,$bd,$bd
    .byt  $be,$be,$be,$be,$be,$be,$be,$bf,$bf,$bf,$bf,$bf

rotSequence        
	.byt 3,7,11,15,2,6,10,14,1,5,9,13,0,4,8,12
		
; 0  1  2  3
; 4  5  6  7	
; 8	 9  10 11
; 12 13 14 15

blocIncXSeq
	.byt 1,1,1,$fd,1,1,1,$fd,1,1,1,$fd,1,1,1,$fd
blocIncYSeq
	.byt 0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0

	
bloc_AddrLow		
	.byt <bloc0
	.byt <bloc1
	.byt <bloc2
	.byt <bloc3
	.byt <bloc4
	.byt <bloc5
	.byt <bloc6
	.byt <bloc0
	
bloc_AddrHigh		
	.byt >bloc0
	.byt >bloc1
	.byt >bloc2
	.byt >bloc3
	.byt >bloc4
	.byt >bloc5
	.byt >bloc6
	.byt >bloc0

bloc0
	.byt 0,0,1,0
	.byt 0,0,1,0
	.byt 0,0,1,0
	.byt 0,0,1,0

bloc1
	.byt 0,0,0,0
	.byt 0,1,1,0
	.byt 0,1,1,0
	.byt 0,0,0,0
	
bloc2
	.byt 0,0,0,0
	.byt 0,0,1,0
	.byt 0,1,1,1
	.byt 0,0,0,0

bloc3
	.byt 0,0,0,0
	.byt 0,0,1,1
	.byt 0,1,1,0
	.byt 0,0,0,0
	
bloc4
	.byt 0,0,0,0
	.byt 0,1,1,0
	.byt 0,0,1,1
	.byt 0,0,0,0

bloc5
	.byt 0,0,1,0
	.byt 0,0,1,0
	.byt 0,1,1,0
	.byt 0,0,0,0
	
bloc6
	.byt 0,1,0,0
	.byt 0,1,0,0
	.byt 0,1,1,0
	.byt 0,0,0,0
	
;// Scan codes:
;// 172 = Left
;// 188 = Right
;// 180 = Down
;// 156 = Up
;// 132 = Space
KeyboardRouter_ScanCode		
	.byt 172 ;left
	.byt 188 ;right
	.byt 180 ;down
	.byt 156 ;up
	.byt 132 ;space
	.byt 157 ;p
	.byt 175 ;ret
	.byt 0

KeyboardRouter_AddrLow		
	.byt <moveLeft
	.byt <moveRight
	.byt <moveDown
	.byt <moveUpRotate
	.byt <moveSpace
	.byt <pauseKey
	.byt <returnKey

	
KeyboardRouter_AddrHigh		
	.byt >moveLeft
	.byt >moveRight
	.byt >moveDown
	.byt >moveUpRotate
	.byt >moveSpace
	.byt >pauseKey
	.byt >returnKey

scrTxt
	.asc "Hello, this is my 1st game for oric coded in assembler, now with IJK joystick ctrl. "
	.asc "The ORIC is an extraordinary computer, "
	.asc "and i take great pleasure in rediscovering it again after 40 years. "
	.asc "I would like to salute all the CEO.ORIC.ORG club members as well as my friends "
	.asc "from the PROFECY Amiga Demomaker group in the 90's (SPEEDPOWER, SPECTRE, WOOPER). "
	.asc "Maybe this little game will make them want to get " 
	.asc "into retrocomputing. Don't forget to visit my site on the oric dedicated mainly "
	.asc "to hardware : github.com/f4goh/oric . See you soon, F4GOH ... 2024                        ",0 
	    
	
cornerUP_LEFT ;[5b b6d8
	.byt %00000000
	.byt %00000001
	.byt %00000011
	.byt %00000111
	.byt %00001111
	.byt %00011111
	.byt %00111111
	.byt %01111111

cornerUP_RIGHT ;]5d b6e8 
	.byt %11000000
	.byt %11100000
	.byt %11110000
	.byt %11111000
	.byt %11111100
	.byt %11111110
	.byt %11111111
	.byt %11111111

cornerDOWN_LEFT ;{7b b7d8
	.byt %01111111
	.byt %00111111
	.byt %00011111
	.byt %00001111
	.byt %00000111
	.byt %00000011
	.byt %00000001
	.byt %00000000
	
cornerDOWN_RIGHT ;}7d b7e8
	.byt %11111111
	.byt %11111111
	.byt %11111110
	.byt %11111100
	.byt %11111000
	.byt %11110000
	.byt %11100000
	.byt %11000000

intersection ;@64 b600
	.byt %11111110
	.byt %11111100
	.byt %11111000
	.byt %11110000
	.byt %11110000
	.byt %11111000
	.byt %11111100
	.byt %11111110

	
smiley ;£5f b6f8
	.byt %00000000
	.byt %00000000
	.byt %00001100
	.byt %00011110
	.byt %00111111
	.byt %00111111
	.byt %00011110
	.byt %00001100

	
obrick_logo
	.byt $06,$5b,$7f,$5d,$01,$7f,$7f,$5d,$02,$7f,$7f,$5d,$03,$5f,$04,$5b,$7f,$7f,$05,$7f,$20,$7f
	.byt $06,$7f,$20,$7f,$01,$7f,$20,$7f,$02,$7f,$20,$7f,$03,$20,$04,$7f,$20,$20,$05,$7f,$20,$7f
	.byt $06,$7f,$20,$7f,$01,$7f,$7f,$40,$02,$7f,$7f,$40,$03,$7f,$04,$7f,$20,$20,$05,$7f,$7f,$40
	.byt $06,$7f,$20,$7f,$01,$7f,$20,$7f,$02,$7f,$20,$7f,$03,$7f,$04,$7f,$20,$20,$05,$7f,$20,$7f
	.byt $06,$7b,$7f,$7d,$01,$7f,$7f,$7d,$02,$7f,$20,$7f,$03,$7f,$04,$7b,$7f,$7f,$05,$7f,$20,$7f
	.byt $07
	.asc "F4GOH ceo.oric.org V2" 
	
   
obrick_logo_adr_low
	.byt <$bb80+(40*0)+15
	.byt <$bb80+(40*1)+15
	.byt <$bb80+(40*2)+15
	.byt <$bb80+(40*3)+15
	.byt <$bb80+(40*4)+15
	.byt <$bb80+(40*5)+15

obrick_logo_adr_high
	.byt >$bb80+(40*0)+15
	.byt >$bb80+(40*1)+15
	.byt >$bb80+(40*2)+15
	.byt >$bb80+(40*3)+15
	.byt >$bb80+(40*4)+15
	.byt >$bb80+(40*5)+15

rulesCmdTxt
	.asc 3,"Use <- -> arrows to move",0
	.asc 4,"^ rotate, Space :falling",0
	.asc 5,"P : Pause, Return :play",0
joyNotFound
	.asc 1,"IJK joy int. not  found",0
	.asc 1,"-",2,"-",3,"-",4,"-",5,"-",6,"-",7,"-",1,"-",2,"-",3,"-",4,"-",5,"-",0
	.asc 1,0
	.asc 1,0
	.asc 1,0
	.asc 6,10,"lines score : 00",0
	.asc 6,10,"lines score : 00",0

joyFound
	.asc "erface"
	
score_adr_low
	.byt <$bb80+(40*16)+31	;dz
	;.byt <$bb80+(40*17)+31

score_adr_high
	.byt >$bb80+(40*16)+31  ;dz
	;.byt >$bb80+(40*17)+31

	
rulesCmd_adr_low
	.byt <$bb80+(40*8)+15
	.byt <$bb80+(40*9)+15
	.byt <$bb80+(40*10)+15
	.byt <$bb80+(40*11)+15
	.byt <$bb80+(40*12)+15
	.byt <$bb80+(40*13)+15
	.byt <$bb80+(40*14)+15
	.byt <$bb80+(40*15)+15
	.byt <$bb80+(40*16)+15
	.byt <$bb80+(40*17)+15
	
rulesCmd_adr_high
	.byt >$bb80+(40*8)+15
	.byt >$bb80+(40*9)+15
	.byt >$bb80+(40*10)+15
	.byt >$bb80+(40*11)+15
	.byt >$bb80+(40*12)+15
	.byt >$bb80+(40*13)+15
	.byt >$bb80+(40*14)+15
	.byt >$bb80+(40*15)+15
	.byt >$bb80+(40*16)+15
	.byt >$bb80+(40*17)+15

pressReturnTxt
	.asc 6,14,"Press return to play",0

gameOverTxt
	.asc 7,14,"      Game Over     ",0

gamePausedTxt
	.asc 3,14,"     Game Paused    ",0
	
cleanTxt
	.asc 6,14,"                    ",0

	
msg_adr_low
	.byt <$bb80+(40*20)+15	;dz
	.byt <$bb80+(40*21)+15

msg_adr_high
	.byt >$bb80+(40*20)+15  ;dz
	.byt >$bb80+(40*21)+15

tableVitesse
        ; 0  1  2  3 4 5 6 7 8 9
	.byt 75,66,60,51,45,39,30,24,18,15
	