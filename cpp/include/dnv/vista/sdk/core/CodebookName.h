/**
 * @file CodebookName.h
 * @brief Codebook name enumeration and conversion utilities
 * @details Defines the CodebookName enum representing various VIS codebook types
 *          (quantity, content, calculation, etc.) and provides utilities for converting
 *          between enum values and their string prefixes used in VIS identifiers
 */

#pragma once

#include <cstdint>
#include <string_view>

namespace dnv::vista::sdk
{
    /**
     * @brief Enumeration of VIS codebook types
     */
    enum class CodebookName : std::uint8_t
    {
        Quantity = 1,
        Content,
        Calculation,
        State,
        Command,
        Type,
        FunctionalServices,
        MaintenanceCategory,
        ActivityType,
        Position,
        Detail
    };

    /**
     * @brief Utility functions for converting between CodebookName enum values and string prefixes
     * @details Provides functions to convert codebook name prefixes (e.g., "qty", "cnt", "calc")
     *          to their corresponding CodebookName enum values and vice versa. This is used when
     *          parsing and serializing VIS local identifiers and paths
     */
    namespace CodebookNames
    {
        /**
         * @brief Convert a string prefix to a CodebookName enum value
         * @param prefix The string prefix (e.g., "qty", "cnt", "calc", "state", etc.)
         * @return The corresponding CodebookName enum value
         * @throws std::invalid_argument if the prefix is not recognized
         */
        inline CodebookName fromPrefix(std::string_view prefix);

        /**
         * @brief Convert a CodebookName enum value to its string prefix
         * @param name The CodebookName enum value
         * @return The corresponding string prefix (e.g., "qty" for Quantity, "cnt" for Content)
         * @throws std::invalid_argument if the codebook name is invalid
         */
        inline std::string_view toPrefix(CodebookName name);

        /**
         * @brief Convert a CodebookName enum value to its human-readable name
         * @param name The CodebookName enum value
         * @return Human-readable name (e.g., "Quantity", "Content")
         * @throws std::invalid_argument if the codebook name is invalid
         */
        inline std::string_view toString(CodebookName name);
    } // namespace CodebookNames
} // namespace dnv::vista::sdk

#include "dnv/vista/sdk/detail/core/CodebookName.inl"
