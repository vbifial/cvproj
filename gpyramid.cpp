#include "gpyramid.h"
#include "gconvol.h"

GPyramid::GPyramid()
{
    
}

GPyramid::GPyramid(GImage &img, float _s0, int _olayers)
{
    int val = min(img.width, img.height);
    ocnt = 1;
    while (val > 10) {
        val /= 2;
        ocnt++;
    }
    int cwidth = img.width;
    int cheight = img.height;
    
    s0 = _s0;
    olayers = _olayers;
    a = make_unique<GImage[]>(olayers * ocnt);
    a[0] = img;
    
    double sdiff = exp2(1. / olayers);
    double effec = s0;
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
        
        effec /= 2;
        cwidth /= 2;
        cheight /= 2;
    }
    
}

