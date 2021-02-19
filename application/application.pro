QT       += core gui thelib concurrent frisbee tdesktopenvironment
TARGET = thefile

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    filecolumn.cpp \
    filecolumnaction.cpp \
    filemodel.cpp \
    filetab.cpp \
    hiddenfilesproxymodel.cpp \
    jobs/filetransferjob.cpp \
    jobs/widgets/filetransferjobwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    popovers/deletepermanentlypopover.cpp \
    sidebar/devicesmodel.cpp \
    sidebar/sidebar.cpp

HEADERS += \
    filecolumn.h \
    filecolumnaction.h \
    filemodel.h \
    filetab.h \
    hiddenfilesproxymodel.h \
    jobs/filetransferjob.h \
    jobs/widgets/filetransferjobwidget.h \
    mainwindow.h \
    popovers/deletepermanentlypopover.h \
    sidebar/devicesmodel.h \
    sidebar/sidebar.h

FORMS += \
    filecolumn.ui \
    filecolumnaction.ui \
    filetab.ui \
    jobs/widgets/filetransferjobwidget.ui \
    mainwindow.ui \
    popovers/deletepermanentlypopover.ui \
    sidebar/sidebar.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libthefile/release/ -llibthefile
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libthefile/debug/ -llibthefile
else:unix: LIBS += -L$$OUT_PWD/../libthefile/ -llibthefile

INCLUDEPATH += $$PWD/../libthefile
DEPENDPATH += $$PWD/../libthefile

DISTFILES += \
    defaults.conf

RESOURCES += \
    resources.qrc
