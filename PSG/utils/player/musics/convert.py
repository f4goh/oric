#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Nom du fichier source binaire (14 octets par frame)
INPUT_FILE = "xenon2.ym"

# Nom du fichier JS de sortie
OUTPUT_FILE = "xenon2.js"

# Métadonnées
TITLE = "Antiriad"
AUTHOR = "Richard Joseph"
COMMENTS = "Converted from AY binary"
FREQUENCY = 1000000
FRAMERATE = 50


def read_frames(filename):
    with open(filename, "rb") as f:
        data = f.read()

    if len(data) % 14 != 0:
        raise ValueError("Le fichier doit être un multiple de 14 octets.")

    frames = []
    for i in range(0, len(data), 14):
        frame14 = list(data[i:i+14])
        frame16 = frame14 + [0x00, 0x00]  # Ajout des 2 octets manquants
        frames.append(frame16)

    return frames


def export_js(frames, filename):
    with open(filename, "w", encoding="utf-8") as js:
        js.write("export const Music = {\n")
        js.write(f"    title: '{TITLE}',\n")
        js.write(f"    author: '{AUTHOR}',\n")
        js.write(f"    comments: '{COMMENTS}',\n")
        js.write(f"    frequency: {FREQUENCY},\n")
        js.write(f"    framerate: {FRAMERATE},\n")
        js.write(f"    length: {len(frames)},\n")
        js.write("    frames: [\n")

        for frame in frames:
            hex_values = ", ".join(f"0x{b:02x}" for b in frame)
            js.write(f"        [ {hex_values} ],\n")

        js.write("    ]\n")
        js.write("};\n")


def main():
    frames = read_frames(INPUT_FILE)
    export_js(frames, OUTPUT_FILE)
    print(f"OK : {len(frames)} frames converties → {OUTPUT_FILE}")


if __name__ == "__main__":
    main()
