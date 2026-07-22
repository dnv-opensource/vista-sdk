/**
 * @file Extensions.h
 * @brief Mapping between the TimeSeriesData domain model and JSON DTOs
 */

#pragma once

#include "TimeSeriesDataPackage.h"
#include "dnv/vista/sdk/transport/timeseries/TimeSeriesData.h"
#include "dnv/vista/sdk/utils/StringBuilder.h"

namespace dnv::vista::sdk::transport::serialization::json::timeseries
{
    /**
     * @brief Convert a TimeSeriesDataPackage domain object to its JSON DTO representation
     */
    [[nodiscard]] TimeSeriesDataPackageDto toDto(
        const dnv::vista::sdk::transport::timeseries::TimeSeriesDataPackage& domain);

    /**
     * @brief Convert a JSON DTO to its TimeSeriesDataPackage domain representation
     */
    [[nodiscard]] dnv::vista::sdk::transport::timeseries::TimeSeriesDataPackage toDomain(
        const TimeSeriesDataPackageDto& dto);

    /**
     * @brief Serialize a DTO to JSON, writing into the given buffer
     * @param buffer External buffer to write into. The caller owns it and may reuse it
     *               across multiple calls to retain its capacity between documents
     * @param dto The DTO to serialize
     * @param prettyPrint If true, output is indented with 2-space indent. If false, compact
     */
    void toJsonString(
        dnv::vista::sdk::StringBuilder& buffer, const TimeSeriesDataPackageDto& dto, bool prettyPrint = false);

    /**
     * @brief Deserialize a JSON string into a DTO
     */
    [[nodiscard]] std::optional<TimeSeriesDataPackageDto> fromJsonString(std::string_view json);

    /**
     * @brief Serialize a domain object directly to JSON, writing into the given buffer
     * @param buffer External buffer to write into. The caller owns it and may reuse it
     *               across multiple calls to retain its capacity between documents
     * @param domain The domain object to serialize
     * @param prettyPrint If true, output is indented with 2-space indent. If false, compact
     */
    inline void toJsonString(
        dnv::vista::sdk::StringBuilder& buffer,
        const dnv::vista::sdk::transport::timeseries::TimeSeriesDataPackage& domain,
        bool prettyPrint = false)
    {
        toJsonString(buffer, toDto(domain), prettyPrint);
    }

    /**
     * @brief Deserialize a JSON string directly into a domain object
     */
    [[nodiscard]] inline std::optional<dnv::vista::sdk::transport::timeseries::TimeSeriesDataPackage> fromJsonString(
        std::string_view json, std::nullptr_t /*domain tag*/)
    {
        auto dto = fromJsonString(json);
        if (!dto)
        {
            return std::nullopt;
        }

        try
        {
            return toDomain(*dto);
        }
        catch (const std::exception&)
        {
            return std::nullopt;
        }
    }
} // namespace dnv::vista::sdk::transport::serialization::json::timeseries
