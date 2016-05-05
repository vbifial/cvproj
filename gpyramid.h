#ifndef GPYRAMID_H
#define GPYRAMID_H
#include "common.h"
#include "gimage.h"

class GPyramid
{
public:
    GPyramid();
    
    unique_ptr<GImage[]> a;
    float sbase;
    float s0;
    int olayers;
    int ocnt;
    int width;
    int height;
    
    GPyramid(GImage &img, float _sbase, float _s0, int _olayers);
    
    float L(int x, int y, float sig);
    
    //move semantics
    GPyramid(GPyramid&& pyr) : a(move(pyr.a)) {}
    GPyramid& operator=(GPyramid&& pyr) {
        this->sbase = pyr.sbase;
        this->s0 = pyr.s0;
        this->olayers = pyr.olayers;
        this->ocnt = pyr.ocnt;
        this->width = pyr.width;
        this->height = pyr.height;
        a = move(pyr.a);
        return *this;
    }
    
    GPyramid getDOG();
};

#endif // GPYRAMID_H
