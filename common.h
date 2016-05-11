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

const float CSIGSIZE = sqrtf(2.f);

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
    float bx = 0.f;
    float by = 0.f;
};

struct gdescriptor;

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

vector<float> polinomialInterpolation(float* x, float* y, int size);
tuple<float, float, float> getParabolicInterpolation(float *x, float *y);
pair<float, float> getParabolicExtremum(float *x, float *y);

#endif // COMMON

