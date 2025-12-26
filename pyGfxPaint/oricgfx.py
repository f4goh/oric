
ORIC_WIDTH=240
ORIC_HEIGHT=200

"""
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
"""

inverted =[7,6,5,4,3,2,1,0]

INK=0
PAPER=1
NOIP=2
INVERTED=True

class Oricgfx:
    def __init__(self):
        self.ram = [[0X40 for _ in range(ORIC_WIDTH//6)] for _ in range(ORIC_HEIGHT)]
        self.pixels = [[0 for _ in range(ORIC_WIDTH)] for _ in range(ORIC_HEIGHT)]
        self.ptr=0
        
    def affiche(self):
        # Afficher les 10 premières lignes en hexadécimal
        for y in range(5):
            ligne = self.ram[y]
            # Convertir chaque valeur en hexadécimal sur 2 chiffres
            hex_line = " ".join(f"{ligne[n]:02X}" for n in range(len(ligne)//2))
            print(f"L {y:02d}: {hex_line}")
    
    def getLinePx(self,line):
        return self.pixels[line]
    
    def getLineAtt(self,line):
        return self.ram[line]

    def mapSixPaper(self,line,paper,invert):
        if invert:
            ncoul=inverted[paper]
        else:
            ncoul=paper
        for n in range (6):
            self.pixels[line][self.ptr]=ncoul
            self.ptr+=1
 
    def mapSixPixel(self,line,paper,ink,invert,pixel):
        if invert:
            ncoulPaper=inverted[paper]
            ncoulInk=inverted[ink]
        else:
            ncoulPaper=paper
            ncoulInk=ink
        mask=0x20
        for n in range (6):
            if pixel & mask:
                self.pixels[line][self.ptr]=ncoulInk
            else:
                self.pixels[line][self.ptr]=ncoulPaper
            self.ptr+=1
            mask>>=1

    def convertLine(self, line):
        ink=7
        paper=0
        coul=0
        self.ptr=0
        for mot in self.ram[line]:
            invert=mot & 0x80
            if mot & 0x60==0: #attribut
                attribut=mot & 0x1f               
                if attribut & 0x10 == 0:  #ink
                    ink=attribut & 0x07  #avant 0x0f
                    self.mapSixPaper(line,paper,invert)
                else:
                    paper=attribut & 0x07 #avant 0x0f
                    self.mapSixPaper(line,paper,invert)                
            else:#pixel
                self.mapSixPixel(line,paper,ink,invert,mot & 0x3f)


    def setPixel(self,x,y):
        octet=x//6
        msk=1 << (5 - (x%6))        
        val=self.ram[y][octet]
        if val & 0x40==0x40:
            self.ram[y][octet]|=msk
        self.convertLine(y)

    def clearPixel(self,x,y):
        octet=x//6
        msk=1 << (5 - (x%6))
        msk=(~msk) & 0b11111111
        val=self.ram[y][octet]
        if val & 0x40==0x40:
            self.ram[y][octet]&=msk
        self.convertLine(y)
        
    def setAttribut(self,x,y,coul,attFlag,attInvert):
        octet=x//6
        if attInvert==INVERTED:
            self.ram[y][octet]|=0x80
        else:
            self.ram[y][octet]&=0x7f
        if attFlag!=NOIP:
            if attFlag==PAPER:
                coul=coul+16
                self.ram[y][octet]&=0x80
                self.ram[y][octet]|=coul
            elif attFlag==INK:
                self.ram[y][octet]&=0x80
                self.ram[y][octet]|=coul
        #print(hex(self.ram[y][octet]))
        self.convertLine(y)    
        

    def resetAttribut(self,x,y):
        octet=x//6
        self.ram[y][octet]=0x40
        self.convertLine(y)
        
    def saveTap(self, filename="test.tap"):
        # Conversion du header C en liste Python
        
        header = [
            0x16, 0x16, 0x16, 0x24, 0x00, 0xFF, 0x00, 0xC7, 0x05, 0x11, 0x05, 0x01, 0x00,
            0x48, 0x49, 0x52, 0x4C, 0x4F, 0x41, 0x44, 0x00, 0x07, 0x05, 0x0A, 0x00, 0xA2,
            0x00, 0x0F, 0x05, 0x14, 0x00, 0xB6, 0x22, 0x22, 0x00, 0x00, 0x00, 0x55,
            0x16, 0x16, 0x16, 0x24, 0x00, 0x00, 0x80, 0x00, 0xBF, 0x3F, 0xA0, 0x00,
            0x00, 0x00
        ]
        #header = [0x16, 0x16, 0x16, 0x24, 0x00, 0x00, 0x80, 0x00, 0xBF, 0x3F, 0xA0, 0x00,0x00, 0x00]
        with open(filename, "wb") as f:
            # Écriture du header
            f.write(bytes(header))

            # Écriture de la RAM (ligne par ligne)
            for y in range(ORIC_HEIGHT):
                f.write(bytes(self.ram[y]))
        print("saved")

    def loadBin(self, filename):
        # Lecture du fichier binaire
        with open(filename, "rb") as f:
            data = f.read()

        # Vérification de la taille
        if len(data) != 8000:
            raise ValueError(f"Le fichier doit faire 8000 octets, reçu {len(data)}")

        # Remplissage de la RAM
        index = 0
        for y in range(ORIC_HEIGHT):
            for x in range(ORIC_WIDTH // 6):
                self.ram[y][x] = data[index]
                index += 1
                self.convertLine(y)
    

    def dumpHex(self):
        #for y in range(ORIC_HEIGHT):
        #for y in range(10):
            y=40
            line = " ".join(f"{v:02X}" for v in self.ram[y])
            print(line)


    


    
        
    