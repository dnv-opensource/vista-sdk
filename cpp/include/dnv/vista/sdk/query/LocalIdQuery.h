/**
 * @file LocalIdQuery.h
 * @brief Immutable query pattern matching for LocalId instances
 * @details Provides LocalIdQuery and LocalIdQueryBuilder for declarative matching
 *          of LocalId instances. Supports matching on primary/secondary items via
 *          GmodPathQuery and metadata tags via MetadataTagsQuery
 */

#pragma once

#include <dnv/vista/sdk/Export.h>

#include "dnv/vista/sdk/core/LocalId.h"
#include "GmodPathQuery.h"
#include "MetadataTagsQuery.h"

#include <functional>
#include <optional>

namespace dnv::vista::sdk
{
    class LocalIdQueryBuilder;

    /**
     * @class LocalIdQuery
     * @brief Immutable query for matching LocalId instances
     * @details Orchestrates GmodPathQuery and MetadataTagsQuery to match LocalId instances
     *          Built using LocalIdQueryBuilder fluent API
     *          Once constructed, LocalIdQuery is immutable and thread-safe
     *
     * Query matching rules:
     * - Primary item: Must match if specified
     * - Secondary item: Must match if specified, or checked for presence/absence if required
     * - Metadata tags: Must match if specified
     * - All specified criteria must match (AND semantics)
     */
    class LocalIdQuery final
    {
        friend class LocalIdQueryBuilder;

    private:
        /**
         * @brief Construct from builder
         * @param builder Builder containing query configuration
         */
        DNV_VISTA_SDK_CPP_API explicit LocalIdQuery(const LocalIdQueryBuilder& builder);

    public:
        LocalIdQuery() = delete;
        LocalIdQuery(const LocalIdQuery&) = default;
        LocalIdQuery(LocalIdQuery&&) noexcept = default;
        ~LocalIdQuery() = default;

        LocalIdQuery& operator=(const LocalIdQuery&) = default;
        LocalIdQuery& operator=(LocalIdQuery&&) noexcept = default;

        /**
         * @brief Match a LocalId instance against this query
         * @param localId The LocalId to test against query criteria
         * @return True if localId matches all query criteria, false otherwise
         * @note If localId version < latest VIS version, automatic version conversion is attempted
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API bool match(const LocalId& localId) const;

        /**
         * @brief Match a LocalId string against this query
         * @param localIdStr LocalId string in dnv-v2 format
         * @return True if parsed LocalId matches all query criteria, false if parsing fails or no match
         */
        [[nodiscard]] inline bool match(std::string_view localIdStr) const;

    private:
        std::optional<GmodPathQuery> m_primaryItem;
        std::optional<GmodPathQuery> m_secondaryItem;
        std::optional<MetadataTagsQuery> m_tags;
        std::optional<bool> m_requireSecondaryItem; ///< true = must have, false = must not have, nullopt = don't care
    };

    /**
     * @class LocalIdQueryBuilder
     * @brief Fluent builder for constructing LocalIdQuery instances
     * @details Provides declarative API for building LocalId queries with support for:
     *          - Primary/secondary item matching via GmodPathQuery
     *          - Metadata tags matching via MetadataTagsQuery
     *          - Secondary item presence/absence requirements
     */
    class LocalIdQueryBuilder final
    {
        friend class LocalIdQuery;

        LocalIdQueryBuilder() = default;

    public:
        LocalIdQueryBuilder(const LocalIdQueryBuilder&) = default;
        LocalIdQueryBuilder(LocalIdQueryBuilder&&) noexcept = default;
        ~LocalIdQueryBuilder() = default;

        LocalIdQueryBuilder& operator=(const LocalIdQueryBuilder&) = default;
        LocalIdQueryBuilder& operator=(LocalIdQueryBuilder&&) noexcept = default;

        /**
         * @brief Create empty builder (matches all LocalIds)
         * @return New LocalIdQueryBuilder instance
         */
        [[nodiscard]] static inline LocalIdQueryBuilder create() noexcept;

        /**
         * @brief Create builder from existing LocalId
         * @param localId LocalId to build query from
         * @return LocalIdQueryBuilder configured to match the given LocalId exactly
         */
        [[nodiscard]] static inline LocalIdQueryBuilder from(const LocalId& localId) noexcept;

        /**
         * @brief Create builder from LocalId string
         * @param localIdStr LocalId string in dnv-v2 format
         * @return Optional containing builder if parsing succeeded, nullopt otherwise
         */
        [[nodiscard]] static inline std::optional<LocalIdQueryBuilder> from(std::string_view localIdStr) noexcept;

