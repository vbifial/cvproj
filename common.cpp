#include "common.h"
#include "gconvol.h"
#include "gimage.h"
#include "gpyramid.h"

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
    ret.a = unique_ptr<float[]>(new float[9] {-1.f / 8, 0, 1.f / 8, 
            -2.f / 8, 0, 2.f / 8, -1.f / 8, 0, 1.f / 8});
    return ret;
}

GConvol getSobelY()
{
    GConvol ret;
    ret.r = 1;
    ret.a = unique_ptr<float[]>(new float[9] {-1.f / 8, -2.f / 8, 
            -1.f / 8, 0, 0, 0, 1.f / 8, 2.f / 8, 1.f / 8});
    return ret;
}

GConvol getFaridXSeparate()
{
    GConvol ret;
    ret.r = 2;
    ret.a = unique_ptr<float[]>(new float[25] {0.104550f, 0.292315f, 
            0.000000f, -0.292315f, -0.104550, 
            0.f, 0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 0.f, 0.f, 0.f,
            0.f, 0.f, 0.f, 0.f, 0.f,
            0.f, 0.f, 0.f, 0.f, 0.f});
    return ret;
}

GConvol getFaridYSeparate()
{
    GConvol ret;
    ret.r = 2;
    ret.a = unique_ptr<float[]>(new float[25] {0.f, 0.f, 1.f, 0.f, 0.f,
            0.104550f, 0.292315f, 0.000000f, 
            -0.292315f, -0.104550, 
            0.f, 0.f, 0.f, 0.f, 0.f,
            0.f, 0.f, 0.f, 0.f, 0.f,
            0.f, 0.f, 0.f, 0.f, 0.f});
    return ret;
}

GConvol getFaridX2Separate()
{
    GConvol ret;
    ret.r = 2;
    ret.a = unique_ptr<float[]>(new float[25] {0.232905f, 0.002668f, 
            -0.471147f, 0.002668f, 0.232905f,
            0.f, 0.f, 1.f, 0.f, 0.f, 
            0.f, 0.f, 0.f, 0.f, 0.f,
            0.f, 0.f, 0.f, 0.f, 0.f,
            0.f, 0.f, 0.f, 0.f, 0.f});
    return ret;
}

GConvol getFaridY2Separate()
{
    GConvol ret;
    ret.r = 2;
    ret.a = unique_ptr<float[]>(new float[25] {0.f, 0.f, 1.f, 0.f, 0.f,
            0.232905f, 0.002668f, -0.471147f, 0.002668f, 0.232905f, 
            0.f, 0.f, 0.f, 0.f, 0.f,
            0.f, 0.f, 0.f, 0.f, 0.f,
            0.f, 0.f, 0.f, 0.f, 0.f});
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
    for (int i = 0; i < size * size; i++)
            ret.a[i] /= sum;
    
    return ret;
}

GConvol getGaussianSeparate(float sigma)
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
    for (int i = 0; i < size * size; i++)
        ret.a[i] /= sum;
    
    vector<float> v(size);
    fill(begin(v), end(v), 0.f);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            v[i] += ret.a[i * size + j];
        }
    }
    for (int i = 0; i < size; i++)
        ret.a[i] = ret.a[i + size] = v[i];
    
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
                poi p;
                p.x = j;
                p.y = i;
                p.function = op[i * img.width + j];
                ret.push_back(p);
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
    
    float sqs = wrad / 2.;
    sqs *= sqs;
    
    for (int i = wrad; i < img.height - wrad; i++) {
        for (int j = wrad; j < img.width - wrad; j++) {
            float ha = 0;
            float hb = 0;
            float hc = 0;
            for (int k = -wrad; k <= wrad; k++) {
                for (int l = -wrad; l <= wrad; l++) {
                    float cdx = dx.a[(i + k) * img.width + j + l];
                    float cdy = dy.a[(i + k) * img.width + j + l];
                    float mag = expf(-(k * k + l * l) / (2.f * sqs));
                    cdx *= mag;
                    cdy *= mag;
                    ha += cdx * cdx;
                    hb += cdx * cdy;
                    hc += cdy * cdy;
                }
            }
            float det = ha * hc - hb * hb;
            float trace = (ha + hc);
            lm[i * img.width + j] = det - _k * trace * trace;
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
//            if (o)
//                cout << lm[i * img.width + j] << endl;
            if (o && lm[i * img.width + j] > thres) {
                poi p;
                p.x = j;
                p.y = i;
                p.function = lm[i * img.width + j];
                p.scale = 1.f;
                ret.push_back(p);
            }
        }
    }
    
    GImage q(img.width, img.height);
    for (int i = 0; i < q.height; i++)
        for (int j = 0; j < q.width; j++)
            q.a[i * q.width + j] = sqrtf(lm[i * img.width + j]);
    q.normalizeMinMax();
    q.save("xhar.jpg");
    
    return ret;
}

