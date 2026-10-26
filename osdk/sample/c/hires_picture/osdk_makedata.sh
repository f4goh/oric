#!/bin/bash

# Conversion de l'image PNG directement en fichier source Assembleur (.s)
# L'argument -o4 indique le format de sortie assembleur texte
"$OSDK/bin/pictconv" -f1 -d0 -o4 ../../data/picture.png picture.s

# Fin du script (équivalent de pause)
echo "Press enter to continue..."
read -r

