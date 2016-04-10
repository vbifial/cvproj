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

vector<pair<int, int> > getMoravec(GImage &img, int wrad, int mrad, float thres)
{
    vector<pair<int, int> > ret;
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
                ret.push_back(make_pair(j, i));
            }
        }
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
