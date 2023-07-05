/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ImageBmpMono.cpp
 * Author: ale
 * 
 * Created on 30 juin 2023, 08:36
 */

#include "ImageBmpMono.h"




ImageBmpMono::ImageBmpMono() :
width(0),
height(0) {
}

ImageBmpMono::ImageBmpMono(const ImageBmpMono& orig) {
}

ImageBmpMono::~ImageBmpMono() {
}

int ImageBmpMono::loadImage(String fileName) {
    
    fileName = "/" + fileName;
    File file = SPIFFS.open(fileName, "r");
    
   if (!file) {
        printf("Erreur lors de l'ouverture du fichier BMP.\n");
        return 0;
    }
    // Lecture de l'en-tête BMP
    BMPHeader header;
    
    Serial.println(file.readBytes((char *)&header,sizeof(BMPHeader)));
        
    Serial.println(header.signature, HEX);
    Serial.println(header.file_size,HEX);
    Serial.println(header.width);
    Serial.println(header.height);
    Serial.println(header.bpp);
    Serial.println(header.pixel_offset); //la ou démarre l'image 62
    
    width = header.width;
    height = header.height;
    
    //Serial.println(header.x_resolution);
    //Serial.println(header.y_resolution);
    
    
    // Vérification de la compatibilité de l'image
    if (header.signature != 0x4D42 || header.bpp != 1 || header.width != 240 || header.height != 200) {
        printf("L'image n'est pas compatible 240x200 monochrome PSE.\n");
        file.close();
        return 0;
    }

    //file.seek(header.pixel_offset,SEEK_SET);    
    file.seek(header.pixel_offset,(SeekMode)SEEK_SET);    
    
    // Lecture des pixels
    
    file.readBytes((char *)pixels,32*header.height);
 
     
    // Fermeture du fichier BMP
    file.close();
    
    return 1;
}

int ImageBmpMono::getWidth() {
    return width;
}

int ImageBmpMono::getHeight() {
    return height;
}

uint8_t ImageBmpMono::getPixel(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        Serial.println("Les coordonnées du pixel sont en dehors des limites de l'image.");
    }
    int pixel_index = (height-1-y) * 32 + x/8;    //calclu le bon index
    uint8_t pixel_value = pixels[pixel_index];
    uint8_t offset=7-(x%8);  //calcul le décalage dans l'octet
    return   (pixel_value>>offset)&1; //récupère le pixel
}
