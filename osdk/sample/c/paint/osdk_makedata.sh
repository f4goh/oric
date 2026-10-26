#!/bin/bash

# Conversion de l'image de test directement en fichier source Assembleur (.s)
"$OSDK/bin/pictconv" -f1 -d0 -o4 ../../data/flood_fill_test.png picture.s

# Fin du script (équivalent de pause)
echo "Press enter to continue..."
read -r

