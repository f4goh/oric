#!/bin/bash

# Sauvegarde du répertoire courant
ORIGINAL_DIR=$(pwd)

# Vérification de la configuration
if [ -z "$OSDK" ]; then
    echo "== ERROR =="
    echo "The Oric SDK was not configured properly."
    exit 1
fi

# Chargement optionnel du nom du programme si osdk_config.sh est présent
if [ -f "./osdk_config.sh" ]; then
    source ./osdk_config.sh
fi
[ -z "$OSDKNAME" ] && OSDKNAME="OSDK"

# --- GESTION DE L'ARGUMENT DISQUE (.dsk) ---
# Si un argument est passé, on l'utilise comme fichier cible, sinon on prend le .tap par défaut
if [ -n "$1" ]; then
    TARGET_FILE="$1"
    echo "Mode Disquette détecté. Assurez-vous que 'disktype = microdisc' est configuré dans oricutron.cfg."
else
    TARGET_FILE="BUILD/${OSDKNAME}.tap"
fi

# Choix de l'émulateur
if [ -z "$OSDKEMUL" ] || [ "$OSDKEMUL" = "ORICUTRON" ]; then
    echo "Lancement d'Oricutron (SDL2) avec le fichier : $TARGET_FILE..."
    
    # 1. On tente d'utiliser l'exécutable oricutron-sdl2 du dossier OSDK
    if [ -f "$OSDK/oricutron/oricutron-sdl2" ]; then
        "$OSDK/oricutron/oricutron-sdl2" $OSDKEMULPARAMS "$TARGET_FILE"
        
    # 2. Si le dossier est en majuscule (Oricutron)
    elif [ -f "$OSDK/Oricutron/oricutron-sdl2" ]; then
        "$OSDK/Oricutron/oricutron-sdl2" $OSDKEMULPARAMS "$TARGET_FILE"
        
    # 3. Repli sur la commande globale si installée sur le système
    elif command -v oricutron-sdl2 >/dev/null 2>&1; then
        oricutron-sdl2 $OSDKEMULPARAMS "$TARGET_FILE"
        
    else
        echo "== ERROR == L'exécutable oricutron-sdl2 est introuvable."
        exit 1
    fi
    
elif [ "$OSDKEMUL" = "EUPHORIC" ]; then
    echo "Euphoric n'est pas nativement supporté sous Linux sans DOSBox."
    exit 1
else
    echo "== ERROR == Valeur OSDKEMUL invalide ($OSDKEMUL)."
    exit 1
fi

# Retour au répertoire d'origine
cd "$ORIGINAL_DIR"


