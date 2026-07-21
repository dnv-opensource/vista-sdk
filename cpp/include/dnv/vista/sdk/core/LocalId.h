/**
 * @file LocalId.h
 * @brief Immutable VIS Local ID representation
 * @details Provides an immutable LocalId class representing a unique sensor identifier
 *          according to the dnv-v2 naming rule. LocalId combines GmodPath(s) with
 *          metadata tags to create globally unique sensor identifiers within a vessel
 *          Format: /dnv-v2/vis-3-4a/411.1/H101/meta/qty-power/cnt-fuel.oil
 */

#pragma once

#include "dnv/vista/sdk/utils/StringBuilder.h"
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
         * @brief Append string representation to a buffer
         * @param builder Builder to append to
         */
        inline void toString(StringBuilder& builder) const;

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

    namespace mqtt
    {
        /**
         * @brief MQTT-compatible LocalId implementation
         * @details Provides MQTT-specific formatting over the same validated builder state as
         *          sdk::LocalId:
         *          - Underscores instead of slashes in Gmod paths for topic compatibility
         *          - No leading slash to match MQTT topic conventions
         *          - No "meta/" prefix section for cleaner IoT topics
         *          - Placeholder handling for missing components
         *
         * @note LocalId follows the MQTT topic format:
         *       dnv-v2/vis-{version}/{primary-item}[_{secondary-item}][/qty-{value}][/cnt-{value}]...
         *       Missing components are replaced by underscore placeholders
         */
        class LocalId final
        {
        public:
            /**
             * @brief Constructs MQTT LocalId from validated LocalIdBuilder
             * @param builder Valid LocalIdBuilder instance
             * @throws std::invalid_argument If builder is invalid or empty
             */
            explicit LocalId(const sdk::LocalIdBuilder& builder);

            LocalId() = delete;
            LocalId(const LocalId& other) = default;
            LocalId(LocalId&& other) noexcept = default;
            ~LocalId() = default;

            LocalId& operator=(const LocalId&) = default;
            LocalId& operator=(LocalId&& other) noexcept = default;

            [[nodiscard]] inline bool operator==(const LocalId& other) const noexcept;

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
             * @brief Get the builder that created this LocalId
             * @return Const reference to the internal builder
             */
            [[nodiscard]] inline const LocalIdBuilder& builder() const noexcept;

            /**
             * @brief Converts LocalId to MQTT-compatible topic string
             * @details Provides MQTT-specific formatting:
             *          - No leading slash
             *          - Underscores instead of slashes in paths
             *          - No "meta/" section
             *          - Placeholders for missing components
             * @return MQTT-compatible Local ID topic string
             */
            [[nodiscard]] inline std::string toString() const&;

            /**
             * @brief Append MQTT topic string to a string
             * @param builder Builder to append the MQTT topic to
             */
            inline void toString(StringBuilder& builder) const;

        private:
            /** @brief Internal separator for MQTT paths */
            static constexpr char m_separator = '_';

            /**
             * @brief Appends Gmod path to builder with MQTT formatting
             * @param builder Builder to append the formatted path to
             * @param path GmodPath to append with underscore separators
             */
            inline void appendPath(StringBuilder& builder, const GmodPath& path) const;

            /**
             * @brief Appends primary item to builder in MQTT format
             * @param builder Builder to append the primary item to
             */
            inline void appendPrimaryItem(StringBuilder& builder) const;

            /**
             * @brief Appends secondary item or placeholder to builder in MQTT format
             * @param builder Builder to append the secondary item to
             */
            inline void appendSecondaryItem(StringBuilder& builder) const;

            /**
             * @brief Appends metadata tag or placeholder to builder in MQTT format
             * @details Appends the metadata tag value if present, otherwise appends an underscore placeholder
             *          Always appends a trailing forward slash for MQTT topic formatting
             * @param builder Builder to append the tag (or placeholder) to
             * @param tag Optional metadata tag to append (or placeholder if empty)
             */
            inline void appendMeta(StringBuilder& builder, const std::optional<MetadataTag>& tag) const;

            LocalIdBuilder m_builder;
        };
    } // namespace mqtt
} // namespace dnv::vista::sdk

#include "dnv/vista/sdk/detail/core/LocalId.inl"
