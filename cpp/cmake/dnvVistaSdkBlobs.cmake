#==============================================================================
# dnv::vista::sdk::Blobs - Embed binary files as C++ byte arrays
#==============================================================================

#----------------------------------------------
# Blob generator tool
#----------------------------------------------

if(NOT TARGET dnv-vista-sdk-blobgen)
    if(CMAKE_CROSSCOMPILING AND DNV_VISTA_SDK_BLOBGEN_HOST_PATH)
        add_executable(dnv-vista-sdk-blobgen IMPORTED GLOBAL)
        set_target_properties(dnv-vista-sdk-blobgen
            PROPERTIES IMPORTED_LOCATION "${DNV_VISTA_SDK_BLOBGEN_HOST_PATH}"
        )
        add_executable(dnv::vista::sdk::blobgen ALIAS dnv-vista-sdk-blobgen)
    else()
        add_executable(dnv-vista-sdk-blobgen
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/BlobGen.cpp
        )
        add_executable(dnv::vista::sdk::blobgen ALIAS dnv-vista-sdk-blobgen)

        set_target_properties(dnv-vista-sdk-blobgen
            PROPERTIES
                CXX_STANDARD          20
                CXX_STANDARD_REQUIRED ON
                CXX_EXTENSIONS        OFF
                DEBUG_POSTFIX         "-d"
        )
    endif()
endif()

#----------------------------------------------
# CMake helper function
#----------------------------------------------

