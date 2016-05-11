#ifndef GDESCRIPTOR
#define GDESCRIPTOR
#include "common.h"

const int DRAD = 6; // descriptor`s radius
const int DBOXES = 4; // boxes quantity per dimension
const int BDIRS = 8; // directions quantity in box
const int DSIZE = DBOXES * DBOXES * BDIRS; // descriptor`s size

struct gdescriptor {
    float vec[DSIZE];
    poi p;
};

gdvector getDescriptors(const GImage &img, const poivec &vpoi);

poivec calculateOrientations(GImage &img, poivec &vpoi);

vector<pair<int, int> > getMatches(const gdvector &dfirst, 
                                   const gdvector &dsecond, const float thres);


#endif // GDESCRIPTOR

