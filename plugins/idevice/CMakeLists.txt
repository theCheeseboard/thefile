project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets DBus)
find_package(libcontemporary)
find_package(QuaZip-Qt6 REQUIRED)
cntp_find_pkgconfig(imobiledevice libimobiledevice-1.0)
cntp_find_pkgconfig(plist libplist++-2.0)

set(SOURCES
    plugin.cpp
    idevicewatcher.cpp
    idevice.cpp
    idevicemodel.cpp
    isoftwareupdatefile.cpp
    idevicefilecolumnwidget.cpp idevicefilecolumnwidget.ui

    sidebar/idevicesidebarsection.cpp
    sidebar/idevicesidebarsectionfactory.cpp

    popovers/idevicerestorepopover.cpp popovers/idevicerestorepopover.ui

    jobs/idevicerestorejob.cpp
    jobs/progress/idevicerestorejobprogress.cpp jobs/progress/idevicerestorejobprogress.ui

    directories/idevicerootdirectory.cpp
    directories/idevicedirectoryhandler.cpp
)

set(HEADERS
    plugin.h
    idevicewatcher.h
    idevice.h
    idevicemodel.h
    isoftwareupdatefile.h
    idevicefilecolumnwidget.h

    sidebar/idevicesidebarsection.h
    sidebar/idevicesidebarsectionfactory.h

    popovers/idevicerestorepopover.h

    jobs/idevicerestorejob.h
    jobs/progress/idevicerestorejobprogress.h

    directories/idevicerootdirectory.h
    directories/idevicedirectoryhandler.h
)

add_library(plugin-idevice SHARED ${SOURCES} ${HEADERS})
target_include_directories(plugin-idevice PUBLIC ../../libthefile/)

cntp_init_plugin(thefile plugin-idevice 20 ideviceplugin)
cntp_translate(plugin-idevice)

set_target_properties(plugin-idevice PROPERTIES
        OUTPUT_NAME idevicePlugin
        FRAMEWORK FALSE)

target_link_libraries(plugin-idevice Qt6::Widgets Qt6::DBus libcontemporary libthefile PkgConfig::imobiledevice PkgConfig::plist QuaZip::QuaZip)
