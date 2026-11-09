#==============================================================================
# dnv::vista::sdk - Benchmarks
#==============================================================================

include(FetchContent)

FetchContent_Declare(
    nanobench
    GIT_REPOSITORY https://github.com/martinus/nanobench.git
    GIT_TAG        v4.3.11
    GIT_SHALLOW    TRUE
)

set(_dnv_vista_sdk_saved_build_shared_libs ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF)
FetchContent_MakeAvailable(nanobench)
set(BUILD_SHARED_LIBS ${_dnv_vista_sdk_saved_build_shared_libs})
unset(_dnv_vista_sdk_saved_build_shared_libs)

#----------------------------------------------
# SSE4.2 compiler support probe (GCC/Clang)
#----------------------------------------------

if(NOT MSVC)
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(-msse4.2 COMPILER_SUPPORTS_SSE42)
endif()

#----------------------------------------------
# AVX2 host support probe (MSVC)
#----------------------------------------------

if(MSVC)
    set(DNV_VISTA_SDK_AVX2_PROBE_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/avx2_probe.cpp")
    file(WRITE "${DNV_VISTA_SDK_AVX2_PROBE_SOURCE}"
"#include <immintrin.h>
#include <intrin.h>
int main()
{
    int info[4] = { 0, 0, 0, 0 };
    __cpuidex(info, 7, 0);
    if (!(info[1] & (1 << 5))) // AVX2 bit
        return 1;

    __m256i v = _mm256_set1_epi32(1);
    v = _mm256_add_epi32(v, v);
    return _mm256_extract_epi32(v, 0) == 2 ? 0 : 1;
}
")

    try_run(
        DNV_VISTA_SDK_AVX2_RUN_RESULT
        DNV_VISTA_SDK_AVX2_COMPILE_RESULT
        "${CMAKE_CURRENT_BINARY_DIR}/avx2_probe"
        "${DNV_VISTA_SDK_AVX2_PROBE_SOURCE}"
        COMPILE_DEFINITIONS "/arch:AVX2"
    )

    if(DNV_VISTA_SDK_AVX2_COMPILE_RESULT AND DNV_VISTA_SDK_AVX2_RUN_RESULT EQUAL 0)
        set(DNV_VISTA_SDK_HOST_SUPPORTS_AVX2 ON)
    else()
        set(DNV_VISTA_SDK_HOST_SUPPORTS_AVX2 OFF)
        message(STATUS "[dnv-vista-sdk] Host does not support AVX2 (or probe failed); building benchmarks without /arch:AVX2")
    endif()
endif()

file(GLOB_RECURSE DNV_VISTA_SDK_NANOBENCH_SOURCES CONFIGURE_DEPENDS "*.cpp")

foreach(source IN LISTS DNV_VISTA_SDK_NANOBENCH_SOURCES)
    cmake_path(RELATIVE_PATH source BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE source_rel)
    string(REPLACE "/" "-" source_id "${source_rel}")
    string(REPLACE ".cpp" "" source_id "${source_id}")
    set(target "dnv-vista-sdk-benchmark-${source_id}")

    add_executable(${target} ${source})

    set_target_properties(${target}
        PROPERTIES
            CXX_STANDARD          20
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS        OFF
            DEBUG_POSTFIX         "-d"
    )

    target_link_libraries(${target}
        PRIVATE
            nanobench::nanobench
            dnv::vista::sdk
            dnv::vista::sdk::schemas
            dnv::vista::sdk::warnings
    )

    if(COMPILER_SUPPORTS_SSE42)
        target_compile_options(${target} PRIVATE -march=native)
    endif()
    if(MSVC AND DNV_VISTA_SDK_HOST_SUPPORTS_AVX2)
        target_compile_options(${target} PRIVATE /arch:AVX2)
    endif()
endforeach()
