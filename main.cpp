#include "common.h"
#include "gimage.h"
#include "gconvol.h"
#include "gpyramid.h"

int main(int argc, char *argv[])
{
    int time = getTimeMill();
    QCoreApplication a(argc, argv);
    
    QImage qimg("input.jpg");
    GImage gimg(qimg);
    
    QImage qimg2("input2.jpg");
    GImage gimg2(qimg2);
    
    cout << gimg.width << " " << gimg.height << std::endl;
    
//    gimg.save("output.jpg");
    
//    GPyramid pyr(gimg, 1.6, .5, 4);
    
    auto har1 = getHarris(gimg, 2, 2, .2);
    har1 = anms(har1, 200, .1);
    
    auto har2 = getHarris(gimg2, 2, 2, .2);
    har2 = anms(har2, 200, .1);
    
    auto desc1 = getDescriptors(gimg, har1);
    auto desc2 = getDescriptors(gimg2, har2);
    
    auto matches = getMatches(desc1, desc2, 1e-1);
    
    QImage out(gimg.width + gimg2.width, 
               max(gimg.height, gimg2.height), QImage::Format_RGB32);
    
    for (int i = 0; i < gimg.height; i++) {
        for (int j = 0; j < gimg.width; j++) {
            out.setPixel(j, i, toRGB(gimg.a[i * gimg.width + j]));
        }
    }
    for (int i = 0; i < gimg2.height; i++) {
        for (int j = 0; j < gimg2.width; j++) {
            out.setPixel(j + gimg.width, i, 
                         toRGB(gimg2.a[i * gimg2.width + j]));
        }
    }
    
    uniform_int_distribution<uint32_t> uint_dist(0, (1 << 24) - 1);
    mt19937 rnd;
    
    for (uint i = 0; i  < matches.size(); i++) {
        int color = uint_dist(rnd);
        auto &l = desc1[matches[i].first];
        auto &r = desc2[matches[i].second];        
        drawLine(out, get<1>(l), get<2>(l), 
                 get<1>(r) + gimg.width, get<2>(r), color);
    }
    
    out.save("mathches.jpg", 0, 99);
    
    time = getTimeMill() - time;
    cout << "Completed in " << time << "ms." << endl;
    return a.exec();
}
