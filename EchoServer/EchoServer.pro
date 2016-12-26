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
    securesocket.cpp \
    insertkeydialog.cpp

HEADERS  += echoserver.h \
    serverthread.h \
    sqlwrapper.h \
    protocol.h \
    cryption.h \
    securesocket.h \
    insertkeydialog.h

RESOURCES +=


win32: LIBS += -L$$PWD/../../cryptopp/x64/Output/Debug/ -lcryptlib

INCLUDEPATH += $$PWD/../../cryptopp
DEPENDPATH += $$PWD/../../cryptopp

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../cryptopp/x64/Output/Debug/cryptlib.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../../cryptopp/x64/Output/Debug/libcryptlib.a

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../usr/local/lib/release/ -lcryptopp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../usr/local/lib/debug/ -lcryptopp
else:unix: LIBS += -L$$PWD/../../../../../usr/local/lib/ -lcryptopp

INCLUDEPATH += $$PWD/../../../../../usr/local/include/cryptopp
DEPENDPATH += $$PWD/../../../../../usr/local/include/cryptopp
