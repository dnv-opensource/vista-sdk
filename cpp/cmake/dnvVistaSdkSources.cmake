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

    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/core/internal/parsing/LocalIdParsingErrorBuilder.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/core/internal/parsing/LocationParsingErrorBuilder.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/core/internal/versioning/GmodVersioning.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/core/Codebook.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/core/Codebooks.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/core/Gmod.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/core/GmodNode.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/core/GmodPath.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/core/ImoNumber.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/core/LocalId.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/core/LocalIdBuilder.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/core/Locations.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/core/LocationBuilder.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/core/ParsingErrors.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/core/UniversalId.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/core/UniversalIdBuilder.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/core/VIS.cpp

    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/query/GmodPathQuery.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/query/LocalIdQuery.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/query/MetadataTagsQuery.cpp

    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/transport/serialization/json/SerializableDocument.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/transport/ISO19848.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/transport/ShipId.cpp

    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/types/datetime/DateTime.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/types/datetime/DateTimeOffset.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/types/datetime/TimeSpan.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/types/decimal/Decimal.cpp
    ${DNV_VISTA_SDK_SOURCE_DIR}/SDK/types/decimal/Int128.cpp
)
