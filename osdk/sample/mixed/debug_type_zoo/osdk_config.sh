#!/bin/bash

# Configuration des paramètres du projet DBGZOO
export OSDKADDR='$800'
export OSDKNAME="DBGZOO"
export OSDKFILE="main world asm_data asm_helpers"

# -g1 génère les informations de débogage pour le code C (VS Code debugger)
export OSDKCOMP="-O2 -g1"

