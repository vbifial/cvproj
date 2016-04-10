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
    
    vector<pair<int, int> > mor = getMoravec(gimg, 2, 2, 0.1);
    
    cout << mor.size() << endl;
    
    QImage out = gimg.convert();
    for (uint i = 0; i < mor.size(); i++) {
        mark(out, mor[i].first, mor[i].second);
    }
    out.save("moravec.jpg", 0, 99);
    
    time = getTimeMill() - time;
    cout << "Completed in " << time << "ms." << endl;
    return a.exec();
}
