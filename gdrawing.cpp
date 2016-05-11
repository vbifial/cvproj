#include "common.h"
#include "gdrawing.h"
#include "gimage.h"
#include "gdescriptor.h"
#include "transform.h"

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

void mark(QImage &img, int x, int y)
{
    int col = 255 << 16;
    drawCircle(img, x, y, 1, col);
    drawCircle(img, x, y, 2, col);
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

GImage getOverlapping(const GImage& a, const GImage& b, const vector<float>& h)
{
    int minx = 0;
    int miny = 0;
    int maxx = a.width;
    int maxy = a.height;
    int x[] = {0, 0, b.width, b.width};
    int y[] = {0, b.height, 0, b.height};
    for (int i = 0; i < 4; i++) {
        poi p;
        p.x = x[i];
        p.y = y[i];
        p = transformPOI(h, p);
        minx = min(minx, int(round(p.x)));
        miny = min(miny, int(round(p.y)));
        maxx = max(maxx, int(round(p.x)));
        maxy = max(maxy, int(round(p.y)));
    }
    GImage ret(maxx - minx, maxy - miny);
    for (int i = 0; i < a.height; i++)
        for (int j = 0; j < a.width; j++)
            ret.a[(i - miny) * ret.width + (j - minx)] = 
                    a.a[i * a.width + j];
    for (int i = 0; i < b.height; i++)
        for (int j = 0; j < b.width; j++) {
            poi p;
            p.x = j;
            p.y = i;
            poi v = transformPOI(h, p);
            p.x++;
            p.y++;
            p = transformPOI(h, p);
            float d = sqrtf((p.x - v.x) * (p.x - v.x) + 
                    (p.y - v.y) * (p.y - v.y));
            int r = roundf(d / 2.9f);
            for (int l = -r; l < r + 1; l++)
                for (int k = -r; k < r + 1; k++) {
                    int nx = roundf(v.x + k);
                    int ny = roundf(v.y + l);
                    if (nx >= minx && ny >= miny && 
                            nx < maxx && ny < maxy) {
                        ret.a[(ny - miny) * ret.width + nx - minx] = 
                                b.a[i * b.width + j];
                    }
                }
        }
    
    return ret;
}

