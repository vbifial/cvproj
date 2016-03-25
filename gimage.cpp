#include "gimage.h"

GImage::GImage()
{
    
}

GImage::GImage(int width, int height)
{
    this->width = width;
    this->height = height;
    unique_ptr<float[]> t(new float[width * height]);
    a = move(t);
}

GImage::GImage(QImage &img)
{
    this->width = img.width();
    this->height = img.height();
    unique_ptr<float[]> a(new float[width * height]);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            QRgb color = img.pixel(j, i);
            a[i * width + j] = fromRGB(color);
        }
    }
    this->a = move(a);
}

void GImage::save(const char *filename) {
    QImage img(width, height, QImage::Format_RGB32);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            img.setPixel(j, i, toRGB(a[i * width + j]));
        }
    }
    img.save(filename, "jpg", 98);
}

void GImage::normalizeMinMax()
{
    float min = 1e20;
    float max = -1e20;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            float val = a[i * width + j];
            if (val < min)
                min = val;
            if (val > max)
                max = val;
        }
    }
    float dlt = max - min + 1e-5;
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            float val = a[i * width + j];
            a[i * width + j] = (val - min) / dlt;
        }
    }
}

GImage::~GImage()
{
    
}

