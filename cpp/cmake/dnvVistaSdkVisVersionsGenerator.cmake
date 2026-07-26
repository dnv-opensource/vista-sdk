#==============================================================================
# dnv::vista::sdk::VisVersionsGenerator
#==============================================================================

if(CMAKE_CROSSCOMPILING AND DNV_VISTA_SDK_VISVERSIONSGENERATOR_HOST_PATH)
    add_executable(dnv-vista-sdk-visversionsgenerator IMPORTED GLOBAL)
    set_target_properties(dnv-vista-sdk-visversionsgenerator
        PROPERTIES IMPORTED_LOCATION "${DNV_VISTA_SDK_VISVERSIONSGENERATOR_HOST_PATH}"
    )
else()
    add_executable(dnv-vista-sdk-visversionsgenerator
        ${DNV_VISTA_SDK_SOURCE_DIR}/SourceGenerators/VisVersionsGenerator.cpp
    )
    set_target_properties(dnv-vista-sdk-visversionsgenerator
        PROPERTIES
            CXX_STANDARD          20
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS        OFF
            DEBUG_POSTFIX         "-d"
    )
endif()
add_executable(dnv::vista::sdk::visversionsgenerator ALIAS dnv-vista-sdk-visversionsgenerator)

set(DNV_VISTA_SDK_GENERATED_HEADER     "${DNV_VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/core/VisVersions.h")
set(DNV_VISTA_SDK_GENERATED_EXTENSIONS "${DNV_VISTA_SDK_SOURCE_DIR}/SDK/core/VisVersionsExtensions.h")

add_custom_command(
    OUTPUT "${DNV_VISTA_SDK_GENERATED_HEADER}" "${DNV_VISTA_SDK_GENERATED_EXTENSIONS}"
    COMMAND $<TARGET_FILE:dnv-vista-sdk-visversionsgenerator> "${DNV_VISTA_SDK_GENERATED_HEADER}" "${DNV_VISTA_SDK_GENERATED_EXTENSIONS}" "${DNV_VISTA_SDK_VERSION}"
    COMMAND clang-format -i "${DNV_VISTA_SDK_GENERATED_HEADER}" || ${CMAKE_COMMAND} -E true
    COMMAND clang-format -i "${DNV_VISTA_SDK_GENERATED_EXTENSIONS}" || ${CMAKE_COMMAND} -E true
    DEPENDS dnv-vista-sdk-visversionsgenerator
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Generating VisVersions.h and VisVersionsExtensions.h from embedded resources..."
    VERBATIM
)

add_custom_target(dnv-vista-sdk-generate-visversions ALL
    DEPENDS "${DNV_VISTA_SDK_GENERATED_HEADER}" "${DNV_VISTA_SDK_GENERATED_EXTENSIONS}"
)
