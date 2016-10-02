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
    sqlwrapper.cpp

HEADERS  += echoserver.h \
    serverthread.h \
    sqlwrapper.h \
    protocol.h
