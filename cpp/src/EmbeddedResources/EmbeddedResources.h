/**
 * @file EmbeddedResources.h
 * @brief Resource loading utilities for Vista SDK
 * @details Provides access to compile-time embedded VIS resources with automatic decompression
 */

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace dnv::vista::sdk
{
    struct GmodDto;
    struct GmodVersioningDto;
    struct CodebooksDto;
    struct LocationsDto;
    struct DataChannelTypeNamesDto;
    struct FormatDataTypesDto;

    /**
     * @brief Access to embedded VIS resource data
     * @details Loads and decompresses compile-time embedded .gz resources.
     *          Each call decompresses and parses the resource on demand.
     */
    class EmbeddedResources final
    {
    public:
        /**
         * @brief Get list of available VIS versions
         * @return Vector of version strings (e.g., ["3-4a", "3-5a", "3-6a"])
         */
        static std::vector<std::string> visVersions();

        /**
         * @brief Get list of available ISO 19848 versions
         * @return Vector of version strings (e.g., ["v2018", "v2024"])
         */
        static std::vector<std::string> iso19848Versions();

        /**
         * @brief Load Gmod data for a VIS version
         * @param visVersion VIS version string (e.g., "3-4a")
         * @return Gmod DTO if found, nullopt otherwise
         */
        static std::optional<GmodDto> gmod(std::string_view visVersion);

        /**
         * @brief Load Codebooks data for a VIS version
         * @param visVersion VIS version string (e.g., "3-4a")
         * @return Codebooks DTO if found, nullopt otherwise
         */
        static std::optional<CodebooksDto> codebooks(std::string_view visVersion);

        /**
         * @brief Load Locations data for a VIS version
         * @param visVersion VIS version string (e.g., "3-4a")
         * @return Locations DTO if found, nullopt otherwise
         */
        static std::optional<LocationsDto> locations(std::string_view visVersion);

        /**
         * @brief Load Gmod versioning data
         * @return Map of Gmod versioning data if available, nullopt otherwise
         */
        static std::optional<std::unordered_map<std::string, GmodVersioningDto>> gmodVersioning();

        /**
         * @brief Load DataChannelType names for ISO19848
         * @param version Version string
         * @return DataChannelType names DTO if found, nullopt otherwise
         */
        static std::optional<DataChannelTypeNamesDto> dataChannelTypeNames(std::string_view version);

        /**
         * @brief Load FormatDataTypes for ISO19848
         * @param version Version string
         * @return FormatDataTypes DTO if found, nullopt otherwise
         */
        static std::optional<FormatDataTypesDto> formatDataTypes(std::string_view version);
    };
} // namespace dnv::vista::sdk
