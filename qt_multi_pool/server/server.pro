QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

TEMPLATE = app
TARGET = server
DEPENDPATH += .
INCLUDEPATH += .

OBJECTS_DIR = _build
DESTDIR  = ../bin

QT += network

SOURCES += main.cpp \
    server.cpp

HEADERS += \
    ../common/common.h \
    server.h
