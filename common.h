#pragma once
#ifndef COMMON
#define COMMON
#define _USE_MATH_DEFINES
#include <QCoreApplication>
#include <iostream>
#include <QImage>
#include <memory>
#include <cmath>
#include <chrono>
#include <vector>
#include <iomanip>

using namespace std;

enum EdgeType {
    EdgeType_Zero,
    EdgeType_BorderCopy,
    EdgeType_Mirror,
    EdgeType_Wrap
};

const int DSIZE = 128;
const float CSIGSIZE = sqrtf(2.f);

float fromRGB(int color);
int toRGB(float color);
int getTimeMill();
class GConvol;
class GImage;
class GPyramid;

struct poi {
    float x = 0.f;
    float y = 0.f;
    float scale = 0.f;
    float orient = 0.f;
    float function = 0.f;
};

struct gdescriptor {
    float vec[DSIZE];
    poi p;
};

typedef vector<poi> poivec;
typedef vector<gdescriptor> gdvector;

GConvol getSobelX();
GConvol getSobelY();

GConvol getFaridXSeparate();
GConvol getFaridYSeparate();
GConvol getFaridX2Separate();
GConvol getFaridY2Separate();
GImage getSobel(const GImage &img);
GConvol getGaussian(float sigma);
GConvol getGaussianSeparate(float sigma);
GImage prepareEdges(const GImage &source, EdgeType edge, int r);

poivec getMoravec(const GImage &img, int wrad, int mrad, float thres);
poivec getHarris(const GImage &img, int wrad, int mrad, float thres, float _k = .08);
poivec anms(poivec &in, int target, float diff);

poivec getBlobs(GPyramid &pyr);
poivec getDOGDetection(const GImage &img);

gdvector getDescriptors(const GImage &img, const poivec &vpoi);

poivec calculateOrientations(GImage &img, poivec &vpoi);

vector<pair<int, int> > getMatches(const gdvector &dfirst, 
                                   const gdvector &dsecond, const float thres);

void drawLine(QImage &img, int x1, int y1, int x2, int y2, int color);
void drawCircle(QImage &img, int x, int y, float r, int color);

void mark(QImage &img, int x, int y);

QImage drawPoints(const GImage &img, poivec &vpoi);
QImage drawMatches(const GImage &img1, const GImage &img2, 
                   gdvector &desc1, gdvector &desc2, vector<pair<int, int> > &matches, 
                   bool lines, bool marks);
QImage drawBlobs(const GImage &img, poivec &blobs, bool drawDirections);
void saveJpeg(QImage &img, const char* filename);

vector<float> polinomialInterpolation(float* x, float* y, int size);
tuple<float, float, float> getParabolicInterpolation(float *x, float *y);
pair<float, float> getParabolicExtremum(float *x, float *y);

const float rgbLum[] = {0.213, 0.715, 0.072};

#endif // COMMON

