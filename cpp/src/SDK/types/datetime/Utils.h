#pragma once

#include "dnv/vista/sdk/detail/types/datetime/Constants.h"
#include "dnv/vista/sdk/types/datetime/DateTime.h"
#include "dnv/vista/sdk/utils/StringUtils.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace dnv::vista::sdk::internal
{
    /** @brief Convert ticks to date components */
    constexpr void dateComponentsFromTicks(
        std::int64_t ticks, std::int32_t& year, std::int32_t& month, std::int32_t& day) noexcept
    {
        // Extract date components using Gregorian 400-year cycle algorithm (O(1) complexity)
        std::int64_t totalDays{ ticks / constants::TICKS_PER_DAY };

        // Use 400-year cycles for O(1) year calculation
        std::int64_t num400Years{ totalDays / constants::DAYS_PER_400_YEARS };
        totalDays %= constants::DAYS_PER_400_YEARS;

        // Extract 100-year periods (handle leap year edge case at 400-year boundary)
        std::int64_t num100Years{ totalDays / constants::DAYS_PER_100_YEARS };
        if (num100Years > 3)
        {
            num100Years = 3; // Year divisible by 400 is a leap year
        }
        totalDays -= num100Years * constants::DAYS_PER_100_YEARS;

        // Extract 4-year cycles
        std::int64_t num4Years{ totalDays / constants::DAYS_PER_4_YEARS };
        totalDays %= constants::DAYS_PER_4_YEARS;

        // Extract remaining years (handle leap year edge case at 4-year boundary)
        std::int64_t numYears{ totalDays / constants::DAYS_PER_YEAR };
        if (numYears > 3)
        {
            numYears = 3; // 4th year in cycle is a leap year
        }
        totalDays -= numYears * constants::DAYS_PER_YEAR;

        // Calculate final year (add 1 because year 1 is the base)
        year = static_cast<std::int32_t>(1 + num400Years * 400 + num100Years * 100 + num4Years * 4 + numYears);

        // Find the month by iterating through months (already O(1) - max 12 iterations)
        month = 1;
        while (month <= 12)
        {
            std::int32_t daysInCurrentMonth{ DateTime::daysInMonth(year, month) };
            if (totalDays < daysInCurrentMonth)
            {
                break;
            }

            totalDays -= daysInCurrentMonth;
            ++month;
        }

        // Remaining days is the day of month (add 1 because day is 1-based)
        day = static_cast<std::int32_t>(totalDays) + 1;
    }

    /** @brief Convert ticks to time components */
    constexpr void timeComponentsFromTicks(
        std::int64_t ticks,
        std::int32_t& hour,
        std::int32_t& minute,
        std::int32_t& second,
        std::int32_t& millisecond) noexcept
    {
        std::int64_t timeTicks{ ticks % internal::constants::TICKS_PER_DAY };

        hour = static_cast<std::int32_t>(timeTicks / internal::constants::TICKS_PER_HOUR);
        timeTicks %= internal::constants::TICKS_PER_HOUR;

        minute = static_cast<std::int32_t>(timeTicks / internal::constants::TICKS_PER_MINUTE);
        timeTicks %= internal::constants::TICKS_PER_MINUTE;

        second = static_cast<std::int32_t>(timeTicks / internal::constants::TICKS_PER_SECOND);
        timeTicks %= internal::constants::TICKS_PER_SECOND;

        millisecond = static_cast<std::int32_t>(timeTicks / internal::constants::TICKS_PER_MILLISECOND);
    }

    /** @brief Validate that all positions contain digits */
    [[nodiscard]] constexpr bool areDigits(const char* p, std::size_t count) noexcept
    {
        for (std::size_t i = 0; i < count; ++i)
        {
            if (!string::isDigit(p[i]))
            {
                return false;
            }
        }
        return true;
    }

    /** @brief Fast parse 2 digits without validation */
    [[nodiscard]] constexpr std::int32_t parse2Digits(const char* p) noexcept
    {
        return (p[0] - '0') * 10 + (p[1] - '0');
    }

    /** @brief Fast parse 4 digits without validation */
    [[nodiscard]] constexpr std::int32_t parse4Digits(const char* p) noexcept
    {
        return (p[0] - '0') * 1000 + (p[1] - '0') * 100 + (p[2] - '0') * 10 + (p[3] - '0');
    }

    /** @brief Append two-digit zero-padded integer */
    inline void appendTwoDigits(std::string& out, std::int32_t value) noexcept
    {
        out += static_cast<char>('0' + (value / 10));
        out += static_cast<char>('0' + (value % 10));
    }

    /** @brief Append four-digit zero-padded integer */
    inline void appendFourDigits(std::string& out, std::int32_t value) noexcept
    {
        out += static_cast<char>('0' + (value / 1000));
        out += static_cast<char>('0' + ((value / 100) % 10));
        out += static_cast<char>('0' + ((value / 10) % 10));
        out += static_cast<char>('0' + (value % 10));
    }

    /** @brief Parse integer from string view using fast digit parsing */
    [[nodiscard]] inline bool fastParseInt(std::string_view str, double& value) noexcept
    {
        if (str.empty())
        {
            return false;
        }

        value = 0;
        for (char c : str)
        {
            if (!string::isDigit(c))
            {
                return false;
            }
            value = value * 10 + (c - '0');
        }
        return true;
    }

    /** @brief Parse fractional seconds (e.g., "45.5S" -> 45.5) */
    [[nodiscard]] inline bool fastParseDecimal(std::string_view str, double& value) noexcept
    {
        if (str.empty())
        {
            return false;
        }

        auto dotPos = str.find('.');
        if (dotPos == std::string_view::npos)
        {
            // No decimal point - parse as integer
            return fastParseInt(str, value);
        }

        // Parse integer part
        double intPart = 0;
        if (dotPos > 0 && !fastParseInt(str.substr(0, dotPos), intPart))
        {
            return false;
        }

        // Parse fractional part
        double fracPart = 0;
        if (dotPos + 1 < str.length())
        {
            auto fracStr = str.substr(dotPos + 1);
            if (!fastParseInt(fracStr, fracPart))
            {
                return false;
            }

            // Convert to fraction (e.g., "5" from "45.5" -> 0.5)
            double divisor = 1;
            for (std::size_t i = 0; i < fracStr.length(); ++i)
            {
                divisor *= 10;
            }
            fracPart /= divisor;
        }

        value = intPart + fracPart;
        return true;
    }
} // namespace dnv::vista::sdk::internal
