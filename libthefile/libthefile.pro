QT += gui widgets thelib

TEMPLATE = lib
DEFINES += LIBTHEFILE_LIBRARY

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    directories/localfilesystemdirectory.cpp \
    directories/trashdirectory.cpp \
    directory.cpp \
    directoryHandlers/localfiledirectoryhandler.cpp \
    directoryHandlers/trashdirectoryhandler.cpp \
    directoryhandler.cpp \
    resourcemanager.cpp

HEADERS += \
    directories/localfilesystemdirectory.h \
    directories/trashdirectory.h \
    directory.h \
    directoryHandlers/localfiledirectoryhandler.h \
    directoryHandlers/trashdirectoryhandler.h \
    directoryhandler.h \
    libthefile_global.h \
    resourcemanager.h

# Default rules for deployment.
unix {
    # Include the-libs build tools
    equals(THELIBS_BUILDTOOLS_PATH, "") {
        THELIBS_BUILDTOOLS_PATH = $$[QT_INSTALL_PREFIX]/share/the-libs/pri
    }
    include($$THELIBS_BUILDTOOLS_PATH/varset.pri)

    target.path = $$THELIBS_INSTALL_LIB

    INSTALLS += target
}
