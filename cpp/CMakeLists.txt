#==============================================================================
# dnv::vista::sdk - Top-level CMake configuration
#==============================================================================

#----------------------------------------------
# Project definition
#----------------------------------------------

cmake_minimum_required(VERSION 3.25)

project(dnv-vista-sdk
    VERSION 0.3.3
    DESCRIPTION "C++ implementation of DNV Vista SDK for VIS and ISO 19848 standard"
    HOMEPAGE_URL "https://github.com/dnv-opensource/vista-sdk"
    LANGUAGES CXX
)

#----------------------------------------------
# C++ standard
#----------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_DEBUG_POSTFIX "-d")

#----------------------------------------------
# Directory paths
#----------------------------------------------

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH VISTA_SDK_ROOT)

set(DNV_VISTA_SDK_DIR         "${PROJECT_SOURCE_DIR}"    CACHE PATH "C++ SDK root directory")
set(DNV_VISTA_SDK_SOURCE_DIR  "${DNV_VISTA_SDK_DIR}/src" CACHE PATH "C++ source directory")

#----------------------------------------------
# Build type
#----------------------------------------------

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (Debug/Release)" FORCE)
endif()

#----------------------------------------------
# Build options
#----------------------------------------------

option(DNV_VISTA_SDK_BUILD_TESTS "Build dnv::vista::sdk tests"     OFF)
option(DNV_VISTA_SDK_WARNINGS    "Enable strict compiler warnings" OFF)

#----------------------------------------------
# CMake modules configuration
#----------------------------------------------

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(dnvVistaSdkBlobs)
include(dnvVistaSdkTargets)

#----------------------------------------------
# Tests
#----------------------------------------------

if(DNV_VISTA_SDK_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
