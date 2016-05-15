#include "common.h"
#include "gimage.h"
#include "gconvol.h"
#include "gpyramid.h"
#include "transform.h"
#include "gdrawing.h"
#include "gdetectors.h"
#include "gdescriptor.h"

int main(int argc, char *argv[])
{
    int time = getTimeMill();
    QCoreApplication a(argc, argv);
    
//    gsl_matrix_alloc(1, 1);
    
//    QImage qimg("input1.jpg");
//    QImage qimg2("input2.jpg");
//    QImage qimg("input3.jpg");
//    QImage qimg2("input4.jpg");
    QImage qimg("input5.jpg");
    QImage qimg2("input6.jpg");
    
    GImage gimg(qimg);
    GImage gimg2(qimg2);
    
    cout << gimg.width << " " << gimg.height << std::endl;
    
//    gimg.save("output.jpg");
    
//    QImage qbimg("binput.jpg");
//    QImage qbimg("old\\input-face.jpg");
//    GImage bimg(qbimg);
//    GPyramid pyr(bimg, 1.6, .5, 3);
    
//    GPyramid dpyr = pyr.getDOG();
//    GPyramid& ypyr = dpyr;
    
//    char c[20] = "pyr??.jpg";
    
//    for (int i = 0; i < ypyr.ocnt; i++) {
//        c[3] = '0' + i;
//        for (int j = (i == 0 ? 0 : -1); j < ypyr.olayers + 1; j++) {
//            GImage& gout = ypyr.a[i * (ypyr.olayers + 2) + j];
//            c[4] = '0' + j + 1;
//            for (int x = 0; x < gout.height * gout.width; x++)
//                gout.a[x] = -gout.a[x] * 0.5f + 0.5f;
//            gout.save(c);
//        }
//    }
//    for (int i = 0; i < pyr.ocnt; i++) {
//        c[3] = '0' + i;
//        for (int j = 0; j < pyr.olayers + 2; j++) {
//            GImage& gout = pyr.a[i * (pyr.olayers + 3) + j];
//            c[4] = '0' + j;
////            for (int x = 0; x < gout.height * gout.width; x++)
////                gout.a[x] = -gout.a[x] * 0.5f + 0.5f;
//            gout.save(c);
//        }
//    }
    
    
    
//    auto blobs = getBlobs(pyr);
//    auto blobs = getDOGDetection(bimg);
    
//    cout << "Blobs " << blobs.size() << endl;
    
//////    auto out = drawBlobs(pyr.a[0], blobs);
//    auto out = drawBlobs(bimg, blobs, false);
//    saveJpeg(out, "blobs.jpg");
    
    
    auto pv1 = getDOGDetection(gimg);
    cout << "Points 1 " << pv1.size() << endl;
    pv1 = calculateOrientations(gimg, pv1);
    cout << "Points 1 " << pv1.size() << endl;
    auto pv2 = getDOGDetection(gimg2);
    cout << "Points 2 " << pv2.size() << endl;
    pv2 = calculateOrientations(gimg2, pv2);
    cout << "Points 2 " << pv2.size() << endl;
    
    auto out1 = drawBlobs(gimg, pv1, true);
    cout << "draw1" << endl;
    saveJpeg(out1, "out1.jpg");
    auto out2 = drawBlobs(gimg2, pv2, true);
    cout << "draw2" << endl;
    saveJpeg(out2, "out2.jpg");
    
    auto bdesc1 = getDescriptors(gimg, pv1);
    cout << "bdesc 1: " << bdesc1.size() << endl;
    auto bdesc2 = getDescriptors(gimg2, pv2);
    cout << "bdesc 2: " << bdesc2.size() << endl;
    
    
//    auto bmatches = getMatches(bdesc1, bdesc2, 1e0);
    auto bmatches = getMatches(bdesc1, bdesc2,
                               numeric_limits<float>::max());
    cout << "matches : " << bmatches.size() << endl;
    
    QImage bout = drawMatches(gimg, gimg2, bdesc1, bdesc2, bmatches, 
                              true, false);
    saveJpeg(bout, "bmathches.jpg");
    QImage bout2 = drawMatches(gimg, gimg2, bdesc1, bdesc2, bmatches, 
                              false, true);
    saveJpeg(bout2, "bmathches2.jpg");
    
    poivec l(bmatches.size());
    poivec r(bmatches.size());
    for (size_t i = 0; i < bmatches.size(); i++) {
        l[i] = bdesc1[bmatches[i].first].p;
        r[i] = bdesc2[bmatches[i].second].p;
    }
    
////    auto h = getRansacTransform(r, l, 50.f, .4f);
//    auto h = getHoughTransform(r, l, gimg.width, gimg.height,
//                               1e-3, 1e3, 200, 200, 27, 16);
////    auto h = getTransformation(r, l);
//    cout << "got transformation" << endl;
//    for (size_t i = 0; i < r.size(); i++) {
//        r[i] = transformPOI(h, r[i]);
//    }
//    auto out3 = drawBlobs(gimg, r, true);
//    saveJpeg(out3, "out3.jpg");
    
//    auto out4 = getOverlapping(gimg, gimg2, h, true);
//    auto out5 = drawBorder(gimg, gimg2, h);
//    out4.save("out4.jpg");
//    saveJpeg(out5, "out5.jpg");
    
    
    
//    auto har1 = getHarris(gimg, 2, 2, 1e-8);
//    cout << "Harris 1: " << har1.size() << endl;
//    har1 = anms(har1, 200, .1);
//    cout << "anms 1: " << har1.size() << endl;
    
//    auto har2 = getHarris(gimg2, 2, 2, 1e-8);
//    cout << "Harris 2: " << har2.size() << endl;
////    har2 = anms(har2, 200, .1);
//    cout << "anms 2: " << har2.size() << endl;
    
//    QImage qh1 = drawPoints(gimg, har1);
//    saveJpeg(qh1, "har1.jpg");
//    QImage qh2 = drawPoints(gimg2, har2);
//    saveJpeg(qh2, "har2.jpg");
    
//    auto desc1 = getDescriptors(gimg, har1);
//    cout << "desc 1: " << desc1.size() << endl;
//    auto desc2 = getDescriptors(gimg2, har2);
//    cout << "desc 2: " << desc2.size() << endl;
    
//    auto matches = getMatches(desc1, desc2, 5e-1);
////    auto matches = getMatches(desc1, desc2, 1e2);
//    cout << "matches : " << matches.size() << endl;
    
//    QImage out = drawMatches(gimg, gimg2, desc1, desc2, matches);
    
//    saveJpeg(out, "mathches.jpg");
    
    time = getTimeMill() - time;
    cout << "Completed in " << time << "ms." << endl;
    return a.exec();
}
