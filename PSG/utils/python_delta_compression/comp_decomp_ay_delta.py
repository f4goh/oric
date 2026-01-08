# ---------------------------------------------------------
# Compression + Décompression AY : Delta + Bitmask
# Avec DEBUG complet frame par frame
# ---------------------------------------------------------

INPUT_FILE  = "monkey.ym"
COMP_FILE   = "monkey.comp"
DECOMP_FILE = "monkey.bin"

FRAME_SIZE = 14


# ---------------------------------------------------------
# LECTURE DES FRAMES
# ---------------------------------------------------------
def read_frames(filename):
    with open(filename, "rb") as f:
        data = f.read()

    if len(data) % FRAME_SIZE != 0:
        raise ValueError("Le fichier n'est pas un multiple de 14 octets")

    frames = [
        list(data[i:i+FRAME_SIZE])
        for i in range(0, len(data), FRAME_SIZE)
    ]
    return frames


# ---------------------------------------------------------
# COMPRESSION
# ---------------------------------------------------------
def compress_frames(frames):
    output = bytearray()

    print("\n=== COMPRESSION ===")

    # Frame 0 brute
    print("\nFrame 0 originale :", frames[0])
    output.extend(frames[0])

    prev = frames[0]

    # Frames suivantes
    for idx, frame in enumerate(frames[1:], start=1):

        print(f"\n--- Frame {idx} originale ---")
        print(frame)

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

        print("Bitmask low  :", hex(bitmask_low))
        print("Bitmask high :", hex(bitmask_high))
        print("Valeurs modifiées :", values)

        # Stockage
        output.append(bitmask_low)
        output.append(bitmask_high)
        output.extend(values)

        prev = frame

    return output


# ---------------------------------------------------------
# DECOMPRESSION
# ---------------------------------------------------------
def decompress(data):
    pos = 0
    out = bytearray()

    print("\n=== DECOMPRESSION ===")

    # Frame 0 brute
    regs = list(data[pos:pos+FRAME_SIZE])
    pos += FRAME_SIZE

    print("\nFrame 0 décompressée :", regs)
    out.extend(regs)

    # Frames suivantes
    frame_index = 1

    while pos < len(data):

        print(f"\n--- Décompression frame {frame_index} ---")

        bitmask_low = data[pos]
        bitmask_high = data[pos+1]
        pos += 2

        print("Bitmask low  :", hex(bitmask_low))
        print("Bitmask high :", hex(bitmask_high))

        for i in range(FRAME_SIZE):
            if i < 8:
                bit = (bitmask_low >> i) & 1
            else:
                bit = (bitmask_high >> (i - 8)) & 1

            if bit == 1:
                regs[i] = data[pos]
                pos += 1

        print("Frame décompressée :", regs)
        out.extend(regs)

        frame_index += 1

    return out


# ---------------------------------------------------------
# MAIN
# ---------------------------------------------------------
def main():
    frames = read_frames(INPUT_FILE)

    # Compression
    compressed = compress_frames(frames)
    with open(COMP_FILE, "wb") as f:
        f.write(compressed)

    # Décompression
    decompressed = decompress(compressed)
    with open(DECOMP_FILE, "wb") as f:
        f.write(decompressed)

    print("\n=== RESULTATS ===")
    print("Taille originale   :", len(frames) * FRAME_SIZE)
    print("Taille compressée  :", len(compressed))
    print("Taille décompressée:", len(decompressed))

    # Vérification
    original_bytes = b"".join(bytes(f) for f in frames)
    if original_bytes == decompressed:
        print("\nOK : Le fichier décompressé est IDENTIQUE à l'original")
    else:
        print("\nERREUR : Le fichier décompressé est DIFFERENT !")


if __name__ == "__main__":
    main()
