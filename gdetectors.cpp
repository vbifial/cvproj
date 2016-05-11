#include "gdetectors.h"
#include "gimage.h"
#include "gconvol.h"
#include "gpyramid.h"

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
                        p.y = (cy + gy + 0.5f) * (1 << i) - 0.5f;
                        p.bx = (pyr.width / 2.f - p.x) / sg;
                        p.by = (pyr.width / 2.f - p.y) / sg;
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


