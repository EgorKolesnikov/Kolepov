#-------------------------------------------------
#
# Project created by QtCreator 2016-10-01T13:22:22
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EchoClient
TEMPLATE = app


SOURCES += main.cpp\
        echoclient.cpp \
    connectdialog.cpp

HEADERS  += echoclient.h \
    connectdialog.h