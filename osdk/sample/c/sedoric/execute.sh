#!/bin/bash

# Vérification de la variable d'environnement OSDK
if [ -z "$OSDK" ]; then
    echo "== ERROR =="
    echo "The Oric SDK was not configured properly."
    echo "You should have an OSDK environment variable set to the location of the SDK."
    exit 1
fi

# Chargement de la configuration (définit OSDKNAME="SEDDEMO")
if [ -f "./osdk_config.sh" ]; then
    source ./osdk_config.sh
fi

# Sécurité si OSDKNAME n'est pas configuré
[ -z "$OSDKNAME" ] && OSDKNAME="OSDK"

# Convertit le nom en minuscules pour correspondre à "seddemo.dsk"
OSDKNAME_LOWER=$(echo "$OSDKNAME" | tr '[:upper:]' '[:lower:]')

# --- CHOIX DU FICHIER ---
if [ -n "$1" ]; then
    # Si vous passez un argument au clavier, il reste prioritaire
    TARGET="$1"
elif [ -f "BUILD/${OSDKNAME_LOWER}.dsk" ]; then
    # Si BUILD/seddemo.dsk existe, on le prend en priorité
    TARGET="BUILD/${OSDKNAME_LOWER}.dsk"
else
    # Sinon, repli sur le .tap par défaut
    TARGET="BUILD/${OSDKNAME}.tap"
fi

# Lancement de l'émulateur via le script global de l'OSDK
$OSDK/bin/execute.sh "$TARGET"

