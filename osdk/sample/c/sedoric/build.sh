#!/bin/bash

# 1. Vérification de la variable d'environnement OSDK
if [ -z "$OSDK" ]; then
    echo "== ERROR =="
    echo "The Oric SDK was not configured properly."
    echo "You should have an OSDK environment variable set to the location of the SDK."
    exit 1
fi

# 2. Chargement de la configuration du projet
if [ -f "./osdk_config.sh" ]; then
    source ./osdk_config.sh
else
    echo "Warning: osdk_config.sh not found."
fi

# 3. Création du dossier BUILD s'il n'existe pas
if [ ! -d "BUILD" ]; then
    mkdir "BUILD"
fi

# 4. Lancement de la compilation unifiée (sans guillemets autour de $OSDKFILE)
echo "Building the main program..."
"$OSDK/bin/make.sh" $OSDKFILE

# Vérification si le fichier .tap principal a bien été généré
# Note : L'OSDK génère souvent le dossier en majuscules (BUILD/) ou minuscules (build/) selon les scripts,
# nous testons les deux pour éviter les erreurs de casse sous Linux.
TARGET_TAP=""
if [ -f "BUILD/${OSDKNAME}.tap" ]; then
    TARGET_TAP="BUILD/${OSDKNAME}.tap"
elif [ -f "build/${OSDKNAME}.tap" ]; then
    TARGET_TAP="build/${OSDKNAME}.tap"
else
    echo "== ERROR == Main TAP file (${OSDKNAME}.tap) was not generated."
    exit 1
fi

# 5. Conversion de l'image en fichier TAP chargeable à l'adresse $A000
echo "Converting picture to TAP..."
"$OSDK/bin/pictconv" -f1 -d0 -o1 ../../data/picture.png BUILD/picture.tap

# Vérification si le TAP de l'image est prêt
if [ ! -f "BUILD/picture.tap" ]; then
    echo "== ERROR == pictconv failed to generate BUILD/picture.tap"
    exit 1
fi

# 6. Construction de la disquette SEDORIC
# tap2dsk regroupe les TAPs et old2mfm convertit l'image disque au format MFM pour l'émulateur
echo "Creating SEDORIC floppy image..."
"$OSDK/bin/tap2dsk" -nSEDDEMO -iSEDDEMO "$TARGET_TAP" BUILD/picture.tap BUILD/seddemo.dsk

if [ -f "BUILD/seddemo.dsk" ]; then
    "$OSDK/bin/old2mfm" BUILD/seddemo.dsk
    echo "Floppy build successful: BUILD/seddemo.dsk is ready!"
else
    echo "== ERROR == tap2dsk failed to generate BUILD/seddemo.dsk"
    exit 1
fi

