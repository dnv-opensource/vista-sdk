#include <stdexcept>

namespace dnv::vista::sdk
{
    inline const Codebook& Codebooks::operator[](CodebookName name) const
    {
        auto index = static_cast<size_t>(name) - 1;

        if (index >= m_codebooks.size() || !m_codebooks[index].has_value())
        {
            throw std::out_of_range{ "Invalid codebook name" };
        }

        return *m_codebooks[index];
    }

    inline VisVersion Codebooks::version() const noexcept
    {
        return m_version;
    }
} // namespace dnv::vista::sdk
