"""
https://osdk.org/index.php?page=articles&ref=ART9
https://www.gladir.com/CODER/ORICEXTENDEDBASIC/graphiques-avances.htm

rappel hires

d7 d6 d5 d4 d3 d2 d1 d0

d7 inverse 

d6 ou d5 = 1 c'est des pixels ink
d6 et d5 = 0 c'est un paper


1000 0000 inverse le paper
1100 0000 inverse le paper et ink
f2 monitor
fw <addr> <len> <file>  Sauver une zone mémoire sur la disquette (bin file write)
Ce fichier <file> est sauvé dans la racine du répertoire Oricutron. 
fw $A000 $1f40 ghost.bin

"""

import pygame
import sys
from oricgfx import Oricgfx

colors = [
    (0,   0,   0),     # 0 : noir 16
    (255, 0,   0),     # 1 : red  17
    (0,   255, 0),     # 2 : green 18
    (255, 255, 0),     # 3 : yellow 19
    (0,   0,   255),   # 4 : blue   20
    (255, 0,   255),   # 5 : magenta 21
    (0,   255, 255),   # 6 : cyan    22
    (255, 255, 255)    # 7 : white 23
]


GRAY = (80, 80, 80)
GRAY_LIGHT = (150, 150, 150)

WHITE = colors[7]
INK=0
PAPER=1
NOIP=2
INVERTED=True


pygame.init()

LOGICAL_W, LOGICAL_H = 240, 200
ZOOM = 3
SCREEN_W, SCREEN_H = LOGICAL_W * ZOOM, LOGICAL_H * ZOOM

screen = pygame.display.set_mode((SCREEN_W, SCREEN_H))
pygame.display.set_caption("Double buffer test")

buffer_pixels = pygame.Surface((LOGICAL_W, LOGICAL_H))
buffer_attrib = pygame.Surface((LOGICAL_W, LOGICAL_H))

barreBack = LOGICAL_W // ZOOM


def draw_vertical_grid(surface, spacing=6):
    """
    Trace des lignes verticales espacées de `spacing` pixels
    sur la surface logique (240x200).
    Les lignes sont en gris clair pour être visibles.
    """

    width, height = surface.get_size()

    for x in range(0, width, spacing):
        pygame.draw.line(surface, GRAY, (x, 0), (x, height))

def draw_5px_horizontal(surface, x, y, color_index, colors,attFlag,attInvert):
    """
    Trace un trait horizontal de 5 pixels vers la droite
    avec pygame.draw.line
    """
    x=(x//6)*6+1
    #if color_index==0:
    #    color_index=8
    color = colors[color_index]

    if attFlag==INK:
    # Ligne horizontale : (x, y) → (x+4, y)
        pygame.draw.line(surface, color, (x+1, y), (x + 3, y))
    elif attFlag==PAPER:
        pygame.draw.line(surface, color, (x, y), (x + 4, y))
    if attInvert==INVERTED:
        pygame.draw.line(surface, GRAY_LIGHT, (x, y), (x + 1, y))
    else:
        rgb = surface.get_at((x+2, y))
        if attFlag==INK:
            surface.set_at((x, y), colors[0])
        else:
            surface.set_at((x, y), rgb)
        surface.set_at((x+1, y), rgb)
        
            


def updateAll(pixels,attrib):
    for y in range(LOGICAL_H):
        pixelsLine=gfx.getLinePx(y)
        for px in range(LOGICAL_W):
            ncoul=pixelsLine[px]
            rgb=colors[ncoul]
            pixels.set_at((px, y), rgb)

        attLine=gfx.getLineAtt(y)
        for col in range(LOGICAL_W//6):
            mot=attLine[col]
            attInvert = True if (mot & 0x80) else False
            attFlg=NOIP
            if mot & 0x60==0: #attribut
                if mot & 0x10 == 0:  #ink
                    attFlg=INK
                else:
                    attFlg=PAPER
            draw_5px_horizontal(attrib, col*6, y, mot & 0x07, colors,attFlg,attInvert)
            


show_attrib = False
clock = pygame.time.Clock()

draw_vertical_grid(buffer_attrib)

gfx=Oricgfx()

running = True

attFlag=0
codeFlag=0
attInvert=False

running = True
while running:
    # --- Gestion du dessin en continu (drag) ---
    buttons = pygame.mouse.get_pressed()  # (gauche, milieu, droit)
    mx, my = pygame.mouse.get_pos()
    x = mx // ZOOM
    y = my // ZOOM
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        if event.type == pygame.KEYDOWN:
            if event.unicode:
                ascii_code=ord(event.unicode)
                if event.key == pygame.K_SPACE:
                    show_attrib = not show_attrib
                elif event.key == pygame.K_i:
                    print("ink")
                    attFlag = INK
                elif event.key == pygame.K_p:
                    print("paper")
                    attFlag = PAPER
                elif event.key == pygame.K_n:
                    print("noip")
                    attFlag = NOIP            
                elif event.key == pygame.K_v:
                    attInvert = not attInvert
                elif event.key == pygame.K_s:
                    gfx.saveTap()
                elif event.key == pygame.K_u:
                    gfx.loadBin("ghost.bin")
                    updateAll(buffer_pixels,buffer_attrib)
                    print("ok")
                if 0x30<=ascii_code<=0x37:
                    codeFlag=ascii_code-0x30

                
    if show_attrib==False:
        if 0 <= x < LOGICAL_W and 0 <= y < LOGICAL_H:
            if buttons[0]:  # bouton gauche
                #buffer_pixels.set_at((x, y), (255, 255, 255))
                gfx.setPixel(x,y)
            if buttons[2]:  # bouton droit
                #buffer_pixels.set_at((x, y), (0, 0, 0))
                gfx.clearPixel(x,y)   
    else:
        if 0 <= x < LOGICAL_W and 0 <= y < LOGICAL_H:
            if buttons[0]:  # bouton gauche
                draw_5px_horizontal(buffer_attrib, x, y, codeFlag, colors,attFlag,attInvert)
                gfx.setAttribut(x,y,codeFlag,attFlag,attInvert)
                print(codeFlag,attFlag,attInvert)
            if buttons[2]:  # bouton droit
                draw_5px_horizontal(buffer_attrib, x, y, 0, colors,PAPER,False)
                gfx.resetAttribut(x,y)   

    if buttons[0] or buttons[2]:
        if 0 <= y < LOGICAL_H:
            pixelsLine=gfx.getLinePx(y)
            for px in range(LOGICAL_W):
                ncoul=pixelsLine[px]
                rgb=colors[ncoul]
                buffer_pixels.set_at((px, y), rgb)


    src = buffer_attrib if show_attrib else buffer_pixels
    scaled = pygame.transform.scale(src, (SCREEN_W, SCREEN_H))
    screen.blit(scaled, (0, 0))
    pygame.display.flip()

    clock.tick(60)

pygame.quit()
sys.exit()


