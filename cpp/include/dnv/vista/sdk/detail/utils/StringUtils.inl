namespace dnv::vista::sdk::string
{
    namespace
    {
        template <typename T>
        inline bool parseNumericImpl(std::string_view str, T& result) noexcept
        {
            if (str.empty())
            {
                result = T{};
                return false;
            }

            const char* const begin = str.data();
            const char* const end = std::next(begin, static_cast<std::ptrdiff_t>(str.size()));
            const auto parseResult = std::from_chars(begin, end, result);

            return parseResult.ec == std::errc{} && parseResult.ptr == end;
        }
    } // namespace

    inline bool iequals(std::string_view lhs, std::string_view rhs) noexcept
    {
        if (lhs.size() != rhs.size())
        {
            return false;
        }

        return std::equal(
            lhs.begin(), lhs.end(), rhs.begin(), [](char a, char b) noexcept { return toLower(a) == toLower(b); });
    }

    inline constexpr bool contains(std::string_view str, std::string_view substr) noexcept
    {
        return str.find(substr) != std::string_view::npos;
    }

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

    inline constexpr std::string collapseWhitespace(std::string_view str)
    {
        std::string result;
        result.reserve(str.size());

        bool in_whitespace = true; // Start as true to skip leading whitespace

        for (char c : str)
        {
            if (isWhitespace(c))
            {
                if (!in_whitespace)
                {
                    result += ' '; // Replace any whitespace with single space
                    in_whitespace = true;
                }
            }
            else
            {
                result += c;
                in_whitespace = false;
            }
        }

        // Remove trailing space if added
        if (!result.empty() && result.back() == ' ')
        {
            result.pop_back();
        }

        return result;
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

    inline constexpr char toLower(char c) noexcept
    {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + ('a' - 'A')) : c;
    }

    inline constexpr std::string toLower(std::string_view str)
    {
        std::string result;
        result.reserve(str.size());

        for (char c : str)
        {
            result.push_back(toLower(c));
        }

        return result;
    }

    template <typename T>
        requires(
            std::is_same_v<std::decay_t<T>, bool> || std::is_same_v<std::decay_t<T>, int> ||
            std::is_same_v<std::decay_t<T>, std::uint32_t> || std::is_same_v<std::decay_t<T>, std::int64_t> ||
            std::is_same_v<std::decay_t<T>, std::uint64_t> || std::is_same_v<std::decay_t<T>, float> ||
            std::is_same_v<std::decay_t<T>, double>)
    inline constexpr bool fromString(std::string_view str, T& result) noexcept
    {
        // Boolean parsing: supports multiple common representations (case-insensitive)
        if constexpr (std::is_same_v<std::decay_t<T>, bool>)
        {
            if (str.empty())
            {
                result = false;
                return false;
            }

            // Handle single character cases first
            if (str.size() == 1)
            {
                const char c{ toLower(str[0]) };
                if (c == '1' || c == 't' || c == 'y')
                {
                    result = true;
                    return true;
                }
                else if (c == '0' || c == 'f' || c == 'n')
                {
                    result = false;
                    return true;
                }
                return false;
            }

            // Handle multi-character cases
            if (str.size() == 2)
            {
                if ((toLower(str[0]) == 'o' && toLower(str[1]) == 'n'))
                {
                    result = true;
                    return true;
                }
                else if ((toLower(str[0]) == 'n' && toLower(str[1]) == 'o'))
                {
                    result = false;
                    return true;
                }
            }
            else if (str.size() == 3)
            {
                if (toLower(str[0]) == 'y' && toLower(str[1]) == 'e' && toLower(str[2]) == 's')
                {
                    result = true;
                    return true;
                }
                else if (toLower(str[0]) == 'o' && toLower(str[1]) == 'f' && toLower(str[2]) == 'f')
                {
                    result = false;
                    return true;
                }
            }
            else if (str.size() == 4)
            {
                if (toLower(str[0]) == 't' && toLower(str[1]) == 'r' && toLower(str[2]) == 'u' &&
                    toLower(str[3]) == 'e')
                {
                    result = true;
                    return true;
                }
            }
            else if (str.size() == 5)
            {
                if (toLower(str[0]) == 'f' && toLower(str[1]) == 'a' && toLower(str[2]) == 'l' &&
                    toLower(str[3]) == 's' && toLower(str[4]) == 'e')
                {
                    result = false;
                    return true;
                }
            }

            // No valid boolean representation found
            result = false;
            return false;
        }
        // 32-bit signed integer parsing
        else if constexpr (std::is_same_v<std::decay_t<T>, int>)
        {
            return parseNumericImpl(str, result);
        }
        // 64-bit signed integer parsing
        else if constexpr (std::is_same_v<std::decay_t<T>, std::int64_t>)
        {
            return parseNumericImpl(str, result);
        }
        // 32-bit unsigned integer parsing
        else if constexpr (std::is_same_v<std::decay_t<T>, std::uint32_t>)
        {
            return parseNumericImpl(str, result);
        }
        // 64-bit unsigned integer parsing
        else if constexpr (std::is_same_v<std::decay_t<T>, std::uint64_t>)
        {
            return parseNumericImpl(str, result);
        }
        // Float parsing: handles decimal, scientific notation, and special values (nan, inf)
        else if constexpr (std::is_same_v<std::decay_t<T>, float>)
        {
            return parseNumericImpl(str, result);
        }
        // Double parsing: handles decimal, scientific notation, and special values (nan, inf)
        else if constexpr (std::is_same_v<std::decay_t<T>, double>)
        {
            return parseNumericImpl(str, result);
        }
    }
} // namespace dnv::vista::sdk::string
