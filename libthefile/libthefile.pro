QT += gui widgets thelib frisbee tdesktopenvironment

TEMPLATE = lib
DEFINES += LIBTHEFILE_LIBRARY
TARGET = thefile

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    bookmarkmanager.cpp \
    directories/localfilesystemdirectory.cpp \
    directories/trashdirectory.cpp \
    directory.cpp \
    directoryHandlers/localfiledirectoryhandler.cpp \
    directoryHandlers/trashdirectoryhandler.cpp \
    directoryhandler.cpp \
    filecolumn.cpp \
    filecolumnaction.cpp \
    filecolumnfloater.cpp \
    filecolumnmanager.cpp \
    filemodel.cpp \
    filetab.cpp \
    hiddenfilesproxymodel.cpp \
    popovers/unlockencryptedpopover.cpp \
    resourcemanager.cpp \
    sidebar/bookmarksmodel.cpp \
    sidebar/devicesmodel.cpp \
    sidebar/sidebar.cpp

HEADERS += \
    bookmarkmanager.h \
    directories/localfilesystemdirectory.h \
    directories/trashdirectory.h \
    directory.h \
    directoryHandlers/localfiledirectoryhandler.h \
    directoryHandlers/trashdirectoryhandler.h \
    directoryhandler.h \
    filecolumn.h \
    filecolumnaction.h \
    filecolumnfloater.h \
    filecolumnmanager.h \
    filemodel.h \
    filetab.h \
    hiddenfilesproxymodel.h \
    libthefile_global.h \
    popovers/unlockencryptedpopover.h \
    resourcemanager.h \
    sidebar/bookmarksmodel.h \
    sidebar/devicesmodel.h \
    sidebar/sidebar.h

# Default rules for deployment.
unix {
    # Include the-libs build tools
    equals(THELIBS_BUILDTOOLS_PATH, "") {
        THELIBS_BUILDTOOLS_PATH = $$[QT_INSTALL_PREFIX]/share/the-libs/pri
    }
    include($$THELIBS_BUILDTOOLS_PATH/varset.pri)

    target.path = $$THELIBS_INSTALL_LIB

    headers.path = $$THELIBS_INSTALL_HEADERS/libthefile
    headers.files = $$files(*.h, true)

    module.files = qt_thefile.pri
    module.path = $$THELIBS_INSTALL_MODULES

    INSTALLS += target headers
}

FORMS += \
    filecolumn.ui \
    filecolumnaction.ui \
    filecolumnfloater.ui \
    filetab.ui \
    popovers/unlockencryptedpopover.ui \
    sidebar/sidebar.ui

DISTFILES += \
    qt_thefile.pri