poivec anms(poivec &in, int target, float diff) {
    poivec ret;
    ret.reserve(in.size());
    float l = 0.;
    float r = numeric_limits<float>::max();
    vector<bool> u(in.size());
    fill(begin(u), end(u), false);
    while (true) {
        float m = (l + r) / 2.;
        if ((uint)target > in.size())
            m = 0;
        int cnt = 0;
        for (uint i = 0; i < in.size(); i++) {
            bool o = true;
            for (uint j = 0; j < in.size(); j++) {
                if (i != j) {
                    float dx = in[i].x - in[j].x;
                    float dy = in[i].y - in[j].y;
                    float dist = dx * dx + dy * dy;
                    if (dist < m * m && in[i].function <= in[j].function) {
                        o = false;
                        break;
                    }
                }
            }
            if (o)
                cnt++;
            u[i] = o;
        }
        if ((uint)target > in.size())
            break;
        if (fabs((cnt * 1. / target) - 1) < diff) {
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
    drawCircle(img, x, y, 1, 255 << 16);
    drawCircle(img, x, y, 2, 255 << 16);
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
    const int DRAD = 6; // descriptor-s radius
    const int DBOXES = 4; // boxes quantity per dimension
    const int BDIRS = 8; // directions quantity in box
    
    gdvector ret;
    ret.reserve(vpoi.size());
    
    int width = img.width;
    int mrad = 0;
    for (size_t i = 0; i < vpoi.size(); i++)
        mrad = max(mrad, int(ceilf(vpoi[i].scale * DRAD)));
    
    int cwidth = width + mrad * 2;
    GImage wimg = prepareEdges(img, EdgeType_BorderCopy, mrad);
    
    GImage sx = getFaridXSeparate().applySeparate(wimg, EdgeType_BorderCopy);
    GImage sy = getFaridYSeparate().applySeparate(wimg, EdgeType_BorderCopy);
    
    float sqs = mrad / 2.;
    sqs *= sqs;
    
    for (int i = 0; i < int(vpoi.size()); i++) {
        int bx = roundf(vpoi[i].x) + mrad;
        int by = roundf(vpoi[i].y) + mrad;
        gdescriptor cdesc;
        cdesc.p = vpoi[i];
        float* dv = cdesc.vec;
        fill(&dv[0], &dv[DSIZE], 0.f);
        
        float ran = vpoi[i].orient;
        float rsin = sinf(ran);
        float rcos = cosf(ran);
        int crad = int(ceilf(vpoi[i].scale * DRAD));
        sqs = vpoi[i].scale * DRAD / 2.;
        sqs *= sqs;
        
        for (int cy = -crad; cy <= crad; cy++) {
            for (int cx = -crad; cx <= crad; cx++) {
                if (cy * cy + cx * cx > crad * crad)
                    continue;
                int y = by + cy;
                int x = bx + cx;
                
                float dy = sy.a[y * cwidth + x];
                float dx = sx.a[y * cwidth + x];
                float fi = (atan2f(-dy, dx) + M_PI) - ran;
                if (fi < 0.)
                    fi += M_PI * 2.;
                float len = sqrtf(dy * dy + dx * dx);
                len *= expf(-(cy * cy + cx * cx) / (2.f * sqs));
                
                float alph = fi * BDIRS * .5 / M_PI;
                int drcn = int(alph);
                int drnx = drcn + 1;
                if (drnx == BDIRS)
                    drnx = 0;
                
                float weight = alph - drcn;
                
                // getting normalized coorditates
                float qy = -((-cy) * rcos - (cx) * rsin);
                float qx = (cx) * rcos + (-cy) * rsin;
                // box`s selection
                float ybox = (qy + crad) / DBOXES + 0.5f;
                float xbox = (qx + crad) / DBOXES + 0.5f;
                int ybox1 = int(ybox);
                int ybox2 = ybox1 + 1;
                int xbox1 = int(xbox);
                int xbox2 = xbox1 + 1;
                float yweight = ybox - ybox1;
                float xweight = xbox - xbox1;
                
                // overflow
                if (ybox < 0) {
                    ybox1 = 0;
                    ybox2 = 1;
                    yweight = 0.f;
                }
                if (ybox2 >= DBOXES) {
                    ybox1 = DBOXES - 2;
                    ybox2 = DBOXES - 1;
                    yweight = 1.f;
                }
                if (xbox < 0) {
                    xbox1 = 0;
                    xbox2 = 1;
                    xweight = 0.f;
                }
                if (xbox2 >= DBOXES) {
                    xbox1 = DBOXES - 2;
                    xbox2 = DBOXES - 1;
                    xweight = 1.f;
                }
                
                // distribution
                dv[(ybox1 * DBOXES + xbox1) * BDIRS + drcn] += 
                        len * (1 - weight) * (1 - xweight) * (1 - yweight);
                dv[(ybox1 * DBOXES + xbox1) * BDIRS + drnx] += 
                        len * weight * (1 - xweight) * (1 - yweight);
                
                dv[(ybox1 * DBOXES + xbox2) * BDIRS + drcn] += 
                        len * (1 - weight) * (xweight) * (1 - yweight);
                dv[(ybox1 * DBOXES + xbox2) * BDIRS + drnx] += 
                        len * weight * (xweight) * (1 - yweight);
                
                dv[(ybox2 * DBOXES + xbox1) * BDIRS + drcn] += 
                        len * (1 - weight) * (1 - xweight) * (yweight);
                dv[(ybox2 * DBOXES + xbox1) * BDIRS + drnx] += 
                        len * weight * (1 - xweight) * (yweight);
                
                dv[(ybox2 * DBOXES + xbox2) * BDIRS + drcn] += 
                        len * (1 - weight) * (xweight) * (yweight);
                dv[(ybox2 * DBOXES + xbox2) * BDIRS + drnx] += 
                        len * weight * (xweight) * (yweight);
            }
        }
        // independent normalization
        for (int box = 0; box < DBOXES * DBOXES; box++) {
            float len = 0.;
            for (int j = 0; j < BDIRS; j++) {
                len = max(len, dv[box * BDIRS + j]);
            }
            len += 1e-15f;
            for (int j = 0; j < BDIRS; j++) {
                dv[box * BDIRS + j] /= len;
            }
        }
        ret.push_back(cdesc);
    }
    
    return ret;
}

vector<pair<int, int> > getMatches(const gdvector &dfirst, const gdvector &dsecond, const float thres)
{
    vector<pair<int, int> > ret;
    ret.reserve(min(dfirst.size(), dsecond.size()));
    
    vector<size_t> lp(dfirst.size());
    vector<size_t> rp(dsecond.size());    
    
    vector<float> left(dfirst.size());
    vector<float> right(dsecond.size());
    
    fill(begin(left), end(left), numeric_limits<float>::max());
    fill(begin(right), end(right), numeric_limits<float>::max());
    
    for (size_t i = 0; i < dfirst.size(); i++) {
        for (size_t j = 0; j < dsecond.size(); j++) {
            float cur = 0.f;
            for (size_t k = 0; k < DSIZE; k++) {
                float dim = dfirst[i].vec[k] - dsecond[j].vec[k];
                cur += dim * dim;
            }
            if (left[i] > cur) {
                left[i] = cur;
                lp[i] = j;
            }
            if (right[j] > cur) {
                right[j] = cur;
                rp[j] = i;
            }
        }
    }
    
    for (size_t i = 0; i < dfirst.size(); i++) {
        int j = lp[i];
        if (rp[j] == i && left[i] <= thres * thres) {
            ret.push_back(make_pair(i, j));
        }
    }
    
    return ret;
}

void drawLine(QImage &img, int x1, int y1, int x2, int y2, int color)
{
    float dlt = .3 / max(abs(x1 - x2), abs(y1 - y2));
    
    for (float i = 0.; i <= 1.; i += dlt) {
        float j = 1. - i;
        float x = int(roundf(x1 * i + x2 * j));
        float y = int(roundf(y1 * i + y2 * j));
        if (x > -1 && y > -1 && x < img.width() && y < img.height())
        img.setPixel(x, y, color);
    }
}

void drawCircle(QImage &img, int x, int y, float r, int color)
{
    float dlt = 1 / (r * 12.f);
    
    for (float i = 0.; i <= 1.; i += dlt) {
        int x1 = int(roundf(cosf(i * M_PI * 2.f) * r + x));
        int y1 = int(roundf(sinf(i * M_PI * 2.f) * r + y));
        if (x1 > -1 && y1 > -1 && x1 < img.width() && y1 < img.height())
            img.setPixel(x1, y1, color);
    }
}

QImage drawPoints(const GImage &img, poivec &vpoi)
{
    QImage ret = img.convert();
    for (uint i = 0; i < vpoi.size(); i++) {
        mark(ret, vpoi[i].x, vpoi[i].y);
    }
    return ret;
}

void saveJpeg(QImage &img, const char* filename)
{
    img.save(filename, 0, 99);
}

QImage drawMatches(const GImage &gimg, const GImage &gimg2, gdvector &desc1, gdvector &desc2, 
                   vector<pair<int, int> > &matches, bool lines, bool marks)
{
    QImage ret(gimg.width + gimg2.width, 
               max(gimg.height, gimg2.height), QImage::Format_RGB32);
    
    for (int i = 0; i < gimg.height; i++) {
        for (int j = 0; j < gimg.width; j++) {
            ret.setPixel(j, i, toRGB(gimg.a[i * gimg.width + j]));
        }
    }
    for (int i = 0; i < gimg2.height; i++) {
        for (int j = 0; j < gimg2.width; j++) {
            ret.setPixel(j + gimg.width, i, 
                         toRGB(gimg2.a[i * gimg2.width + j]));
        }
    }
    
    uniform_int_distribution<uint32_t> uint_dist(0, (1 << 24) - 1);
    mt19937 rnd;
    
    for (uint i = 0; i  < matches.size(); i++) {
        int color = uint_dist(rnd);
        auto &l = desc1[matches[i].first].p;
        auto &r = desc2[matches[i].second].p;
        if (lines)
            drawLine(ret, l.x, l.y, r.x + gimg.width, r.y, color);
        if (marks) {
            int rad = max(int(roundf(l.scale)), 20);
            drawLine(ret, l.x, l.y, l.x + rad * cosf(l.orient),
                     l.y - rad * sinf(l.orient), color);
            drawCircle(ret, l.x, l.y, l.scale, color);
            rad = max(int(roundf(r.scale)), 20);
            drawLine(ret, r.x + gimg.width, r.y, r.x + 
                     rad * cosf(r.orient) + gimg.width,
                     r.y - rad * sinf(r.orient), color);
            drawCircle(ret, r.x + gimg.width, r.y, r.scale, color);
        }
        
    }
    return ret;
}

QImage drawBlobs(const GImage &img, poivec &blobs, bool drawDirections)
{
    uniform_int_distribution<uint32_t> uint_dist(0, (1 << 24) - 1);
    mt19937 rnd;
    
    QImage ret(img.width, img.height, QImage::Format_RGB32);
    
    for (int i = 0; i < img.height; i++) {
        for (int j = 0; j < img.width; j++) {
            ret.setPixel(j, i, toRGB(img.a[i * img.width + j]));
        }
    }
    
    for (uint i = 0; i  < blobs.size(); i++) {
        int color = uint_dist(rnd);
        int r = max(int(roundf(blobs[i].scale)), 20);
        if (drawDirections)
            drawLine(ret, blobs[i].x, blobs[i].y, 
                     blobs[i].x + r * cosf(blobs[i].orient),
                     blobs[i].y - r * sinf(blobs[i].orient), color);
        
        drawCircle(ret, blobs[i].x, blobs[i].y, 
                 blobs[i].scale, color);
    }
    return ret;
}

poivec getBlobs(GPyramid &pyr)
{
    poivec ret;
    ret.reserve(pyr.height * pyr.width);
    
    GPyramid doG = pyr.getDOG();
    
    int dirs = 9;
    int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1, 0};
    int dy[] = {1, 0, -1, 1, -1, 1, 0, -1, 0};
    
    for (int i = 0; i < doG.ocnt; i++) {
        for (int j = (i == 0 ? 1 : 0); j < doG.olayers; j++) {
            int lId = i * (doG.olayers + 2) + j;
            GImage& cur = doG.a[lId];
            GImage& prv = doG.a[lId - 1];
            GImage& nxt = doG.a[lId + 1];
            GImage* imgs[] = {&cur, &prv, &nxt};
            
            for (int y = 1; y < cur.height - 1; y++) {
                for (int x = 1; x < cur.width - 1; x++) {
                    float val = cur.a[y * cur.width + x];
                    bool extMax = val > 0.f;
                    bool extMin = val < 0.f;
                    
                    // checking extremum
                    int cdirs = dirs - 1;
                    for (int cimg = 0; cimg < 3; cimg++) {
                        for (int k = 0; k < cdirs; k++) {
                            float cval = imgs[cimg]->
                                    a[(y + dy[k]) * cur.width + x + dx[k]];
                            if (cval <= val)
                                extMin = false;
                            if (cval >= val)
                                extMax = false;
                        }
                        cdirs = dirs;
                    }
                    
                    if (extMax || extMin) {
                        poi p;
                        p.x = x << i;
                        p.y = y << i;
                        p.scale = exp2f(float(i * (doG.olayers) + j) / 
                                        (doG.olayers)) * pyr.sbase * CSIGSIZE;
                        ret.push_back(p);
                    }
                }
            }
            
        }
    }
    
    return ret;
}

poivec getDOGDetection(const GImage &img)
{
    poivec ret;
    GPyramid pyr(img, 1.6, 0.5, 7);
    ret.reserve(pyr.height * pyr.width);
    
    GPyramid doG = pyr.getDOG();
    
    int dirs = 9;
    int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1, 0};
    int dy[] = {1, 0, -1, 1, -1, 1, 0, -1, 0};
    const float EDGE_R = 10.f;
//    int shifts = 0; // debug
    GConvol farx = getFaridXSeparate();
    GConvol fary = getFaridYSeparate();
    GConvol farx2 = getFaridX2Separate();
    GConvol fary2 = getFaridY2Separate();
    
    for (int i = 0; i < doG.ocnt; i++) {
        for (int j = (i == 0 ? 1 : 0); j < doG.olayers; j++) {
            int lId = i * (doG.olayers + 2) + j;
            GImage& cur = doG.a[lId];
            GImage& prv = doG.a[lId - 1];
            GImage& nxt = doG.a[lId + 1];
            GImage* imgs[] = {&cur, &prv, &nxt};
            
            GImage sdx = farx.applySeparate(cur, EdgeType_BorderCopy);
            GImage sdy = fary.applySeparate(cur, EdgeType_BorderCopy);
            GImage sdx2 = farx2.applySeparate(cur, EdgeType_BorderCopy);
            GImage sdy2 = fary2.applySeparate(cur, EdgeType_BorderCopy);
            GImage sdxy = farx.applySeparate(sdy, EdgeType_BorderCopy);
            
            for (int y = 1; y < cur.height - 1; y++) {
                for (int x = 1; x < cur.width - 1; x++) {
                    float val = cur.a[y * cur.width + x];
                    bool extMax = val > 0.f;
                    bool extMin = val < 0.f;
                    
                    // checking extremum
                    int cdirs = dirs - 1;
                    for (int cimg = 0; cimg < 3; cimg++) {
                        for (int k = 0; k < cdirs; k++) {
                            float cval = imgs[cimg]->
                                    a[(y + dy[k]) * cur.width + x + dx[k]];
                            if (cval <= val)
                                extMin = false;
                            if (cval >= val)
                                extMax = false;
                        }
                        cdirs = dirs;
                    }
                    
                    // processing extremum
                    if (extMax || extMin) {
                        int cx = x;
                        int cy = y;
                        
                        float cdx, cdy, ha, hb, hc, det, trace, gx, gy, gs;
                        float cdx2, cdy2, cdxy;
//                        cout << cx << " " << cy << endl;
                        
                        // adjusting position
                        for (int k = 0; k < 3; k++) {
                            cdx = sdx.a[cy * sdx.width + cx];
                            cdy = sdy.a[cy * sdx.width + cx];
                            cdx2 = sdx2.a[cy * sdx.width + cx];
                            cdy2 = sdy2.a[cy * sdx.width + cx];
                            cdxy = sdxy.a[cy * sdx.width + cx];
//                            cout << "diff " << cdx << " " << cdy << endl;
                            ha = cdx2;
                            hb = cdxy;
                            hc = cdy2;
//                            cout << ha << " " << hb << " " << hc << endl;
                            det = ha * hc - hb * hb;
//                            cout << "det " << det << endl;
                            trace = (ha + hc);
                            gx = (hb * cdy - hc * cdx) / det;
                            gy = (hb * cdx - ha * cdy) / det;
                            if (fabsf(gx) < 0.5f && fabsf(gy) < 0.5f)
                                break;
//                            cout << cx << " " << cy << endl;
//                            cout << "shift " << gx << " " << gy << endl;
                            // TODO
                            // debug
//                            if (fabsf(gx) > 1.f && fabsf(gy) > 1.f)
//                                shifts++;
                            break;
                            if (fabsf(gx) >= 0.5f)
                                cx += (gx > 0) ? 1 : -1;
                            if (fabsf(gy) >= 0.5f)
                                cy += (gy > 0) ? 1 : -1;
                        }
                        // temporary interpolation
                        float ax[3] {-1.f, 0.f, 1.f};
                        float ay1[3] {cur.a[cy * cur.width + cx - 1], 
                                    cur.a[cy * cur.width + cx], 
                                    cur.a[cy * cur.width + cx + 1]};
                        float ay2[3] {cur.a[(cy - 1) * cur.width + cx], 
                                    cur.a[cy * cur.width + cx], 
                                    cur.a[(cy + 1) * cur.width + cx]};
                        float ay3[3] {prv.a[cy * cur.width + cx], 
                                    cur.a[cy * cur.width + cx], 
                                    nxt.a[cy * cur.width + cx]};
                        gx = gy = gs = 0;
                        auto e1 = getParabolicExtremum(ax, ay1);
                        auto e2 = getParabolicExtremum(ax, ay2);
                        auto e3 = getParabolicExtremum(ax, ay3);
                        gx += e1.first;
                        gy += e2.first;
                        gs += e3.first;
//                        cout << "shift " << gx << " " << gy << endl;
                        
                        float nd = cur.a[cy * cur.width + cx] + 
                                gx * cdx / 2.f + gy * cdy / 2.f;
                        // filtering low-contrast points
                        if (fabs(nd) < 0.03f)
                            continue;
                        // filtering edges
                        if (trace * trace / det > (EDGE_R + 1) * (EDGE_R + 1) / EDGE_R)
                            continue;
//                        gx = gy = 0;
                        float sg = exp2f((i * (doG.olayers) + j + gs) / 
                                       (doG.olayers)) * pyr.sbase * CSIGSIZE;
                        poi p;
                        p.x = (cx + gx + 0.5f) * (1 << i) - 0.5f;
                        p.y = (cy + gy + 0.5f) * (1 << i);
                        p.scale = sg;
                        ret.push_back(p);
                    }
                }
            }
        }
    }
    // debug
//    cout << "shifts " << shifts << endl;
    
    return ret;
}

