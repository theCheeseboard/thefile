set(BUILD_PLUGIN_IDEVICE ON CACHE BOOL "Build iDevice plugin")
set(BUILD_PLUGIN_QNEARBYSHARE ON CACHE BOOL "Build QNearbyShare plugin")

if(BUILD_PLUGIN_IDEVICE)
    add_subdirectory(idevice)
endif()

if(BUILD_PLUGIN_QNEARBYSHARE)
add_subdirectory(qnearbyshare)
endif()