        /**
         * @brief Set primary item query using GmodPath
         * @param primaryItem GmodPath to match
         * @return New builder with primary item set
         */
        [[nodiscard]] inline LocalIdQueryBuilder withPrimaryItem(const GmodPath& primaryItem) const&;

        /**
         * @brief Set primary item query using GmodPath
         * @param primaryItem GmodPath to match
         * @return This builder with primary item set
         */
        [[nodiscard]] inline LocalIdQueryBuilder withPrimaryItem(const GmodPath& primaryItem) &&;

        /**
         * @brief Set primary item query
         * @param primaryItem GmodPathQuery to use
         * @return New builder with primary item set
         */
        [[nodiscard]] inline LocalIdQueryBuilder withPrimaryItem(const GmodPathQuery& primaryItem) const&;

        /**
         * @brief Set primary item query
         * @param primaryItem GmodPathQuery to use
         * @return This builder with primary item set
         */
        [[nodiscard]] inline LocalIdQueryBuilder withPrimaryItem(const GmodPathQuery& primaryItem) &&;

        /**
         * @brief Set primary item query using Nodes builder configuration
         * @param configure Function receiving GmodPathQueryBuilder::Nodes by value
         * @return New builder with primary item set
         */
        [[nodiscard]] inline LocalIdQueryBuilder withPrimaryItem(
            std::function<GmodPathQuery(GmodPathQueryBuilder::Nodes)> configure) const&;

        /**
         * @brief Set primary item query using Nodes builder configuration
         * @param configure Function receiving GmodPathQueryBuilder::Nodes by value
         * @return This builder with primary item set
         */
        [[nodiscard]] inline LocalIdQueryBuilder withPrimaryItem(
            std::function<GmodPathQuery(GmodPathQueryBuilder::Nodes)> configure) &&;

        /**
         * @brief Set primary item query using Path builder configuration
         * @param primaryItem Base GmodPath to configure
         * @param configure Function receiving GmodPathQueryBuilder::Path by value
         * @return New builder with primary item set
         */
        [[nodiscard]] inline LocalIdQueryBuilder withPrimaryItem(
            const GmodPath& primaryItem, std::function<GmodPathQuery(GmodPathQueryBuilder::Path)> configure) const&;

        /**
         * @brief Set primary item query using Path builder configuration
         * @param primaryItem Base GmodPath to configure
         * @param configure Function receiving GmodPathQueryBuilder::Path by value
         * @return This builder with primary item set
         */
        [[nodiscard]] inline LocalIdQueryBuilder withPrimaryItem(
            const GmodPath& primaryItem, std::function<GmodPathQuery(GmodPathQueryBuilder::Path)> configure) &&;

        /**
         * @brief Set secondary item query using GmodPath
         * @param secondaryItem GmodPath to match
         * @return New builder with secondary item set
         */
        [[nodiscard]] inline LocalIdQueryBuilder withSecondaryItem(const GmodPath& secondaryItem) const&;

        /**
         * @brief Set secondary item query using GmodPath
         * @param secondaryItem GmodPath to match
         * @return This builder with secondary item set
         */
        [[nodiscard]] inline LocalIdQueryBuilder withSecondaryItem(const GmodPath& secondaryItem) &&;

        /**
         * @brief Set secondary item query
         * @param secondaryItem GmodPathQuery to use
         * @return New builder with secondary item set
         */
        [[nodiscard]] inline LocalIdQueryBuilder withSecondaryItem(const GmodPathQuery& secondaryItem) const&;

        /**
         * @brief Set secondary item query
         * @param secondaryItem GmodPathQuery to use
         * @return This builder with secondary item set
         */
        [[nodiscard]] inline LocalIdQueryBuilder withSecondaryItem(const GmodPathQuery& secondaryItem) &&;

        /**
         * @brief Set secondary item query using Nodes builder configuration
         * @param configure Function receiving GmodPathQueryBuilder::Nodes by value
         * @return New builder with secondary item set
         */
        [[nodiscard]] inline LocalIdQueryBuilder withSecondaryItem(
            std::function<GmodPathQuery(GmodPathQueryBuilder::Nodes)> configure) const&;

        /**
         * @brief Set secondary item query using Nodes builder configuration
         * @param configure Function receiving GmodPathQueryBuilder::Nodes by value
         * @return This builder with secondary item set
         */
        [[nodiscard]] inline LocalIdQueryBuilder withSecondaryItem(
            std::function<GmodPathQuery(GmodPathQueryBuilder::Nodes)> configure) &&;