poivec calculateOrientations(GImage &img, poivec &vpoi)
{
    poivec ret;
    ret.reserve(vpoi.size() * 2);
    int width = img.width;
    int mrad = 0;
    for (size_t i = 0; i < vpoi.size(); i++)
        mrad = max(mrad, int(ceilf(vpoi[i].scale * 3.f)));
    
    int cwidth = width + mrad * 2;
    GImage wimg = prepareEdges(img, EdgeType_BorderCopy, mrad);
    
    GImage sx = getFaridXSeparate().applySeparate(wimg, EdgeType_BorderCopy);
    GImage sy = getFaridYSeparate().applySeparate(wimg, EdgeType_BorderCopy);
    
    const int ABCOUNT = 36; // directions quantity for orientation
    
    vector<int> dirs;
    dirs.reserve(2);
    float angBoxes[ABCOUNT];
    
    for (int i = 0; i < int(vpoi.size()); i++) {
        poi cur = vpoi[i];
        int bx = roundf(cur.x) + mrad;
        int by = roundf(cur.y) + mrad;
        fill(begin(angBoxes), end(angBoxes), 0.);
        
        int crad = int(ceilf(cur.scale * 3.f));
        float sqs = cur.scale * 1.5f;
        sqs *= sqs;
        
        // orientation search
        for (int cy = -crad; cy <= crad; cy++) {
            for (int cx = -crad; cx <= crad; cx++) {
                if (cy * cy + cx * cx > crad * crad)
                    continue;
                int qy = by + cy;
                int qx = bx + cx;
                float dy = sy.a[qy * cwidth + qx];
                float dx = sx.a[qy * cwidth + qx];
                
                float fi = atan2f(-dy, dx) + M_PI;
                float len = sqrtf(dy * dy + dx * dx);
                len *= expf(-(cy * cy + cx * cx) / (2.f * sqs));
                
                float alph = fi * ABCOUNT * 0.5f / M_PI;
                int drcn = int(alph);
                int drnx = (drcn + 1) % ABCOUNT;
                
                float weight = alph - drcn;
                angBoxes[drcn] += len * (1 - weight);
                angBoxes[drnx] += len * weight;
                
            }
        }
        
        // maximum`s selection
        dirs.clear();
        int maxId = 0;
        for (int j = 1; j < ABCOUNT; j++) {
            if (angBoxes[j] > angBoxes[maxId])
                maxId = j;
        }
        dirs.push_back(maxId);
        // second maximum`s selection
        maxId = (maxId + 1) % ABCOUNT;
        for (int j = 0; j < ABCOUNT; j++) {
            if (j != dirs[0] && angBoxes[j] > angBoxes[maxId])
                maxId = j;
        }
        if (angBoxes[maxId] >= angBoxes[dirs[0]] * 0.8f)
            dirs.push_back(maxId);
        
        for (size_t j = 0; j < dirs.size(); j++) {
            
            // interpolation init
            int x2 = dirs[j];
            int x1 = (x2 + ABCOUNT - 1) % ABCOUNT;
            int x3 = (x2 + 1) % ABCOUNT;
            float y1 = angBoxes[x1];
            float y2 = angBoxes[x2];
            float y3 = angBoxes[x3];
            if (y2 < y1 || y2 < y3)
                continue;
            
            float ax[] = {float(x2 - 1), float(x2), float(x2 + 1)};
            float ay[] = {y1, y2, y3};
            
            auto q1 = getParabolicInterpolation(ax, ay);
            
            // resulting orientation
            float rtx = -get<1>(q1) / (2.f * get<2>(q1));
            if (rtx < 0)
                rtx += ABCOUNT;
            
            cur.orient = float(M_PI) * 2.f * (rtx) / ABCOUNT;
            ret.push_back(cur);
        }
    }
    return ret;
}

