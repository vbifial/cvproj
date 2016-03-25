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
    unique_ptr<float[]> a;
    
    // move semantics
    GImage(GImage&& img) : a(move(img.a)){}
    GImage& operator=(GImage&& img) {
        a = move(img.a);
        return *this;
    }
    
    void save(const char *filename);
    void normalizeMinMax();
    
    ~GImage();
};

#endif // GIMAGE_H
