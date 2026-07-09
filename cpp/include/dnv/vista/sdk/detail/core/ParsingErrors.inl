#include <algorithm>

namespace dnv::vista::sdk
{
    inline ParsingErrors::ParsingErrors(std::vector<ErrorEntry>&& errors) noexcept
        : m_errors{ std::move(errors) }
    {}

    inline bool ParsingErrors::operator==(const ParsingErrors& other) const noexcept
    {
        return m_errors == other.m_errors;
    }

    inline size_t ParsingErrors::count() const noexcept
    {
        return m_errors.size();
    }

    inline bool ParsingErrors::hasErrors() const noexcept
    {
        return !m_errors.empty();
    }

    inline bool ParsingErrors::hasErrorType(std::string_view type) const noexcept
    {
        return std::any_of(
            m_errors.begin(), m_errors.end(), [type](const ErrorEntry& entry) { return entry.type == type; });
    }

    inline ParsingErrors::Iterator ParsingErrors::begin() const noexcept
    {
        return m_errors.begin();
    }

    inline ParsingErrors::Iterator ParsingErrors::end() const noexcept
    {
        return m_errors.end();
    }
} // namespace dnv::vista::sdk
