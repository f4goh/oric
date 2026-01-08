# ---------------------------------------------------------
# Compression AY : Delta + Bitmask (14 registres par frame)
# ---------------------------------------------------------

INPUT_FILE  = "COSMICP1.BIN"   # fichier source (multiples de 14 octets)
OUTPUT_FILE = "COSMICP1.COMP"    # fichier compressé

FRAME_SIZE = 14  # 14 registres AY (R0..R13)


def read_frames(filename):
    """Lit le fichier et retourne une liste de frames de 14 octets."""
    with open(filename, "rb") as f:
        data = f.read()

    if len(data) % FRAME_SIZE != 0:
        raise ValueError("Le fichier n'est pas un multiple de 14 octets")

    frames = [
        list(data[i:i+FRAME_SIZE])
        for i in range(0, len(data), FRAME_SIZE)
    ]
    return frames


def compress_frames(frames):
    """Compresse les frames avec delta + bitmask."""
    output = bytearray()

    # --- Frame 0 : stockée brute ---
    output.extend(frames[0])

    # --- Frames suivantes ---
    prev = frames[0]

    for frame in frames[1:]:
        bitmask_low = 0
        bitmask_high = 0
        values = []

        for i in range(FRAME_SIZE):
            if frame[i] != prev[i]:
                if i < 8:
                    bitmask_low |= (1 << i)
                else:
                    bitmask_high |= (1 << (i - 8))
                values.append(frame[i])

        # écrire bitmask + valeurs
        output.append(bitmask_low)
        output.append(bitmask_high)
        output.extend(values)

        prev = frame

    return output


def main():
    frames = read_frames(INPUT_FILE)
    compressed = compress_frames(frames)

    with open(OUTPUT_FILE, "wb") as f:
        f.write(compressed)

    print("Compression terminée.")
    print(f"Frames : {len(frames)}")
    print(f"Taille originale : {len(frames)*FRAME_SIZE} octets")
    print(f"Taille compressée : {len(compressed)} octets")


if __name__ == "__main__":
    main()
