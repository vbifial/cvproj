#pragma once
#ifndef GCONVOL_H
#define GCONVOL_H
#include "common.h"

class GConvol
{
private:
    
    GImage prepare(const GImage &source, EdgeType edge);

public:
    GConvol();
    GConvol(int r);
    int r;
    unique_ptr<float[]> a;
    
    // move semantics
    GConvol(GConvol&& conv) : a(move(conv.a)){}
    GConvol& operator=(GConvol&& conv) {
        a = move(conv.a);
        return *this;
    }
    
    GImage apply(const GImage &img, EdgeType edge);
    GImage applySeparate(const GImage &img, EdgeType edge);
    
    ~GConvol();
};

#endif // GCONVOL_H
