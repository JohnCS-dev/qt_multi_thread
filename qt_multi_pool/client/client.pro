QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

TEMPLATE = app
TARGET = client
DEPENDPATH += .
INCLUDEPATH += .

OBJECTS_DIR = _build
DESTDIR  = ../bin

QT += network

SOURCES += main.cpp \
    client.cpp

HEADERS += \
    ../common/common.h \
    client.h
