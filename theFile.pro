#-------------------------------------------------
#
# Project created by QtCreator 2016-08-22T21:16:34
#
#-------------------------------------------------

QT       += core gui dbus
CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = thefile
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    tablewidget.cpp

HEADERS  += mainwindow.h \
    tablewidget.h

FORMS    += mainwindow.ui
