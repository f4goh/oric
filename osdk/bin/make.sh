#!/bin/bash

# ==============================================================================
# OSDK Oric - Script de Build Unifié (C, ASM, BASIC & MIXTE) pour Linux
# Calqué sur la logique officielle du make.bat Windows
# ==============================================================================

OSDK_BUILD_START=$(date +%s)

# --- Vérifications Initiales ---
if [ -z "$OSDK" ]; then
    echo "== ERROR =="
    echo "The Oric SDK was not configured properly"
    echo "You should have an OSDK environment variable set to the location of the SDK"
    exit 1
fi

if [ -z "$1" ]; then
    echo "== ERROR =="
    echo "This script is supposed to compile files."
    echo "You should specify one or more files to compile (without extension)."
    exit 1
fi

# --- Configuration des Valeurs par Défaut ---
[ -z "$OSDKNAME" ] && OSDKNAME="OSDK"
[ -z "$OSDKTAPNAME" ] && OSDKTAPNAME="OSDK"
[ -z "$OSDKADDR" ] && OSDKADDR="\$600"
[ -z "$OSDKCOMP" ] && OSDKCOMP="-O2"
[ -z "$OSDKXAPARAMS" ] && OSDKXAPARAMS="-W -C"
[ -z "$OSDKMACROEXPAND" ] && OSDKMACROEXPAND="1"
[ -z "$OSDKVERBOSITY" ] && OSDKVERBOSITY=2
[ -z "$OSDKLIB" ] && OSDKLIB="$OSDK/lib/"

OSDKB="$OSDK/bin"
[ -z "$OSDKT" ] && OSDKT="$OSDK/TMP"
OSDKLINKLIST=""
export TMP="$OSDKT"
export TEMP="$OSDKT"

# Configuration du préprocesseur par défaut (mcpp sous Linux)
if [ -z "$OSDKCPP" ]; then
    #OSDKCPP="$OSDKB/mcpp"
    OSDKCPP="/usr/bin/mcpp"
    [ -z "$OSDKCPPMODEFLAGS" ] && OSDKCPPMODEFLAGS="-V199901L -I-"
else
    [ -z "$OSDKCPPMODEFLAGS" ] && OSDKCPPMODEFLAGS="-lang-c++ -nostdinc"
fi

# Lecture de la version OSDK
OSDKVERSION="unknown"
if [ -f "$OSDK/version.txt" ]; then
    OSDKVERSION=$(head -n 1 "$OSDK/version.txt")
fi

# --- Préparation des Répertoires de Sortie ---
mkdir -p BUILD
rm -rf "$OSDKT"
mkdir -p "$OSDKT"

echo "Building the program $OSDKNAME at address $OSDKADDR [OSDK $OSDKVERSION]"

# --- Nettoyage des anciens artefacts ---
rm -f BUILD/symbols BUILD/symbols_ext BUILD/final.out BUILD/xaerr.txt BUILD/${OSDKNAME}.tap BUILD/payload.tap
rm -f "$OSDKT/${OSDKNAME}.bas"
rm -f BUILD/*.pak

# ==============================================================================
# BOUCLE DE TRAITEMENT DES FICHIERS TRANSMIS
# ==============================================================================
# On stocke tous les arguments dans un tableau pour itérer comme le SHIFT de Windows
FILES=("$@")

for FILE in "${FILES[@]}"; do
    # Forcer la vérification en insensible à la casse sur les extensions
    # (Recherche .c, .C, .s, .S, .asm, .ASM, .bas, .BAS)
    FILE_UPPER=$(echo "$FILE" | tr '[:upper:]' '[:lower:]')
    
    # 1. Traitement des fichiers C
    if [ -f "${FILE}.c" ] || [ -f "${FILE}.C" ]; then
        REAL_FILE=$([ -f "${FILE}.c" ] && echo "${FILE}.c" || echo "${FILE}.C")
        [ "$OSDKVERBOSITY" -ge 2 ] && echo "Compiling $REAL_FILE"
        
        # Phase - Preprocess (Nettoyage des variables de conflits d'inclusions système)
        [ "$OSDKVERBOSITY" -ge 2 ] && echo "  - preprocess"
        INCLUDE="" C_INCLUDE_PATH="" CPLUS_INCLUDE_PATH="" \
        "$OSDKCPP" $OSDKCPPMODEFLAGS $OSDKCPPFLAGS -I "$OSDK/include" -D__16BIT__ -D__NOFLOAT__ -DATMOS -DOSDKNAME_${OSDKNAME} -DOSDKVER=\"$OSDKVERSION\" "$REAL_FILE" "$OSDKT/${FILE}.c"
        
        # Phase - Compiler (génération du code assembleur brut .c2)
        [ "$OSDKVERBOSITY" -ge 2 ] && echo "  - compile"
        "$OSDKB/compiler" -N"${FILE}" $OSDKCOMP $OSDKDEBUG "$OSDKT/${FILE}.c" > "$OSDKT/${FILE}.c2"
        if [ $? -ne 0 ]; then echo "ERROR : Compilation failed."; exit 1; fi
        
        # Phase - MacroSplitter (Nettoyage et expansion des macros OSDK)
        if [ "$OSDKMACROEXPAND" = "1" ]; then
            [ "$OSDKVERBOSITY" -ge 2 ] && echo "  - expand macros and cleanup output"
            "$OSDKB/macrosplitter" $OSDKMACRO -M"$OSDK/macro/macros.h" "$OSDKT/${FILE}.c2" "$OSDKT/${FILE}"
        else
            [ "$OSDKVERBOSITY" -ge 2 ] && echo "  - convert C to assembly code (legacy)"
            "$OSDKB/cpp" -lang-c++ -imacros "$OSDK/macro/macros.h" -DXA -traditional -P "$OSDKT/${FILE}.c2" "$OSDKT/${FILE}.s"
            [ "$OSDKVERBOSITY" -ge 2 ] && echo "  - cleanup output"
            "$OSDKB/macrosplitter" $OSDKMACRO "$OSDKT/${FILE}.s" "$OSDKT/${FILE}"
        fi
        
        OSDKLINKLIST="$OSDKLINKLIST $OSDKT/${FILE}"

    # 2. Traitement des fichiers ASSEMBLEUR (.s / .asm)
    elif [ -f "${FILE}.s" ] || [ -f "${FILE}.S" ] || [ -f "${FILE}.asm" ] || [ -f "${FILE}.ASM" ]; then
        # Détection de l'extension réelle employée
        if [ -f "${FILE}.s" ]; then REAL_EXT="s"; elif [ -f "${FILE}.S" ]; then REAL_EXT="S"; elif [ -f "${FILE}.asm" ]; then REAL_EXT="asm"; else REAL_EXT="ASM"; fi
        
        [ "$OSDKVERBOSITY" -ge 2 ] && echo "Assembling ${FILE}.${REAL_EXT}"
        
        # Recréer l'arborescence dans le sous-dossier temporaire si nécessaire
        DIR_PART=$(dirname "${FILE}")
        if [ "$DIR_PART" != "." ]; then mkdir -p "$OSDKT/$DIR_PART"; fi
        
        # Copie vers le répertoire de travail temporaire
        cp "${FILE}.${REAL_EXT}" "$OSDKT/${FILE}.s"
        OSDKLINKLIST="$OSDKLINKLIST $OSDKT/${FILE}.s"

    # 3. Traitement des fichiers BASIC (.bas)
    elif [ -f "${FILE}.bas" ] || [ -f "${FILE}.BAS" ]; then
        REAL_FILE=$([ -f "${FILE}.bas" ] && echo "${FILE}.bas" || echo "${FILE}.BAS")
        [ "$OSDKVERBOSITY" -ge 2 ] && echo "Converting BASIC program $REAL_FILE"
        
        # Concaténation dans le fichier BASIC composite global
        echo "#file ${REAL_FILE}" >> "$OSDKT/${OSDKNAME}.bas"
        cat "$REAL_FILE" >> "$OSDKT/${OSDKNAME}.bas"

    else
        echo "== ERROR =="
        echo "The file \"${FILE}\" is not a C, assembly code or BASIC file (.C/.S/.ASM/.BAS suffix)"
        echo "You should specify the name of the files without any extension."
        exit 1
    fi
done

# ==============================================================================
# PHASE DE LINK ET ASSEMBLAGE FINAL DES CONTENUS BINAIRES (C / ASM)
# ==============================================================================
HAS_BINARY=false
if [ -n "$(echo "$OSDKLINKLIST" | tr -d ' ')" ]; then
    HAS_BINARY=true
fi

if [ "$HAS_BINARY" = true ]; then
    [ "$OSDKVERBOSITY" -ge 2 ] && echo "Linking"
    
    # Exécution directe de l'éditeur de liens link65
    "$OSDKB/link65" $OSDKLINK -d "$OSDKLIB" -i "$OSDKLIB" -o "$OSDKT/linked.s" -f -q $OSDKLINKLIST
    if [ $? -ne 0 ]; then echo "ERROR : Linker failed."; exit 1; fi
    
    [ "$OSDKVERBOSITY" -ge 2 ] && echo "Assembling"
    # Assemblage final avec xa
    "$OSDKB/xa" -I"$OSDKLIB" "$OSDKT/linked.s" -o BUILD/final.out -e BUILD/xaerr.txt -l BUILD/symbols -S BUILD/symbols_ext -bt "$OSDKADDR" -DASSEMBLER=XA $OSDKXAPARAMS -DOSDKNAME_${OSDKNAME}
    if [ ! -f "BUILD/final.out" ]; then echo "ERROR : Assembler failed. Check BUILD/xaerr.txt"; exit 1; fi
    
    # --- Compression optionnelle (FilePack) ---
    if [ -n "$OSDKPACKADDR" ]; then
        [ "$OSDKVERBOSITY" -ge 2 ] && echo "Compressing"
        "$OSDKB/FilePack" -p0 BUILD/final.out "$OSDKT/final.pak"
        
        [ "$OSDKVERBOSITY" -ge 2 ] && echo "  - Converting binary to text format"
        "$OSDKB/bin2txt" -s1 -f2 "$OSDKT/final.pak" "$OSDKT/final_pak.s" _PackedStart > /dev/null
        
        [ "$OSDKVERBOSITY" -ge 2 ] && echo "  - Appending depacking code"
        cat "$OSDKT/final_fp.s" "$OSDKB/unpack.s" "$OSDKT/final_pak.s" > "$OSDKT/pak_linked.s" 2>/dev/null
        
        [ "$OSDKVERBOSITY" -ge 2 ] && echo "  - Re-Assembling"
        "$OSDKB/xa" "$OSDKT/pak_linked.s" -o BUILD/final.out -e "$OSDKT/xaerr.txt" -l "$OSDKT/symbols" -bt "$OSDKPACKADDR"
        if [ ! -f "BUILD/final.out" ]; then echo "ERROR : Compressed re-assembly failed."; exit 1; fi
        
        OSDKADDR=$OSDKPACKADDR
    fi
    
    # --- Ajout du header de la Cassette (.TAP) ---
    [ "$OSDKVERBOSITY" -ge 2 ] && echo "Creating TAPE image ${OSDKNAME}.tap"
    "$OSDKB/header" -n"$OSDKTAPNAME" $OSDKHEAD BUILD/final.out BUILD/${OSDKNAME}.tap "$OSDKADDR"
fi

# ==============================================================================
# SÉQUENCE LOADER ET CODE MIXTE BASIC + BINAIRE
# ==============================================================================
if [ -f "$OSDKT/${OSDKNAME}.bas" ]; then
    if [ "$HAS_BINARY" = true ]; then
        echo "Preparing payload for the BASIC program"
        rm -f BUILD/payload.tap
        mv BUILD/${OSDKNAME}.tap BUILD/payload.tap
    fi
    
    echo "Generating TAPE file from BASIC source code"
    "$OSDKB/bas2tap" -b2t1 -color1 "$OSDKT/${OSDKNAME}.bas" BUILD/${OSDKNAME}.tap
    if [ $? -ne 0 ]; then echo "ERROR : bas2tap compilation failed."; exit 1; fi
    
    if [ -f "BUILD/payload.tap" ]; then
        # Fusionner le BASIC (.tap) et le binaire compilé (.tap) en un seul fichier mixte
        cat BUILD/${OSDKNAME}.tap BUILD/payload.tap > BUILD/${OSDKNAME}.tap.tmp
        mv BUILD/${OSDKNAME}.tap.tmp BUILD/${OSDKNAME}.tap
        rm -f BUILD/payload.tap
    fi
fi

# ==============================================================================
# CRÉATION OPTIONNELLE D'IMAGES DISQUE (.DSK)
# ==============================================================================
if [ -n "$OSDKDISK" ] && [ -n "$OSDKFILE" ]; then
    if [ -f "BUILD/${OSDKNAME}.tap" ]; then
        "$OSDKB/tap2dsk" -n"$OSDKNAME" -i"$OSDKINIST" $OSDKTAP2DSKPARAMS BUILD/${OSDKNAME}.tap BUILD/${OSDKDISK}.dsk
        "$OSDKB/old2mfm" BUILD/${OSDKDISK}.dsk
    fi
fi

# --- Calcul du temps d'exécution ---
OSDK_BUILD_END=$(date +%s)
DURATION=$((OSDK_BUILD_END - OSDK_BUILD_START))
echo "Build of ${OSDKNAME}.tap finished in ${DURATION}s."

exit 0


