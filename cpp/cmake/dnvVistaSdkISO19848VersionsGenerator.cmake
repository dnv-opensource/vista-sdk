#==============================================================================
# dnv::vista::sdk::ISO19848VersionsGenerator
#==============================================================================

if(CMAKE_CROSSCOMPILING AND DNV_VISTA_SDK_ISO19848VERSIONSGENERATOR_HOST_PATH)
    add_executable(dnv-vista-sdk-iso19848versionsgenerator IMPORTED GLOBAL)
    set_target_properties(dnv-vista-sdk-iso19848versionsgenerator
        PROPERTIES IMPORTED_LOCATION "${DNV_VISTA_SDK_ISO19848VERSIONSGENERATOR_HOST_PATH}"
    )
else()
    add_executable(dnv-vista-sdk-iso19848versionsgenerator
        ${DNV_VISTA_SDK_SOURCE_DIR}/SourceGenerators/ISO19848VersionsGenerator.cpp
    )
    set_target_properties(dnv-vista-sdk-iso19848versionsgenerator
        PROPERTIES
            CXX_STANDARD          20
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS        OFF
            DEBUG_POSTFIX         "-d"
    )
endif()
add_executable(dnv::vista::sdk::iso19848versionsgenerator ALIAS dnv-vista-sdk-iso19848versionsgenerator)

set(DNV_VISTA_SDK_ISO19848_GENERATED_HEADER     "${DNV_VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/transport/ISO19848Versions.h")
set(DNV_VISTA_SDK_ISO19848_GENERATED_EXTENSIONS "${DNV_VISTA_SDK_SOURCE_DIR}/SDK/transport/ISO19848VersionsExtensions.h")

add_custom_command(
    OUTPUT "${DNV_VISTA_SDK_ISO19848_GENERATED_HEADER}" "${DNV_VISTA_SDK_ISO19848_GENERATED_EXTENSIONS}"
    COMMAND $<TARGET_FILE:dnv-vista-sdk-iso19848versionsgenerator> "${DNV_VISTA_SDK_ISO19848_GENERATED_HEADER}" "${DNV_VISTA_SDK_ISO19848_GENERATED_EXTENSIONS}" "${DNV_VISTA_SDK_VERSION}"
    COMMAND clang-format -i "${DNV_VISTA_SDK_ISO19848_GENERATED_HEADER}" || ${CMAKE_COMMAND} -E true
    COMMAND clang-format -i "${DNV_VISTA_SDK_ISO19848_GENERATED_EXTENSIONS}" || ${CMAKE_COMMAND} -E true
    DEPENDS dnv-vista-sdk-iso19848versionsgenerator
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Generating ISO19848Versions.h and ISO19848VersionsExtensions.h from embedded resources..."
    VERBATIM
)

add_custom_target(dnv-vista-sdk-generate-iso19848versions ALL
    DEPENDS "${DNV_VISTA_SDK_ISO19848_GENERATED_HEADER}" "${DNV_VISTA_SDK_ISO19848_GENERATED_EXTENSIONS}"
)
