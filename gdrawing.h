#ifndef GDRAWING_H
#define GDRAWING_H

const float rgbLum[] = {0.213, 0.715, 0.072};

float fromRGB(int color);
int toRGB(float color);

void drawLine(QImage &img, int x1, int y1, int x2, int y2, int color);
void drawCircle(QImage &img, int x, int y, float r, int color);

void mark(QImage &img, int x, int y);

QImage drawPoints(const GImage &img, poivec &vpoi);
QImage drawMatches(const GImage &img1, const GImage &img2, 
                   gdvector &desc1, gdvector &desc2, vector<pair<int, int> > &matches, 
                   bool lines, bool marks);

QImage drawBlobs(const GImage &img, poivec &blobs, bool drawDirections);
void saveJpeg(QImage &img, const char* filename);

GImage getOverlapping(const GImage& a, const GImage& b, const vector<float>& h);

#endif // GDRAWING_H

