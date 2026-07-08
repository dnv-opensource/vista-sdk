/**
 * @file StringUtils.h
 * @brief String utility functions for fast parsing and comparison
 */

#pragma once

#include <algorithm>
#include <string_view>

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
} // namespace dnv::vista::sdk::string

#include "dnv/vista/sdk/detail/utils/StringUtils.inl"
