#-------------------------------------------------
#
# Project created by QtCreator 2016-10-01T15:55:57
#
#-------------------------------------------------

QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EchoServer
TEMPLATE = app


SOURCES += main.cpp\
        echoserver.cpp \
    serverthread.cpp \
    sqlwrapper.cpp \
    protocol.cpp \
    cryption.cpp \
    securesocket.cpp

HEADERS  += echoserver.h \
    serverthread.h \
    sqlwrapper.h \
    protocol.h \
    cryption.h \
    securesocket.h

RESOURCES +=


win32: LIBS += -L$$PWD/../../cryptopp/x64/Output/Debug/ -lcryptlib

INCLUDEPATH += $$PWD/../../cryptopp
DEPENDPATH += $$PWD/../../cryptopp

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../cryptopp/x64/Output/Debug/cryptlib.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../../cryptopp/x64/Output/Debug/libcryptlib.a
