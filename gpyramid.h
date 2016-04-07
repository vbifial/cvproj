#ifndef GPYRAMID_H
#define GPYRAMID_H
#include "common.h"
#include "gimage.h"

class GPyramid
{
public:
    GPyramid();
    
    unique_ptr<GImage[]> a;
    float s0;
    int olayers;
    int ocnt;
    
    GPyramid(GImage &img, float _s0, int _olayers);
    
    //move semantics
    GPyramid(GPyramid&& pyr) : a(move(pyr.a)) {}
    GPyramid& operator=(GPyramid&& pyr) {
        a = move(pyr.a);
        return *this;
    }
    
};

#endif // GPYRAMID_H
