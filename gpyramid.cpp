#include "gpyramid.h"
#include "gconvol.h"

GPyramid::GPyramid()
{
    
}

GPyramid::GPyramid(GImage &img, float _sbase, float _s0, int _olayers)
{
    int val = min(img.width, img.height);
    ocnt = 1;
    while (val > 10) {
        val /= 2;
        ocnt++;
    }
    int cwidth = img.width;
    int cheight = img.height;
    this->width = cwidth;
    this->height = cheight;
    
    s0 = _s0;
    sbase = _sbase;
    olayers = _olayers;
    a = make_unique<GImage[]>(olayers * ocnt);
    double initDlt = sqrt(sbase * sbase - s0 * s0);
    a[0] = getGaussian(initDlt).applySeparate(img, EdgeType_Mirror);
    
    double sdiff = exp2(1. / olayers);
    double effec = sbase;
    for (int i = 0; i < ocnt; i++) {
        if (i != 0) {
            double snext = effec * sdiff;
            double dlt = sqrt(snext * snext - effec * effec);
            GConvol conv = getGaussian((float)dlt);
            
            GImage tmp = conv.applySeparate(a[i * olayers - 1], 
                    EdgeType_Mirror);
            
            GImage wrk(cwidth, cheight);
            
            for (int y = 0; y < cheight; y++)
                for (int x = 0; x < cwidth; x++)
                    wrk.a[y * cwidth + x] = tmp.a[(y * tmp.width + x) * 2];
            
            a[i * olayers] = move(wrk);
            effec = snext;
        }
        
        for (int j = 1; j < olayers; j++) {
            double snext = effec * sdiff;
            double dlt = sqrt(snext * snext - effec * effec);
            GConvol conv = getGaussian((float)dlt);
            a[i * olayers + j] = conv.applySeparate(a[i * olayers + j - 1], 
                    EdgeType_Mirror);
            effec = snext;
        }
        
        effec /= 2.;
        cwidth++;
        cwidth /= 2;
        cheight++;
        cheight /= 2;
    }
    
}

float GPyramid::L(int x, int y, float sig)
{
    int layer = (int)round(log2(sig / sbase) / (1. / olayers));
    if (layer < 0)
        layer = 0;
    if (layer >= ocnt * olayers)
        layer = ocnt * olayers - 1;
    int coct = layer / olayers;
    x >>= coct;
    y >>= coct;
    int xwidth = width;
    for (int i = 0; i < coct; i++) {
        xwidth++;
        xwidth >>= 1;
    }
    return a[layer].a[y * xwidth + x];
}

