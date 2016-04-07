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
    
    GPyramid pyr(gimg, 1.6, 10);
    
    char st[] = "pyr??.jpg";
    for (int i = 0; i < pyr.ocnt; i++) {
        st[3] = '0' + i;
        for (int j = 0; j < pyr.olayers; j++) {
            st[4] = '0' + j;
            pyr.a[i * pyr.olayers + j].save(st);
        }
    }
    
    time = getTimeMill() - time;
    cout << "Completed in " << time << "ms." << endl;
    return a.exec();
}
