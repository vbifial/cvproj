#include "common.h"
#include "gimage.h"
#include "gconvol.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    QImage qimg("input.jpg");
    GImage gimg(qimg);
    
    cout << gimg.width << " " << gimg.height << std::endl;
    
    GConvol c;
    c.r = 100;
    
    unique_ptr<GImage> out(getSobel(gimg));
    out->normalizeMinMax();
    out->save("sobel.jpg");
    
    gimg.save("output.jpg");
    
    cout << "Completed." << endl;
    return a.exec();
}

