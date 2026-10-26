#!/bin/bash

# 1. Vérification de la configuration
if [ -z "$OSDK" ]; then
    echo "== ERROR =="
    echo "The Oric SDK was not configured properly"
    echo "You should have an OSDK environment variable setted to the location of the SDK"
    exit 1
fi

# 2. Chargement des paramètres de compilation
if [ -f "./osdk_config.sh" ]; then
    source ./osdk_config.sh
else
    echo "Warning: osdk_config.sh not found."
fi

# 3. Génération de l'image
if [ ! -d "BUILD" ]; then
    mkdir "BUILD"
fi

# Appel direct de pictconv (validé par votre ls)
"$OSDK/bin/pictconv" -f1 -d0 -o2 ../../data/picture.png BUILD/picture.bin

# 4. Lancement de la compilation
if [ -f "$OSDK/bin/make.sh" ]; then
    echo "Launching compilation..."
    source "$OSDK/bin/make.sh" "$OSDKFILE"
else
    echo "Error: $OSDK/bin/make.sh not found."
    exit 1
fi

echo "Done."

