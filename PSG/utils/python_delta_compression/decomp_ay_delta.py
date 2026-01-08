# ---------------------------------------------------------
# Décompression AY : Delta + Bitmask (14 registres par frame)
# ---------------------------------------------------------

INPUT_FILE  = "monkey.comp"     # fichier compressé
OUTPUT_FILE = "monkey.bin"   # fichier décompressé

FRAME_SIZE = 14  # 14 registres AY (R0..R13)


def decompress_file():
    with open(INPUT_FILE, "rb") as f:
        data = f.read()

    pos = 0
    out = bytearray()

    # --- Lire la première frame brute ---
    regs = list(data[pos:pos+FRAME_SIZE])
    pos += FRAME_SIZE

    out.extend(regs)

    # --- Décompression des frames suivantes ---
    while pos < len(data):

        # Lire les bitmasks
        bitmask_low = data[pos]
        bitmask_high = data[pos+1]
        pos += 2

        # Pour chaque registre
        for i in range(FRAME_SIZE):
            if i < 8:
                bit = (bitmask_low >> i) & 1
            else:
                bit = (bitmask_high >> (i - 8)) & 1

            if bit == 1:
                # Lire nouvelle valeur
                regs[i] = data[pos]
                pos += 1
            else:
                # Valeur inchangée
                pass

        # Ajouter la frame décompressée
        out.extend(regs)

    # Écrire le fichier décompressé
    with open(OUTPUT_FILE, "wb") as f:
        f.write(out)

    print("Décompression terminée.")
    print(f"Taille décompressée : {len(out)} octets")


if __name__ == "__main__":
    decompress_file()
"""
ALGORITHME DE DECOMPRESSION AY DELTA + BITMASK
----------------------------------------------

CONSTANTES :
- Une frame AY contient 14 registres : R0..R13
- Format compressé :
    * Frame 0 : 14 octets bruts
    * Frame N (N>=1) :
        - bitmask_low  (1 octet)  -> registres R0..R7
        - bitmask_high (1 octet)  -> registres R8..R13
        - valeurs des registres modifiés (0 à 14 octets)

VARIABLES :
- regs[14] : tableau contenant les valeurs courantes des registres AY

ETAPE 1 : LIRE LA PREMIERE FRAME (FRAME 0)
------------------------------------------
Pour i de 0 à 13 :
    regs[i] = lire_octet()

Envoyer regs[] au chip AY
Attendre 20 ms

ETAPE 2 : POUR CHAQUE FRAME SUIVANTE
------------------------------------

1) Lire les deux bitmasks :
    bitmask_low  = lire_octet()
    bitmask_high = lire_octet()

2) Pour chaque registre i de 0 à 13 :

    Si i < 8 :
        bit = (bitmask_low >> i) & 1
    Sinon :
        bit = (bitmask_high >> (i - 8)) & 1

    Si bit == 1 :
        # Le registre a changé
        regs[i] = lire_octet()
    Sinon :
        # Le registre reste identique
        regs[i] = regs[i]

3) Envoyer les 14 registres au chip AY :
    pour i de 0 à 13 :
        ecrire_AY(i, regs[i])

4) Attendre 20 ms (50 Hz)

5) Revenir à l'étape 2 pour la frame suivante

----------------------------------------------
FIN DE L'ALGORITHME
----------------------------------------------
"""

