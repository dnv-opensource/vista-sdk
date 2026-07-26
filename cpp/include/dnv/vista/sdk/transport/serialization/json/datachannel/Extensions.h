/**
 * @file Extensions.h
 * @brief Mapping between the DataChannelList domain model and JSON DTOs
 */

#pragma once

#include <dnv/vista/sdk/Export.h>

#include "DataChannelListPackage.h"
#include "dnv/vista/sdk/transport/datachannel/DataChannel.h"
#include "dnv/vista/sdk/utils/StringBuilder.h"

namespace dnv::vista::sdk::transport::serialization::json::datachannel
{
    /**
     * @brief Convert a DataChannelListPackage domain object to its JSON DTO representation
     * @param domain The domain object to convert
     * @return Equivalent DTO with all fields mapped
     */
    [[nodiscard]] DNV_VISTA_SDK_CPP_API DataChannelListPackageDto
    toDto(const dnv::vista::sdk::transport::datachannel::DataChannelListPackage& domain);

    /**
     * @brief Convert a JSON DTO to its DataChannelListPackage domain representation
     * @param dto The DTO to convert
     * @return Equivalent domain object
     */
    [[nodiscard]] DNV_VISTA_SDK_CPP_API dnv::vista::sdk::transport::datachannel::DataChannelListPackage toDomain(
        const DataChannelListPackageDto& dto);

    /**
     * @brief Serialize a DTO to JSON, writing into the given buffer
     * @param buffer External buffer to write into. The caller owns it and may reuse it
     *               across multiple calls to retain its capacity between documents
     * @param dto The DTO to serialize
     * @param prettyPrint If true, output is indented with 2-space indent. If false, compact
     */
    DNV_VISTA_SDK_CPP_API void toJsonString(
        dnv::vista::sdk::StringBuilder& buffer, const DataChannelListPackageDto& dto, bool prettyPrint = false);

    /**
     * @brief Serialize a DTO to JSON, returning a new string
     * @details Convenience overload for one-off serialization. Prefer the StringBuilder& overload
     *          when serializing repeatedly (e.g. in a loop), to reuse the buffer's capacity
     * @param dto The DTO to serialize
     * @param prettyPrint If true, output is indented with 2-space indent. If false, compact
     */
    [[nodiscard]] inline std::string toJsonString(const DataChannelListPackageDto& dto, bool prettyPrint = false)
    {
        dnv::vista::sdk::StringBuilder buffer;
        toJsonString(buffer, dto, prettyPrint);
        return buffer.toString();
    }

    /**
     * @brief Deserialize a JSON string into a DTO
     * @param json The JSON string to parse
     * @return Parsed DTO, or nullopt if parsing fails
     */
    [[nodiscard]] DNV_VISTA_SDK_CPP_API std::optional<DataChannelListPackageDto> fromJsonString(std::string_view json);

    /**
     * @brief Serialize a domain object directly to JSON, writing into the given buffer
     * @param buffer External buffer to write into. The caller owns it and may reuse it
     *               across multiple calls to retain its capacity between documents
     * @param domain The domain object to serialize
     * @param prettyPrint If true, output is indented with 2-space indent. If false, compact
     */
    DNV_VISTA_SDK_CPP_API void toJsonString(
        dnv::vista::sdk::StringBuilder& buffer,
        const dnv::vista::sdk::transport::datachannel::DataChannelListPackage& domain,
        bool prettyPrint = false);

    /**
     * @brief Serialize a domain object directly to JSON, returning a new string
     * @details Convenience overload for one-off serialization. Prefer the StringBuilder& overload
     *          when serializing repeatedly (e.g. in a loop), to reuse the buffer's capacity
     * @param domain The domain object to serialize
     * @param prettyPrint If true, output is indented with 2-space indent. If false, compact
     */
    [[nodiscard]] inline std::string toJsonString(
        const dnv::vista::sdk::transport::datachannel::DataChannelListPackage& domain, bool prettyPrint = false)
    {
        dnv::vista::sdk::StringBuilder buffer;
        toJsonString(buffer, domain, prettyPrint);
        return buffer.toString();
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
