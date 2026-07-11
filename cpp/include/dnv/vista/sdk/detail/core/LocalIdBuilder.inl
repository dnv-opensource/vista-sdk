#include "ISO19848AnnexC.h"

namespace dnv::vista::sdk
{
    inline bool LocalIdBuilder::operator==(const LocalIdBuilder& other) const noexcept
    {
        if (m_visVersion != other.m_visVersion)
        {
            return false;
        }

        return m_verboseMode == other.m_verboseMode && m_primaryItem == other.m_primaryItem &&
               m_secondaryItem == other.m_secondaryItem && m_quantity == other.m_quantity &&
               m_content == other.m_content && m_calculation == other.m_calculation && m_state == other.m_state &&
               m_command == other.m_command && m_type == other.m_type && m_position == other.m_position &&
               m_detail == other.m_detail;
    }

    inline constexpr std::string_view LocalIdBuilder::namingRule() noexcept
    {
        return internal::iso19848::annexC::VersionedNamingRule;
    }

    inline std::optional<VisVersion> LocalIdBuilder::version() const noexcept
    {
        return m_visVersion;
    }

    inline bool LocalIdBuilder::isVerboseMode() const noexcept
    {
        return m_verboseMode;
    }

    inline bool LocalIdBuilder::isValid() const noexcept
    {
        return m_visVersion.has_value() && m_primaryItem.has_value() &&
               (m_quantity.has_value() || m_content.has_value() || m_calculation.has_value() || m_state.has_value() ||
                m_command.has_value() || m_type.has_value() || m_position.has_value() || m_detail.has_value());
    }

    inline bool LocalIdBuilder::isEmpty() const noexcept
    {
        return !m_primaryItem.has_value() && !m_secondaryItem.has_value() && isEmptyMetadata();
    }

    inline bool LocalIdBuilder::isEmptyMetadata() const noexcept
    {
        return !m_quantity.has_value() && !m_content.has_value() && !m_calculation.has_value() &&
               !m_state.has_value() && !m_command.has_value() && !m_type.has_value() && !m_position.has_value() &&
               !m_detail.has_value();
    }

    inline bool LocalIdBuilder::hasCustomTag() const noexcept
    {
        return (m_quantity.has_value() && m_quantity->isCustom()) || (m_content.has_value() && m_content->isCustom()) ||
               (m_calculation.has_value() && m_calculation->isCustom()) ||
               (m_state.has_value() && m_state->isCustom()) || (m_command.has_value() && m_command->isCustom()) ||
               (m_type.has_value() && m_type->isCustom()) || (m_position.has_value() && m_position->isCustom()) ||
               (m_detail.has_value() && m_detail->isCustom());
    }

    inline const std::optional<GmodPath>& LocalIdBuilder::primaryItem() const noexcept
    {
        return m_primaryItem;
    }

    inline const std::optional<GmodPath>& LocalIdBuilder::secondaryItem() const noexcept
    {
        return m_secondaryItem;
    }

    inline std::vector<MetadataTag> LocalIdBuilder::metadataTags() const noexcept
    {
        std::vector<MetadataTag> tags;
        tags.reserve(8);

        if (m_quantity.has_value())
        {
            tags.push_back(*m_quantity);
        }
        if (m_content.has_value())
        {
            tags.push_back(*m_content);
        }
        if (m_calculation.has_value())
        {
            tags.push_back(*m_calculation);
        }
        if (m_state.has_value())
        {
            tags.push_back(*m_state);
        }
        if (m_command.has_value())
        {
            tags.push_back(*m_command);
        }
        if (m_type.has_value())
        {
            tags.push_back(*m_type);
        }
        if (m_position.has_value())
        {
            tags.push_back(*m_position);
        }
        if (m_detail.has_value())
        {
            tags.push_back(*m_detail);
        }
        return tags;
    }

    inline const std::optional<MetadataTag>& LocalIdBuilder::quantity() const noexcept
    {
        return m_quantity;
    }

    inline const std::optional<MetadataTag>& LocalIdBuilder::content() const noexcept
    {
        return m_content;
    }

    inline const std::optional<MetadataTag>& LocalIdBuilder::calculation() const noexcept
    {
        return m_calculation;
    }

