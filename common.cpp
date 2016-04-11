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

GConvol getSobelX()
{
    GConvol ret;
    ret.r = 1;
    ret.a = unique_ptr<float[]>(new float[9] {-1, 0, 1, -2, 0, 2, -1, 0, 1});
    return ret;
}

GConvol getSobelY()
{
    GConvol ret;
    ret.r = 1;
    ret.a = unique_ptr<float[]>(new float[9] {-1, -2, -1, 0, 0, 0, 1, 2, 1});
    return ret;
}

GImage getSobel(GImage &img)
{
    int width = img.width;
    int height = img.height;
    GImage ret(width, height);
    
    GConvol sx = getSobelX();
    GConvol sy = getSobelY();
    GImage dx = sx.apply(img, EdgeType_Mirror);
    GImage dy = sy.apply(img, EdgeType_Mirror);
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            float vx = dx.a[i * width + j];
            float vy = dy.a[i * width + j];
            
            float val = sqrtf(vx * vx + vy * vy);
            
            ret.a[i * width + j] = val;
        }
    }
    
    return ret;
}

GConvol getGaussian(float sigma)
{
    int r = (int)round((sigma * 3) + 0.5);
    GConvol ret(r);
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
            ret.a[i * size + j] = val;
        }
    }
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            ret.a[i * size + j] /= sum;
        }
    }
    
    return ret;
}

int getTimeMill()
{
    return chrono::duration_cast<chrono::milliseconds>
            (chrono::system_clock::now().time_since_epoch()).count();
}

poivec getMoravec(GImage &img, int wrad, int mrad, float thres)
{
    poivec ret;
    ret.reserve(img.width * img.height);
    
    int dirs = 8;
    int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dy[] = {1, 0, -1, 1, -1, 1, 0, -1};
    
    float op[img.height][img.width];
    
    for (int i = 0; i < img.height; i++)
        for (int j = 0; j < img.width; j++)
            op[i][j] = 0.;
    
    for (int i = wrad + 1; i < img.height - wrad - 1; i++) {
        for (int j = wrad + 1; j < img.width - wrad - 1; j++) {
            float mssd = 1e10;
            for (int k = 0; k < dirs; k++) {
                float sum = 0;
                for (int y = -wrad; y <= wrad; y++) {
                    for (int x = -wrad; x <= wrad; x++) {
                        float val = img.a[(i + y) * img.width + j + x] - 
                                img.a[(i + y + dy[k]) * img.width + j + x + dx[k]];
                        sum += val * val;
                    }
                }
                mssd = min(mssd, sum);
            }
            op[i][j] = mssd;
        }
    }
    
    for (int i = wrad + 1; i < img.height - wrad - 1; i++) {
        for (int j = wrad + 1; j < img.width - wrad - 1; j++) {
            bool o = true;
            for (int k = -mrad; k <= mrad; k++) {
                for (int l = -mrad; l <= mrad; l++) {
                    if (op[i][j] <= op[i + k][j + l] && (k != 0 || l != 0)) {
                        o = false;
                        break;
                    }
                    if (!o)
                        break;
                }
            }
            if (o && op[i][j] > thres) {
                ret.push_back(make_tuple(j, i, op[i][j]));
            }
        }
    }
    
    return ret;
}

poivec getHarris(GImage &img, int wrad, int mrad, float thres, float _k)
{
    poivec ret;
    ret.reserve(img.width * img.height);
    
    GConvol sx = getSobelX();
    GConvol sy = getSobelY();
    GImage dx = sx.apply(img, EdgeType_BorderCopy);
    GImage dy = sy.apply(img, EdgeType_BorderCopy);
    
    float lm[img.height][img.width];
    for (int i = 0; i < img.height; i++)
        for (int j = 0; j < img.width; j++)
            lm[i][j] = 0.;
    
    for (int i = wrad; i < img.height - wrad; i++) {
        for (int j = wrad; j < img.width - wrad; j++) {
            float ha = 0;
            float hb = 0;
            float hc = 0;
            for (int k = -wrad; k <= wrad; k++) {
                for (int l = -wrad; l <= wrad; l++) {
                    float cdx = dx.a[(i + k) * img.width + j + l];
                    float cdy = dy.a[(i + k) * img.width + j + l];
                    ha += cdx * cdx;
                    hb += cdx * cdy;
                    hc += cdy * cdy;
                }
            }
            lm[i][j] = ha * hc - hb * hb - _k * (ha + hc);
        }
    }
    
    for (int i = wrad + 1; i < img.height - wrad - 1; i++) {
        for (int j = wrad + 1; j < img.width - wrad - 1; j++) {
            bool o = true;
            for (int k = -mrad; k <= mrad; k++) {
                for (int l = -mrad; l <= mrad; l++) {
                    if (lm[i][j] <= lm[i + k][j + l] && (k != 0 || l != 0)) {
                        o = false;
                        break;
                    }
                    if (!o)
                        break;
                }
            }
            if (o && lm[i][j] > thres) {
                ret.push_back(make_tuple(j, i, lm[i][j]));
            }
        }
    }
    
//    GImage q(img.width, img.height);
//    for (int i = 0; i < q.height; i++)
//        for (int j = 0; j < q.width; j++)
//            q.a[i * q.width + j] = sqrtf(lm[i][j]);
//    q.normalizeMinMax();
//    q.save("xhar.jpg");
    
    return ret;
}

poivec anms(poivec &in, int target, float diff) {
    poivec ret;
    ret.reserve(in.size());
    float l = 0.;
    float r = 1e5;
    bool u[in.size()];
    while (true) {
        float m = (l + r) / 2.;
        int cnt = 0;
        for (uint i = 0; i < in.size(); i++) {
            bool o = true;
            for (uint j = 0; j < in.size(); j++) {
                if (i != j) {
                    float dx = get<0>(in[i]) - get<0>(in[j]);
                    float dy = get<1>(in[i]) - get<1>(in[j]);
                    float dist = dx * dx + dy * dy;
                    if (dist < m * m && get<2>(in[i]) <= get<2>(in[j])) {
                        o = false;
                        break;
                    }
                }
            }
            if (o)
                cnt++;
            u[i] = o;
        }
        if (fabs((cnt * 1. / target) - 1) < diff) {
            l = m;
            break;
        }
        if (cnt < target)
            r = m;
        else
            l = m;
    }
    for (uint i = 0; i < in.size(); i++)
        if (u[i]) {
            ret.push_back(in[i]);
        }
    return ret;
}

void mark(QImage &img, int x, int y)
{
    int size = 4;
    int dx[] = {0, 0, -1, 1};
    int dy[] = {-1, 1, 0, 0};
    for (int i = 0; i < size; i++) {
        if (x + dx[i] > -1 && y + dy[i] > -1 &&
                x + dx[i] < img.width() && 
                y + dy[i] < img.height()) {
            img.setPixel(x + dx[i], y + dy[i], 255 << 8);
        }
    }
}
