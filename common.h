#pragma once
#ifndef COMMON
#define COMMON
#include <QCoreApplication>
#include <iostream>
#include <QImage>
#include <memory>

using namespace std;

float fromRGB(int color);
int toRGB(float color);
class GConvol;
class GImage;

GConvol* getSobelX();
GConvol* getSobelY();
GImage* getSobel(GImage &img);

const float rgbLum[] = {0.213, 0.715, 0.072};

enum EdgeType {
    EdgeType_Zero,
    EdgeType_BorderCopy,
    EdgeType_Mirror,
    EdgeType_Wrap
};

#endif // COMMON

