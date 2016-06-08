#-------------------------------------------------
#
# Project created by QtCreator 2016-02-22T14:25:57
#
#-------------------------------------------------

QT       += core gui x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT_CONFIG -= no-pkg-config

include (srtParser/srtParser.pri)
include (mediaProcessor/mediaProcessor.pri)
include (waveformView/waveformView.pri)
include (videoPlayer/videoPlayer.pri)

QMAKE_CXXFLAGS -= -O2
QMAKE_CXXFLAGS += -O0

TARGET = NameToBeAssigned
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    subtitlemodel.cpp \
    newprojectdialog.cpp \
    extractwavdialog.cpp

HEADERS  += mainwindow.h \
    subtitlemodel.h \
    newprojectdialog.h \
    extractwavdialog.h \
    constrain.h

FORMS    += mainwindow.ui \
    newprojectdialog.ui \
    extractwavdialog.ui

CONFIG += c++11 link_pkgconfig
