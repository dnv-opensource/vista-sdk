#==============================================================================
# dnv-vista-sdk - install() / find_package() support
#==============================================================================

#----------------------------------------------
# Install
#----------------------------------------------

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(
    TARGETS
        dnv-vista-sdk
        dnv-vista-sdk-resources
        dnv-vista-sdk-schemas
        dnv-vista-sdk-warnings
    EXPORT
        dnv-vista-sdkTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    DIRECTORY   "${DNV_VISTA_SDK_INCLUDE_DIR}/"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/include/dnv/vista/sdk/Version.h"
        "${CMAKE_CURRENT_BINARY_DIR}/include/dnv/vista/sdk/Export.h"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dnv/vista/sdk
)

install(
    EXPORT      dnv-vista-sdkTargets
    NAMESPACE   dnv::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dnv-vista-sdk
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/dnv-vista-sdkConfigVersion.cmake"
    VERSION       ${PROJECT_VERSION}
    COMPATIBILITY SameMinorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dnv-vista-sdkConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/dnv-vista-sdkConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dnv-vista-sdk
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/dnv-vista-sdkConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/dnv-vista-sdkConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dnv-vista-sdk
)

#----------------------------------------------
# Uninstall
#----------------------------------------------

if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dnvVistaSdkUninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/dnvVistaSdkUninstall.cmake"
        IMMEDIATE @ONLY
    )

    add_custom_target(
        uninstall
        COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/dnvVistaSdkUninstall.cmake"
        COMMENT "Uninstalling dnv-vista-sdk"
        VERBATIM
    )
endif()
