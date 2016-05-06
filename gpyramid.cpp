#include "gpyramid.h"
#include "gconvol.h"

GPyramid::GPyramid()
{
    
}

GPyramid::GPyramid(const GImage &img, float _sbase, float _s0, int _olayers)
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
    a = make_unique<GImage[]>((olayers + 1) * ocnt);
    double initDlt = sqrt(sbase * sbase - s0 * s0);
    a[0] = getGaussian(initDlt).applySeparate(img, EdgeType_Mirror);
    
    double sdiff = exp2(1. / olayers);
    double effec = sbase;
    for (int i = 0; i < ocnt; i++) {
        if (i != 0) {
            GImage& tmp = a[i * (olayers + 1) - 1];
            GImage wrk(cwidth, cheight);
            auto count = make_unique<int[]>(cwidth * cheight);
            fill(&count[0], &count[cwidth * cheight], 0);
            fill(&wrk.a[0], &wrk.a[cwidth * cheight], 0.f);
            for (int y = 0; y < tmp.height; y++)
                for (int x = 0; x < tmp.width; x++) {
                    wrk.a[(y / 2) * cwidth + x / 2] += tmp.a[y * tmp.width + x];
                    count[(y / 2) * cwidth + x / 2]++;
                }
            for (int k = 0; k < cheight * cwidth; k++)
                    wrk.a[k] /= count[k]++;
            a[i * (olayers + 1)] = move(wrk);
        }
        
        for (int j = 1; j <= olayers; j++) {
            double snext = effec * sdiff;
            double dlt = sqrt(snext * snext - effec * effec);
            GConvol conv = getGaussian((float)dlt);
            a[i * (olayers + 1) + j] = conv.applySeparate(a[i * (olayers + 1) + j - 1], 
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
    layer = layer % olayers + coct * (olayers + 1);
    x >>= coct;
    y >>= coct;
    int xwidth = width;
    for (int i = 0; i < coct; i++) {
        xwidth++;
        xwidth >>= 1;
    }
    return a[layer].a[y * xwidth + x];
}

GPyramid GPyramid::getDOG()
{
    GPyramid ret;
    ret.s0 = s0;
    ret.sbase = sbase;
    ret.width = width;
    ret.height = height;
    ret.ocnt = ocnt;
    ret.olayers = olayers + 1;
    ret.a = make_unique<GImage[]>((olayers + 2) * ocnt);
    float sdiff = exp2f(1.f / (olayers));
    
    for (int i = 0; i < ocnt; i++) {
        for (int j = 0; j < olayers; j++) {
            GImage& top = a[i * (olayers + 1) + j + 1];
            GImage& bot = a[i * (olayers + 1) + j];
            GImage tmp(top.width, top.height);
            float div = (sdiff - 1.f);
            for (int y = 0; y < top.height; y++) {
                for (int x = 0; x < top.width; x++) {
                    int id = y * top.width + x;
                    tmp.a[id] = (top.a[id] - bot.a[id]) / div;
                }
            }
            ret.a[i * (ret.olayers + 1) + j + 1] = move(tmp);
        }
        if (i != 0) {
            GImage& prev = ret.a[i * (ret.olayers + 1) - 2];
            GImage& next = ret.a[i * (ret.olayers + 1) + 1];
            GImage tmp(prev.width, prev.height);
            for (int y = 0; y < prev.height; y++)
                for (int x = 0; x < prev.width; x++)
                    tmp.a[y * prev.width + x] = 
                            next.a[y / 2 * next.width + x / 2];
            ret.a[i * (ret.olayers + 1) - 1] = move(tmp);
            
            GImage to(next.width, next.height);
            auto cnt = make_unique<int[]>(next.width * next.height);
            fill(&to.a[0], &to.a[to.width * to.height], 0.f);
            fill(&cnt[0], &cnt[to.width * to.height], 0);
            for (int y = 0; y < prev.height; y++)
                for (int x = 0; x < prev.width; x++) {
                    to.a[(y / 2) * to.width + x / 2] +=
                            prev.a[y * prev.width + x];
                    cnt[(y / 2) * to.width + x / 2]++;
                }
            for (int y = 0; y < to.width * to.height; y++)
                to.a[y] /= cnt[y];
            ret.a[i * (ret.olayers + 1)] = move(to);
        }
    }
    ret.a[0] = ret.a[1];
    ret.a[ret.ocnt * (ret.olayers + 1) - 1] = 
            ret.a[ret.ocnt * (ret.olayers + 1) - 2];
    
    return ret;
}

