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
