#==============================================================================
# dnv-vista-sdk - CMake targets
#==============================================================================

#----------------------------------------------
# Warnings
#----------------------------------------------

add_library(dnv-vista-sdk-warnings INTERFACE)
add_library(dnv::vista::sdk::warnings ALIAS dnv-vista-sdk-warnings)

target_compile_options(dnv-vista-sdk-warnings
    INTERFACE
        $<$<AND:$<BOOL:${DNV_VISTA_SDK_WARNINGS}>,$<CXX_COMPILER_ID:GNU,Clang>>:-Wall -Wextra -Wpedantic>
        $<$<AND:$<BOOL:${DNV_VISTA_SDK_WARNINGS}>,$<CXX_COMPILER_ID:MSVC>>:/W4>
)

#----------------------------------------------
# Blobs
#----------------------------------------------

dnv_vista_sdk_embed_blobs(
    TARGET          dnv-vista-sdk-resources
    RESOURCE_DIR    "${VISTA_SDK_ROOT}/resources"
    OUTPUT_DIR      "${CMAKE_BINARY_DIR}/blobs/resources"
    NAMESPACE       dnv::vista::sdk::resources
    REGISTRY_NAME   ResourcesRegistry
    PATTERN         "*.json.gz"
)
add_library(dnv::vista::sdk::resources ALIAS dnv-vista-sdk-resources)

dnv_vista_sdk_embed_blobs(
    TARGET          dnv-vista-sdk-schemas
    RESOURCE_DIR    "${VISTA_SDK_ROOT}/schemas/json"
    OUTPUT_DIR      "${CMAKE_BINARY_DIR}/blobs/schemas"
    NAMESPACE       dnv::vista::sdk::schemas
    REGISTRY_NAME   SchemasRegistry
    PATTERN         "*.json"
)
add_library(dnv::vista::sdk::schemas ALIAS dnv-vista-sdk-schemas)

#----------------------------------------------
# Code generation
#----------------------------------------------

target_link_libraries(dnv-vista-sdk-visversionsgenerator
    PRIVATE
        dnv::vista::sdk::resources
        dnv::vista::sdk::warnings
)

target_link_libraries(dnv-vista-sdk-iso19848versionsgenerator
    PRIVATE
        dnv::vista::sdk::resources
        dnv::vista::sdk::warnings
)

#----------------------------------------------
# SDK
#----------------------------------------------

add_library(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD          20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS        OFF
        DEBUG_POSTFIX         "-d"
        EXPORT_NAME           vista::sdk
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

target_sources(${PROJECT_NAME}
    PRIVATE
        ${dnv_vista_sdk_sources}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        dnv::vista::sdk::resources
        dnv::vista::sdk::warnings
)

include(GenerateExportHeader)

generate_export_header(${PROJECT_NAME}
    EXPORT_MACRO_NAME     DNV_VISTA_SDK_CPP_API
    EXPORT_FILE_NAME      "${CMAKE_CURRENT_BINARY_DIR}/include/dnv/vista/sdk/Export.h"
)

if(BUILD_SHARED_LIBS)
    set_target_properties(${PROJECT_NAME}
        PROPERTIES
            CXX_VISIBILITY_PRESET        hidden
            VISIBILITY_INLINES_HIDDEN    ON
            VERSION                      ${PROJECT_VERSION}
            SOVERSION                    ${PROJECT_VERSION_MAJOR}
    )

    if(WIN32)
        target_sources(${PROJECT_NAME}
            PRIVATE
                "${CMAKE_CURRENT_BINARY_DIR}/dnvVistaSdk.rc"
        )
    endif()
endif()

add_library(dnv::vista::sdk ALIAS dnv-vista-sdk)

add_dependencies(${PROJECT_NAME} dnv-vista-sdk-generate-visversions)
add_dependencies(${PROJECT_NAME} dnv-vista-sdk-generate-iso19848versions)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${DNV_VISTA_SDK_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${DNV_VISTA_SDK_SOURCE_DIR}
)
