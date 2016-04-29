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

GImage getSobel(const GImage &img)
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

poivec getMoravec(const GImage &img, int wrad, int mrad, float thres)
{
    poivec ret;
    ret.reserve(img.width * img.height);
    
    int dirs = 8;
    int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dy[] = {1, 0, -1, 1, -1, 1, 0, -1};
    
    unique_ptr<float[]> op = make_unique<float[]>(img.height * img.width);
    
    std::fill(&op[0], &op[img.height * img.width], 0.);
    
    for (int i = wrad + 1; i < img.height - wrad - 1; i++) {
        for (int j = wrad + 1; j < img.width - wrad - 1; j++) {
            float mssd = numeric_limits<float>::max();
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
            op[i * img.width + j] = mssd;
        }
    }
    
    for (int i = wrad + 1; i < img.height - wrad - 1; i++) {
        for (int j = wrad + 1; j < img.width - wrad - 1; j++) {
            bool o = true;
            for (int k = -mrad; k <= mrad; k++) {
                for (int l = -mrad; l <= mrad; l++) {
                    if (op[i * img.width + j] <= op[(i + k) * img.width + j + l] && 
                            (k != 0 || l != 0)) {
                        o = false;
                        break;
                    }
                    if (!o)
                        break;
                }
            }
            if (o && op[i * img.width + j] > thres) {
                ret.push_back(make_tuple(j, i, op[i * img.width + j]));
            }
        }
    }
    
    return ret;
}

poivec getHarris(const GImage &img, int wrad, int mrad, float thres, float _k)
{
    poivec ret;
    ret.reserve(img.width * img.height);
    
    GConvol sx = getSobelX();
    GConvol sy = getSobelY();
    GImage dx = sx.apply(img, EdgeType_BorderCopy);
    GImage dy = sy.apply(img, EdgeType_BorderCopy);
    
    unique_ptr<float[]> lm = make_unique<float[]>(img.height * img.width);
    
    fill(&lm[0], &lm[img.height * img.width], 0.);
    
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
            lm[i * img.width + j] = ha * hc - hb * hb - _k * (ha + hc);
        }
    }
    
    for (int i = wrad + 1; i < img.height - wrad - 1; i++) {
        for (int j = wrad + 1; j < img.width - wrad - 1; j++) {
            bool o = true;
            for (int k = -mrad; k <= mrad; k++) {
                for (int l = -mrad; l <= mrad; l++) {
                    if (lm[i * img.width + j] <= lm[(i + k) * img.width + j + l] && 
                            (k != 0 || l != 0)) {
                        o = false;
                        break;
                    }
                    if (!o)
                        break;
                }
            }
            if (o && lm[i * img.width + j] > thres) {
                ret.push_back(make_tuple(j, i, lm[i * img.width + j]));
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
    float r = numeric_limits<float>::max();
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
        if (fabs((cnt * 1. / target) - 1) < diff || ((uint)target > in.size())) {
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

GImage prepareEdges(const GImage &source, EdgeType edge, int r)
{
    int width = source.width;
    int height = source.height;
    
    int cwidth = width + 2 * r;
    int cheight = height + 2 * r;
    GImage wcopy(cwidth, cheight);
    
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
            wcopy.a[(i + r) * cwidth + j + r] = val;
        }
    }
    
    return wcopy;
}

gdvector getDescriptors(const GImage &img, const poivec &vpoi)
{
    gdvector ret;
    ret.reserve(vpoi.size());
    
    int width = img.width;
//    int height = img.height;
    int r = 4;
    int cwidth = width + r * 2;
//    int cheight = height + r * 2;
    GImage wimg = prepareEdges(img, EdgeType_BorderCopy, r);
    
    GImage sx = getSobelX().apply(wimg, EdgeType_BorderCopy);
    GImage sy = getSobelY().apply(wimg, EdgeType_BorderCopy);
    
    for (int i = 0; i < int(vpoi.size()); i++) {
        int bx = get<0>(vpoi[i]) + r;
        int by = get<1>(vpoi[i]) + r;
        int curBox = 0;
        gdescriptor cdesc;
        get<1>(cdesc) = bx - r;
        get<2>(cdesc) = by - r;
        float* dv = get<0>(cdesc);
        for (int cy = -1; cy < 1; cy++) {
            for (int cx = -1; cx < 1; cx++) {
                int qy = by + cy * 4 + 1;
                int qx = bx + cx * 4 + 1;
                
                for (int gy = 0; gy < 4; gy++) {
                    for (int gx = 0; gx < 4; gx++) {
                        int y = qy + gy;
                        int x = qx + gx;
                        float dy = sy.a[y * cwidth + x];
                        float dx = sx.a[y * cwidth + x];
                        float fi = atan2f(dy, dx) + M_PI;
                        float len = sqrtf(dy * dy + dx * dx);
                        
                        float alph = fi * 4. / M_PI;
                        int drcn = int(alph);
                        int drnx = drcn + 1;
                        if (drnx == 8)
                            drnx = 0;
                        
                        float weight = alph - drcn;
                        
                        dv[curBox * 8 + drcn] += len * (1 - weight);
                        dv[curBox * 8 + drnx] += len * weight;
                    }
                }
                curBox++;
            }
        }
        for (int box = 0; box < 4; box++) {
            float len = 0.;
            for (int j = 0; j < 8; j++) {
                len = max(len, dv[box * 8 + j]);
            }
            len += 1e-5;
            for (int j = 0; j < 8; j++) {
                dv[box * 8 + j] /= len;
            }
        }
        ret.push_back(cdesc);
    }
    
    return ret;
}

vector<pair<int, int> > getMatches(const gdvector &dfirst, const gdvector &dsecond, const float thres)
{
    vector<pair<int, int> > ret;
    ret.reserve(max(dfirst.size(), dsecond.size()));
    
    for (uint i = 0; i < dfirst.size(); i++) {
        float dist = numeric_limits<float>::max();
        int id = -1;
        for (uint j = 0; j < dsecond.size(); j++) {
            float cur = 0.;
            for (uint k = 0; k < DSIZE; k++) {
                float dim = get<0>(dfirst[i])[k] - get<0>(dsecond[j])[k];
                cur += dim * dim;
            }
            if (cur < dist) {
                dist = cur;
                id = j;
            }
        }
        if (dist < thres * thres) {
            ret.push_back(make_pair(i, id));
        }
    }
    
    return ret;
}

void drawLine(QImage &img, int x1, int y1, int x2, int y2, int color)
{
    float dlt = .3 / max(abs(x1 - x2), abs(y1 - y2));
    
    for (float i = 0.; i <= 1.; i += dlt) {
        float j = 1. - i;
        img.setPixel(int(roundf(x1 * i + x2 * j)), int(roundf(y1 * i + y2 * j)), color);
    }
}
