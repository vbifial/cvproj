QT += core
QT += gui

TARGET = cvproj
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11
CONFIG += c++14

TEMPLATE = app

INCLUDEPATH += ../libs/gsl/include
DEPENDPATH += ../libs/gsl/include
LIBS += ../libs/gsl/lib/libgsl-0-vc8.lib
LIBS += ../libs/gsl/lib/libgslcblas-0-vc8.lib

SOURCES += main.cpp \
    gimage.cpp \
    common.cpp \
    gconvol.cpp \
    gpyramid.cpp \
    transform.cpp \
    gdrawing.cpp \
    gdetectors.cpp \
    gdescriptor.cpp

HEADERS += \
    gimage.h \
    common.h \
    gconvol.h \
    gpyramid.h \
    transform.h \
    gdrawing.h \
    gdetectors.h \
    gdescriptor.h