        /**
         * @brief Set secondary item query using Path builder configuration
         * @param secondaryItem Base GmodPath to configure
         * @param configure Function receiving GmodPathQueryBuilder::Path by value
         * @return New builder with secondary item set
         */
        [[nodiscard]] inline LocalIdQueryBuilder withSecondaryItem(
            const GmodPath& secondaryItem, std::function<GmodPathQuery(GmodPathQueryBuilder::Path)> configure) const&;

        /**
         * @brief Set secondary item query using Path builder configuration
         * @param secondaryItem Base GmodPath to configure
         * @param configure Function receiving GmodPathQueryBuilder::Path by value
         * @return This builder with secondary item set
         */
        [[nodiscard]] inline LocalIdQueryBuilder withSecondaryItem(
            const GmodPath& secondaryItem, std::function<GmodPathQuery(GmodPathQueryBuilder::Path)> configure) &&;

        /**
         * @brief Match any LocalId regardless of secondary item presence
         * @return New builder with secondary requirement cleared
         */
        [[nodiscard]] inline LocalIdQueryBuilder withAnySecondaryItem() const&;

        /**
         * @brief Match any LocalId regardless of secondary item presence
         * @return This builder with secondary requirement cleared
         */
        [[nodiscard]] inline LocalIdQueryBuilder withAnySecondaryItem() &&;

        /**
         * @brief Match only LocalIds without secondary item
         * @return New builder with secondary requirement set to false
         */
        [[nodiscard]] inline LocalIdQueryBuilder withoutSecondaryItem() const&;

        /**
         * @brief Match only LocalIds without secondary item
         * @return This builder with secondary requirement set to false
         */
        [[nodiscard]] inline LocalIdQueryBuilder withoutSecondaryItem() &&;

        /**
         * @brief Set metadata tags query
         * @param tags MetadataTagsQuery to use
         * @return New builder with tags set
         */
        [[nodiscard]] inline LocalIdQueryBuilder withTags(const MetadataTagsQuery& tags) const&;

        /**
         * @brief Set metadata tags query
         * @param tags MetadataTagsQuery to use
         * @return This builder with tags set
         */
        [[nodiscard]] inline LocalIdQueryBuilder withTags(const MetadataTagsQuery& tags) &&;

        /**
         * @brief Set metadata tags query using builder configuration
         * @param configure Function receiving MetadataTagsQueryBuilder reference
         * @return New builder with tags set
         */
        [[nodiscard]] inline LocalIdQueryBuilder withTags(
            std::function<MetadataTagsQuery(MetadataTagsQueryBuilder&)> configure) const&;

        /**
         * @brief Set metadata tags query using builder configuration
         * @param configure Function receiving MetadataTagsQueryBuilder reference
         * @return This builder with tags set
         */
        [[nodiscard]] inline LocalIdQueryBuilder withTags(
            std::function<MetadataTagsQuery(MetadataTagsQueryBuilder&)> configure) &&;

        /**
         * @brief Remove all location requirements from primary and secondary items
         * @return New builder with locations cleared
         * @note Only affects items configured as GmodPathQueryBuilder::Path (not Nodes)
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API LocalIdQueryBuilder withoutLocations() const&;

        /**
         * @brief Remove all location requirements from primary and secondary items
         * @return This builder with locations cleared
         * @note Only affects items configured as GmodPathQueryBuilder::Path (not Nodes)
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API LocalIdQueryBuilder withoutLocations() &&;

        /**
         * @brief Get the primary item path (if configured as Path query)
         * @return Pointer to GmodPath if primary item is a Path query, nullptr otherwise
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API const GmodPath* primaryItem() const noexcept;

        /**
         * @brief Get the secondary item path (if configured as Path query)
         * @return Pointer to GmodPath if secondary item is a Path query, nullptr otherwise
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API const GmodPath* secondaryItem() const noexcept;

        /**
         * @brief Build immutable LocalIdQuery
         * @return Constructed LocalIdQuery instance
         */
        [[nodiscard]] inline LocalIdQuery build() const;

        /**
         * @brief Get the metadata tags query
         * @return Optional containing tags query if set
         */
        [[nodiscard]] inline const std::optional<MetadataTagsQuery>& tags() const noexcept;

    private:
        /**
         * @brief Get the secondary item requirement flag
         * @return Optional: true = must have, false = must not have, nullopt = don't care
         */
        [[nodiscard]] inline std::optional<bool> requireSecondaryItem() const noexcept;

    private:
        std::optional<GmodPathQuery> m_primaryItem;
        std::optional<GmodPathQuery> m_secondaryItem;
        std::optional<MetadataTagsQuery> m_tags;
        std::optional<bool> m_requireSecondaryItem;
    };
} // namespace dnv::vista::sdk

#include "dnv/vista/sdk/detail/query/LocalIdQuery.inl"
