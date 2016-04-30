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
    
    GImage(const GImage &img);
    GImage& operator=(const GImage& img);
    
    int width;
    int height;
    unique_ptr<float[]> a;
    
    // move semantics
    GImage(GImage&& img) : width(img.width), height(img.height), a(move(img.a)) {}
    GImage& operator=(GImage&& img) {
        this->width = img.width;
        this->height = img.height;
        a = move(img.a);
        return *this;
    }
    
    void save(const char *filename);
    void normalizeMinMax();
    QImage convert() const;
    
    ~GImage();
};

#endif // GIMAGE_H
