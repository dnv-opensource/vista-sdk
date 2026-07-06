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
