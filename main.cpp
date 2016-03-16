#include "common.h"
#include "gimage.h"
#include "gconvol.h"
#include <chrono>

int main(int argc, char *argv[])
{
    int time = chrono::duration_cast<chrono::milliseconds>
            (chrono::system_clock::now().time_since_epoch()).count();
    QCoreApplication a(argc, argv);
    
    QImage qimg("input.jpg");
    GImage gimg(qimg);
    
    cout << gimg.width << " " << gimg.height << std::endl;
    
    unique_ptr<GImage> out(getSobel(gimg));
    out->normalizeMinMax();
    out->save("sobel.jpg");
    
    unique_ptr<GConvol> gauss(getGaussian(100));
//    unique_ptr<GImage> smooth(gauss->apply(gimg, EdgeType_Mirror));
    unique_ptr<GImage> smooth(gauss->applySeparate(gimg, EdgeType_Mirror));
    smooth->save("gauss.jpg");
    
    gimg.save("output.jpg");
    
    time -= chrono::duration_cast<chrono::milliseconds>
            (chrono::system_clock::now().time_since_epoch()).count();
    time = -time;
    cout << "Completed in " << time << "ms." << endl;
    return a.exec();
}
