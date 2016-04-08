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
    
    GPyramid pyr(gimg, 1.6, .5, 4);
    
    GImage out(gimg.width, gimg.height);
    
    for (int i = 0; i < out.height; i++) {
        for (int j = 0; j < out.width; j++) {
            out.a[i * out.width + j] = pyr.L(j, i, 64.);
        }
    }
    out.save("out.jpg");
    
    time = getTimeMill() - time;
    cout << "Completed in " << time << "ms." << endl;
    return a.exec();
}
