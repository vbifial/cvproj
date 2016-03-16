#include "gconvol.h"
#include "gimage.h"

GImage* GConvol::prepare(GImage &source, EdgeType edge)
{
    int width = source.width;
    int height = source.height;
    
    int cwidth = width + 2 * r;
    int cheight = height + 2 * r;
    GImage* wcopy = new GImage(cwidth, cheight);
    
    for (int i = -r; i < height + r; i++) {
        int l = i;
        if (l < 0) {
            switch (edge) {
            case EdgeType_BorderCopy:
                l = 0;
                break;
            case EdgeType_Mirror:
                l = -l;
                break;
            case EdgeType_Wrap:
                l += height;
                break;
            default:
                l = -1;
                break;
            }
        }
        if (l >= height) {
            switch (edge) {
            case EdgeType_BorderCopy:
                l = height - 1;
                break;
            case EdgeType_Mirror:
                l = 2 * height - l - 1;
                break;
            case EdgeType_Wrap:
                l -= height;
                break;
            default:
                l = -1;
                break;
            }
        }
        for (int j = -r; j < width + r; j++) {
            int k = j;
            if (k < 0) {
                switch (edge) {
                case EdgeType_BorderCopy:
                    k = 0;
                    break;
                case EdgeType_Mirror:
                    k = -k;
                    break;
                case EdgeType_Wrap:
                    k += width;
                    break;
                default:
                    k = -1;
                    break;
                }
            }
            if (k >= width) {
                switch (edge) {
                case EdgeType_BorderCopy:
                    k = width - 1;
                    break;
                case EdgeType_Mirror:
                    k = 2 * width - k - 1;
                    break;
                case EdgeType_Wrap:
                    k -= width;
                    break;
                default:
                    k = -1;
                    break;
                }
            }
            float val = 0.;
            if (l != -1 && k != -1)
                val = source.a[l * width + k];
            wcopy->a[(i + r) * cwidth + j + r] = val;
        }
    }
    
    return wcopy;
}

GConvol::GConvol()
{
    r = 0;
}

GConvol::GConvol(int r)
{
    this->r = r;
    a = new float[(r * 2 + 1) * (r * 2 + 1)];
}

GImage* GConvol::apply(GImage &img, EdgeType edge)
{
    int width = img.width;
    int height = img.height;
    int cwidth = width + 2 * r;
    unique_ptr<GImage> wcopy(prepare(img, edge));
    GImage* res = new GImage(width, height);
    
    int csize = r * 2 + 1;
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            float cur = 0;
            for (int l = 0; l < csize; l++) {
                for (int k = 0; k < csize; k++) {
                    cur += a[(csize - l - 1) * csize + (csize - k - 1)] * 
                            wcopy->a[(i + l) * cwidth + j + k];
                }
            }
            res->a[i * width + j] = cur;
        }
    }
    
    return res;
}

GImage* GConvol::applySeparate(GImage &img, EdgeType edge)
{
    int width = img.width;
    int height = img.height;
//    int cwidth = width + 2 * r;
//    int cheight = height + 2 * r;
    GImage* wcopy = prepare(img, edge);
    
    
    
    return wcopy;
}



GConvol::~GConvol()
{
    if (a != nullptr)
        delete[] a;
}