    inline const std::optional<MetadataTag>& LocalIdBuilder::state() const noexcept
    {
        return m_state;
    }

    inline const std::optional<MetadataTag>& LocalIdBuilder::command() const noexcept
    {
        return m_command;
    }

    inline const std::optional<MetadataTag>& LocalIdBuilder::type() const noexcept
    {
        return m_type;
    }

    inline const std::optional<MetadataTag>& LocalIdBuilder::position() const noexcept
    {
        return m_position;
    }

    inline const std::optional<MetadataTag>& LocalIdBuilder::detail() const noexcept
    {
        return m_detail;
    }

    inline LocalIdBuilder LocalIdBuilder::create(VisVersion visVersion) noexcept
    {
        LocalIdBuilder builder;
        builder.m_visVersion = visVersion;
        return builder;
    }

    inline LocalIdBuilder LocalIdBuilder::withVisVersion(VisVersion version) const&
    {
        LocalIdBuilder builder{ *this };
        builder.m_visVersion = version;
        return builder;
    }

    inline LocalIdBuilder LocalIdBuilder::withVisVersion(VisVersion version) && noexcept
    {
        m_visVersion = version;
        return std::move(*this);
    }

    inline LocalIdBuilder LocalIdBuilder::withVisVersion(std::string_view visVersionStr) const&
    {
        VisVersion version;
        if (!VisVersions::fromString(visVersionStr, version))

            throw std::invalid_argument{ "Invalid VisVersion string" };

        return withVisVersion(version);
    }

    inline LocalIdBuilder LocalIdBuilder::withVisVersion(std::string_view visVersionStr) &&
    {
        VisVersion version;
        if (!VisVersions::fromString(visVersionStr, version))
            throw std::invalid_argument{ "Invalid VisVersion string" };

        return std::move(*this).withVisVersion(version);
    }

    inline LocalIdBuilder LocalIdBuilder::withoutVisVersion() const&
    {
        LocalIdBuilder builder{ *this };
        builder.m_visVersion.reset();
        return builder;
    }

    inline LocalIdBuilder LocalIdBuilder::withoutVisVersion() && noexcept
    {
        m_visVersion.reset();
        return std::move(*this);
    }

    inline LocalIdBuilder LocalIdBuilder::withPrimaryItem(const GmodPath& path) const&
    {
        LocalIdBuilder builder{ *this };
        builder.m_primaryItem = path;
        return builder;
    }

    inline LocalIdBuilder LocalIdBuilder::withPrimaryItem(const GmodPath& path) &&
    {
        m_primaryItem = path;
        return std::move(*this);
    }

    inline LocalIdBuilder LocalIdBuilder::withPrimaryItem(const std::optional<GmodPath>& path) const&
    {
        if (!path.has_value())
        {
            throw std::invalid_argument{ "Primary item cannot be null" };
        }
        return withPrimaryItem(*path);
    }

    inline LocalIdBuilder LocalIdBuilder::withPrimaryItem(const std::optional<GmodPath>& path) &&
    {
        if (!path.has_value())
        {
            throw std::invalid_argument{ "Primary item cannot be null" };
        }
        return std::move(*this).withPrimaryItem(*path);
    }

    inline LocalIdBuilder LocalIdBuilder::withoutPrimaryItem() const&
    {
        LocalIdBuilder builder{ *this };
        builder.m_primaryItem.reset();
        return builder;
    }

    inline LocalIdBuilder LocalIdBuilder::withoutPrimaryItem() && noexcept
    {
        m_primaryItem.reset();
        return std::move(*this);
    }

    inline LocalIdBuilder LocalIdBuilder::withSecondaryItem(const GmodPath& path) const&
    {
        LocalIdBuilder builder{ *this };
        builder.m_secondaryItem = path;
        return builder;
    }

    inline LocalIdBuilder LocalIdBuilder::withSecondaryItem(const GmodPath& path) &&
    {
        m_secondaryItem = path;
        return std::move(*this);
    }

    inline LocalIdBuilder LocalIdBuilder::withSecondaryItem(const std::optional<GmodPath>& path) const&
    {
        if (!path.has_value())
        {
            return withoutSecondaryItem();
        }
        return withSecondaryItem(*path);
    }

