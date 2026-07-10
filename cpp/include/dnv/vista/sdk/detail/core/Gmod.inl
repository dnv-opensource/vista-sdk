#include <stdexcept>

namespace dnv::vista::sdk
{
    inline const GmodNode& Gmod::operator[](std::string_view key) const
    {
        auto it = m_nodeMap.find(std::string{ key });
        if (it != m_nodeMap.end())
        {
            return it->second;
        }

        throw std::out_of_range{ "Key not found in Gmod node map: " + std::string{ key } };
    }

    inline VisVersion Gmod::version() const noexcept
    {
        return m_visVersion;
    }

    inline const GmodNode& Gmod::rootNode() const noexcept
    {
        return *m_rootNode;
    }

    inline std::optional<const GmodNode*> Gmod::node(std::string_view code) const noexcept
    {
        auto it = m_nodeMap.find(std::string{ code });
        if (it != m_nodeMap.end())
        {
            return &it->second;
        }
        return std::nullopt;
    }

    inline auto Gmod::begin() const noexcept
    {
        return m_nodeMap.begin();
    }

    inline auto Gmod::end() const noexcept
    {
        return m_nodeMap.end();
    }

    inline auto Gmod::cbegin() const noexcept
    {
        return m_nodeMap.cbegin();
    }

    inline auto Gmod::cend() const noexcept
    {
        return m_nodeMap.cend();
    }
} // namespace dnv::vista::sdk
