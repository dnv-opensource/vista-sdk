/**
 * @file DataChannelListPackage.h
 * @brief JSON DTO types for ISO 19848 DataChannelList messages
 * @details Flat, JSON-serializable structs mirroring the ISO 19848 JSON schema
 *          Custom extension points (xs:any) are represented as SerializableDocument,
 *          allowing the caller to use any JSON library via the CPO adapter
 */

#pragma once

#include "dnv/vista/sdk/transport/serialization/json/SerializableDocument.h"

#include <optional>
#include <string>
#include <vector>

namespace dnv::vista::sdk::transport::serialization::json::datachannel
{
    /**
     * @brief XSD restriction facets for a data channel value type
     * @details Maps directly to the ISO 19848 `Restriction` schema element
     *          All facets are optional. Only those present in the message are set
     */
    struct RestrictionDto
    {
        std::optional<std::vector<std::string>> enumeration;
        std::optional<std::uint32_t> fractionDigits; ///< Maximum number of fractional digits
        std::optional<std::uint32_t> length;         ///< Exact string length
        std::optional<double> maxExclusive;          ///< Exclusive upper bound
        std::optional<double> maxInclusive;          ///< Inclusive upper bound
        std::optional<std::uint32_t> maxLength;      ///< Maximum string length
        std::optional<double> minExclusive;          ///< Exclusive lower bound
        std::optional<double> minInclusive;          ///< Inclusive lower bound
        std::optional<std::uint32_t> minLength;      ///< Minimum string length
        std::optional<std::string> pattern;          ///< XSD regex pattern constraint
        std::optional<std::uint32_t> totalDigits;    ///< Maximum total number of digits
        std::optional<std::string> whiteSpace;       ///< Whitespace normalization rule
    };

    /**
     * @brief Value format descriptor for a data channel
     * @details Maps to the ISO 19848 `Format` schema element
     */
    struct FormatDto
    {
        std::string type; ///< XSD built-in type name (e.g. @c "decimal", @c "string")
        std::optional<RestrictionDto> restriction;
    };

    /**
     * @brief Data channel type and timing parameters
     * @details Maps to the ISO 19848 `DataChannelType` schema element
     */
    struct DataChannelTypeDto
    {
        std::string type;                        ///< Channel type (e.g. @c "Inst", @c "Mean", @c "Calculated")
        std::optional<double> updateCycle;       ///< Nominal update interval in seconds
        std::optional<double> calculationPeriod; ///< Calculation window in seconds (for averaged/calculated types)
    };

    /**
     * @brief Inclusive numeric range for a data channel value
     */
    struct RangeDto
    {
        double low;
        double high;
    };

    /**
     * @brief Physical unit descriptor for a data channel
     * @details Maps to the ISO 19848 `Unit` schema element
     */
    struct UnitDto
    {
        std::string unitSymbol;                  ///< SI or domain unit symbol (e.g. @c "degC", @c "kPa")
        std::optional<std::string> quantityName; ///< Human-readable quantity name (e.g. @c "Temperature")
        std::optional<serialization::json::SerializableDocument> customElements; ///< @c xs:any extension point
    };

    /**
     * @brief Full property descriptor for a data channel
     * @details Maps to the ISO 19848 `Property` schema element
     *          Aggregates type, format, range, unit, and optional metadata
     */
    struct PropertyDto
    {
        DataChannelTypeDto dataChannelType;
        FormatDto format;
        std::optional<RangeDto> range;
        std::optional<UnitDto> unit;
        std::optional<std::string> qualityCoding; ///< Quality coding scheme identifier
        std::optional<std::string> alertPriority; ///< Alert priority level
        std::optional<std::string> name;
        std::optional<std::string> remarks;
        std::optional<serialization::json::SerializableDocument> customProperties; ///< @c xs:any extension point
    };

    /**
     * @brief Naming rule and optional custom name objects for a data channel
     * @details Maps to the ISO 19848 `NameObject` schema element
     */
    struct NameObjectDto
    {
        std::string namingRule; ///< Naming rule identifier (e.g. @c "dnv-v2")
        std::optional<serialization::json::SerializableDocument> customNameObjects; ///< @c xs:any extension point
    };

    /**
     * @brief Composite identifier for a data channel
     * @details Maps to the ISO 19848 `DataChannelId` schema element
     */
    struct DataChannelIdDto
    {
        std::string localId;                ///< ISO 19848 Local ID string
        std::optional<std::string> shortId; ///< Short alphanumeric alias
        std::optional<NameObjectDto> nameObject;
    };

    /**
     * @brief Single data channel entry in a DataChannelList
     * @details Maps to the ISO 19848 `DataChannel` schema element
     */
    struct DataChannelDto
    {
        DataChannelIdDto dataChannelId;
        PropertyDto property;
    };

    /**
     * @brief Reference to a specific DataChannelList configuration
     * @details Maps to the ISO 19848 `ConfigurationReference` schema element
     */
    struct ConfigurationReferenceDto
    {
        std::string id;
        std::string timeStamp; ///< ISO 8601 timestamp of the configuration
        std::optional<std::string> version;
    };

    /**
     * @brief VIS version information for the DataChannelList
     * @details Maps to the ISO 19848 `VersionInformation` schema element
     */
    struct VersionInformationDto
    {
        std::string namingRule;
        std::string namingSchemeVersion;
        std::optional<std::string> referenceUrl; ///< URL to the naming scheme specification
    };

    /**
     * @brief Package header for a DataChannelList message
     * @details Maps to the ISO 19848 `Header` schema element of a DataChannelList package
     */
    struct HeaderDto
    {
        std::string shipId; ///< IMO number or vessel identifier
        ConfigurationReferenceDto dataChannelListId;
        std::optional<VersionInformationDto> versionInformation;
        std::optional<std::string> author;
        std::optional<std::string> dateCreated;                                 ///< ISO 8601 creation date
        std::optional<serialization::json::SerializableDocument> customHeaders; ///< @c xs:any extension point
    };

    /**
     * @brief Container for all data channels in a DataChannelList message
     */
    struct DataChannelListDto
    {
        std::vector<DataChannelDto> dataChannels;
    };

    /**
     * @brief Top-level package content for a DataChannelList message
     */
    struct PackageDto
    {
        HeaderDto header;
        DataChannelListDto dataChannelList;
    };

    /**
     * @brief Root DTO for an ISO 19848 DataChannelList message
     * @details Deserialize from JSON via `Extensions::fromJsonString()`
     *          Serialize via `Extensions::toJsonString()`
     */
    struct DataChannelListPackageDto
    {
        PackageDto package;
    };
} // namespace dnv::vista::sdk::transport::serialization::json::datachannel
