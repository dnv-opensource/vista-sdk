/**
 * @file LocalIdBuilder.h
 * @brief Builder pattern for constructing VIS LocalId instances
 * @details Provides a fluent API for building LocalId objects according to the dnv-v2 naming rule
 *          LocalId combines GmodPath(s) with metadata tags to create unique sensor identifiers
 *          Format: /dnv-v2/vis-3-4a/411.1/C101.62/S205/meta/qty-pressure/cnt-lubricating.oil/pos-inlet
 */

#pragma once

#include "dnv/vista/sdk/utils/StringBuilder.h"
#include "GmodPath.h"
#include "MetadataTag.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dnv::vista::sdk
{
    class LocalId;

    namespace internal
    {
        class LocalIdParsingErrorBuilder;
    } // namespace internal

    /**
     * @class LocalIdBuilder
     * @brief Builder for constructing VIS LocalId instances with fluent API
     * @details Provides immutable builder pattern for creating LocalId objects with validation
     *          Each with* method returns a new builder instance with the modified value,
     *          preserving immutability. Values are validated against the VIS version specification
     *          Use build() to construct the final LocalId object from the builder state
     *
     * @note LocalId follows the dnv-v2 naming rule and supports the following codebooks:
     *       - Quantity, Content, Calculation, State, Command, Type, Position, Detail
     */
    class LocalIdBuilder final
    {
    private:
        LocalIdBuilder() = default;

    public:
        LocalIdBuilder(const LocalIdBuilder&) = default;
        LocalIdBuilder(LocalIdBuilder&&) noexcept = default;
        ~LocalIdBuilder() = default;

        LocalIdBuilder& operator=(const LocalIdBuilder&) = default;
        LocalIdBuilder& operator=(LocalIdBuilder&&) noexcept = default;

        [[nodiscard]] inline bool operator==(const LocalIdBuilder& other) const noexcept;

        /**
         * @brief Get ISO 19848 Annex C naming rule identifier
         * @return String view containing the naming rule identifier ("dnv-v2")
         */
        [[nodiscard]] static constexpr std::string_view namingRule() noexcept;

        /**
         * @brief Get the VIS version for this builder
         * @return Optional containing the VisVersion if set, nullopt otherwise
         */
        [[nodiscard]] inline std::optional<VisVersion> version() const noexcept;

        /**
         * @brief Get the verbose mode flag
         * @return True if verbose mode is enabled
         */
        [[nodiscard]] inline bool isVerboseMode() const noexcept;

        /**
         * @brief Check if builder state is valid for building a LocalId
         * @return True if VisVersion, primary item, and at least one metadata tag are set
         */
        [[nodiscard]] inline bool isValid() const noexcept;

        /**
         * @brief Check if builder is empty (no components set)
         * @return True if no primary item, secondary item, or metadata tags are set
         */
        [[nodiscard]] inline bool isEmpty() const noexcept;

        /**
         * @brief Checks if the builder has no metadata tags set
         * @return True if no metadata tags (quantity, content, etc.) are present, false otherwise
         */
        [[nodiscard]] inline bool isEmptyMetadata() const noexcept;

        /**
         * @brief Check if builder has any custom (non-standard) metadata tags
         * @return True if any metadata tag is custom
         */
        [[nodiscard]] inline bool hasCustomTag() const noexcept;

        /**
         * @brief Get the primary item (Gmod path)
         * @return Optional containing the primary GmodPath if set, nullopt otherwise
         */
        [[nodiscard]] inline const std::optional<GmodPath>& primaryItem() const noexcept;

        /**
         * @brief Get the secondary item (Gmod path)
         * @return Optional containing the secondary GmodPath if set, nullopt otherwise
         */
        [[nodiscard]] inline const std::optional<GmodPath>& secondaryItem() const noexcept;

        /**
         * @brief Get all metadata tags that are set
         * @return Vector containing all set metadata tags in the order: Quantity, Content, Calculation, State, Command,
         * Type, Position, Detail
         */
        [[nodiscard]] inline std::vector<MetadataTag> metadataTags() const noexcept;

        /** @brief Get the Quantity metadata tag (nullopt if not set) */
        [[nodiscard]] inline const std::optional<MetadataTag>& quantity() const noexcept;

        /** @brief Get the Content metadata tag (nullopt if not set) */
        [[nodiscard]] inline const std::optional<MetadataTag>& content() const noexcept;

        /** @brief Get the Calculation metadata tag (nullopt if not set) */
        [[nodiscard]] inline const std::optional<MetadataTag>& calculation() const noexcept;

        /** @brief Get the State metadata tag (nullopt if not set) */
        [[nodiscard]] inline const std::optional<MetadataTag>& state() const noexcept;

        /** @brief Get the Command metadata tag (nullopt if not set) */
        [[nodiscard]] inline const std::optional<MetadataTag>& command() const noexcept;

        /** @brief Get the Type metadata tag (nullopt if not set) */
        [[nodiscard]] inline const std::optional<MetadataTag>& type() const noexcept;

        /** @brief Get the Position metadata tag (nullopt if not set) */
        [[nodiscard]] inline const std::optional<MetadataTag>& position() const noexcept;

        /** @brief Get the Detail metadata tag (nullopt if not set) */
        [[nodiscard]] inline const std::optional<MetadataTag>& detail() const noexcept;

        /**
         * @brief Create a new LocalIdBuilder for the given VIS version
         * @param visVersion VIS version for this LocalId
         * @return New LocalIdBuilder instance with the specified VIS version
         */
        [[nodiscard]] static inline LocalIdBuilder create(VisVersion visVersion) noexcept;

        /**
         * @brief Set the VIS version
         * @param version VIS version enum value
         * @return New LocalIdBuilder with VIS version set
         */
        [[nodiscard]] inline LocalIdBuilder withVisVersion(VisVersion version) const&;

        /**
         * @brief Set the VIS version
         * @param version VIS version enum value
         * @return New LocalIdBuilder with VIS version set
         */
        [[nodiscard]] inline LocalIdBuilder withVisVersion(VisVersion version) && noexcept;

        /**
         * @brief Set the VIS version from string
         * @param visVersionStr String representation of VIS version (e.g., "3-4a")
         * @return New LocalIdBuilder with VIS version set
         * @throws std::invalid_argument if version string is invalid
         */
        [[nodiscard]] inline LocalIdBuilder withVisVersion(std::string_view visVersionStr) const&;

        /**
         * @brief Set the VIS version from string
         * @param visVersionStr String representation of VIS version (e.g., "3-4a")
         * @return New LocalIdBuilder with VIS version set
         * @throws std::invalid_argument if version string is invalid
         */
        [[nodiscard]] inline LocalIdBuilder withVisVersion(std::string_view visVersionStr) &&;

        /**
         * @brief Remove the VIS version
         * @return New LocalIdBuilder with VIS version cleared
         */
        [[nodiscard]] inline LocalIdBuilder withoutVisVersion() const&;

        /**
         * @brief Remove the VIS version
         * @return New LocalIdBuilder with VIS version cleared
         */
        [[nodiscard]] inline LocalIdBuilder withoutVisVersion() && noexcept;

        /**
         * @brief Set the primary item (Gmod path)
         * @param path Primary GmodPath
         * @return New LocalIdBuilder with primary item set
         */
        [[nodiscard]] inline LocalIdBuilder withPrimaryItem(const GmodPath& path) const&;

        /**
         * @brief Set the primary item (Gmod path)
         * @param path Primary GmodPath
         * @return New LocalIdBuilder with primary item set
         */
        [[nodiscard]] inline LocalIdBuilder withPrimaryItem(const GmodPath& path) &&;

        /**
         * @brief Set the primary item from optional GmodPath
         * @param path Optional primary GmodPath
         * @return New LocalIdBuilder with primary item set
         * @throws std::invalid_argument if path is nullopt
         */
        [[nodiscard]] inline LocalIdBuilder withPrimaryItem(const std::optional<GmodPath>& path) const&;

        /**
         * @brief Set the primary item from optional GmodPath
         * @param path Optional primary GmodPath
         * @return New LocalIdBuilder with primary item set
         * @throws std::invalid_argument if path is nullopt
         */
        [[nodiscard]] inline LocalIdBuilder withPrimaryItem(const std::optional<GmodPath>& path) &&;

        /**
         * @brief Remove the primary item
         * @return New LocalIdBuilder with primary item cleared
         */
        [[nodiscard]] inline LocalIdBuilder withoutPrimaryItem() const&;

        /**
         * @brief Remove the primary item
         * @return New LocalIdBuilder with primary item cleared
         */
        [[nodiscard]] inline LocalIdBuilder withoutPrimaryItem() && noexcept;

        /**
         * @brief Set the secondary item (Gmod path)
         * @param path Secondary GmodPath
         * @return New LocalIdBuilder with secondary item set
         */
        [[nodiscard]] inline LocalIdBuilder withSecondaryItem(const GmodPath& path) const&;

        /**
         * @brief Set the secondary item (Gmod path)
         * @param path Secondary GmodPath
         * @return New LocalIdBuilder with secondary item set
         */
        [[nodiscard]] inline LocalIdBuilder withSecondaryItem(const GmodPath& path) &&;

        /**
         * @brief Set the secondary item from optional GmodPath
         * @param path Optional secondary GmodPath
         * @return New LocalIdBuilder with secondary item set
         * @throws std::invalid_argument if path is nullopt
         */
        [[nodiscard]] inline LocalIdBuilder withSecondaryItem(const std::optional<GmodPath>& path) const&;

        /**
         * @brief Set the secondary item from optional GmodPath
         * @param path Optional secondary GmodPath
         * @return New LocalIdBuilder with secondary item set
         * @throws std::invalid_argument if path is nullopt
         */
        [[nodiscard]] inline LocalIdBuilder withSecondaryItem(const std::optional<GmodPath>& path) &&;

        /**
         * @brief Remove the secondary item
         * @return New LocalIdBuilder with secondary item cleared
         */
        [[nodiscard]] inline LocalIdBuilder withoutSecondaryItem() const&;

        /**
         * @brief Remove the secondary item
         * @return New LocalIdBuilder with secondary item cleared
         */
        [[nodiscard]] inline LocalIdBuilder withoutSecondaryItem() && noexcept;

        /**
         * @brief Set a metadata tag
         * @param tag MetadataTag to set (codebook name determines which slot)
         * @return New LocalIdBuilder with the metadata tag set
         */
        [[nodiscard]] inline LocalIdBuilder withMetadataTag(const MetadataTag& tag) const&;

        /**
         * @brief Set a metadata tag
         * @param tag MetadataTag to set (codebook name determines which slot)
         * @return New LocalIdBuilder with the metadata tag set
         */
        [[nodiscard]] inline LocalIdBuilder withMetadataTag(const MetadataTag& tag) &&;

        /**
         * @brief Set a metadata tag from optional
         * @param tag Optional MetadataTag to set
         * @return New LocalIdBuilder with the metadata tag set if tag has value, unchanged copy if nullopt
         * @note If tag is nullopt, returns unchanged copy
         */
        [[nodiscard]] inline LocalIdBuilder withMetadataTag(const std::optional<MetadataTag>& tag) const&;

        /**
         * @brief Set a metadata tag from optional
         * @param tag Optional MetadataTag to set
         * @return New LocalIdBuilder with the metadata tag set if tag has value, moved builder if nullopt
         * @note If tag is nullopt, returns moved builder unchanged
         */
        [[nodiscard]] inline LocalIdBuilder withMetadataTag(const std::optional<MetadataTag>& tag) &&;

        /**
         * @brief Remove a metadata tag by codebook name
         * @param name Codebook name identifying which tag to remove
         * @return New LocalIdBuilder with the specified metadata tag cleared
         */
        [[nodiscard]] inline LocalIdBuilder withoutMetadataTag(CodebookName name) const&;

        /**
         * @brief Remove a metadata tag by codebook name
         * @param name Codebook name identifying which tag to remove
         * @return New LocalIdBuilder with the specified metadata tag cleared
         */
        [[nodiscard]] inline LocalIdBuilder withoutMetadataTag(CodebookName name) && noexcept;

        /**
         * @brief Set verbose mode
         * @param verbose True to enable verbose mode (include common names in string representation)
         * @return New LocalIdBuilder with verbose mode set
         */
        [[nodiscard]] inline LocalIdBuilder withVerboseMode(bool verbose) const&;

        /**
         * @brief Set verbose mode
         * @param verbose True to enable verbose mode (include common names in string representation)
         * @return New LocalIdBuilder with verbose mode set
         */
        [[nodiscard]] inline LocalIdBuilder withVerboseMode(bool verbose) && noexcept;

        /**
         * @brief Build the final LocalId object from builder state
         * @return LocalId instance constructed from current builder state
         * @throws std::invalid_argument if builder state is invalid (no primary item)
         */
        [[nodiscard]] LocalId build() const;

        /**
         * @brief Convert builder state to string representation
         * @return String representation of the LocalId in dnv-v2 format
         * @note Format: /dnv-v2/vis-{version}/{primary-item}[/sec/{secondary-item}]/meta[/qty-{value}][/cnt-{value}]..
         */
        [[nodiscard]] inline std::string toString() const;

        /**
         * @brief Convert builder state to string representation (in-place)
         * @param builder Builder to append the LocalId string representation to
         * @note Format: /dnv-v2/vis-{version}/{primary-item}[/sec/{secondary-item}]/meta[/qty-{value}][/cnt-{value}]..
         */
        void toString(StringBuilder& builder) const;

        /**
         * @brief Create LocalId from string representation
         * @param localIdStr String representation of LocalId in dnv-v2 format (e.g.,
         * "/dnv-v2/vis-3-4a/411.1/H101/meta/qty-power")
         * @return Optional containing LocalId if parsing succeeded, nullopt otherwise
         * @note This method does not throw exceptions. Use the overload with ParsingErrors for diagnostics
         */
        [[nodiscard]] static std::optional<LocalId> fromString(std::string_view localIdStr) noexcept;

        /**
         * @brief Create LocalId from string representation with error reporting
         * @param localIdStr String representation of LocalId in dnv-v2 format
         * @param errors Output parameter filled with parsing errors if parsing fails
         * @return Optional containing LocalId if parsing succeeded, nullopt otherwise
         * @note This method does not throw exceptions. Check the errors parameter for failure details
         */
        [[nodiscard]] static std::optional<LocalId> fromString(
            std::string_view localIdStr, ParsingErrors& errors) noexcept;

    private:
        static std::optional<LocalId> fromString(
            std::string_view localIdStr, internal::LocalIdParsingErrorBuilder* errorBuilder) noexcept;

        std::optional<VisVersion> m_visVersion;
        bool m_verboseMode{ false };
        std::optional<GmodPath> m_primaryItem;
        std::optional<GmodPath> m_secondaryItem;
        std::optional<MetadataTag> m_quantity;
        std::optional<MetadataTag> m_content;
        std::optional<MetadataTag> m_calculation;
        std::optional<MetadataTag> m_state;
        std::optional<MetadataTag> m_command;
        std::optional<MetadataTag> m_type;
        std::optional<MetadataTag> m_position;
        std::optional<MetadataTag> m_detail;
    };
} // namespace dnv::vista::sdk

#include "dnv/vista/sdk/detail/core/LocalIdBuilder.inl"
