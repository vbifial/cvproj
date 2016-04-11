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
    
    cout << gimg.width << " " << gimg.height << std::endl;
    
    gimg.save("output.jpg");
    
//    GPyramid pyr(gimg, 1.6, .5, 4);
    
    auto mor = getMoravec(gimg, 2, 2, 1e-2);
    
    cout << "moravec " << mor.size() << endl;
    mor = anms(mor, 200, .1);
    
    QImage out = gimg.convert();
    for (uint i = 0; i < mor.size(); i++) {
        mark(out, get<0>(mor[i]), get<1>(mor[i]));
    }
    out.save("dmoravec.jpg", 0, 99);
    
    auto har = getHarris(gimg, 2, 2, .2);
    
    cout << "harris " << har.size() << endl;
    har = anms(har, 200, .1);
    
    out = gimg.convert();
    for (uint i = 0; i < har.size(); i++) {
        mark(out, get<0>(har[i]), get<1>(har[i]));
    }
    out.save("dharris.jpg", 0, 99);
    
    time = getTimeMill() - time;
    cout << "Completed in " << time << "ms." << endl;
    return a.exec();
}
