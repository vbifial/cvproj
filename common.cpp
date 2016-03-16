#include "common.h"
#include "gconvol.h"
#include "gimage.h"

float fromRGB(int color) {
    float ret = 0;
    for (int i = 0; i < 3; i++) {
        int cur = (color >> ((2 - i) * 8)) & 255;
        ret += cur / 255. * rgbLum[i];
    }
    return ret;
}

int toRGB(float color) {
    int ret = 0;
    for (int i = 0; i < 3; i++) {
        ret <<= 8;
        ret += (int)(color * 255);
    }
    return ret;
}


GConvol *getSobelX()
{
    GConvol *ret = new GConvol();
    ret->r = 1;
    ret->a = new float[9] {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    return ret;
}

GConvol *getSobelY()
{
    GConvol *ret = new GConvol();
    ret->r = 1;
    ret->a = new float[9] {-1, -2, -1, 0, 0, 0, 1, 2, 1};
    return ret;
}

GImage *getSobel(GImage &img)
{
    int width = img.width;
    int height = img.height;
    GImage *ret = new GImage(width, height);
    
    unique_ptr<GConvol> sx(getSobelX());
    unique_ptr<GConvol> sy(getSobelY());
    unique_ptr<GImage> dx(sx->apply(img, EdgeType_Mirror));
    unique_ptr<GImage> dy(sy->apply(img, EdgeType_Mirror));
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            float vx = dx->a[i * width + j];
            float vy = dy->a[i * width + j];
            
            float val = sqrtf(vx * vx + vy * vy);
            
            ret->a[i * width + j] = val;
        }
    }
    
    return ret;
}

GConvol *getGaussian(float sigma)
{
    int r = (int)round((sigma * 3) + 0.5);
    GConvol *ret = new GConvol(r);
    int size = r * 2 + 1;
    float sum = 0.f;
    float pi = (float)M_PI;
    float sqs = sigma * sigma;
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int dx = i - r;
            int dy = j - r;
            float val = 1.f / (2.f * pi * sqs) * 
                    expf(-(dx * dx + dy * dy) / (2.f * sqs));
            sum += val;
            ret->a[i * size + j] = val;
        }
    }
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            ret->a[i * size + j] /= sum;
        }
    }
    
    return ret;
}
