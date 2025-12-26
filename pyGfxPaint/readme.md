# Éditeur graphique Oric – Mode d’emploi

## 1. Lancement

- Lancer le script Python avec thonny ide, installer la bibliothèque pygame.  
- Fenêtre logique : 240x200 (zoom x3)
- Deux modes :
  - Mode Pixels
  - Mode Attributs (ink/paper/inverse/couleur)

---

## 2. Commandes clavier

### Basculer de mode
- ESPACE : alterner entre Mode Pixels et Mode Attributs

### Gestion des attributs
- i : sélectionner INK
- p : sélectionner PAPER
- n : sélectionner NOIP (aucun attribut ink/paper)
- v : activer/désactiver l’inversion (bit 7)
- 0 à 7 : choisir la couleur d’attribut (0=noir, 7=blanc)

### Sauvegarde / chargement
- s : sauvegarder en TAP via gfx.saveTap()
- u : charger ghost.bin puis rafraîchir l’affichage

---

## 3. Commandes souris

### Mode Pixels (show_attrib = False)
- Clic gauche : gfx.setPixel(x, y)
- Clic droit : gfx.clearPixel(x, y)
- Mise à jour automatique de la ligne modifiée

### Mode Attributs (show_attrib = True)
- Clic gauche :
  - dessine une barre horizontale de 6 pixels (colonne Oric)
  - applique l’attribut : couleur, ink/paper, inverse
- Clic droit :
  - réinitialise l’attribut (gfx.resetAttribut)

---

## 4. Couleurs Oric

0 : noir  
1 : rouge  
2 : vert  
3 : jaune  
4 : bleu  
5 : magenta  
6 : cyan  
7 : blanc  

---

## 5. Structure des attributs Oric (rappel)

Bits : d7 d6 d5 d4 d3 d2 d1 d0

- d7 : inverse
- d6 ou d5 = 1 : attribut ink
- d6 = 0 et d5 = 0 : attribut paper
- d0–d2 : couleur (0–7)

Exemples :
- 1000 0000 : inverse paper
- 1100 0000 : inverse paper + ink

---

## 6. Sauvegarde Oric (monitor)

Dans Oricutron :
- F2 : monitor
- fw <addr> <len> <file> : sauvegarde mémoire en binaire

Exemple :
fw $A000 $1F40 ghost.bin

Le fichier est placé dans la racine d’Oricutron.

---

## 7. Résumé rapide

1. Mode Pixels :
   - clic gauche = dessiner
   - clic droit = effacer

2. Mode Attributs :
   - espace pour basculer
   - i/p/n pour ink/paper/noip
   - 0–7 pour couleur
   - v pour inverse
   - clic gauche = poser attribut
   - clic droit = reset attribut

3. Sauvegarde :
   - s = saveTap()
   - u = charger ghost.bin

