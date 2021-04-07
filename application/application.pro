QT       += core gui thelib concurrent frisbee tdesktopenvironment
SHARE_APP_NAME=thefile

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    filecolumn.cpp \
    filecolumnaction.cpp \
    filecolumnfloater.cpp \
    filecolumnmanager.cpp \
    filemodel.cpp \
    filetab.cpp \
    hiddenfilesproxymodel.cpp \
    jobs/burnjob.cpp \
    jobs/filetransferjob.cpp \
    jobs/widgets/burnjobprogress.cpp \
    jobs/widgets/filetransferjobwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    popovers/burnpopover.cpp \
    popovers/deletepermanentlypopover.cpp \
    popovers/itempropertiespopover.cpp \
    popovers/unlockencryptedpopover.cpp \
    sidebar/devicesmodel.cpp \
    sidebar/sidebar.cpp \
    tabbutton.cpp

HEADERS += \
    filecolumn.h \
    filecolumnaction.h \
    filecolumnfloater.h \
    filecolumnmanager.h \
    filemodel.h \
    filetab.h \
    hiddenfilesproxymodel.h \
    jobs/burnjob.h \
    jobs/filetransferjob.h \
    jobs/widgets/burnjobprogress.h \
    jobs/widgets/filetransferjobwidget.h \
    mainwindow.h \
    popovers/burnpopover.h \
    popovers/deletepermanentlypopover.h \
    popovers/itempropertiespopover.h \
    popovers/unlockencryptedpopover.h \
    sidebar/devicesmodel.h \
    sidebar/sidebar.h \
    tabbutton.h

FORMS += \
    filecolumn.ui \
    filecolumnaction.ui \
    filecolumnfloater.ui \
    filetab.ui \
    jobs/widgets/burnjobprogress.ui \
    jobs/widgets/filetransferjobwidget.ui \
    mainwindow.ui \
    popovers/burnpopover.ui \
    popovers/deletepermanentlypopover.ui \
    popovers/itempropertiespopover.ui \
    popovers/unlockencryptedpopover.ui \
    sidebar/sidebar.ui

DESKTOP_FILE += \
   com.vicr123.thefile.desktop

unix:!macx {
    DESKTOP_FILE = com.vicr123.thefile.desktop
    DESKTOP_FILE_BLUEPRINT = com.vicr123.thefile_blueprint.desktop

    # Include the-libs build tools
    equals(THELIBS_BUILDTOOLS_PATH, "") {
        THELIBS_BUILDTOOLS_PATH = $$[QT_INSTALL_PREFIX]/share/the-libs/pri
    }
    include($$THELIBS_BUILDTOOLS_PATH/buildmaster.pri)

    DEFINES += SYSTEM_LIBRARY_DIRECTORY=\\\"$$THELIBS_INSTALL_LIB\\\"

    QT += thelib
    TARGET = thefile

    LIBS += -L$$OUT_PWD/../libthefile/ -llibthefile

    target.path = $$THELIBS_INSTALL_BIN

    defaults.files = defaults.conf
    defaults.path = $$THELIBS_INSTALL_SETTINGS/theSuite/theBeat/

    blueprint {
        metainfo.files = com.vicr123.thefile_blueprint.metainfo.xml
        icon.files = icons/com.vicr123.thefile_blueprint.svg
    } else {
        metainfo.files = com.vicr123.thefile.metainfo.xml
        icon.files = icons/com.vicr123.thefile.svg
    }

    icon.path = $$THELIBS_INSTALL_PREFIX/share/icons/hicolor/scalable/apps/
    metainfo.path = $$THELIBS_INSTALL_PREFIX/share/metainf

    INSTALLS += target icon defaults metainfo
}


INCLUDEPATH += $$PWD/../libthefile
DEPENDPATH += $$PWD/../libthefile

DISTFILES += \
    defaults.conf

RESOURCES += \
    resources.qrc
