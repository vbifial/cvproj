#pragma once
#ifndef GCONVOL_H
#define GCONVOL_H
#include "common.h"

class GConvol
{
private:
    
    GImage* prepare(GImage &source, EdgeType edge);

public:
    GConvol();
    int r;
    float *a;
    
    GImage* apply(GImage &img, EdgeType edge);
    GImage* applySeparate(GImage &img, EdgeType edge);
    
    ~GConvol();
};

#endif // GCONVOL_H
