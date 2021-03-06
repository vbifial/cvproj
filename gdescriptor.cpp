#include "gdescriptor.h"
#include "gimage.h"
#include "gconvol.h"
#include "gpyramid.h"

gdvector getDescriptors(const GImage &img, const poivec &vpoi)
{
    gdvector ret;
    ret.reserve(vpoi.size());
    
    for (size_t i = 0; i < vpoi.size(); i++) {
        gdescriptor cdesc;
        poi& p = cdesc.p = vpoi[i];
        float* dv = cdesc.vec;
        
        calcHistograms(img, p.x, p.y, p.orient, dv, 
                       DBOXES, p.scale * DRAD, BDIRS, 1);
        
        ret.push_back(cdesc);
    }
    
    return ret;
}

gdvector getDescriptors(const GPyramid &pyr, const poivec &vpoi) {
    gdvector ret;
    ret.reserve(vpoi.size());
    
    for (size_t i = 0; i < vpoi.size(); i++) {
        gdescriptor cdesc;
        poi& p = cdesc.p = vpoi[i];
        float* dv = cdesc.vec;
        
        float sig = p.scale / CSIGSIZE;
        int layer = roundf(log2f(sig / pyr.sbase) * pyr.olayers);
        layer = min(pyr.ocnt * pyr.olayers - 1, max(layer, 0));
        int clayer = layer % pyr.olayers;
        int coct = layer / pyr.olayers;
        const GImage &img = pyr.a[coct * (pyr.olayers + 3) + clayer];
        float esig = pyr.sbase * exp2f(coct);
        
        calcHistograms(img, p.x, p.y, p.orient, dv, DBOXES, 
                       p.scale / esig * DRAD, BDIRS, 1.f / (1 << coct));
        
        ret.push_back(cdesc);
    }
    
    return ret;
}

