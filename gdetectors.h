#ifndef GDETECTORS
#define GDETECTORS
#include "common.h"

poivec getMoravec(const GImage &img, int wrad, int mrad, float thres);
poivec getHarris(const GImage &img, int wrad, int mrad, float thres, float _k = .08);
poivec anms(poivec &in, int target, float diff);

poivec getBlobs(GPyramid &pyr);
poivec getDOGDetection(const GImage &img);

#endif // GDETECTORS

