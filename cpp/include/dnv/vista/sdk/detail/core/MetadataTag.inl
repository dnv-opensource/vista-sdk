#include <stdexcept>

namespace dnv::vista::sdk
{
    inline MetadataTag::MetadataTag(CodebookName name, std::string value, bool isCustom) noexcept
        : m_name{ name },
          m_custom{ isCustom },
          m_value{ std::move(value) }
    {}

    inline bool MetadataTag::operator==(const MetadataTag& other) const
    {
        if (m_name != other.m_name)
        {
            throw std::invalid_argument{ "Cannot compare MetadataTags with different CodebookNames" };
        }
        return m_value == other.m_value;
    }

    inline CodebookName MetadataTag::name() const noexcept
    {
        return m_name;
    }

    inline const std::string& MetadataTag::value() const noexcept
    {
        return m_value;
    }

    inline char MetadataTag::prefix() const noexcept
    {
        return m_custom ? '~' : '-';
    }

    inline bool MetadataTag::isCustom() const noexcept
    {
        return m_custom;
    }

    inline std::string MetadataTag::toString() const
    {
        std::string result;
        toString(result, '\0');
        result.pop_back();
        return result;
    }

    inline void MetadataTag::toString(std::string& out, char separator) const
    {
        out += CodebookNames::toPrefix(m_name);
        out += (m_custom ? '~' : '-');
        out += m_value;
        out += separator;
    }
} // namespace dnv::vista::sdk
