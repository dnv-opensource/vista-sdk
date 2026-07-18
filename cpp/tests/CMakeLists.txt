#==============================================================================
# dnv::vista::sdk - Tests
#==============================================================================

set(FETCHCONTENT_UPDATES_DISCONNECTED ON  CACHE BOOL "")
set(FETCHCONTENT_QUIET                OFF CACHE BOOL "")

include(FetchContent)

FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG        v2.5.3
    GIT_SHALLOW    TRUE
)

set(_dnv_vista_sdk_saved_build_shared_libs ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF)
FetchContent_MakeAvailable(doctest)
set(BUILD_SHARED_LIBS ${_dnv_vista_sdk_saved_build_shared_libs})
unset(_dnv_vista_sdk_saved_build_shared_libs)

list(APPEND CMAKE_MODULE_PATH "${doctest_SOURCE_DIR}/scripts/cmake")

include(doctest)

#----------------------------------------------
# Embed test data
#----------------------------------------------

dnv_vista_sdk_embed_blobs(
    TARGET          dnv-vista-sdk-testdata
    RESOURCE_DIR    "${VISTA_SDK_ROOT}/testdata"
    OUTPUT_DIR      "${CMAKE_BINARY_DIR}/blobs/testdata"
    NAMESPACE       dnv::vista::sdk::testdata
    REGISTRY_NAME   TestdataRegistry
    PATTERN         "*.json" "*.txt"
)

dnv_vista_sdk_embed_blobs(
    TARGET          dnv-vista-sdk-testdata
    RESOURCE_DIR    "${CMAKE_CURRENT_SOURCE_DIR}/transport/_files"
    OUTPUT_DIR      "${CMAKE_BINARY_DIR}/blobs/testdata"
    NAMESPACE       dnv::vista::sdk::testdata::additional
    REGISTRY_NAME   TestdataAdditionalRegistry
    PATTERN         "*.json"
)

target_sources(dnv-vista-sdk-testdata
    PRIVATE
        EmbeddedTestData/EmbeddedTestData.cpp
)

target_include_directories(dnv-vista-sdk-testdata
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/EmbeddedTestData
)

add_library(dnv::vista::sdk::testdata ALIAS dnv-vista-sdk-testdata)

#----------------------------------------------
# Tests
#----------------------------------------------

add_subdirectory(blobs)
add_subdirectory(codegeneration)
add_subdirectory(compression)
add_subdirectory(containers)
add_subdirectory(core)
add_subdirectory(json)
add_subdirectory(query)
add_subdirectory(transport)
add_subdirectory(types)
