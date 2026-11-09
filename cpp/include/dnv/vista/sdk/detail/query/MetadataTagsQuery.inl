namespace dnv::vista::sdk
{
    inline MetadataTagsQueryBuilder MetadataTagsQueryBuilder::create()
    {
        return MetadataTagsQueryBuilder{};
    }

    inline MetadataTagsQueryBuilder MetadataTagsQueryBuilder::withTag(
        CodebookName name, const std::string& value) const&
    {
        return withTag(MetadataTag{ name, value });
    }

    inline MetadataTagsQueryBuilder MetadataTagsQueryBuilder::withTag(CodebookName name, const std::string& value) &&
    {
        return std::move(*this).withTag(MetadataTag{ name, value });
    }

    inline MetadataTagsQueryBuilder MetadataTagsQueryBuilder::withTag(const MetadataTag& tag) const&
    {
        MetadataTagsQueryBuilder builder{ *this };
        builder.m_tags.insert_or_assign(tag.name(), tag);
        return builder;
    }

    inline MetadataTagsQueryBuilder MetadataTagsQueryBuilder::withTag(const MetadataTag& tag) &&
    {
        m_tags.insert_or_assign(tag.name(), tag);
        return std::move(*this);
    }

    inline MetadataTagsQueryBuilder MetadataTagsQueryBuilder::withAllowOtherTags(bool allowOthers) const&
    {
        MetadataTagsQueryBuilder builder{ *this };
        builder.m_matchExact = !allowOthers;
        return builder;
    }

    inline MetadataTagsQueryBuilder MetadataTagsQueryBuilder::withAllowOtherTags(bool allowOthers) &&
    {
        m_matchExact = !allowOthers;
        return std::move(*this);
    }

    inline MetadataTagsQuery MetadataTagsQueryBuilder::build() const
    {
        return MetadataTagsQuery{ *this };
    }
} // namespace dnv::vista::sdk
