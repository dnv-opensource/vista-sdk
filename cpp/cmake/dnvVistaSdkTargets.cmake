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
