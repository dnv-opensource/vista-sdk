#==============================================================================
# dnv::vista::sdk - Samples
#==============================================================================

if(NOT EMSCRIPTEN)
    add_subdirectory(core)
    add_subdirectory(query)
    add_subdirectory(transport)
    add_subdirectory(types)
    add_subdirectory(utils)
endif()

add_subdirectory(wasm)
