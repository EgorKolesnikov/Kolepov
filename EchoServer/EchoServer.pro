#-------------------------------------------------
#
# Project created by QtCreator 2016-10-01T15:55:57
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EchoServer
TEMPLATE = app


SOURCES += main.cpp\
        echoserver.cpp \
    serverthread.cpp

HEADERS  += echoserver.h \
    serverthread.h
