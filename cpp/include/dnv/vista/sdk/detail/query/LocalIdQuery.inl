namespace dnv::vista::sdk
{
    inline bool LocalIdQuery::match(std::string_view localIdStr) const
    {
        auto localId = LocalId::fromString(localIdStr);
        if (!localId.has_value())
        {
            return false;
        }
        return match(localId.value());
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::create() noexcept
    {
        return LocalIdQueryBuilder{};
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::from(const LocalId& localId) noexcept
    {
        LocalIdQueryBuilder builder;

        builder.m_primaryItem = GmodPathQueryBuilder::Path::from(localId.primaryItem()).build();

        if (localId.secondaryItem().has_value())
        {
            builder.m_secondaryItem = GmodPathQueryBuilder::Path::from(localId.secondaryItem().value()).build();
            builder.m_requireSecondaryItem = true;
        }
        else
        {
            builder.m_requireSecondaryItem = false;
        }

        builder.m_tags = MetadataTagsQueryBuilder::from(localId).build();

        return builder;
    }

    inline std::optional<LocalIdQueryBuilder> LocalIdQueryBuilder::from(std::string_view localIdStr) noexcept
    {
        auto localId = LocalId::fromString(localIdStr);
        if (!localId.has_value())
        {
            return std::nullopt;
        }
        return from(localId.value());
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withPrimaryItem(const GmodPath& primaryItem) const&
    {
        LocalIdQueryBuilder builder{ *this };
        return std::move(builder).withPrimaryItem(primaryItem);
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withPrimaryItem(const GmodPath& primaryItem) &&
    {
        m_primaryItem = GmodPathQueryBuilder::from(primaryItem).build();
        return std::move(*this);
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withPrimaryItem(const GmodPathQuery& primaryItem) const&
    {
        LocalIdQueryBuilder builder{ *this };
        return std::move(builder).withPrimaryItem(primaryItem);
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withPrimaryItem(const GmodPathQuery& primaryItem) &&
    {
        m_primaryItem = primaryItem;
        return std::move(*this);
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withPrimaryItem(
        std::function<GmodPathQuery(GmodPathQueryBuilder::Nodes)> configure) const&
    {
        LocalIdQueryBuilder builder{ *this };
        return std::move(builder).withPrimaryItem(configure);
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withPrimaryItem(
        std::function<GmodPathQuery(GmodPathQueryBuilder::Nodes)> configure) &&
    {
        auto nodes = GmodPathQueryBuilder::create();
        m_primaryItem = configure(std::move(nodes));
        return std::move(*this);
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withPrimaryItem(
        const GmodPath& primaryItem, std::function<GmodPathQuery(GmodPathQueryBuilder::Path)> configure) const&
    {
        LocalIdQueryBuilder builder{ *this };
        return std::move(builder).withPrimaryItem(primaryItem, configure);
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withPrimaryItem(
        const GmodPath& primaryItem, std::function<GmodPathQuery(GmodPathQueryBuilder::Path)> configure) &&
    {
        auto path = GmodPathQueryBuilder::from(primaryItem);
        m_primaryItem = configure(std::move(path));
        return std::move(*this);
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withSecondaryItem(const GmodPath& secondaryItem) const&
    {
        LocalIdQueryBuilder builder{ *this };
        return std::move(builder).withSecondaryItem(secondaryItem);
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withSecondaryItem(const GmodPath& secondaryItem) &&
    {
        m_secondaryItem = GmodPathQueryBuilder::from(secondaryItem).build();
        m_requireSecondaryItem = true;
        return std::move(*this);
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withSecondaryItem(const GmodPathQuery& secondaryItem) const&
    {
        LocalIdQueryBuilder builder{ *this };
        return std::move(builder).withSecondaryItem(secondaryItem);
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withSecondaryItem(const GmodPathQuery& secondaryItem) &&
    {
        m_secondaryItem = secondaryItem;
        m_requireSecondaryItem = true;
        return std::move(*this);
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withSecondaryItem(
        std::function<GmodPathQuery(GmodPathQueryBuilder::Nodes)> configure) const&
    {
        LocalIdQueryBuilder builder{ *this };
        return std::move(builder).withSecondaryItem(configure);
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withSecondaryItem(
        std::function<GmodPathQuery(GmodPathQueryBuilder::Nodes)> configure) &&
    {
        auto nodes = GmodPathQueryBuilder::create();
        m_secondaryItem = configure(std::move(nodes));
        m_requireSecondaryItem = true;
        return std::move(*this);
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withSecondaryItem(
        const GmodPath& secondaryItem, std::function<GmodPathQuery(GmodPathQueryBuilder::Path)> configure) const&
    {
        LocalIdQueryBuilder builder{ *this };
        return std::move(builder).withSecondaryItem(secondaryItem, configure);
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withSecondaryItem(
        const GmodPath& secondaryItem, std::function<GmodPathQuery(GmodPathQueryBuilder::Path)> configure) &&
    {
        auto path = GmodPathQueryBuilder::from(secondaryItem);
        m_secondaryItem = configure(std::move(path));
        m_requireSecondaryItem = true;
        return std::move(*this);
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withAnySecondaryItem() const&
    {
        LocalIdQueryBuilder builder{ *this };
        return std::move(builder).withAnySecondaryItem();
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withAnySecondaryItem() &&
    {
        m_secondaryItem = std::nullopt;
        m_requireSecondaryItem = std::nullopt;
        return std::move(*this);
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withoutSecondaryItem() const&
    {
        LocalIdQueryBuilder builder{ *this };
        return std::move(builder).withoutSecondaryItem();
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withoutSecondaryItem() &&
    {
        m_secondaryItem = std::nullopt;
        m_requireSecondaryItem = false;
        return std::move(*this);
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withTags(const MetadataTagsQuery& tags) const&
    {
        LocalIdQueryBuilder builder{ *this };
        return std::move(builder).withTags(tags);
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withTags(const MetadataTagsQuery& tags) &&
    {
        m_tags = tags;
        return std::move(*this);
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withTags(
        std::function<MetadataTagsQuery(MetadataTagsQueryBuilder&)> configure) const&
    {
        LocalIdQueryBuilder builder{ *this };
        return std::move(builder).withTags(configure);
    }

    inline LocalIdQueryBuilder LocalIdQueryBuilder::withTags(
        std::function<MetadataTagsQuery(MetadataTagsQueryBuilder&)> configure) &&
    {
        auto builder = m_tags.has_value() ? m_tags->builder() : MetadataTagsQueryBuilder::create();
        m_tags = configure(builder);
        return std::move(*this);
    }

    inline LocalIdQuery LocalIdQueryBuilder::build() const
    {
        return LocalIdQuery(*this);
    }

    inline const std::optional<MetadataTagsQuery>& LocalIdQueryBuilder::tags() const noexcept
    {
        return m_tags;
    }

    inline std::optional<bool> LocalIdQueryBuilder::requireSecondaryItem() const noexcept
    {
        return m_requireSecondaryItem;
    }
} // namespace dnv::vista::sdk
