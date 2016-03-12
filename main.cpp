#include <QCoreApplication>
#include <iostream>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    std::cout << "hello" << std::endl;
    
    std::cout << argv[0];
    
    
    
    return a.exec();
}

