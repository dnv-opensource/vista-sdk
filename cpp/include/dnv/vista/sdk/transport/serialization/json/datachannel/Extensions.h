/**
 * @file Extensions.h
 * @brief Mapping between the DataChannelList domain model and JSON DTOs
 */

#pragma once

#include "DataChannelListPackage.h"
#include "dnv/vista/sdk/transport/datachannel/DataChannel.h"

namespace dnv::vista::sdk::transport::serialization::json::datachannel
{
    /**
     * @brief Convert a DataChannelListPackage domain object to its JSON DTO representation
     * @param domain The domain object to convert
     * @return Equivalent DTO with all fields mapped
     */
    [[nodiscard]] DataChannelListPackageDto toDto(
        const dnv::vista::sdk::transport::datachannel::DataChannelListPackage& domain);

    /**
     * @brief Convert a JSON DTO to its DataChannelListPackage domain representation
     * @param dto The DTO to convert
     * @return Equivalent domain object
     */
    [[nodiscard]] dnv::vista::sdk::transport::datachannel::DataChannelListPackage toDomain(
        const DataChannelListPackageDto& dto);

    /**
     * @brief Serialize a DTO to a JSON string
     * @param dto The DTO to serialize
     * @param prettyPrint If true, output is indented with 2-space indent. If false, compact
     * @param reserveHint Pre-allocate buffer capacity hint in bytes (0 = default 4096)
     * @return JSON string
     */
    [[nodiscard]] std::string toJsonString(
        const DataChannelListPackageDto& dto, bool prettyPrint = false, size_t reserveHint = 0);

    /**
     * @brief Deserialize a JSON string into a DTO
     * @param json The JSON string to parse
     * @return Parsed DTO, or nullopt if parsing fails
     */
    [[nodiscard]] std::optional<DataChannelListPackageDto> fromJsonString(std::string_view json);

    /**
     * @brief Serialize a domain object directly to a JSON string
     * @param domain The domain object to serialize
     * @param prettyPrint If true, output is indented with 2-space indent. If false, compact
     * @param reserveHint Pre-allocate buffer capacity hint in bytes (0 = default 4096)
     * @return JSON string
     */
    [[nodiscard]] inline std::string toJsonString(
        const dnv::vista::sdk::transport::datachannel::DataChannelListPackage& domain,
        bool prettyPrint = false,
        size_t reserveHint = 0)
    {
        return toJsonString(toDto(domain), prettyPrint, reserveHint);
    }

    /**
     * @brief Deserialize a JSON string directly into a domain object
     * @param json The JSON string to parse
     * @return Parsed domain object, or nullopt if parsing fails
     * @details Returns nullopt both when json fails to parse as a DTO and when the parsed DTO
     *          fails domain construction (e.g. an unrecognized Format/DataChannelType type
     *          string, an invalid Restriction::WhiteSpace value, or a Range with low > high) -
     *          matching the nullopt-on-failure contract of the DTO-only overload above, rather
     *          than propagating toDomain's `std::invalid_argument`/`std::bad_optional_access`
     */
    [[nodiscard]] inline std::optional<dnv::vista::sdk::transport::datachannel::DataChannelListPackage> fromJsonString(
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
} // namespace dnv::vista::sdk::transport::serialization::json::datachannel
