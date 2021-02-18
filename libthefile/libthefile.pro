QT += gui widgets thelib

TEMPLATE = lib
DEFINES += LIBTHEFILE_LIBRARY

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    directories/localfilesystemdirectory.cpp \
    directories/trashschemehandler.cpp \
    directory.cpp \
    directoryHandlers/localfiledirectoryhandler.cpp \
    directoryhandler.cpp \
    resourcemanager.cpp

HEADERS += \
    directories/localfilesystemdirectory.h \
    directories/trashschemehandler.h \
    directory.h \
    directoryHandlers/localfiledirectoryhandler.h \
    directoryhandler.h \
    libthefile_global.h \
    resourcemanager.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
