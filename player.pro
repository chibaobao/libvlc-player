#-------------------------------------------------
#
# Project created by QtCreator 2017-09-11T17:10:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = player
TEMPLATE = app


SOURCES += main.cpp \
    vlcplayerwidget.cpp \
    yuv2rgb16tab.c \
    yuv420rgb888c.c \
    widget.cpp

HEADERS  += \
    vlcplayerwidget.h \
    yuv2rgb.h \
    widget.h
LIBS += -L E:\work\vlc-2.2.1\lib -l vlc
INCLUDEPATH += E:\work\vlc-2.2.1\include
DEFINES += NOT_SO

FORMS += \
    widget.ui
