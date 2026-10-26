#!/bin/bash

# Création du dossier de build local s'il n'existe pas
if [ ! -d "BUILD" ]; then
    mkdir "BUILD"
fi

# 1. Conversion de l'image PNG en format haute résolution Oric (.hir)
echo "Converting image..."
"$OSDK/bin/pictconv" -f1 -d0 -o2 ../../data/picture.png "BUILD/picture.hir"

# Vérification du succès de pictconv
if [ ! -f "BUILD/picture.hir" ]; then
    echo "== ERROR == pictconv failed to generate BUILD/picture.hir"
    echo "Check if ../../data/picture.png exists."
    echo "Press enter to continue..."
    read -r
    exit 1
fi

# 2. Compression/Compactage du fichier d'image obtenu
echo "Compacting file..."
"$OSDK/bin/filepack" -p "BUILD/picture.hir" "BUILD/picture.pak"

# Vérification du succès de filepack
if [ ! -f "BUILD/picture.pak" ]; then
    echo "== ERROR == filepack failed to generate BUILD/picture.pak"
    echo "Press enter to continue..."
    read -r
    exit 1
fi

# 3. Conversion du fichier binaire compressé en fichier source Assembleur (.s)
echo "Generating assembly file (picture.s)..."
"$OSDK/bin/bin2txt" -s1 -f2 "BUILD/picture.pak" picture.s _LabelPicture

echo "Done successfully!"
echo "Press enter to continue..."
read -r