    inline LocalIdBuilder LocalIdBuilder::withSecondaryItem(const std::optional<GmodPath>& path) &&
    {
        if (!path.has_value())
        {
            return std::move(*this).withoutSecondaryItem();
        }
        return std::move(*this).withSecondaryItem(*path);
    }

    inline LocalIdBuilder LocalIdBuilder::withoutSecondaryItem() const&
    {
        LocalIdBuilder builder{ *this };
        builder.m_secondaryItem.reset();
        return builder;
    }

    inline LocalIdBuilder LocalIdBuilder::withoutSecondaryItem() && noexcept
    {
        m_secondaryItem.reset();
        return std::move(*this);
    }

    inline LocalIdBuilder LocalIdBuilder::withMetadataTag(const MetadataTag& tag) const&
    {
        LocalIdBuilder builder{ *this };
        return std::move(builder).withMetadataTag(tag);
    }

    inline LocalIdBuilder LocalIdBuilder::withMetadataTag(const MetadataTag& tag) &&
    {
        switch (tag.name())
        {
            case CodebookName::Quantity:
            {
                m_quantity.emplace(tag);
                break;
            }
            case CodebookName::Content:
            {
                m_content.emplace(tag);
                break;
            }
            case CodebookName::Calculation:
            {
                m_calculation.emplace(tag);
                break;
            }
            case CodebookName::State:
            {
                m_state.emplace(tag);
                break;
            }
            case CodebookName::Command:
            {
                m_command.emplace(tag);
                break;
            }
            case CodebookName::Type:
            {
                m_type.emplace(tag);
                break;
            }
            case CodebookName::Position:
            {
                m_position.emplace(tag);
                break;
            }
            case CodebookName::Detail:
            {
                m_detail.emplace(tag);
                break;
            }
            default:
            {
                throw std::invalid_argument{ "Invalid metadata tag codebook name for LocalId" };
            }
        }

        return std::move(*this);
    }

    inline LocalIdBuilder LocalIdBuilder::withMetadataTag(const std::optional<MetadataTag>& tag) const&
    {
        if (!tag.has_value())
        {
            return *this;
        }
        return withMetadataTag(*tag);
    }

    inline LocalIdBuilder LocalIdBuilder::withMetadataTag(const std::optional<MetadataTag>& tag) &&
    {
        if (!tag.has_value())
        {
            return std::move(*this);
        }
        return std::move(*this).withMetadataTag(*tag);
    }

    inline LocalIdBuilder LocalIdBuilder::withoutMetadataTag(CodebookName name) const&
    {
        LocalIdBuilder builder{ *this };
        return std::move(builder).withoutMetadataTag(name);
    }

    inline LocalIdBuilder LocalIdBuilder::withoutMetadataTag(CodebookName name) && noexcept
    {
        switch (name)
        {
            case CodebookName::Quantity:
            {
                m_quantity.reset();
                break;
            }
            case CodebookName::Content:
            {
                m_content.reset();
                break;
            }
            case CodebookName::Calculation:
            {
                m_calculation.reset();
                break;
            }
            case CodebookName::State:
            {
                m_state.reset();
                break;
            }
            case CodebookName::Command:
            {
                m_command.reset();
                break;
            }
            case CodebookName::Type:
            {
                m_type.reset();
                break;
            }
            case CodebookName::Position:
            {
                m_position.reset();
                break;
            }
            case CodebookName::Detail:
            {
                m_detail.reset();
                break;
            }
            default:
            {
                break;
            }
        }

        return std::move(*this);
    }

    inline LocalIdBuilder LocalIdBuilder::withVerboseMode(bool verbose) const&
    {
        LocalIdBuilder builder{ *this };
        builder.m_verboseMode = verbose;
        return builder;
    }

    inline LocalIdBuilder LocalIdBuilder::withVerboseMode(bool verbose) && noexcept
    {
        m_verboseMode = verbose;
        return std::move(*this);
    }

    inline std::string LocalIdBuilder::toString() const
    {
        std::string out;
        toString(out);
        return out;
    }
} // namespace dnv::vista::sdk