vector<pair<int, int> > getMatches(const gdvector &dfirst, 
                                   const gdvector &dsecond, 
                                   const float thres)
{
    vector<pair<int, int> > ret;
    int size = min(dfirst.size(), dsecond.size());
    if (size == 0)
        return ret;
    ret.reserve(size);
    
    vector<size_t> lp(dfirst.size());
    vector<size_t> rp(dsecond.size());    
    
    vector<float> left(dfirst.size());
    vector<float> right(dsecond.size());
    
    fill(begin(left), end(left), numeric_limits<float>::max());
    fill(begin(right), end(right), numeric_limits<float>::max());
    
    for (size_t i = 0; i < dfirst.size(); i++) {
        const float* v1 = dfirst[i].vec;
        for (size_t j = 0; j < dsecond.size(); j++) {
            const float* v2 = dsecond[j].vec;
            float cur = 0.f;
            for (size_t k = 0; k < DSIZE; k++) {
                float dim = v1[k] - v2[k];
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

pair<poivec, poivec> getMatchingPOIs(const gdvector &dfirst, 
                                const gdvector &dsecond, 
                                const float thres) {
    auto a = getMatches(dfirst, dsecond, thres);
    poivec l;
    poivec r;
    if (a.size() == 0)
        return make_pair(l, r);
    l.resize(a.size());
    r.resize(a.size());
    for (size_t i = 0; i < a.size(); i++) {
        l[i] = dfirst[a[i].first].p;
        r[i] = dsecond[a[i].second].p;
    }
    return make_pair(l, r);
}

poivec calculateOrientations(GImage &img, poivec &vpoi)
{
    poivec ret;
    ret.reserve(vpoi.size() * 2);
    
    for (size_t i = 0; i < vpoi.size(); i++) {
        auto p = calculateOrientations(img, vpoi[i], .5f, 1.f);
        if (p.first.orient != -1.f)
            ret.push_back(p.first);
        if (p.second.orient != -1.f)
            ret.push_back(p.second);
    }
    
    return ret;
}

poivec calculateOrientations(GPyramid &pyr, poivec &vpoi) {
    poivec ret;
    ret.reserve(vpoi.size() * 2);
    
    for (size_t i = 0; i < vpoi.size(); i++) {
        float sig = vpoi[i].scale / CSIGSIZE;
        int layer = roundf(log2f(sig / pyr.sbase) * pyr.olayers);
        layer = min(pyr.ocnt * pyr.olayers - 1, max(layer, 0));
        int clayer = layer % pyr.olayers;
        int coct = layer / pyr.olayers;
        auto p = calculateOrientations
                (pyr.a[coct * (pyr.olayers + 3) + clayer], vpoi[i], 
                pyr.sbase * exp2f(float(layer) / pyr.olayers),
                1.f / (1 << coct));
        if (p.first.orient != -1.f)
            ret.push_back(p.first);
        if (p.second.orient != -1.f)
            ret.push_back(p.second);
    }
    
    return ret;
}

pair<poi, poi> calculateOrientations(GImage &img, poi &p, 
                                     float imgSig, float cscale) {
    auto al = getPOIDirections(img, p, p.scale / imgSig * 3.f, cscale);
    pair<poi, poi> ret;
    ret.first.orient = -1.f;
    ret.second.orient = -1.f;
    for (int j = 0; j < 2; j++) {
        float orient = (j == 0 ? al.first : al.second);
        if (orient == -1.f)
            continue;
        p.orient = orient;
        float osin = sinf(p.orient);
        float ocos = cosf(p.orient);
        float nx = p.bx * ocos + -p.by * osin;
        float ny = -(-p.by * ocos - p.bx * osin);
        p.bx = nx;
        p.by = ny;
        if (j == 0)
            ret.first = p;
        else
            ret.second = p;
    }
    return ret;
}

void calcHistograms(const GImage &img, 
                    float x, float y, float orient, float *dv, 
                    int dboxes, float rad, int bdirs, float cscale)
{
    int width = img.width;
    int height = img.height;
    fill(&dv[0], &dv[dboxes * dboxes * bdirs], 0.f);
    
    float sqs = rad / 2.;
    sqs *= sqs;
    x = (x + 0.5f) * cscale - 0.5f;
    y = (y + 0.5f) * cscale - 0.5f;
    
    float rsin = sinf(orient);
    float rcos = cosf(orient);
    float* a = img.a.get();
    int xl = max(0, int(floorf(x - rad)));
    int xr = min(width - 1, int(ceilf(x + rad)));
    int yl = max(0, int(floorf(y - rad)));
    int yr = min(height - 1, int(ceilf(y + rad)));
    
    for (int ny = yl; ny <= yr; ny++) {
        for (int nx = xl; nx <= xr; nx++) {
            // current vector from center
            float cx = nx - x;
            float cy = ny - y;
            if (cy * cy + cx * cx > rad * rad)
                continue;
            
            float dy = gdy(nx, ny, a, width, height);
            float dx = gdx(nx, ny, a, width);
            float fi = (atan2f(-dy, dx) + M_PI) - orient;
            if (fi < 0.)
                fi += M_PI * 2.;
            float len = sqrtf(dy * dy + dx * dx);
            // normalized Gauss magnitude
            len *= expf(-(cy * cy + cx * cx) / (2.f * sqs));
            // two adjacent angle boxes
            float alph = fi * bdirs * .5 / M_PI;
            int drcn = int(alph);
            int drnx = drcn + 1;
            if (drnx == BDIRS)
                drnx = 0;
            float weight = alph - drcn;
            
            // getting normalized coorditates
            float qy = -((-cy) * rcos - (cx) * rsin);
            float qx = (cx) * rcos + (-cy) * rsin;
            // one box case
            if (dboxes == 1) {
                dv[drcn] += len * (1 - weight);
                dv[drnx] += len * weight;
                continue;
            }
            // box`s selection
            float ybox = (qy + rad) * dboxes / rad / 2.f - 0.5f;
            float xbox = (qx + rad) * dboxes / rad / 2.f - 0.5f;
            // overflow
            if (ybox < 0)
                ybox = 0;
            if (xbox < 0)
                xbox = 0;
            if (ybox >= dboxes - 1)
                ybox = dboxes - 1.00001f;
            if (xbox >= dboxes - 1)
                xbox = dboxes - 1.00001f;
            // boxes indexes
            int ybox1 = int(ybox);
            int ybox2 = ybox1 + 1;
            int xbox1 = int(xbox);
            int xbox2 = xbox1 + 1;
            float yweight = ybox - ybox1;
            float xweight = xbox - xbox1;
            
            // distribution
            dv[(ybox1 * dboxes + xbox1) * bdirs + drcn] += 
                    len * (1 - weight) * (1 - xweight) * (1 - yweight);
            dv[(ybox1 * dboxes + xbox1) * bdirs + drnx] += 
                    len * weight * (1 - xweight) * (1 - yweight);
            
            dv[(ybox1 * dboxes + xbox2) * bdirs + drcn] += 
                    len * (1 - weight) * (xweight) * (1 - yweight);
            dv[(ybox1 * dboxes + xbox2) * bdirs + drnx] += 
                    len * weight * (xweight) * (1 - yweight);
            
            dv[(ybox2 * dboxes + xbox1) * bdirs + drcn] += 
                    len * (1 - weight) * (1 - xweight) * (yweight);
            dv[(ybox2 * dboxes + xbox1) * bdirs + drnx] += 
                    len * weight * (1 - xweight) * (yweight);
            
            dv[(ybox2 * dboxes + xbox2) * bdirs + drcn] += 
                    len * (1 - weight) * (xweight) * (yweight);
            dv[(ybox2 * dboxes + xbox2) * bdirs + drnx] += 
                    len * weight * (xweight) * (yweight);
        }
    }
    // independent normalization
    for (int box = 0; box < dboxes * dboxes; box++) {
        float len = *max_element(&dv[box * bdirs], &dv[(box + 1) * bdirs]);
        len += 1e-15f;
        for (int j = 0; j < bdirs; j++) {
            dv[box * bdirs + j] /= len;
        }
    }
}

pair<float, float> getPOIDirections(const GImage &img, const poi &p, 
                                    float rad, float cscale)
{
    int dcnt = 0;
    int dirs[2];
    float angBoxes[ABCOUNT];
    calcHistograms(img, p.x, p.y, 0, 
                   &angBoxes[0], 1, rad, ABCOUNT, cscale);
    // maximum`s selection
    dcnt = 0;
    int maxId = max_element(begin(angBoxes), end(angBoxes)) - 
            begin(angBoxes);
    dirs[dcnt++] = maxId;
    // second maximum`s selection
    maxId = (maxId + 1) % ABCOUNT;
    for (int j = 0; j < ABCOUNT; j++) {
        if (j != dirs[0] && angBoxes[j] > angBoxes[maxId])
            maxId = j;
    }
    if (angBoxes[maxId] >= angBoxes[dirs[0]] * 0.8f)
        dirs[dcnt++] = maxId;
    pair<float, float> ret = make_pair(-1.f, -1.f);
    for (int j = 0; j < dcnt; j++) {
        
        // interpolation init
        int x2 = dirs[j];
        int x1 = (x2 + ABCOUNT - 1) % ABCOUNT;
        int x3 = (x2 + 1) % ABCOUNT;
        float y1 = angBoxes[x1];
        float y2 = angBoxes[x2];
        float y3 = angBoxes[x3];
        if (y2 < y1 || y2 < y3 || (y2 == y3 && y2 == y1))
            continue;
        
        float ax[] = {float(x2 - 1), float(x2), float(x2 + 1)};
        float ay[] = {y1, y2, y3};
        // resulting orientation
        float rtx = getParabolicExtremum(ax, ay).first;
        if (rtx < 0)
            rtx += ABCOUNT;
        rtx *= float(M_PI) * 2.f / ABCOUNT;
        if (j == 0)
            ret.first = rtx;
        else
            ret.second = rtx;
    }
    return ret;
}
