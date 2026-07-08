/**
 * @file Codebook.h
 * @brief VIS codebook representation with validation and metadata tag creation
 * @details Represents a VIS codebook containing standard values, groups, and validation logic
 *          Codebooks are used to validate metadata tag values and create MetadataTag instances
 *          for use in VIS Local IDs. Special validation logic is provided for Position codebook
 */

#pragma once

#include "CodebookName.h"
#include "MetadataTag.h"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace dnv::vista::sdk
{
    struct CodebookDto;

    /**
     * @enum PositionValidationResult
     * @brief Result codes for position string validation
     * @details Values < 100 indicate errors (invalid), values >= 100 indicate acceptable (valid or custom)
     *          Used by Codebook::validatePosition() to determine if a position string is valid
     */
    enum class PositionValidationResult : std::uint8_t
    {
        Invalid = 0,         ///< Invalid format, whitespace, or non-ISO characters
        InvalidOrder = 1,    ///< Numbers not at end or not alphabetically sorted
        InvalidGrouping = 2, ///< Duplicate groups (except DEFAULT_GROUP)
        Valid = 100,         ///< Standard value, number, or valid composite
        Custom = 101         ///< Custom value not in standard codebook
    };

    /**
     * @class Codebook
     * @brief Represents a VIS codebook with standard values, groups, and validation logic
     * @details A codebook contains validated standard values organized into groups
     *          It provides factory methods to create MetadataTag instances with automatic
     *          validation and custom tag detection. Special validation logic is provided
     *          for the Position codebook (composite positions, ordering, grouping rules)
     */
    class Codebook final
    {
        friend class Codebooks;

    private:
        /**
         * @brief Constructs a Codebook from a DTO
         * @param dto CodebookDto containing raw codebook data
         * @details Parses DTO values, builds group mappings, and extracts standard values and group sets
         */
        explicit Codebook(const CodebookDto& dto);

    public:
        Codebook() = delete;
        Codebook(const Codebook&) = default;
        Codebook(Codebook&&) noexcept = default;
        ~Codebook() = default;

        Codebook& operator=(const Codebook&) = default;
        Codebook& operator=(Codebook&&) noexcept = default;

        /** @brief Get the codebook name */
        inline CodebookName name() const noexcept;

        /** @brief Get the set of standard values */
        inline const std::unordered_set<std::string>& standardValues() const noexcept;

        /** @brief Get the set of group names */
        inline const std::unordered_set<std::string>& groups() const noexcept;

        /**
         * @brief Check if a group exists in this codebook
         * @param group Group name to check
         * @return True if group exists
         */
        inline bool hasGroup(std::string_view group) const noexcept;

        /**
         * @brief Check if a value is a standard value in this codebook
         * @param value Value to check
         * @return True if value is a standard value, or a pure digit string for the Position codebook
         */
        inline bool hasStandardValue(std::string_view value) const noexcept;

        /**
         * @brief Create a metadata tag with validation
         * @param value The tag value to validate
         * @return Optional MetadataTag if value is valid, std::nullopt otherwise
         */
        [[nodiscard]] inline std::optional<MetadataTag> createTag(std::string_view value) const noexcept;

        /**
         * @brief Validate a position string with specific rules
         * @param position The position string to validate
         * @return PositionValidationResult indicating validation outcome
         * @details Validates position strings according to VIS rules:
         *          - ISO string characters only (RFC 3986 unreserved)
         *          - No leading/trailing whitespace
         *          - Standard values and numbers are Valid
         *          - Single non-standard values are Custom
         *          - Composite positions (with '-') must have:
         *            * All parts individually valid
         *            * Numbers only at the end
         *            * Non-numbers alphabetically sorted
         *            * No duplicate groups (except DEFAULT_GROUP)
         * @note Only meaningful for the Position codebook. Results are unspecified for other codebook types
         */
        [[nodiscard]] inline PositionValidationResult validatePosition(std::string_view position) const noexcept;

    private:
        CodebookName m_name;
        std::unordered_map<std::string, std::string> m_groupMap;
        std::unordered_set<std::string> m_standardValues;
        std::unordered_set<std::string> m_groups;
    };
} // namespace dnv::vista::sdk

#include "dnv/vista/sdk/detail/core/Codebook.inl"
