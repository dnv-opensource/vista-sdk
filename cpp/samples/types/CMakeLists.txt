#==============================================================================
# dnv::vista::sdk - Types samples
#==============================================================================

file(GLOB_RECURSE DNV_VISTA_SDK_TYPES_SAMPLES_SOURCES CONFIGURE_DEPENDS "*.cpp")

foreach(source IN LISTS DNV_VISTA_SDK_TYPES_SAMPLES_SOURCES)
    cmake_path(RELATIVE_PATH source BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE source_rel)
    string(REPLACE "/" "-" source_id "${source_rel}")
    string(REPLACE ".cpp" "" source_id "${source_id}")
    set(target "dnv-vista-sdk-sample-type-${source_id}")

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
            dnv::vista::sdk
            dnv::vista::sdk::warnings
    )
endforeach()
