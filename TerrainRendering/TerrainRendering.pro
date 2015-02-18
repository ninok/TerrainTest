#-------------------------------------------------
#
# Project created by QtCreator 2015-02-18T20:48:27
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TerrainRendering
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    renderingwidget.cpp

HEADERS  += mainwindow.h \
    renderingwidget.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc
