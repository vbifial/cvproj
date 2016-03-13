#pragma once
#ifndef GIMAGE_H
#define GIMAGE_H
#include "common.h"

class GImage
{
public:
    GImage();
    GImage(int width, int height);
    GImage(QImage &img);
    int width;
    int height;
    float *a;
    
    void save(const char *filename);
    
    ~GImage();
};

#endif // GIMAGE_H
