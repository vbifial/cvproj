#ifndef TRANSFORM_H
#define TRANSFORM_H
#include "common.h"

vtransform getTransformation(const poivec& left, const poivec& right);

poi transformPOI(const vtransform& h, const poi& p);

vtransform getRansacTransform(const poivec& left, const poivec& right, 
                                 float inlierR, float threshold);

vtransform getHoughTransform(const poivec &left, const poivec &right, 
                             int width, int height, float minScale, 
                             float maxScale, int qx, int qy, 
                             int qscale, int qorient);
pair<vtransform, int> getHough(const poivec &left, const poivec &right, 
                             int width, int height, float minScale, 
                             float maxScale, int qx, int qy, 
                             int qscale, int qorient);

#endif // TRANSFORM_H

