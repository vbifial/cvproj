#include "gconvol.h"
#include "gimage.h"

GConvol::GConvol()
{
    r = 0;
}

GConvol::GConvol(int r)
{
    this->r = r;
    unique_ptr<float[]> a(new float[(r * 2 + 1) * (r * 2 + 1)]);
    this->a = move(a);
}

GImage GConvol::apply(const GImage &img, EdgeType edge)
{
    int width = img.width;
    int height = img.height;
    int cwidth = width + 2 * r;
    GImage wcopy = prepareEdges(img, edge, r);
    GImage res(width, height);
    
    int csize = r * 2 + 1;
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            float cur = 0;
            for (int l = 0; l < csize; l++) {
                for (int k = 0; k < csize; k++) {
                    cur += a[(csize - l - 1) * csize + (csize - k - 1)] * 
                            wcopy.a[(i + l) * cwidth + j + k];
                }
            }
            res.a[i * width + j] = cur;
        }
    }
    
    return res;
}

GImage GConvol::applySeparate(const GImage &img, EdgeType edge)
{
    int width = img.width;
    int height = img.height;
    int cwidth = width + 2 * r;
    int cheight = height + 2 * r;
    
    GImage wcopy = prepareEdges(img, edge, r);
    GImage xcopy = prepareEdges(img, edge, r);
    GImage res(width, height);
    
    int csize = 2 * r + 1;
    float vx[csize];
    float vy[csize];
    for (int i = 0; i < csize; i++)
        vx[i] = vy[i] = 0.f;
    for (int i = 0; i < csize; i++) {
        for (int j = 0; j < csize; j++) {
            float val = a[i * csize + j];
            vx[csize - i - 1] += val;
            vy[csize - j - 1] += val;
        }
    }
    
    for (int i = 0; i < cheight; i++) {
        for (int j = 0; j < width; j++) {
            float val = 0.f;
            for (int k = 0; k < csize; k++) {
                val += vx[k] * wcopy.a[i * cwidth + j + k];
            }
            xcopy.a[i * cwidth + j + r] = val;
        }
    }
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            float val = 0.f;
            for (int k = 0; k < csize; k++) {
                val += vy[k] * xcopy.a[(i + k) * cwidth + j + r];
            }
            res.a[i * width + j] = val;
        }
    }
    
    return res;
}



GConvol::~GConvol()
{
    
}

