cmake_minimum_required(VERSION 3.4.0)

project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)
find_package(libcontemporary REQUIRED)
find_package(libthefrisbee REQUIRED)
find_package(libtdesktopenvironment)

set(SOURCES
    bookmarkmanager.cpp
    directories/localfilesystemdirectory.cpp
    directories/trashdirectory.cpp
    directory.cpp
    directoryHandlers/localfiledirectoryhandler.cpp
    directoryHandlers/trashdirectoryhandler.cpp
    directoryhandler.cpp
    filecolumn.cpp
    filecolumnaction.cpp
    filecolumnfloater.cpp
    filecolumnmanager.cpp
    filemodel.cpp
    filetab.cpp
    hiddenfilesproxymodel.cpp
    popovers/unlockencryptedpopover.cpp
    resourcemanager.cpp
    sidebar/bookmarksmodel.cpp
    sidebar/devicesmodel.cpp
    sidebar/sidebar.cpp
    sidebar/sidebarsection.cpp
    sidebar/sections/placessidebarsection.cpp
    sidebarmanager.cpp
    sidebar/sidebarsectionfactory.cpp
    sidebar/sections/standardsidebarsectionfactory.cpp
    sidebar/sections/drivessidebarsection.cpp
    sidebar/sections/bookmarkssidebarsection.cpp

    filecolumn.ui
    filecolumnaction.ui
    filecolumnfloater.ui
    filetab.ui
    popovers/unlockencryptedpopover.ui
    sidebar/sidebar.ui
)

set(HEADERS
    bookmarkmanager.h
    directories/localfilesystemdirectory.h
    directories/trashdirectory.h
    directory.h
    directoryHandlers/localfiledirectoryhandler.h
    directoryHandlers/trashdirectoryhandler.h
    directoryhandler.h
    filecolumn.h
    filecolumnaction.h
    filecolumnfloater.h
    filecolumnmanager.h
    filemodel.h
    filetab.h
    hiddenfilesproxymodel.h
    libthefile_global.h
    popovers/unlockencryptedpopover.h
    resourcemanager.h
    sidebar/bookmarksmodel.h
    sidebar/devicesmodel.h
    sidebar/sidebar.h
    thefileplugininterface.h
    sidebar/sidebarsection.h
    sidebar/sections/placessidebarsection.h
    sidebarmanager.h
    sidebar/sidebarsectionfactory.h
    sidebar/sections/standardsidebarsectionfactory.h
    sidebar/sections/drivessidebarsection.h
    sidebar/sections/bookmarkssidebarsection.h
)

add_library(libthefile SHARED ${SOURCES} ${HEADERS})
set_target_properties(libthefile PROPERTIES
        OUTPUT_NAME thefile
        FRAMEWORK TRUE
        MACOSX_FRAMEWORK_IDENTIFIER com.vicr123.libthefile
        VERSION 1.0.0
        PUBLIC_HEADER "${HEADERS}"
        CNTP_DATA_SUBDIR thefile/libthefile)
cntp_init(libthefile 20)
cntp_translate(libthefile)

target_link_libraries(libthefile Qt6::Widgets Qt6::DBus Qt6::Multimedia libcontemporary libthefrisbee)
target_compile_definitions(libthefile PRIVATE LIBTHEFILE_LIBRARY)

IF(libtdesktopenvironment_FOUND)
target_link_libraries(libthefile libtdesktopenvironment)
target_compile_definitions(libthefile PUBLIC HAVE_TDESKTOPENVIRONMENT)
ENDIF()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKECONFIG_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/libthefile.framework/Resources/CMake)
    set(HEADER_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
    set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/libthefile.framework/Headers)
    set(LIBRARY_INSTALL_DIR ../)
ELSE()
    set(CMAKECONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/libthefile)
    set(HEADER_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/libthefile)
    set(INCLUDE_INSTALL_DIR ${HEADER_INSTALL_DIR})
    set(LIBRARY_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
ENDIF()

configure_package_config_file(libthefileConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/libthefileConfig.cmake
        INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
        PATH_VARS HEADER_INSTALL_DIR LIBRARY_INSTALL_DIR)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libthefileConfig.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})
install(EXPORT libthefileTargets
        FILE libthefileTargets.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})

install(TARGETS libthefile
        EXPORT libthefileTargets
        PUBLIC_HEADER DESTINATION ${HEADER_INSTALL_DIR}
        INCLUDES DESTINATION ${INCLUDE_INSTALL_DIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR})

cntp_mark_support_library_directory()
