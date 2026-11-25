/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ImageBmpMono.h
 * Author: ale
 *
 * Created on 30 juin 2023, 08:36
 */

#ifndef IMAGEBMPMONO_H
#define IMAGEBMPMONO_H

#include <Arduino.h>
#include "SPIFFS.h"

#pragma pack(push,1)
typedef struct {
    uint16_t signature;
    uint32_t file_size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t pixel_offset;
    uint32_t header_size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bpp;
    uint32_t compression;
    uint32_t image_size;
    int32_t x_resolution;
    int32_t y_resolution;
    uint32_t colors;
    uint32_t important_colors;
} BMPHeader; 
#pragma pack(pop)

class ImageBmpMono {
public:
    ImageBmpMono();
    ImageBmpMono(const ImageBmpMono& orig);
    virtual ~ImageBmpMono();
    int loadImage(String fileName);
    int getWidth();
    int getHeight();
    uint8_t getPixel(int x, int y);
    
private:
    uint8_t pixels[32*200];
    int width;
    int height; 
};

#endif /* IMAGEBMPMONO_H */

