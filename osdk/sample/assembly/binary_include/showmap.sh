#!/bin/bash

# 1. Vérification de la configuration
if [ -z "$OSDK" ]; then
    echo "== ERROR =="
    echo "The Oric SDK was not configured properly"
    echo "You should have an OSDK environment variable setted to the location of the SDK"
    echo "==========="
    echo "Press enter to continue..."
    read -r
    exit 1
fi

# 2. Chargement des paramètres du projet
if [ -f "./osdk_config.sh" ]; then
    source ./osdk_config.sh
else
    echo "Warning: osdk_config.sh not found."
fi

# 3. Génération du fichier HTML (avec memmap en minuscules)
if [ -f "$OSDK/bin/memmap" ]; then
    "$OSDK/bin/memmap" BUILD/symbols BUILD/map.htm "$OSDKNAME" "$OSDK/documentation/documentation.css"
else
    echo "== ERROR == memmap binary not found in $OSDK/bin/"
    exit 1
fi

# 4. Ouverture automatique du fichier HTML sous Linux
if [ -f "BUILD/map.htm" ]; then
    echo "Opening memory map..."
    xdg-open BUILD/map.htm
else
    echo "== ERROR == BUILD/map.htm was not generated."
    exit 1
fi

