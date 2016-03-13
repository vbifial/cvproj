//#include <common.h>
#include "gimage.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    QImage qimg("input.jpg");
    GImage gimg(qimg);
    
    cout << gimg.width << " " << gimg.height << std::endl;
    
    gimg.save("output.jpg");
    
    return a.exec();
}

