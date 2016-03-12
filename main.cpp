#include <QCoreApplication>
#include <iostream>
#include <QImage>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    QImage qimg("input.jpg");
    
    std::cout << qimg.width() << " " << qimg.height() << std::endl;
    
    return a.exec();
}

