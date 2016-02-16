#-------------------------------------------------
#
# Project created by QtCreator 2016-02-13T14:12:34
#
#-------------------------------------------------

QT       += core gui dbus KParts
CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = thefile
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    udisks2.cpp \
    about.cpp \
    copy.cpp \
    filetransfers.cpp \
    copyworker.cpp \
    transferwindow.cpp \
    properties.cpp

HEADERS  += mainwindow.h \
    udisks2.h \
    about.h \
    copy.h \
    filetransfers.h \
    copyworker.h \
    transferwindow.h \
    properties.h

FORMS    += mainwindow.ui \
    about.ui \
    filetransfers.ui \
    transferwindow.ui \
    properties.ui
