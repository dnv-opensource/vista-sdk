/**
 * @file MetadataTag.h
 * @brief Metadata tag for VIS Local ID content/quantity/state markers
 * @details Represents a metadata tag in a VIS Local ID with a codebook name,
 *          value, and custom flag. Tags appear as "prefix-value" or "prefix~value"
 *          in the Local ID string format
 */

#pragma once

#include "CodebookName.h"

#include <string>

namespace dnv::vista::sdk
{
    /**
     * @brief Represents a metadata tag with a codebook name and value
     * @details A metadata tag combines a CodebookName (e.g., Quantity, Content) with
     *          a string value (e.g., "temperature", "oil"). Custom tags are marked
     *          with '~' separator, standard tags use '-' separator
     *          MetadataTag instances can only be created by SDK internals
     */
    class MetadataTag final
    {
        friend class Codebook;

    private:
        /**
         * @brief Constructs a MetadataTag with a codebook name, value, and custom flag
         * @param name The codebook this tag belongs to
         * @param value The value of the metadata tag
         * @param isCustom Whether the tag is custom (true) or a standard codebook value (false)
         */
        inline MetadataTag(CodebookName name, std::string value, bool isCustom = false) noexcept;

    public:
        MetadataTag() = delete;
        MetadataTag(const MetadataTag&) = default;
        MetadataTag(MetadataTag&&) noexcept = default;
        ~MetadataTag() = default;

        MetadataTag& operator=(const MetadataTag&) = default;
        MetadataTag& operator=(MetadataTag&&) noexcept = default;

        /**
         * @brief Compare two metadata tags for equality
         * @param other The tag to compare against
         * @return true if both tags have the same value
         * @throws std::invalid_argument if the tags belong to different codebooks -
         *         tags from different codebooks (e.g. Quantity vs. Content) are not
         *         comparable, so this is a programming error rather than a case
         *         that should silently return false
         */
        [[nodiscard]] inline bool operator==(const MetadataTag& other) const;

        /** @brief Get the codebook name */
        [[nodiscard]] inline CodebookName name() const noexcept;

        /** @brief Get the tag value */
        [[nodiscard]] inline const std::string& value() const noexcept;

        /**
         * @brief Get the prefix separator character
         * @return '~' for custom tags, '-' for standard tags
         */
        [[nodiscard]] inline char prefix() const noexcept;

        /**
         * @brief Check if this is a custom tag
         * @return true if custom, false if standard
         */
        [[nodiscard]] inline bool isCustom() const noexcept;

        /**
         * @brief Returns the formatted tag as a string without trailing separator
         * @return The tag formatted as "prefix-value" or "prefix~value"
         */
        [[nodiscard]] inline std::string toString() const;

        /**
         * @brief Appends the formatted tag to a string
         * @param out String to append the formatted tag to
         * @param separator The separator character to append after the value
         * @details Formats the tag as "prefix-value/" or "prefix~value/"
         */
        inline void toString(std::string& out, char separator = '/') const;

    private:
        CodebookName m_name; ///< The name of the metadata tag, represented by a CodebookName enum value
        bool m_custom; ///< A boolean flag indicating whether this is a custom tag (true) or a standard tag (false)
        std::string m_value; ///< The string value associated with the metadata tag
    };
} // namespace dnv::vista::sdk

#include "dnv/vista/sdk/detail/core/MetadataTag.inl"
