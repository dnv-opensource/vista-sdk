#==============================================================================
# dnv-vista-sdk - Version configuration
#==============================================================================
#
# Exposes the project version as cache variables consumable outside this
# CMakeLists.txt tree, and generates:
#   - <build>/include/dnv/vista/sdk/Version.h (runtime/compile-time queryable version)
#   - <build>/dnvVistaSdk.rc                  (Windows VERSIONINFO resource, WIN32 only)

set(DNV_VISTA_SDK_VERSION_MAJOR ${PROJECT_VERSION_MAJOR} CACHE INTERNAL "dnv-vista-sdk major version")
set(DNV_VISTA_SDK_VERSION_MINOR ${PROJECT_VERSION_MINOR} CACHE INTERNAL "dnv-vista-sdk minor version")
set(DNV_VISTA_SDK_VERSION_PATCH ${PROJECT_VERSION_PATCH} CACHE INTERNAL "dnv-vista-sdk patch version")
set(DNV_VISTA_SDK_VERSION       ${PROJECT_VERSION}       CACHE INTERNAL "dnv-vista-sdk version")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/dnv/vista/sdk/Version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/dnv/vista/sdk/Version.h"
    @ONLY
)

if(WIN32)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dnvVistaSdk.rc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/dnvVistaSdk.rc"
        @ONLY
    )
endif()
