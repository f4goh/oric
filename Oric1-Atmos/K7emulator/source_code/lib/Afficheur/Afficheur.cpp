/* 
 * File:   Afficheur.cpp
 * Author: F4GOH
 * 
 */

#include "Afficheur.h"

// Constructeur de Afficheur
// Sur la carte Ballon adresse 0x3c connecté sur SDA et SCL

Afficheur::Afficheur() :
SSD1306Wire(0x3c, SDA, SCL, GEOMETRY_128_64) {
    init();
    setFont(ArialMT_Plain_10);
    flipScreenVertically();
    mode=CSAVE;
}

Afficheur::~Afficheur() {
}

/**
 * Méthode pour afficher un message
 * @param message String le message à afficher
 */
void Afficheur::afficher(String action,String fileName) {
    clear();
    setFont(ArialMT_Plain_16);
    drawString(20, 0, action);
    drawString(20, 32, fileName);
    display();
}

void Afficheur::afficherAlphabet(char caractereInverse,String fileName) {
    clear();
    setFont(ArialMT_Plain_10);
    // Afficher les lettres de A à Z en majuscule
    // Afficher les lettres de A à Z en majuscule
    for (char c = 'A'; c <= '['; c++) {
        int x = (c - 'A') % 9 * 14; // Position x sur l'écran (9 caractères par ligne, espacement de 14 pixels)
        int y = (c - 'A') / 9 * 16; // Position y sur l'écran (espacement de 16 pixels entre les lignes)

        if (c == caractereInverse) {
            // Dessiner un rectangle rempli pour créer un effet de vidéo inverse
            setColor(INVERSE);
            fillRect(x, y, 14, 16);
            if (c == '[') {
                drawFastImage(x + 2, y+2, 8, 8, okSymbol);
            } else {
                drawString(x + 2, y, String(c));
            }
            setColor(WHITE);

        } else {
            if (c == '[') {
                drawFastImage(x + 2, y+2, 8, 8, okSymbol);
            } else {
                drawString(x + 2, y, String(c));
            }
        }
    }
    drawString(10, 48,fileName);
    
    display();
}



/**
 * Méthode pour mettre a jour les fichiers dans le tableau de String
 */
void Afficheur::updatelisteFiles() {
    /*
    if (!SPIFFS.begin()) {
        Serial.println("Erreur de montage SPIFFS");
        return;
    }
    */
    File root = SPIFFS.open("/");
    if (!root) {
        Serial.println("Erreur d'ouverture du répertoire");
        return;
    }

    if (!root.isDirectory()) {
        Serial.println("Le répertoire n'est pas un dossier");
        return;
    }

    std::vector<String> newFileNames;
    File file = root.openNextFile();

    while (file) {
        //Serial.print("Nom du fichier trouvé : ");
        //Serial.println(file.name());
        newFileNames.push_back(String(file.name()));
        file = root.openNextFile();
    }

    // Vérifiez les différences entre fileNames et newFileNames
    if (newFileNames != fileNames) {
        fileNames = newFileNames;
        //Serial.println("Liste des fichiers mise à jour :");
        //for (const auto& fileName : fileNames) {
        //    Serial.println(fileName);
        }
    // else {
    //    Serial.println("Aucun changement détecté dans la liste des fichiers.");
    //}

}

int Afficheur::getMaxFiles(){
    return  fileNames.size();
}

String Afficheur::getFileName(int index){
    return fileNames[index];
}

modeTap Afficheur::getMode(){
    return mode;
}

/**
 * Méthode pour afficher cload csave
 * @param mode enum CLOAD CSAVE
 */
void Afficheur::select(String ipAddress) {
    clear();
    setFont(ArialMT_Plain_16);
    if (mode == CLOAD) { //swapp
        mode = CSAVE;
    } else {
        mode = CLOAD;
    }
    drawString(8, 0, ipAddress);
    if (mode == CLOAD) {
        // Affichage en vidéo inverse pour CLOAD
        setColor(INVERSE);
        fillRect(0, 32, 128, 16);
        drawString(32, 32, "CLOAD");  // Centré verticalement dans la moitié supérieure
        setColor(WHITE);
        drawString(32, 48, "CSAVE");  // Centré verticalement dans la moitié inférieure
    } else if (mode == CSAVE) {
        // Affichage en vidéo inverse pour CSAVE
        drawString(32, 32, "CLOAD");  // Centré verticalement dans la moitié supérieure
        setColor(INVERSE);
        fillRect(0, 48, 128, 16);
        drawString(32, 48, "CSAVE");  // Centré verticalement dans la moitié inférieure
        setColor(WHITE);
    }

    display();
}
/**
 * Méthode pour afficher la position GPS
 * @param 
 */

void Afficheur::afficherFichiersOLED(int index, int sel) {
   
    clear();
    setFont(ArialMT_Plain_16);

    int yStart = 0;
    int maxFilesToShow = 4;
    if (fileNames.size() > 0) {
        for (int i = 0; i < maxFilesToShow; i++) {
            int currentIndex = index + i;
            if (currentIndex >= fileNames.size()) {
                break; // Arrêter si nous atteignons la fin de la liste de fichiers
            }

            int y = yStart + i * 16; // Position y pour chaque ligne

            if (i == sel) {
                // Affichage en vidéo inverse pour le fichier sélectionné
                setColor(INVERSE);
                fillRect(0, y, getWidth(), 16);
                drawString(0, y, fileNames[currentIndex]);
                setColor(WHITE);
            } else {
                setColor(WHITE);
                drawString(0, y, fileNames[currentIndex]);
            }
        }

        display();
    }
}
