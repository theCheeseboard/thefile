cmake_minimum_required(VERSION 3.21.0)

project(thefile VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets DBus Sql Concurrent)
find_package(libcontemporary REQUIRED)

set(SOURCES
    jobs/burnjob.cpp
    jobs/filetransferjob.cpp
    jobs/widgets/burnjobprogress.cpp
    jobs/widgets/filetransferjobwidget.cpp
    main.cpp
    mainwindow.cpp
    popovers/burnpopover.cpp
    popovers/deletepermanentlypopover.cpp
    popovers/itempropertiespopover.cpp
    tabbutton.cpp

    jobs/widgets/burnjobprogress.ui
    jobs/widgets/filetransferjobwidget.ui
    mainwindow.ui
    popovers/burnpopover.ui
    popovers/deletepermanentlypopover.ui
    popovers/itempropertiespopover.ui

    resources.qrc
)

set(HEADERS
    jobs/burnjob.h
    jobs/filetransferjob.h
    jobs/widgets/burnjobprogress.h
    jobs/widgets/filetransferjobwidget.h
    mainwindow.h
    popovers/burnpopover.h
    popovers/deletepermanentlypopover.h
    popovers/itempropertiespopover.h
    tabbutton.h
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(SOURCES ${SOURCES}
            platformintegration/winplatformintegration.cpp)

    set(HEADERS ${HEADERS}
            platformintegration/winplatformintegration.h)
ENDIF()

add_executable(thefile ${SOURCES} ${HEADERS})

set_target_properties(thefile PROPERTIES
        MACOSX_BUNDLE TRUE
#        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
        MACOSX_BUNDLE_GUI_IDENTIFIER com.vicr123.thefile
        WIN32_EXECUTABLE TRUE
        CNTP_DATA_SUBDIR thefile)

cntp_init(thefile 17)
cntp_translate(thefile)
cntp_target_name(thefile "theFile")
cntp_app_icon(thefile
    BASEICON icons/baseicon.svg
#    BASEICON_MAC icons/baseicon-mac.svg
)
cntp_target_desktop_file(thefile
    DESKTOP_FILE com.vicr123.thefile.desktop
    BLUEPRINT_DESKTOP_FILE com.vicr123.thefile_blueprint.desktop)
cntp_defaults_file(thefile
    DEFAULTS_FILE thefile.conf)

target_link_libraries(thefile Qt6::Widgets Qt6::DBus Qt6::Concurrent libcontemporary libthefile)
target_include_directories(thefile PUBLIC ../libthefile/)

install(TARGETS thefile
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION /Applications)
