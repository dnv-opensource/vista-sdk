/**
 * @file LocalId.h
 * @brief Immutable VIS Local ID representation
 * @details Provides an immutable LocalId class representing a unique sensor identifier
 *          according to the dnv-v2 naming rule. LocalId combines GmodPath(s) with
 *          metadata tags to create globally unique sensor identifiers within a vessel
 *          Format: /dnv-v2/vis-3-4a/411.1/H101/meta/qty-power/cnt-fuel.oil
 */

#pragma once

#include "CodebookName.h"
#include "GmodPath.h"
#include "LocalIdBuilder.h"
#include "MetadataTag.h"
#include "VisVersions.h"

#include <optional>
#include <string>
#include <vector>

namespace dnv::vista::sdk
{
    class ParsingErrors;

    /**
     * @class LocalId
     * @brief Immutable representation of a VIS Local ID
     * @details Represents a unique sensor identifier within a vessel according to the dnv-v2 naming rule
     *          LocalId is constructed from a LocalIdBuilder and provides read-only access to all components
     *          Once constructed, a LocalId cannot be modified. To create variations, access the builder
     *          via builder() and create a new LocalId instance
     *
     * @note LocalId follows the dnv-v2 naming rule format:
     *       /dnv-v2/vis-{version}/{primary-item}[/sec/{secondary-item}]/meta[/qty-{value}][/cnt-{value}]...
     */
    class LocalId final
    {
        friend class LocalIdBuilder;

    private:
        /**
         * @brief Construct LocalId from builder
         * @param builder LocalIdBuilder with validated state
         * @throws std::invalid_argument if builder is invalid (missing primary item)
         */
        explicit LocalId(const LocalIdBuilder& builder);

    public:
        LocalId() = delete;
        LocalId(const LocalId&) = default;
        LocalId(LocalId&&) noexcept = default;
        ~LocalId() = default;

        LocalId& operator=(const LocalId&) = default;
        LocalId& operator=(LocalId&&) noexcept = default;

        [[nodiscard]] inline bool operator==(const LocalId& other) const noexcept;

        /**
         * @brief Get ISO 19848 Annex C naming rule identifier
         * @return String view containing the naming rule identifier ("dnv-v2")
         */
        [[nodiscard]] static constexpr std::string_view namingRule() noexcept;

        /**
         * @brief Get the VIS version for this LocalId
         * @return The VisVersion enum value
         */
        [[nodiscard]] inline VisVersion version() const noexcept;

        /**
         * @brief Get the primary item (Gmod path)
         * @return Reference to the primary GmodPath
         */
        [[nodiscard]] inline const GmodPath& primaryItem() const noexcept;

        /**
         * @brief Get the secondary item (Gmod path)
         * @return Optional containing the secondary GmodPath if set, nullopt otherwise
         */
        [[nodiscard]] inline const std::optional<GmodPath>& secondaryItem() const noexcept;

        /**
         * @brief Get the verbose mode flag
         * @return True if verbose mode is enabled
         */
        [[nodiscard]] inline bool isVerboseMode() const noexcept;

        /**
         * @brief Get all metadata tags that are set
         * @return Vector of all non-empty metadata tags
         */
        [[nodiscard]] inline std::vector<MetadataTag> metadataTags() const;

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
         * @brief Check if LocalId has any custom (non-standard) metadata tags
         * @return True if any metadata tag is custom
         */
        [[nodiscard]] inline bool hasCustomTag() const noexcept;

        /**
         * @brief Get the builder that created this LocalId
         * @return Const reference to the internal builder
         */
        [[nodiscard]] inline const LocalIdBuilder& builder() const noexcept;

        /**
         * @brief Convert to string representation (lvalue-qualified)
         * @return String representation of the LocalId in dnv-v2 format
         */
        [[nodiscard]] inline std::string toString() const&;

        /**
         * @brief Convert to string representation (rvalue-qualified)
         * @return String representation with potential move optimization
         */
        [[nodiscard]] inline std::string toString() &&;

        /**
         * @brief Append string representation to a string
         * @param out String to append the representation to
         */
        inline void toString(std::string& out) const;

        /**
         * @brief Create LocalId from string representation
         * @param localIdStr String representation of LocalId in dnv-v2 format
         * @return Optional containing LocalId if parsing succeeded, nullopt otherwise
         * @note This method silently fails on parse errors. Use the overload with ParsingErrors for diagnostics
         * @note VIS version is extracted from the localIdStr (e.g., /dnv-v2/vis-3-4a/...)
         */
        [[nodiscard]] static std::optional<LocalId> fromString(std::string_view localIdStr) noexcept;

        /**
         * @brief Create LocalId from string representation with error reporting
         * @param localIdStr String representation of LocalId in dnv-v2 format
         * @param errors Output parameter filled with parsing errors if parsing fails
         * @return Optional containing LocalId if parsing succeeded, nullopt otherwise
         * @note VIS version is extracted from the localIdStr (e.g., /dnv-v2/vis-3-4a/...)
         */
        [[nodiscard]] static std::optional<LocalId> fromString(
            std::string_view localIdStr, ParsingErrors& errors) noexcept;

    private:
        LocalIdBuilder m_builder;
    };
} // namespace dnv::vista::sdk

#include "dnv/vista/sdk/detail/core/LocalId.inl"
