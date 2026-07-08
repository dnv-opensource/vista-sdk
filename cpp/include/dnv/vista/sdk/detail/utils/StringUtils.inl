namespace dnv::vista::sdk::string
{
    inline constexpr bool isWhitespace(char c) noexcept
    {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
    }

    inline constexpr bool isNullOrWhiteSpace(std::string_view str) noexcept
    {
        if (str.empty())
        {
            return true;
        }
        return std::all_of(str.begin(), str.end(), isWhitespace);
    }

    inline constexpr bool isDigit(char c) noexcept
    {
        return static_cast<unsigned char>(c - '0') <= 9u;
    }

    inline constexpr bool isAlpha(char c) noexcept
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    inline constexpr bool isAlphaNumeric(char c) noexcept
    {
        return isAlpha(c) || isDigit(c);
    }

    inline constexpr bool isAllDigits(std::string_view str) noexcept
    {
        if (str.empty())
        {
            return false;
        }
        return std::all_of(str.begin(), str.end(), [](char c) { return isDigit(c); });
    }

    inline constexpr bool isUriUnreserved(char c) noexcept
    {
        return isAlphaNumeric(c) || c == '-' || c == '.' || c == '_' || c == '~';
    }

    inline constexpr bool isUriUnreserved(std::string_view str) noexcept
    {
        if (str.empty())
        {
            return false;
        }
        return std::all_of(str.begin(), str.end(), [](char c) { return isUriUnreserved(c); });
    }

    inline constexpr std::string_view trimStart(std::string_view str) noexcept
    {
        std::size_t start = 0;
        while (start < str.size() && isWhitespace(str[start]))
        {
            ++start;
        }
        return str.substr(start);
    }

    inline constexpr std::string_view trimEnd(std::string_view str) noexcept
    {
        std::size_t end = str.size();
        while (end > 0 && isWhitespace(str[end - 1]))
        {
            --end;
        }
        return str.substr(0, end);
    }

    inline constexpr std::string_view trim(std::string_view str) noexcept
    {
        return trimEnd(trimStart(str));
    }
} // namespace dnv::vista::sdk::string
