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
    GImage* out;
    out = c.apply(gimg, EdgeType_BorderCopy);
    out->save("out1.jpg");
    out = c.apply(gimg, EdgeType_Mirror);
    out->save("out2.jpg");
    out = c.apply(gimg, EdgeType_Wrap);
    out->save("out3.jpg");
    out = c.apply(gimg, EdgeType_Zero);
    out->save("out4.jpg");
    
    gimg.save("output.jpg");
    
    return a.exec();
}

