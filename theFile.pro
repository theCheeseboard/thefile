#-------------------------------------------------
#
# Project created by QtCreator 2017-08-15T20:05:36
#
#-------------------------------------------------

QT       += core gui thelib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = thefile
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += BLOCK_SIZE=4194304

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    filetable.cpp \
    filesystemmodel.cpp \
    folderbar.cpp \
    driveslist.cpp \
    drivesmodel.cpp \
    aboutdialog.cpp \
    forceunmountdialog.cpp \
    propertiesdialog.cpp \
    transfers/transferengine.cpp \
    transfers/transferpane.cpp \
    transfers/conflictresolver.cpp

HEADERS += \
        mainwindow.h \
    filetable.h \
    filesystemmodel.h \
    folderbar.h \
    driveslist.h \
    drivesmodel.h \
    aboutdialog.h \
    forceunmountdialog.h \
    propertiesdialog.h \
    transfers/transferengine.h \
    transfers/transferpane.h \
    transfers/conflictresolver.h

FORMS += \
        mainwindow.ui \
    aboutdialog.ui \
    forceunmountdialog.ui \
    propertiesdialog.ui \
    transfers/transferpane.ui \
    transfers/conflictresolver.ui

DISTFILES += \
    thefile.desktop

unix {
    target.path = /usr/bin

    appentry.path = /usr/share/applications
    appentry.files = thefile.desktop

    INSTALLS += target appentry
}
