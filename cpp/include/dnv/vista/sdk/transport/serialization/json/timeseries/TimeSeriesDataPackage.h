/**
 * @file TimeSeriesDataPackage.h
 * @brief JSON DTO types for ISO 19848 TimeSeriesData messages
 * @details Flat, JSON-serializable structs mirroring the ISO 19848 JSON schema
 *          Custom extension points (xs:any) are represented as SerializableDocument
 */

#pragma once

#include "dnv/vista/sdk/transport/serialization/json/SerializableDocument.h"

#include <optional>
#include <string>
#include <vector>

namespace dnv::vista::sdk::transport::serialization::json::timeseries
{
    /**
     * @brief ISO 8601 time interval for a TimeSeriesData package header
     */
    struct TimeSpanDto
    {
        std::string start;
        std::string end;
    };

    /**
     * @brief Reference to a DataChannelList configuration used by this time series
     * @details Maps to the ISO 19848 `ConfigurationReference` schema element
     */
    struct ConfigurationReferenceDto
    {
        std::string id;
        std::string timeStamp; ///< ISO 8601 timestamp of the configuration
    };

    /**
     * @brief Package header for a TimeSeriesData message
     * @details Maps to the ISO 19848 `Header` schema element of a TimeSeriesData package
     */
    struct HeaderDto
    {
        std::string shipId; ///< IMO number or vessel identifier
        std::optional<TimeSpanDto> timeSpan;
        std::optional<std::string> dateCreated;  ///< ISO 8601 creation date
        std::optional<std::string> dateModified; ///< ISO 8601 last modification date
        std::optional<std::string> author;
        std::optional<std::vector<ConfigurationReferenceDto>> systemConfiguration;
        std::optional<serialization::json::SerializableDocument> customHeaders; ///< @c xs:any extension point
    };

    /**
     * @brief Single timestamped row in a tabular time series dataset
     * @details Maps to the ISO 19848 `TabularDataSet` schema element
     */
    struct TabularDataSetDto
    {
        std::string timeStamp;                           ///< ISO 8601 timestamp for this row
        std::vector<std::string> value;                  ///< Values in channel order (parallel to @c dataChannelIds)
        std::optional<std::vector<std::string>> quality; ///< Quality codes in channel order (parallel to @c value)
    };

    /**
     * @brief Tabular time series data block for one or more data channels
     * @details Maps to the ISO 19848 `TabularData` schema element
     *          Values and quality codes are stored in column-parallel arrays
     */
    struct TabularDataDto
    {
        std::optional<std::size_t> numberOfDataSet;
        std::optional<std::size_t> numberOfDataChannel;
        std::optional<std::vector<std::string>> dataChannelIds; ///< Ordered channel IDs (column headers)
        std::optional<std::vector<TabularDataSetDto>> dataSets;
    };

    /**
     * @brief Single event observation in an event-driven time series
     * @details Maps to the ISO 19848 `EventDataSet` schema element
     */
    struct EventDataSetDto
    {
        std::string timeStamp; ///< ISO 8601 timestamp of the event
        std::string dataChannelId;
        std::string value;
        std::optional<std::string> quality;
    };

    /**
     * @brief Event-driven time series data block
     * @details Maps to the ISO 19848 `EventData` schema element
     */
    struct EventDataDto
    {
        std::optional<std::size_t> numberOfDataSet;
        std::optional<std::vector<EventDataSetDto>> dataSet;
    };

    /**
     * @brief One time series data block, either tabular or event-driven
     * @details Maps to the ISO 19848 `TimeSeriesData` schema element
     *          A package may contain multiple blocks of either kind
     */
    struct TimeSeriesDataDto
    {
        std::optional<ConfigurationReferenceDto> dataConfiguration;
        std::optional<std::vector<TabularDataDto>> tabularData;
        std::optional<EventDataDto> eventData;
        std::optional<serialization::json::SerializableDocument> customDataKinds; ///< @c xs:any extension point
    };

    /**
     * @brief Top-level package content for a TimeSeriesData message
     */
    struct PackageDto
    {
        std::optional<HeaderDto> header; ///< Optional per ISO 19848
        std::vector<TimeSeriesDataDto> timeSeriesData;
    };

    /**
     * @brief Root DTO for an ISO 19848 TimeSeriesData message
     * @details Deserialize from JSON via `Extensions::fromJsonString()`
     *          Serialize via `Extensions::toJsonString()`
     */
    struct TimeSeriesDataPackageDto
    {
        PackageDto package;
    };
} // namespace dnv::vista::sdk::transport::serialization::json::timeseries
