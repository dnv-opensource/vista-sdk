#==============================================================================
# dnv-vista-sdk - CMake Sources
#==============================================================================

set(dnv_vista_sdk_sources)

list(APPEND dnv_vista_sdk_sources
    ${DNV_VISTA_SDK_SOURCE_DIR}/Compression/Gzip.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/Compression/Inflate.cpp

    ${DNV_VISTA_SDK_SOURCE_DIR}/EmbeddedResources/EmbeddedResources.cpp

    ${DNV_VISTA_SDK_SOURCE_DIR}/JSON/Builder.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/JSON/Document.cpp

    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/core/VIS.cpp
)
