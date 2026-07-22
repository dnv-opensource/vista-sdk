/**
 * @file VistaSDK.h
 * @brief Umbrella header for the DNV Vista SDK
 * @details Single include for the complete Vista SDK public API: VIS core (Gmod, Codebooks,
 *          Locations, identifiers, queries) and ISO 19848 transport layer (DataChannelList,
 *          TimeSeriesData, JSON serialization)
 */

#pragma once

#include <dnv/vista/sdk/Version.h>

#include "vista/sdk/core/Codebook.h"
#include "vista/sdk/core/CodebookName.h"
#include "vista/sdk/core/Codebooks.h"
#include "vista/sdk/core/Gmod.h"
#include "vista/sdk/core/GmodNode.h"
#include "vista/sdk/core/GmodPath.h"
#include "vista/sdk/core/ImoNumber.h"
#include "vista/sdk/core/LocalId.h"
#include "vista/sdk/core/LocalIdBuilder.h"
#include "vista/sdk/core/LocationBuilder.h"
#include "vista/sdk/core/Locations.h"
#include "vista/sdk/core/MetadataTag.h"
#include "vista/sdk/core/ParsingErrors.h"
#include "vista/sdk/core/UniversalId.h"
#include "vista/sdk/core/UniversalIdBuilder.h"
#include "vista/sdk/core/VIS.h"

#include "vista/sdk/query/GmodPathQuery.h"
#include "vista/sdk/query/LocalIdQuery.h"
#include "vista/sdk/query/MetadataTagsQuery.h"

#include "vista/sdk/transport/serialization/json/datachannel/DataChannelListPackage.h"
#include "vista/sdk/transport/serialization/json/datachannel/Extensions.h"
#include "vista/sdk/transport/serialization/json/timeseries/TimeSeriesDataPackage.h"
#include "vista/sdk/transport/serialization/json/timeseries/Extensions.h"
#include "vista/sdk/transport/serialization/json/SerializableDocument.h"
#include "vista/sdk/transport/datachannel/DataChannel.h"
#include "vista/sdk/transport/timeseries/DataChannelId.h"
#include "vista/sdk/transport/timeseries/TimeSeriesData.h"
#include "vista/sdk/transport/ISO19848.h"
#include "vista/sdk/transport/Results.h"
#include "vista/sdk/transport/ShipId.h"
