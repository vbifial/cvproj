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

enum EdgeType {
    EdgeType_Zero,
    EdgeType_BorderCopy,
    EdgeType_Mirror,
    EdgeType_Wrap
};

const int DSIZE = 32;

float fromRGB(int color);
int toRGB(float color);
int getTimeMill();
class GConvol;
class GImage;
class GPyramid;
typedef vector<tuple<int, int, float> > poivec;
typedef tuple<float[DSIZE], int, int> gdescriptor;
typedef vector<gdescriptor> gdvector;

GConvol getSobelX();
GConvol getSobelY();
GImage getSobel(const GImage &img);
GConvol getGaussian(float sigma);
GImage prepareEdges(const GImage &source, EdgeType edge, int r);

poivec getMoravec(const GImage &img, int wrad, int mrad, float thres);
poivec getHarris(const GImage &img, int wrad, int mrad, float thres, float _k = .08);
poivec anms(poivec &in, int target, float diff);

gdvector getDescriptors(const GImage &img, const poivec &vpoi);

vector<pair<int, int> > getMatches(const gdvector &dfirst, const gdvector &dsecond, const float thres);

void drawLine(QImage &img, int x1, int y1, int x2, int y2, int color);

void mark(QImage &img, int x, int y);

QImage drawPoints(const GImage &img, poivec &vpoi);
QImage drawMatches(const GImage &img1, const GImage &img2, 
                   gdvector &desc1, gdvector &desc2, vector<pair<int, int> > &matches);
void saveJpeg(QImage &img, const char* filename);

const float rgbLum[] = {0.213, 0.715, 0.072};

#endif // COMMON

