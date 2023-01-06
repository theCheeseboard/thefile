project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets DBus)
find_package(libcontemporary)
cntp_find_pkgconfig(imobiledevice libimobiledevice-1.0)
cntp_find_pkgconfig(plist libplist++-2.0)

set(SOURCES
    plugin.cpp
    idevicewatcher.cpp
    idevice.cpp
)

set(HEADERS
    plugin.h
    idevicewatcher.h
    idevice.h
)

add_library(plugin-idevice SHARED ${SOURCES} ${HEADERS})
target_include_directories(plugin-idevice PUBLIC ../../libthefile/)

cntp_init_plugin(thefile plugin-idevice 20 ideviceplugin)
cntp_translate(plugin-idevice)

set_target_properties(plugin-idevice PROPERTIES
        OUTPUT_NAME idevicePlugin
        FRAMEWORK FALSE)

target_link_libraries(plugin-idevice Qt6::Widgets Qt6::DBus libcontemporary libthefile PkgConfig::imobiledevice PkgConfig::plist)
