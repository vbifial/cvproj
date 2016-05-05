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
//            q.a[i * q.width + j] = sqrtf(lm[i * img.width + j]);
//    q.normalizeMinMax();
//    q.save("xhar.jpg");
    
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
    ret.reserve(vpoi.size() * 2);
    
    int width = img.width;
    const int DRAD = 4; // descriptor`s radius
    const int DBOXES = 2; // boxes quantity per dimension
    const int BDIRS = 8; // directions quantity in box
    int cwidth = width + DRAD * 2;
    GImage wimg = prepareEdges(img, EdgeType_BorderCopy, DRAD);
    
    GImage sx = getSobelX().apply(wimg, EdgeType_BorderCopy);
    GImage sy = getSobelY().apply(wimg, EdgeType_BorderCopy);
    
    const int ABCOUNT = 36; // directions quantity for orientation
    
    float sqs = DRAD / 2.;
    sqs *= sqs;
    vector<int> dirs;
    dirs.reserve(2);
    float angBoxes[ABCOUNT];
    
    for (int i = 0; i < int(vpoi.size()); i++) {
        int bx = get<0>(vpoi[i]) + DRAD;
        int by = get<1>(vpoi[i]) + DRAD;
        gdescriptor cdesc;
        get<1>(cdesc) = bx - DRAD;
        get<2>(cdesc) = by - DRAD;
        float* dv = get<0>(cdesc);
        fill(begin(angBoxes), end(angBoxes), 0.);
        
        // orientation search
        for (int cy = -DRAD; cy <= DRAD; cy++) {
            for (int cx = -DRAD; cx <= DRAD; cx++) {
                if (cy * cy + cx * cx > DRAD * DRAD)
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
                int drnx = drcn + 1;
                if (drnx == ABCOUNT)
                    drnx = 0;
                
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
        if (angBoxes[maxId] >= angBoxes[dirs[0]] * 0.8)
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
            
            float q1[3] = {y1, 0, 0}; // current coeffs
            float q2[3]; // additional coeffs
            
            // parabolic angle interpolation
            for (int c1 = 1; c1 < 3; c1++) {
                float co = 1.f;
                for (int c2 = 0; c2 < c1; c2++) {
                    co *= (ax[c1] - ax[c2]);
                }
                float cur = ax[c1] * (ax[c1] * q1[2] + q1[1]) + q1[0];
                co = (ay[c1] - cur) / co;
                fill(begin(q2), end(q2), 0.f);
                q2[0] = 1;
                for (int c2 = 0; c2 < c1; c2++) {
                    float z = 0;
                    for (int c3 = 0; c3 <= c2; c3++) {
                        float nx = q2[c3];
                        q2[c3] = z - (nx * ax[c2]);
                        z = nx;
                    }
                    q2[c2 + 1] = z;
                }
                for (int c2 = 0; c2 < 3; c2++)
                    q1[c2] += q2[c2] * co;
            }
            
            // resulting orientation
            float rtx = -q1[1] / (2.f * q1[2]);
            if (rtx < 0)
                rtx += ABCOUNT;
            
            float ran = float(M_PI) * 2.f * (rtx) / ABCOUNT;
            get<3>(cdesc) = ran;
            
            float rsin = sinf(ran);
            float rcos = cosf(ran);
            
            // 
            for (int cy = -DRAD; cy <= DRAD; cy++) {
                for (int cx = -DRAD; cx <= DRAD; cx++) {
                    if (cy * cy + cx * cx > DRAD * DRAD)
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
                    int ybox = int((qy - 0.5f + DRAD) / DBOXES);
                    int xbox = int((qx - 0.5f + DRAD) / DBOXES);
                    if (ybox < 0)
                        ybox = 0;
                    if (ybox >= DBOXES)
                        ybox = DBOXES - 1;
                    if (xbox < 0)
                        xbox = 0;
                    if (xbox >= DBOXES)
                        xbox = DBOXES - 1;
                    
                    dv[(ybox * DBOXES + xbox) * BDIRS + drcn] += 
                            len * (1 - weight);
                    dv[(ybox * DBOXES + xbox) * BDIRS + drnx] += 
                            len * weight;
                }
            }
            // independent normalization
            for (int box = 0; box < DBOXES * DBOXES; box++) {
                float len = 0.;
                for (int j = 0; j < BDIRS; j++) {
                    len = max(len, dv[box * BDIRS + j]);
                }
                len += 1e-50;
                for (int j = 0; j < BDIRS; j++) {
                    dv[box * BDIRS + j] /= len;
                }
            }
            ret.push_back(cdesc);
        }
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
            float cur = 0.;
            for (size_t k = 0; k < DSIZE; k++) {
                float dim = get<0>(dfirst[i])[k] - get<0>(dsecond[j])[k];
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
        img.setPixel(int(roundf(x1 * i + x2 * j)), int(roundf(y1 * i + y2 * j)), color);
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
        mark(ret, get<0>(vpoi[i]), get<1>(vpoi[i]));
    }
    return ret;
}

void saveJpeg(QImage &img, const char* filename)
{
    img.save(filename, 0, 99);
}

QImage drawMatches(const GImage &gimg, const GImage &gimg2, gdvector &desc1, gdvector &desc2, 
                   vector<pair<int, int> > &matches)
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
        auto &l = desc1[matches[i].first];
        auto &r = desc2[matches[i].second];
        drawLine(ret, get<1>(l), get<2>(l), 
                 get<1>(r) + gimg.width, get<2>(r), color);
    }
    return ret;
}

QImage drawBlobs(const GImage &img, poivec blobs)
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
        
        drawCircle(ret, get<0>(blobs[i]), get<1>(blobs[i]), 
                 get<2>(blobs[i]), color);
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
        for (int j = ((i == 0) ? 1 : 0); 
             j < doG.olayers - (i == doG.ocnt - 1 ? 1 : 0); j++) {
            int lId = i * (doG.olayers + 1) + j;
            GImage& cur = doG.a[lId];
            GImage& prv = doG.a[lId - 1];
            GImage& nxt = doG.a[lId + 1];
            
            for (int y = 1; y < cur.height - 1; y++) {
                for (int x = 1; x < cur.width - 1; x++) {
                    float val = cur.a[y * cur.width + x];
                    bool extMax = val > 0.f;
                    bool extMin = val < 0.f;
                    
                    for (int k = 0; k < dirs - 1; k++) {
                        float cval = cur.a[(y + dy[k]) * cur.width + x + dx[k]];
                        if (cval <= val)
                            extMin = false;
                        if (cval >= val)
                            extMax = false;
                    }
                    
                    for (int k = 0; k < dirs; k++) {
                        float cval = nxt.a[(y + dy[k]) * cur.width + x + dx[k]];
                        if (cval <= val)
                            extMin = false;
                        if (cval >= val)
                            extMax = false;
                    }
                    int cy = y;
                    int cx = x;
                    if (j == 0) {
                        cy = cy * 2 - 1;
                        cx = cx * 2 - 1;
                    }
                    
                    for (int k = 0; k < dirs; k++) {
                        float cval = prv.a[(cy + dy[k]) * cur.width + cx + dx[k]];
                        if (cval <= val)
                            extMin = false;
                        if (cval >= val)
                            extMax = false;
                    }
                    
                    if (extMax || extMin) {
                        int cx = x << i;
                        int cy = y << i;
                        float sg = exp2f(float(i * (doG.olayers - 1) + j) / 
                                       (doG.olayers - 1));
                        ret.push_back(make_tuple(cx, cy, sg));
                    }
                }
            }
            
        }
    }
    
    return ret;
}
