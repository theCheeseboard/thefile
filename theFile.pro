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
    tablewidget.cpp \
    runningtransfers.cpp \
    copyjob.cpp \
    aboutdialog.cpp

HEADERS  += mainwindow.h \
    tablewidget.h \
    runningtransfers.h \
    copyjob.h \
    aboutdialog.h

FORMS    += mainwindow.ui \
    runningtransfers.ui \
    aboutdialog.ui
