QT += core
QT += gui

TARGET = cvproj
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11
CONFIG += c++14

TEMPLATE = app

SOURCES += main.cpp \
    gimage.cpp \
    common.cpp \
    gconvol.cpp \
    gpyramid.cpp

HEADERS += \
    gimage.h \
    common.h \
    gconvol.h \
    gpyramid.h

