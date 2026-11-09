#include "ISO19848AnnexC.h"

#include <cassert>

namespace dnv::vista::sdk
{
    inline bool LocalId::operator==(const LocalId& other) const noexcept
    {
        return m_builder == other.m_builder;
    }

    inline constexpr std::string_view LocalId::namingRule() noexcept
    {
        return internal::iso19848::annexC::VersionedNamingRule;
    }

    inline VisVersion LocalId::version() const noexcept
    {
        assert(m_builder.version().has_value());
        return *m_builder.version();
    }

    inline bool LocalId::isVerboseMode() const noexcept
    {
        return m_builder.isVerboseMode();
    }

    inline const GmodPath& LocalId::primaryItem() const noexcept
    {
        assert(m_builder.primaryItem().has_value());
        return *m_builder.primaryItem();
    }

    inline const std::optional<GmodPath>& LocalId::secondaryItem() const noexcept
    {
        return m_builder.secondaryItem();
    }

    inline const std::optional<MetadataTag>& LocalId::quantity() const noexcept
    {
        return m_builder.quantity();
    }

    inline const std::optional<MetadataTag>& LocalId::content() const noexcept
    {
        return m_builder.content();
    }

    inline const std::optional<MetadataTag>& LocalId::calculation() const noexcept
    {
        return m_builder.calculation();
    }

    inline const std::optional<MetadataTag>& LocalId::state() const noexcept
    {
        return m_builder.state();
    }

    inline const std::optional<MetadataTag>& LocalId::command() const noexcept
    {
        return m_builder.command();
    }

    inline const std::optional<MetadataTag>& LocalId::type() const noexcept
    {
        return m_builder.type();
    }

    inline const std::optional<MetadataTag>& LocalId::position() const noexcept
    {
        return m_builder.position();
    }

    inline const std::optional<MetadataTag>& LocalId::detail() const noexcept
    {
        return m_builder.detail();
    }

    inline std::vector<MetadataTag> LocalId::metadataTags() const
    {
        return m_builder.metadataTags();
    }

    inline bool LocalId::hasCustomTag() const noexcept
    {
        return m_builder.hasCustomTag();
    }

    inline const LocalIdBuilder& LocalId::builder() const noexcept
    {
        return m_builder;
    }

    inline std::string LocalId::toString() const&
    {
        return m_builder.toString();
    }

    inline std::string LocalId::toString() &&
    {
        return std::move(m_builder).toString();
    }

    inline void LocalId::toString(StringBuilder& builder) const
    {
        m_builder.toString(builder);
    }

    namespace mqtt
    {
        inline bool LocalId::operator==(const LocalId& other) const noexcept
        {
            return m_builder == other.m_builder;
        }

        inline VisVersion LocalId::version() const noexcept
        {
            assert(m_builder.version().has_value());
            return *m_builder.version();
        }

        inline const GmodPath& LocalId::primaryItem() const noexcept
        {
            assert(m_builder.primaryItem().has_value());
            return *m_builder.primaryItem();
        }

        inline const std::optional<GmodPath>& LocalId::secondaryItem() const noexcept
        {
            return m_builder.secondaryItem();
        }

        inline const std::optional<MetadataTag>& LocalId::quantity() const noexcept
        {
            return m_builder.quantity();
        }

        inline const std::optional<MetadataTag>& LocalId::content() const noexcept
        {
            return m_builder.content();
        }

        inline const std::optional<MetadataTag>& LocalId::calculation() const noexcept
        {
            return m_builder.calculation();
        }

        inline const std::optional<MetadataTag>& LocalId::state() const noexcept
        {
            return m_builder.state();
        }

        inline const std::optional<MetadataTag>& LocalId::command() const noexcept
        {
            return m_builder.command();
        }

        inline const std::optional<MetadataTag>& LocalId::type() const noexcept
        {
            return m_builder.type();
        }

        inline const std::optional<MetadataTag>& LocalId::position() const noexcept
        {
            return m_builder.position();
        }

        inline const std::optional<MetadataTag>& LocalId::detail() const noexcept
        {
            return m_builder.detail();
        }

        inline const LocalIdBuilder& LocalId::builder() const noexcept
        {
            return m_builder;
        }

        inline std::string LocalId::toString() const&
        {
            StringBuilder s;
            toString(s);
            return s.toString();
        }

        inline void LocalId::toString(StringBuilder& builder) const
        {
            builder += internal::iso19848::annexC::VersionedNamingRule;
            builder += '/';
            builder += "vis-";
            builder += VisVersions::toString(version());
            builder += '/';

            appendPrimaryItem(builder);
            appendSecondaryItem(builder);
            appendMeta(builder, quantity());
            appendMeta(builder, content());
            appendMeta(builder, calculation());
            appendMeta(builder, state());
            appendMeta(builder, command());
            appendMeta(builder, type());
            appendMeta(builder, position());
            appendMeta(builder, detail());

            if (!builder.isEmpty() && builder.back() == '/')
            {
                builder.pop_back();
            }
        }

        inline void LocalId::appendPath(StringBuilder& builder, const GmodPath& path) const
        {
            path.toString(builder, m_separator);
            builder += '/';
        }

        inline void LocalId::appendPrimaryItem(StringBuilder& builder) const
        {
            assert(m_builder.primaryItem().has_value());
            appendPath(builder, *m_builder.primaryItem());
        }

        inline void LocalId::appendSecondaryItem(StringBuilder& builder) const
        {
            if (m_builder.secondaryItem().has_value())
            {
                appendPath(builder, m_builder.secondaryItem().value());
            }
            else
            {
                builder += "_/";
            }
        }

        inline void LocalId::appendMeta(StringBuilder& builder, const std::optional<MetadataTag>& tag) const
        {
            if (!tag.has_value())
            {
                builder += "_/";
            }
            else
            {
                tag.value().toString(builder);
            }
        }
    } // namespace mqtt
} // namespace dnv::vista::sdk
