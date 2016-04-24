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

using namespace std;

float fromRGB(int color);
int toRGB(float color);
int getTimeMill();
class GConvol;
class GImage;
class GPyramid;
typedef vector<tuple<int, int, float> > poivec;

GConvol getSobelX();
GConvol getSobelY();
GImage getSobel(const GImage &img);
GConvol getGaussian(float sigma);
poivec getMoravec(const GImage &img, int wrad, int mrad, float thres);
poivec getHarris(const GImage &img, int wrad, int mrad, float thres, float _k = .08);
poivec anms(poivec &in, int target, float diff);

void mark(QImage &img, int x, int y);

const float rgbLum[] = {0.213, 0.715, 0.072};

enum EdgeType {
    EdgeType_Zero,
    EdgeType_BorderCopy,
    EdgeType_Mirror,
    EdgeType_Wrap
};

#endif // COMMON

