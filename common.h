#pragma once
#ifndef COMMON
#define COMMON
#include <QCoreApplication>
#include <iostream>
#include <QImage>

using namespace std;

float fromRGB(int color);
int toRGB(float color);

const float rgbLum[] = {0.213, 0.715, 0.072};

#endif // COMMON

