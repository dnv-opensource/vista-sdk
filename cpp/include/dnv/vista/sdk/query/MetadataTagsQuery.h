/**
 * @file MetadataTagsQuery.h
 * @brief Query API for matching LocalId metadata tags
 * @details Provides fluent API for building queries to filter LocalIds based on their metadata tags
 *          Supports exact matching (must have only these tags) or subset matching (must have at least these tags)
 */

#pragma once

#include "dnv/vista/sdk/core/CodebookName.h"
#include "dnv/vista/sdk/core/LocalId.h"
#include "dnv/vista/sdk/core/MetadataTag.h"

#include <unordered_map>

namespace dnv::vista::sdk
{
    class LocalId;
    class MetadataTagsQuery;

    /**
     * @class MetadataTagsQueryBuilder
     * @brief Fluent builder for MetadataTagsQuery
     * @details Allows specifying metadata tags to match and whether to allow additional tags
     *          Two matching modes:
     *          - Subset match (default): LocalId must have AT LEAST the specified tags (can have more)
     *          - Exact match: LocalId must have EXACTLY the specified tags (no more, no less)
     */
    class MetadataTagsQueryBuilder final
    {
        friend class MetadataTagsQuery;

    private:
        MetadataTagsQueryBuilder() = default;

    public:
        MetadataTagsQueryBuilder(const MetadataTagsQueryBuilder&) = default;
        MetadataTagsQueryBuilder(MetadataTagsQueryBuilder&&) noexcept = default;
        ~MetadataTagsQueryBuilder() = default;

        MetadataTagsQueryBuilder& operator=(const MetadataTagsQueryBuilder&) = default;
        MetadataTagsQueryBuilder& operator=(MetadataTagsQueryBuilder&&) noexcept = default;

        /**
         * @brief Create empty builder (matches any LocalId by default)
         * @return New empty builder
         */
        [[nodiscard]] static MetadataTagsQueryBuilder create();

        /**
         * @brief Create builder from existing LocalId
         * @param localId LocalId to copy tags from
         * @param allowOtherTags If true, allow additional tags. If false, exact match only
         * @return Builder initialized with localId's tags
         */
        [[nodiscard]] static MetadataTagsQueryBuilder from(const LocalId& localId, bool allowOtherTags = true);

        /**
         * @brief Add a metadata tag to match
         * @param name Codebook name
         * @param value Tag value
         * @return New builder with tag added
         * @note If a tag for this codebook already exists, it will be replaced
         */
        [[nodiscard]] inline MetadataTagsQueryBuilder withTag(CodebookName name, const std::string& value) const&;

        /**
         * @brief Add a metadata tag to match
         * @param name Codebook name
         * @param value Tag value
         * @return New builder with tag added
         * @note If a tag for this codebook already exists, it will be replaced
         */
        [[nodiscard]] inline MetadataTagsQueryBuilder withTag(CodebookName name, const std::string& value) &&;

        /**
         * @brief Add a metadata tag to match
         * @param tag MetadataTag to match
         * @return New builder with tag added
         * @note If a tag for this codebook already exists, it will be replaced
         */
        [[nodiscard]] inline MetadataTagsQueryBuilder withTag(const MetadataTag& tag) const&;

        /**
         * @brief Add a metadata tag to match
         * @param tag MetadataTag to match
         * @return New builder with tag added
         * @note If a tag for this codebook already exists, it will be replaced
         */
        [[nodiscard]] inline MetadataTagsQueryBuilder withTag(const MetadataTag& tag) &&;

        /**
         * @brief Set whether to allow tags not specified in the query
         * @param allowOthers If true (default), LocalId can have extra tags (subset matching)
         *                    If false, LocalId must have EXACTLY the specified tags (exact matching)
         * @return New builder with setting updated
         */
        [[nodiscard]] inline MetadataTagsQueryBuilder withAllowOtherTags(bool allowOthers) const&;

        /**
         * @brief Set whether to allow tags not specified in the query
         * @param allowOthers If true (default), LocalId can have extra tags (subset matching)
         *                    If false, LocalId must have EXACTLY the specified tags (exact matching)
         * @return New builder with setting updated
         */
        [[nodiscard]] inline MetadataTagsQueryBuilder withAllowOtherTags(bool allowOthers) &&;

        /**
         * @brief Build immutable query
         * @return MetadataTagsQuery instance
         */
        [[nodiscard]] inline MetadataTagsQuery build() const;

    private:
        std::unordered_map<CodebookName, MetadataTag> m_tags;
        bool m_matchExact = false;
    };

    /**
     * @class MetadataTagsQuery
     * @brief Immutable query for matching LocalId metadata tags
     * @details Created via MetadataTagsQueryBuilder. Provides match() method to test LocalIds
     */
    class MetadataTagsQuery final
    {
        friend class MetadataTagsQueryBuilder;

    private:
        /**
         * @brief Construct from a builder
         * @param builder Builder this query is created from
         */
        explicit MetadataTagsQuery(MetadataTagsQueryBuilder builder);

    public:
        MetadataTagsQuery(const MetadataTagsQuery&) = default;
        MetadataTagsQuery(MetadataTagsQuery&&) noexcept = default;
        ~MetadataTagsQuery() = default;

        MetadataTagsQuery& operator=(const MetadataTagsQuery&) = default;
        MetadataTagsQuery& operator=(MetadataTagsQuery&&) noexcept = default;

        /**
         * @brief Test whether a LocalId matches this query
         * @param localId LocalId to test
         * @return true if localId matches the query criteria
         */
        [[nodiscard]] bool match(const LocalId& localId) const;

        /**
         * @brief Get the builder this query was created from
         * @return The originating builder
         * @note Used to reconfigure/extend an existing query (e.g. adding more tags) without
         *       losing previously set state
         */
        [[nodiscard]] const MetadataTagsQueryBuilder& builder() const noexcept;

    private:
        MetadataTagsQueryBuilder m_builder;
    };
} // namespace dnv::vista::sdk

#include "dnv/vista/sdk/detail/query/MetadataTagsQuery.inl"
