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
    a = nullptr;
}

GImage* GConvol::apply(GImage &img, EdgeType edge)
{
    GImage* wcopy = prepare(img, edge);
    
    return wcopy;
}

GImage* GConvol::applySeparate(GImage &img, EdgeType edge)
{
    GImage* wcopy = prepare(img, edge);
    
    
    
    return wcopy;
}



GConvol::~GConvol()
{
    if (a != nullptr)
        delete[] a;
}

