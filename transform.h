#ifndef TRANSFORM_H
#define TRANSFORM_H
#include "common.h"

vector<float> getTransformation(const poivec& left, const poivec& right);

poi transformPOI(const vector<float>& h, const poi& p);

vector<float> getRansacTransform(const poivec& left, const poivec& right, 
                                 float inlierR, float threshold);

GImage getOverlapping(const GImage& a, const GImage& b, const vector<float>& h);

#endif // TRANSFORM_H

