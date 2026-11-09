#include "dnv/vista/sdk/query/MetadataTagsQuery.h"

#include "dnv/vista/sdk/core/LocalId.h"

namespace dnv::vista::sdk
{
    MetadataTagsQuery::MetadataTagsQuery(MetadataTagsQueryBuilder builder)
        : m_builder{ std::move(builder) }
    {}

    bool MetadataTagsQuery::match(const LocalId& localId) const
    {
        const auto& tags = m_builder.m_tags;
        const bool matchExact = m_builder.m_matchExact;

        std::unordered_map<CodebookName, MetadataTag> metadataTags;
        for (const auto& tag : localId.metadataTags())
        {
            metadataTags.emplace(tag.name(), tag);
        }

        if (!tags.empty())
        {
            if (matchExact)
            {
                if (tags.size() != metadataTags.size())
                {
                    return false;
                }

                for (const auto& [name, tag] : tags)
                {
                    auto it = metadataTags.find(name);
                    if (it == metadataTags.end() || it->second != tag)
                    {
                        return false;
                    }
                }
                return true;
            }
            else
            {
                // Subset match: must have AT LEAST these tags (can have more)
                for (const auto& [name, tag] : tags)
                {
                    auto it = metadataTags.find(name);
                    if (it == metadataTags.end())
                    {
                        return false;
                    }
                    if (it->second != tag)
                    {
                        return false;
                    }
                }
                return true;
            }
        }
        else
        {
            // No tags specified:
            // - If matchExact (allowOthers=false): match only LocalIds with no tags
            // - If !matchExact (allowOthers=true): match any LocalId
            return !matchExact;
        }
    }

    const MetadataTagsQueryBuilder& MetadataTagsQuery::builder() const noexcept
    {
        return m_builder;
    }

    MetadataTagsQueryBuilder MetadataTagsQueryBuilder::from(const LocalId& localId, bool allowOtherTags)
    {
        MetadataTagsQueryBuilder builder;
        for (const auto& tag : localId.metadataTags())
        {
            builder = std::move(builder).withTag(tag);
        }
        return std::move(builder).withAllowOtherTags(allowOtherTags);
    }
} // namespace dnv::vista::sdk