function(dnv_vista_sdk_embed_blobs)
    cmake_parse_arguments(
        PARSE_ARGV 0
        ARG
        "RECURSE"
        "TARGET;RESOURCE_DIR;OUTPUT_DIR;NAMESPACE;REGISTRY_NAME"
        "PATTERN"
    )

    if(NOT ARG_TARGET)
        message(FATAL_ERROR "dnv_vista_sdk_embed_blobs: TARGET is required")
    endif()
    if(NOT ARG_RESOURCE_DIR)
        message(FATAL_ERROR "dnv_vista_sdk_embed_blobs: RESOURCE_DIR is required")
    endif()
    if(NOT ARG_OUTPUT_DIR)
        message(FATAL_ERROR "dnv_vista_sdk_embed_blobs: OUTPUT_DIR is required")
    endif()
    if(NOT ARG_NAMESPACE)
        message(FATAL_ERROR "dnv_vista_sdk_embed_blobs: NAMESPACE is required")
    endif()
    if(NOT ARG_REGISTRY_NAME)
        message(FATAL_ERROR "dnv_vista_sdk_embed_blobs: REGISTRY_NAME is required")
    endif()

    set(FILE_PATTERN "${ARG_PATTERN}")
    if(NOT FILE_PATTERN)
        set(FILE_PATTERN "*")
    endif()

    file(MAKE_DIRECTORY "${ARG_OUTPUT_DIR}")

    set(GLOB_PATTERNS "")
    foreach(PATTERN ${FILE_PATTERN})
        list(APPEND GLOB_PATTERNS "${ARG_RESOURCE_DIR}/${PATTERN}")
    endforeach()

    if(ARG_RECURSE)
        file(GLOB_RECURSE RESOURCE_FILES CONFIGURE_DEPENDS ${GLOB_PATTERNS})
    else()
        file(GLOB RESOURCE_FILES CONFIGURE_DEPENDS ${GLOB_PATTERNS})
    endif()

    if(NOT RESOURCE_FILES)
        message(WARNING "dnv_vista_sdk_embed_blobs: No files found in ${ARG_RESOURCE_DIR} matching: ${FILE_PATTERN}")
    endif()

    list(REMOVE_DUPLICATES RESOURCE_FILES)
    list(SORT RESOURCE_FILES COMPARE NATURAL)

    set(ALL_FILES "")
    set(ALL_IDS   "")

    foreach(RESOURCE_FILE ${RESOURCE_FILES})
        if(ARG_RECURSE)
            file(RELATIVE_PATH RESOURCE_NAME "${ARG_RESOURCE_DIR}" "${RESOURCE_FILE}")
            string(REPLACE "/" "_" RESOURCE_ID_BASE "${RESOURCE_NAME}")
            string(MAKE_C_IDENTIFIER "${RESOURCE_ID_BASE}" RESOURCE_ID)
        else()
            get_filename_component(RESOURCE_NAME "${RESOURCE_FILE}" NAME)
            string(MAKE_C_IDENTIFIER "${RESOURCE_NAME}" RESOURCE_ID)
        endif()

        set(OUTPUT_CPP "${ARG_OUTPUT_DIR}/blob_${RESOURCE_ID}.cpp")

        list(FIND ALL_IDS "${RESOURCE_ID}" EXISTING_ID_INDEX)
        if(NOT EXISTING_ID_INDEX EQUAL -1)
            message(FATAL_ERROR
                "dnv_vista_sdk_embed_blobs: identifier collision for '${RESOURCE_ID}' (resource '${RESOURCE_NAME}'). "
                "Rename one resource or adjust naming inputs.")
        endif()

        add_custom_command(
            OUTPUT  "${OUTPUT_CPP}"
            COMMAND $<TARGET_FILE:dnv::vista::sdk::blobgen>
                    "${RESOURCE_FILE}"
                    "${OUTPUT_CPP}"
                    "${ARG_NAMESPACE}"
                    "${RESOURCE_NAME}"
                    "${RESOURCE_ID}"
            DEPENDS "${RESOURCE_FILE}" dnv::vista::sdk::blobgen
            COMMENT "Embedding: ${RESOURCE_NAME}"
            VERBATIM
        )

        list(APPEND ALL_FILES "${OUTPUT_CPP}")
        list(APPEND ALL_IDS   "${RESOURCE_ID}")
    endforeach()

    set(REGISTRY_CPP "${ARG_OUTPUT_DIR}/${ARG_REGISTRY_NAME}.cpp")
    set(REGISTRY_H   "${ARG_OUTPUT_DIR}/${ARG_REGISTRY_NAME}.h")
    set(REGISTRY_CPP_TMP "${REGISTRY_CPP}.tmp")
    set(REGISTRY_H_TMP   "${REGISTRY_H}.tmp")

    file(WRITE "${REGISTRY_H_TMP}"
"// Auto-generated resource registry header
#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

#ifndef DNV_VISTA_SDK_BLOB_DEFINED
#define DNV_VISTA_SDK_BLOB_DEFINED
namespace dnv::vista::sdk
{
    struct Blob
    {
        std::string_view name;
        const uint8_t* data;
        std::size_t size;

        [[nodiscard]] std::string_view str() const noexcept
        {
            return { reinterpret_cast<const char*>(data), size };
        }
    };
}
#endif

namespace ${ARG_NAMESPACE}
{
    const std::vector<dnv::vista::sdk::Blob>& all();

    [[nodiscard]] inline const dnv::vista::sdk::Blob* find(std::string_view name) noexcept
    {
        const auto& res = all();
        auto it = std::find_if(res.begin(), res.end(),
            [name](const dnv::vista::sdk::Blob& r) { return r.name == name; });
        return it != res.end() ? &(*it) : nullptr;
    }
} // namespace ${ARG_NAMESPACE}
")

    file(WRITE "${REGISTRY_CPP_TMP}"
"// Auto-generated resource registry implementation
#include \"${ARG_REGISTRY_NAME}.h\"

namespace ${ARG_NAMESPACE}
{
")

    foreach(RESOURCE_ID IN LISTS ALL_IDS)
        file(APPEND "${REGISTRY_CPP_TMP}" "    extern const uint8_t ${RESOURCE_ID}_data[];\n")
        file(APPEND "${REGISTRY_CPP_TMP}" "    extern const size_t ${RESOURCE_ID}_size;\n")
        file(APPEND "${REGISTRY_CPP_TMP}" "    extern const char ${RESOURCE_ID}_name[];\n")
    endforeach()

    file(APPEND "${REGISTRY_CPP_TMP}"
"
    const std::vector<dnv::vista::sdk::Blob>& all()
    {
        static const std::vector<dnv::vista::sdk::Blob> resources = {
")

    foreach(RESOURCE_ID IN LISTS ALL_IDS)
        file(APPEND "${REGISTRY_CPP_TMP}" "            { ${RESOURCE_ID}_name, ${RESOURCE_ID}_data, ${RESOURCE_ID}_size },\n")
    endforeach()

    file(APPEND "${REGISTRY_CPP_TMP}"
"        };
        return resources;
    }

} // namespace ${ARG_NAMESPACE}
")

    configure_file("${REGISTRY_H_TMP}" "${REGISTRY_H}" COPYONLY)
    configure_file("${REGISTRY_CPP_TMP}" "${REGISTRY_CPP}" COPYONLY)

    list(APPEND ALL_FILES "${REGISTRY_CPP}")
    set_source_files_properties(${ALL_FILES} PROPERTIES GENERATED TRUE)

    if(NOT TARGET ${ARG_TARGET})
        add_library(${ARG_TARGET} OBJECT ${ALL_FILES})

        target_include_directories(${ARG_TARGET}
            PUBLIC $<BUILD_INTERFACE:${ARG_OUTPUT_DIR}>
        )

        set_target_properties(${ARG_TARGET}
            PROPERTIES
                CXX_STANDARD          20
                CXX_STANDARD_REQUIRED ON
                CXX_EXTENSIONS        OFF
                POSITION_INDEPENDENT_CODE ON
        )
    else()
        target_sources(${ARG_TARGET} PRIVATE ${ALL_FILES})
        target_include_directories(${ARG_TARGET} PUBLIC $<BUILD_INTERFACE:${ARG_OUTPUT_DIR}>)
    endif()

    list(LENGTH ALL_IDS NUM_RESOURCES)
    message(STATUS "[dnv-vista-sdk] Embedded ${NUM_RESOURCES} resource(s) into ${ARG_TARGET}")
endfunction()