vector<float> polinomialInterpolation(float *x, float *y, int size)
{
    vector<float> q1(size); // current coeffs
    vector<float> q2(size); // additional coeffs
    
    q1[0] = y[0];
    // parabolic angle interpolation
    for (int c1 = 1; c1 < size; c1++) {
        float co = 1.f;
        for (int c2 = 0; c2 < c1; c2++) {
            co *= (x[c1] - x[c2]);
        }
        float cur = 0;
        for (int c2 = c1; c2 > -1; c2--) {
            cur *= x[c1];
            cur += q1[c2];
        }
        co = (y[c1] - cur) / co;
        fill(begin(q2), end(q2), 0.f);
        q2[0] = 1;
        for (int c2 = 0; c2 < c1; c2++) {
            float z = 0;
            for (int c3 = 0; c3 <= c2; c3++) {
                float nx = q2[c3];
                q2[c3] = z - (nx * x[c2]);
                z = nx;
            }
            q2[c2 + 1] = z;
        }
        for (int c2 = 0; c2 < 3; c2++)
            q1[c2] += q2[c2] * co;
    }
    return q1;
}

tuple<float, float, float> getParabolicInterpolation(float *x, float *y)
{
    float x21 = x[1] - x[0];
    float x31 = x[2] - x[0];
    float x32 = x[2] - x[1];
    float y21 = y[1] - y[0];
    float y31 = y[2] - y[0];
    float a = y31 / x31 / x32 - y21 / x21 / x32;
    float b = -a * (x[0] + x[1]) + y21 / x21;
    float c = x[0] * x[1] * a - x[0] * y21 / x21 + y[0];
    return make_tuple(c, b, a);
}

pair<float, float> getParabolicExtremum(float *x, float *y)
{
    auto d = getParabolicInterpolation(x, y);
    float x1 = -get<1>(d) * 0.5f / get<2>(d);
    float y1 = x1 * (x1 * get<2>(d) + get<1>(d)) + get<0>(d);
    return make_pair(x1, y1);
}
