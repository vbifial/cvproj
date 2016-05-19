#include "common.h"
#include "gimage.h"
#include "gconvol.h"
#include "gpyramid.h"
#include "transform.h"
#include "gdrawing.h"
#include "gdetectors.h"
#include "gdescriptor.h"
#include <QDir>

QLatin1String s1("."), s2("..");
GImage gpattern;
gdvector pdescs;

void processFile(QString &path) {
    QImage img(path);
    if (img.isNull())
        return;
    GImage gimg = GImage(img);
    GPyramid pyr(gimg, 1.6f, 0.5f, 3);
    auto p = getDOGDetection(pyr);
    p = calculateOrientations(pyr, p);
    auto descs = getDescriptors(pyr, p);
    auto m = getMatchingPOIs(pdescs, descs, 
                             numeric_limits<float>::max());
    int k = getHough(m.first, m.second, gimg.width, gimg.height, 
                     1e-3f, 1e3f, 100, 100, 22, 10).second;
    if (k >= 4)
        cout << path.toStdString() << endl;
}

void processDir(QString &path) {
    cout << "dir " << path.toStdString() << endl;
    QDir dir(path);
    auto list = dir.entryList(QDir::Files);
    for (int i = 0; i < list.size(); i++) {
        QString filePath = path + "/" + list[i];
        processFile(filePath);
    }
    list = dir.entryList(QDir::Dirs);
    for (int i = 0; i < list.size(); i++) {
        if (list[i] == s1 || list[i] == s2)
            continue;
        QString np = path + "/" + list[i];
        processDir(np);
    }
}

int main(int argc, char *argv[])
{
    int time = getTimeMill();
    QCoreApplication a(argc, argv);
    
    QString dirPath = (argc > 1) ? a.arguments()[1] : 
            a.applicationDirPath();
    QString patternPath = (argc > 2) ? a.arguments()[2] :
            QString("input.jpg");
    QImage pattern(patternPath);
    if (pattern.isNull()) {
        pattern = QImage("input.jpg");
    }
    if (pattern.isNull())
        cout << "No pattern image." << endl;
    else {
        gpattern = GImage(pattern);
        GPyramid pyr(gpattern, 1.6f, 0.5f, 3);
        poivec p = getDOGDetection(pyr);
        p = calculateOrientations(pyr, p);
        pdescs = getDescriptors(pyr, p);
        processDir(dirPath);
    }
    
    time = getTimeMill() - time;
    cout << "Completed in " << time << "ms." << endl;
    return 0;
//    return a.exec();
}
