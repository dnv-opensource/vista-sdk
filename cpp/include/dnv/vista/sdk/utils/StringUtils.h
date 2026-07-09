/**
 * @file StringUtils.h
 * @brief String utility functions for fast parsing and comparison
 */

#pragma once

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <string_view>
#include <system_error>
#include <type_traits>

namespace dnv::vista::sdk::string
{
    /**
     * @brief Check if character is whitespace
     * @param c Character to check
     * @return True if character is space, tab, newline, carriage return, form feed, or vertical tab
     */
    inline constexpr bool isWhitespace(char c) noexcept;

    /**
     * @brief Fast check if string is null, empty, or contains only whitespace
     * @param str String to check
     * @return True if string is empty or contains only whitespace characters
     * @details Considers space, tab, newline, carriage return, form feed, and vertical tab as whitespace characters
     */
    inline constexpr bool isNullOrWhiteSpace(std::string_view str) noexcept;

    /**
     * @brief Check if character is ASCII digit
     * @param c Character to check
     * @return True if character is 0-9
     */
    inline constexpr bool isDigit(char c) noexcept;

    /**
     * @brief Check if character is ASCII alphabetic
     * @param c Character to check
     * @return True if character is a-z or A-Z
     */
    inline constexpr bool isAlpha(char c) noexcept;

    /**
     * @brief Check if character is ASCII alphanumeric
     * @param c Character to check
     * @return True if character is a-z, A-Z, or 0-9
     */
    inline constexpr bool isAlphaNumeric(char c) noexcept;

    /**
     * @brief Check if string contains only ASCII digits
     * @param str String to check
     * @return True if string is non-empty and contains only digits (0-9)
     * @details Returns false for empty strings
     */
    inline constexpr bool isAllDigits(std::string_view str) noexcept;

    /**
     * @brief Check if character is URI unreserved (RFC 3986 Section 2.3)
     * @param c Character to check
     * @return True if character is A-Z, a-z, 0-9, -, ., _, or ~
     */
    inline constexpr bool isUriUnreserved(char c) noexcept;

    /**
     * @brief Check if string contains only URI unreserved characters
     * @param str String to check
     * @return True if string is non-empty and contains only unreserved characters
     * @details Returns false for empty strings (RFC 3986 Section 2.3)
     */
    inline constexpr bool isUriUnreserved(std::string_view str) noexcept;

    /**
     * @brief Remove leading whitespace from string
     * @param str String to trim
     * @return String view with leading whitespace removed
     * @details Returns a view into the original string, no allocation performed
     */
    inline constexpr std::string_view trimStart(std::string_view str) noexcept;

    /**
     * @brief Remove trailing whitespace from string
     * @param str String to trim
     * @return String view with trailing whitespace removed
     * @details Returns a view into the original string, no allocation performed
     */
    inline constexpr std::string_view trimEnd(std::string_view str) noexcept;

    /**
     * @brief Remove leading and trailing whitespace from string
     * @param str String to trim
     * @return String view with leading and trailing whitespace removed
     * @details Returns a view into the original string, no allocation performed
     */
    inline constexpr std::string_view trim(std::string_view str) noexcept;

    /**
     * @brief Convert ASCII character to lowercase
     * @param c Character to convert
     * @return Lowercase character if ASCII letter, otherwise unchanged
     * @details Only works with ASCII characters (A-Z, a-z). Non-ASCII characters are returned unchanged
     */
    inline constexpr char toLower(char c) noexcept;

    /**
     * @brief Convert string to lowercase
     * @param str String to convert
     * @return New string with all ASCII characters converted to lowercase
     * @details Only ASCII characters (A-Z) are converted. Non-ASCII characters are preserved unchanged
     *          This function allocates a new std::string
     */
    inline constexpr std::string toLower(std::string_view str);

    /**
     * @brief Parse string to type T using output parameter
     * @tparam T Target type (bool, int, uint32_t, int64_t, uint64_t, float, double)
     * @param str String to parse
     * @param result Output parameter to store parsed value
     * @return True if parsing succeeded, false otherwise
     */
    template <typename T>
        requires(
            std::is_same_v<std::decay_t<T>, bool> || std::is_same_v<std::decay_t<T>, int> ||
            std::is_same_v<std::decay_t<T>, std::uint32_t> || std::is_same_v<std::decay_t<T>, std::int64_t> ||
            std::is_same_v<std::decay_t<T>, std::uint64_t> || std::is_same_v<std::decay_t<T>, float> ||
            std::is_same_v<std::decay_t<T>, double>)
    inline constexpr bool fromString(std::string_view str, T& result) noexcept;
} // namespace dnv::vista::sdk::string

#include "dnv/vista/sdk/detail/utils/StringUtils.inl"
