#include "common.h"
#include "gimage.h"
#include "gconvol.h"
#include <chrono>

int main(int argc, char *argv[])
{
    int time = getTimeMill();
    QCoreApplication a(argc, argv);
    
    QImage qimg("input.jpg");
    GImage gimg(qimg);
    
    cout << gimg.width << " " << gimg.height << std::endl;
    
    GImage out = getSobel(gimg);
    out.normalizeMinMax();
    out.save("sobel.jpg");
    cout << "Got Sobel." << std::endl;
    
    GConvol gauss = getGaussian(3);
//    unique_ptr<GImage> smooth(gauss->apply(gimg, EdgeType_Mirror));
    GImage smooth = gauss.applySeparate(gimg, EdgeType_Mirror);
    smooth.save("gauss.jpg");
    
    gimg.save("output.jpg");
    
    time = getTimeMill() - time;
    cout << "Completed in " << time << "ms." << endl;
    return a.exec();
}
