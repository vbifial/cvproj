#include "common.h"
#include "gconvol.h"
#include "gimage.h"
#include "gpyramid.h"

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

float getVproj(float x1, float y1, float x2, float y2)
{
    return x2 * y1 - x1 * y2;
}
