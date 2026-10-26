#!/bin/bash

if [ -z "$OSDK" ]; then
    echo "== ERROR =="
    echo "The Oric SDK was not configured properly."
    exit 1
fi

source ./osdk_config.sh

# Génération du fichier HTML via l'utilitaire MemMap compilé sous Linux
if [ -f "$OSDK/bin/MemMap" ]; then
    "$OSDK/bin/MemMap" BUILD/symbols BUILD/map.htm "$OSDKNAME" "$OSDK/documentation/documentation.css"
elif [ -f "$OSDK/bin/memmap" ]; then
    "$OSDK/bin/memmap" BUILD/symbols BUILD/map.htm "$OSDKNAME" "$OSDK/documentation/documentation.css"
else
    echo "== ERROR == L'utilitaire memmap est introuvable dans $OSDK/bin"
    exit 1
fi

# Ouverture automatique de la carte mémoire dans votre navigateur Linux par défaut
if [ -f BUILD/map.htm ]; then
    echo "Ouverture de la carte mémoire dans le navigateur..."
    xdg-open BUILD/map.htm
else
    echo "== ERROR == Le fichier BUILD/map.htm n'a pas pu être généré."
fi

