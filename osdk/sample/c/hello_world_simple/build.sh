#!/bin/bash

# Vérification de la variable d'environnement OSDK
if [ -z "$OSDK" ]; then
    echo "== ERROR =="
    echo "The Oric SDK was not configured properly."
    echo "You should have an OSDK environment variable set to the location of the SDK."
    exit 1
fi

# Chargement de la configuration
source ./osdk_config.sh

# Lancement de la compilation unifiée
$OSDK/bin/make.sh $OSDKFILE

