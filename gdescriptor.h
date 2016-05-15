#ifndef GDESCRIPTOR
#define GDESCRIPTOR
#include "common.h"

const int DRAD = 6; // descriptor`s radius
const int DBOXES = 4; // boxes quantity per dimension
const int BDIRS = 8; // directions quantity in box
const int DSIZE = DBOXES * DBOXES * BDIRS; // descriptor`s size
const int ABCOUNT = 36; // directions quantity for orientation

struct gdescriptor {
    float vec[DSIZE];
    poi p;
};

gdvector getDescriptors(const GImage &img, const poivec &vpoi);

poivec calculateOrientations(GImage &img, poivec &vpoi);
pair<poi, poi> calculateOrientations(GImage &img, poi &p, float imgSig);
//poivec calculateOrientations(GPyramid &pyr, poivec &vpoi);

vector<pair<int, int> > getMatches(const gdvector &dfirst, 
                                   const gdvector &dsecond, const float thres);

void calcHistograms(const GImage &img, 
                    float x, float y, float orient, float* dv, 
                    int dboxes, float rad, int bdirs);

pair<float, float> getPOIDirections(const GImage& img, const poi& p, float rad);

#endif // GDESCRIPTOR

