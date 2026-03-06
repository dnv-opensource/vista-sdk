import { Client } from "./Client";
import { Codebook } from "./Codebook";
import { CodebookName, CodebookNames } from "./CodebookName";
import { Codebooks } from "./Codebooks";
import { Gmod } from "./Gmod";
import { GmodNode } from "./GmodNode";
import { GmodIndividualizableSet, GmodPath } from "./GmodPath";
import {
    NodesBuilder as GmodPathNodesBuilder,
    PathBuilder as GmodPathPathBuilder,
    GmodPathQuery,
    GmodPathQueryBuilder,
} from "./GmodPathQuery";
import { GmodVersioning } from "./GmodVersioning";
import { ILocalId, ILocalIdGeneric } from "./ILocalId";
import { ILocalIdBuilder, ILocalIdBuilderGeneric } from "./ILocalIdBuilder";
import { ImoNumber } from "./ImoNumber";
import { LocalId } from "./LocalId";
import { LocalIdBuilder } from "./LocalId.Builder";
import { LocalIdParser } from "./LocalId.Parsing";
import {
    LocalIdQuery,
    LocalIdQueryBuilder,
    NodesQueryConfiguration,
    PathQueryConfiguration,
} from "./LocalIdQuery";
import { Location, LocationGroup, Locations } from "./Location";
import { LocationBuilder } from "./LocationBuilder";
import { MetadataTag } from "./MetadataTag";
import {
    MetadataTagsQuery,
    MetadataTagsQueryBuilder,
} from "./MetadataTagsQuery";
import { Pmod } from "./Pmod";
import { PmodNode } from "./PmodNode";
import { UniversalId } from "./UniversalId";
import { UniversalIdBuilder } from "./UniversalId.Builder";
import { UniversalIdParser } from "./UniversalId.Parsing";
import { VIS } from "./VIS";
import { VisVersion, VisVersionExtension, VisVersions } from "./VisVersion";
import { LocalIdParsingErrorBuilder } from "./internal/LocalIdParsingErrorBuilder";
import { parseVisVersion } from "./internal/Parsing";
import { LocalIdItems } from "./LocalId.Items";
import {
    DataChannelId,
    DataChannelList,
    ShipId,
    TimeSeries,
    Version,
} from "./transport/domain";
import {
    DataChannelListDto,
    JSONExtensions,
    TimeSeriesDto,
    VistaJSONSerializer,
} from "./transport/json";
import { isNullOrWhiteSpace } from "./util/util";
import { GmodNodeMetadata } from "./types/GmodNode";
import {
    ConversionType,
    GmodNodeConversion,
    GmodNodeConversionDto,
    GmodVersioningDto,
} from "./types/GmodVersioning";
import { ParsingState } from "./types/LocalId";
import { NotRelevant, PmodInfo } from "./types/Pmod";
import { Err, Ok, Result } from "./types/Result";
import { TreeNode } from "./types/Tree";

// Types
export type { GmodNodeMetadata, PmodInfo, TreeNode };
// VisVersion
export { VisVersion, VisVersionExtension, VisVersions };
// VIS
export { VIS };
// Codebooks and metadata
export {
    Codebook,
    CodebookName,
    CodebookNames,
    Codebooks,
    MetadataTag,
    MetadataTagsQuery,
    MetadataTagsQueryBuilder,
};
// LocalId
export {
    ILocalId,
    ILocalIdBuilder,
    ILocalIdBuilderGeneric,
    ILocalIdGeneric,
    ImoNumber,
    LocalId,
    LocalIdBuilder,
    LocalIdParser,
    LocalIdParsingErrorBuilder,
    LocalIdQuery,
    LocalIdQueryBuilder,
    ParsingState,
};
export type { NodesQueryConfiguration, PathQueryConfiguration };

// Locations
export { Location, LocationBuilder, LocationGroup, Locations };

// Experimental - moved to dnv-vista-sdk-experimental package

// UniversalId
export { UniversalId, UniversalIdBuilder, UniversalIdParser };

// Gmod
export {
    ConversionType,
    Gmod,
    GmodIndividualizableSet,
    GmodNode,
    GmodPath,
    GmodPathNodesBuilder,
    GmodPathPathBuilder,
    GmodPathQuery,
    GmodPathQueryBuilder,
    GmodVersioning,
};
export type { GmodNodeConversion, GmodNodeConversionDto, GmodVersioningDto };
// Pmod
export { NotRelevant, Pmod, PmodNode };
// Client
export { Client };

// General
export { Err, Ok, Result };

// Transport
export {
    DataChannelId,
    DataChannelList,
    DataChannelListDto,
    JSONExtensions,
    ShipId,
    TimeSeries,
    TimeSeriesDto,
    Version,
    VistaJSONSerializer,
};

// Internal utilities (exported for experimental SDK)
export { isNullOrWhiteSpace, LocalIdItems, parseVisVersion };
