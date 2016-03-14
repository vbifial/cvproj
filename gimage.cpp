#include "gimage.h"

GImage::GImage()
{
    a = nullptr;
}

GImage::GImage(int width, int height)
{
    this->width = width;
    this->height = height;
    a = new float[width * height];
}

GImage::GImage(QImage &img)
{
    this->width = img.width();
    this->height = img.height();
    a = new float[width * height];
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            QRgb color = img.pixel(j, i);
            a[i * width + j] = fromRGB(color);
        }
    }
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

GImage::~GImage()
{
    if (a != nullptr)
        delete[] a;
}

