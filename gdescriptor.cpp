#include "gdescriptor.h"
#include "gimage.h"
#include "gconvol.h"

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
            float osin = sinf(cur.orient);
            float ocos = cosf(cur.orient);
            float nx = cur.bx * ocos + -cur.by * osin;
            float ny = -(-cur.by * ocos - cur.bx * osin);
            cur.bx = nx;
            cur.by = ny;
            ret.push_back(cur);
        }
    }
    return ret;
}


