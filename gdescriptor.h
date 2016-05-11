#ifndef GDESCRIPTOR
#define GDESCRIPTOR
#include "common.h"

const int DSIZE = 128;

struct gdescriptor {
    float vec[DSIZE];
    poi p;
};

gdvector getDescriptors(const GImage &img, const poivec &vpoi);

poivec calculateOrientations(GImage &img, poivec &vpoi);

vector<pair<int, int> > getMatches(const gdvector &dfirst, 
                                   const gdvector &dsecond, const float thres);


#endif // GDESCRIPTOR

