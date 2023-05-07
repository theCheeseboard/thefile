project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets DBus)
find_package(libcontemporary)

set(SOURCES
    plugin.cpp
    nearbysharewidget.cpp nearbysharewidget.ui

    directories/nearbysharedirectoryhandler.cpp
    directories/nearbysharerootdirectory.cpp

    dbus/nearbysharemanager.cpp
    dbus/nearbysharetargetdiscovery.cpp
    dbus/nearbysharelistening.cpp
)

set(HEADERS
    plugin.h
    nearbysharewidget.h

    directories/nearbysharedirectoryhandler.h
    directories/nearbysharerootdirectory.h

    dbus/nearbysharemanager.h
    dbus/nearbysharetargetdiscovery.h
    dbus/nearbysharelistening.h
)

add_library(plugin-qnearbyshare SHARED ${SOURCES} ${HEADERS})
target_include_directories(plugin-qnearbyshare PUBLIC ../../libthefile/)

cntp_init_plugin(thefile plugin-qnearbyshare 20 ideviceplugin)
cntp_translate(plugin-qnearbyshare)

set_target_properties(plugin-qnearbyshare PROPERTIES
        OUTPUT_NAME idevicePlugin
        FRAMEWORK FALSE)

target_link_libraries(plugin-qnearbyshare Qt6::Widgets Qt6::DBus libcontemporary libthefile)
