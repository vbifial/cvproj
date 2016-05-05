#include "common.h"
#include "gimage.h"
#include "gconvol.h"
#include "gpyramid.h"

int main(int argc, char *argv[])
{
    int time = getTimeMill();
    QCoreApplication a(argc, argv);
    
    QImage qimg("input1.jpg");
    QImage qimg2("input2.jpg");
//    QImage qimg("input3.jpg");
//    QImage qimg2("input4.jpg");
    
    GImage gimg(qimg);
    GImage gimg2(qimg2);
    
    cout << gimg.width << " " << gimg.height << std::endl;
    
//    gimg.save("output.jpg");
    
    QImage qbimg("binput.jpg");
    GImage bimg(qbimg);
    GPyramid pyr(bimg, 1.6, .5, 4);
    
    GPyramid ypyr = pyr.getDOG();
    
    char c[20] = "pyr??.jpg";
    
    for (int i = 0; i < ypyr.ocnt; i++) {
        c[3] = '0' + i;
        for (int j = 0; j <= ypyr.olayers; j++) {
            GImage& gout = ypyr.a[i * (ypyr.olayers + 1) + j];
            c[4] = '0' + j;
            for (int x = 0; x < gout.height * gout.width; x++)
                gout.a[x] = gout.a[x] * 2.f - 0.5f;
            gout.save(c);
        }
    }
    
    auto blobs = getBlobs(pyr);
    
    cout << "Blobs " << blobs.size() << endl;
    
    auto out = drawBlobs(bimg, blobs);
    saveJpeg(out, "blobs.jpg");
    
//    auto har1 = getHarris(gimg, 2, 2, 1e-20);
//    cout << "Harris 1: " << har1.size() << endl;
//    har1 = anms(har1, 200, .1);
//    cout << "anms 1: " << har1.size() << endl;
    
//    auto har2 = getHarris(gimg2, 2, 2, 1e-20);
//    cout << "Harris 2: " << har2.size() << endl;
//    har2 = anms(har2, 200, .1);
//    cout << "anms 2: " << har2.size() << endl;
    
//    QImage qh1 = drawPoints(gimg, har1);
//    saveJpeg(qh1, "har1.jpg");
//    QImage qh2 = drawPoints(gimg2, har2);
//    saveJpeg(qh2, "har2.jpg");
    
//    auto desc1 = getDescriptors(gimg, har1);
//    cout << "desc 1: " << desc1.size() << endl;
//    auto desc2 = getDescriptors(gimg2, har2);
//    cout << "desc 2: " << desc2.size() << endl;
    
//    auto matches = getMatches(desc1, desc2, 7e-1);
////    auto matches = getMatches(desc1, desc2, 1e2);
//    cout << "matches : " << matches.size() << endl;
    
//    QImage out = drawMatches(gimg, gimg2, desc1, desc2, matches);
    
//    saveJpeg(out, "mathches.jpg");
    
    time = getTimeMill() - time;
    cout << "Completed in " << time << "ms." << endl;
    return a.exec();
}
