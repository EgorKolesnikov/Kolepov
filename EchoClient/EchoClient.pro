#-------------------------------------------------
#
# Project created by QtCreator 2016-10-01T13:22:22
#
#-------------------------------------------------

QT       += core gui network
QMAKE_CXXFLAGS += -std=c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EchoClient
TEMPLATE = app


SOURCES += main.cpp\
        echoclient.cpp \
    connectdialog.cpp \
    protocol.cpp \
    cryption.cpp \
    securesocket.cpp

HEADERS  += echoclient.h \
    connectdialog.h \
    protocol.h \
    cryption.h \
    securesocket.h

RESOURCES += \
    resource.qrc



win32: LIBS += -L$$PWD/../../cryptopp/x64/Output/Debug/ -lcryptlib

INCLUDEPATH += $$PWD/../../cryptopp
DEPENDPATH += $$PWD/../../cryptopp

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../cryptopp/x64/Output/Debug/cryptlib.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../../cryptopp/x64/Output/Debug/libcryptlib.a
