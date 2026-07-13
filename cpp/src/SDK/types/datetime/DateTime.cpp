#include "dnv/vista/sdk/types/datetime/DateTime.h"

#include "Utils.h"

#include <charconv>
#include <cstring>
#include <istream>
#include <limits>

namespace dnv::vista::sdk
{
    namespace internal
    {
        /**
         * @brief Minimum DateTime value that can safely round-trip through std::chrono::system_clock
         * @details This is platform-dependent. On systems with 64-bit nanosecond precision,
         *          this is approximately year 1678.
         */
        static constexpr std::int64_t MIN_CHRONO_SAFE_TICKS = std::max(
            constants::MIN_DATETIME_TICKS,
            constants::UNIX_EPOCH_TICKS + (std::numeric_limits<std::int64_t>::min() / 100));

        /**
         * @brief Maximum DateTime value that can safely round-trip through std::chrono::system_clock
         * @details This is platform-dependent. On systems with 64-bit nanosecond precision,
         *          this is approximately year 2262.
         */
        static constexpr std::int64_t MAX_CHRONO_SAFE_TICKS = std::min(
            constants::MAX_DATETIME_TICKS,
            constants::UNIX_EPOCH_TICKS + (std::numeric_limits<std::int64_t>::max() / 100));

        /** @brief Convert date components to ticks */
        static constexpr std::int64_t dateToTicks(std::int32_t year, std::int32_t month, std::int32_t day) noexcept
        {
            // Calculate total days since January 1, year 1 using Gregorian 400-year cycle algorithm (O(1) complexity)
            std::int64_t totalDays{ 0 };

            // Adjust to 0-based year for calculation
            std::int32_t y{ year - 1 };

            // Add days for complete 400-year cycles
            totalDays += (y / 400) * internal::constants::DAYS_PER_400_YEARS;
            y %= 400;

            // Add days for complete 100-year periods
            totalDays += (y / 100) * internal::constants::DAYS_PER_100_YEARS;
            y %= 100;

            // Add days for complete 4-year cycles
            totalDays += (y / 4) * internal::constants::DAYS_PER_4_YEARS;
            y %= 4;

            // Add days for remaining years
            totalDays += y * internal::constants::DAYS_PER_YEAR;

            // Add days for complete months in the given year
            // Clamp month to valid range to prevent undefined behavior in loop
            const std::int32_t validMonth = (month < 1) ? 1 : (month > 12) ? 12 : month;
            for (std::int32_t m{ 1 }; m < validMonth; ++m)
            {
                totalDays += DateTime::daysInMonth(year, m);
            }

            // Add days in the current month (subtract 1 because day is 1-based)
            totalDays += day - 1;

            return totalDays * internal::constants::TICKS_PER_DAY;
        }

        /** @brief Convert time components to ticks */
        static constexpr std::int64_t timeToTicks(
            std::int32_t hour, std::int32_t minute, std::int32_t second, std::int32_t millisecond) noexcept
        {
            return (static_cast<std::int64_t>(hour) * internal::constants::TICKS_PER_HOUR) +
                   (static_cast<std::int64_t>(minute) * internal::constants::TICKS_PER_MINUTE) +
                   (static_cast<std::int64_t>(second) * internal::constants::TICKS_PER_SECOND) +
                   (static_cast<std::int64_t>(millisecond) * internal::constants::TICKS_PER_MILLISECOND);
        }

        /** @brief Validate date components */
        static constexpr bool isValidDate(std::int32_t year, std::int32_t month, std::int32_t day) noexcept
        {
            if (year < internal::constants::MIN_YEAR || year > internal::constants::MAX_YEAR)
            {
                return false;
            }
            if (month < 1 || month > 12)
            {
                return false;
            }
            if (day < 1 || day > DateTime::daysInMonth(year, month))
            {
                return false;
            }

            return true;
        }

        /** @brief Validate time components */
        static constexpr bool isValidTime(
            std::int32_t hour, std::int32_t minute, std::int32_t second, std::int32_t millisecond) noexcept
        {
            return hour >= 0 && hour <= internal::constants::HOURS_PER_DAY - 1 && minute >= 0 &&
                   minute <= internal::constants::MINUTES_PER_HOUR - 1 && second >= 0 &&
                   second <= internal::constants::SECONDS_PER_MINUTE - 1 && millisecond >= 0 &&
                   millisecond <= internal::constants::MILLISECONDS_PER_SECOND - 1;
        }
    } // namespace internal

    namespace
    {
        /**
         * @brief Fast-path parser for standard ISO 8601 formats
         * @details Handles the most common formats with fixed positions:
         *          - "YYYY-MM-DD" (10 chars)
         *          - "YYYY-MM-DDTHH:mm:ss" (19 chars)
         *          - "YYYY-MM-DDTHH:mm:ssZ" (20 chars)
         *          - "YYYY-MM-DDTHH:mm:ss.f" (21-27 chars)
         *          - "YYYY-MM-DDTHH:mm:ss.fZ" (22-28 chars)
         * @return true if parsed successfully via fast path, false if fallback needed
         */
        [[nodiscard]] bool tryParseFastPath(std::string_view str, DateTime& result) noexcept
        {
            const std::size_t len = str.length();

            // Fast path requirements: minimum 10 chars (YYYY-MM-DD)
            if (len < 10)
            {
                return false;
            }

            const char* data = str.data();

            // Validate fixed separators and digit positions for date part
            if (data[4] != '-' || data[7] != '-' || !internal::areDigits(data, 4) ||
                !internal::areDigits(data + 5, 2) || !internal::areDigits(data + 8, 2))
            {
                return false;
            }

            // Parse date components using fast digit parsing
            const std::int32_t year = internal::parse4Digits(data);
            const std::int32_t month = internal::parse2Digits(data + 5);
            const std::int32_t day = internal::parse2Digits(data + 8);

            // Validate date components
            if (!internal::isValidDate(year, month, day))
            {
                return false;
            }

            // Date-only format: "YYYY-MM-DD"
            if (len == 10)
            {
                const std::int64_t ticks = internal::dateToTicks(year, month, day);
                result = DateTime{ ticks };
                return true;
            }

            // Time part must start with 'T'
            if (data[10] != 'T')
            {
                return false;
            }

            // Need at least "YYYY-MM-DDTHH:mm:ss" (19 chars)
            if (len < 19)
            {
                return false;
            }

            // Validate time separators and digits
            if (data[13] != ':' || data[16] != ':' || !internal::areDigits(data + 11, 2) ||
                !internal::areDigits(data + 14, 2) || !internal::areDigits(data + 17, 2))
            {
                return false;
            }

            // Parse time components
            const std::int32_t hour = internal::parse2Digits(data + 11);
            const std::int32_t minute = internal::parse2Digits(data + 14);
            const std::int32_t second = internal::parse2Digits(data + 17);

            // Validate time components
            if (!internal::isValidTime(hour, minute, second, 0))
            {
                return false;
            }

            std::int32_t fractionalTicks = 0;
            std::size_t pos = 19;

            // Handle optional 'Z' at position 19
            if (len == 20 && data[19] == 'Z')
            {
                // "YYYY-MM-DDTHH:mm:ssZ" - no fractional seconds
                pos = 20;
            }
            // Handle fractional seconds
            else if (len > 19 && data[19] == '.')
            {
                pos = 20; // Start after '.'

                // Parse up to 7 fractional digits (100ns precision)
                std::int32_t fractionValue = 0;
                std::int32_t fractionDigits = 0;

                while (pos < len && string::isDigit(data[pos]) && fractionDigits < 7)
                {
                    fractionValue = fractionValue * 10 + (data[pos] - '0');
                    ++fractionDigits;
                    ++pos;
                }

                if (fractionDigits == 0)
                {
                    return false; // '.' must be followed by at least one digit
                }

                // Pad to 7 digits (convert to 100ns ticks)
                while (fractionDigits < 7)
                {
                    fractionValue *= 10;
                    ++fractionDigits;
                }

                fractionalTicks = fractionValue;

                // Skip remaining fractional digits beyond our precision
                while (pos < len && string::isDigit(data[pos]))
                {
                    ++pos;
                }

                // Optional 'Z' after fractional seconds
                if (pos < len && data[pos] == 'Z')
                {
                    ++pos;
                }
            }
            else if (len != 19)
            {
                // Unexpected format - not a standard ISO 8601 we can fast-path
                return false;
            }

            // Should have consumed the entire string
            if (pos != len)
            {
                return false;
            }

            // Calculate total ticks
            const std::int64_t ticks = internal::dateToTicks(year, month, day) +
                                       internal::timeToTicks(hour, minute, second, 0) + fractionalTicks;

            result = DateTime{ ticks };
            return true;
        }
    } // namespace

    DateTime::DateTime(std::int32_t year, std::int32_t month, std::int32_t day) noexcept
    {
        if (!internal::isValidDate(year, month, day))
        {
            m_ticks = internal::constants::MIN_DATETIME_TICKS;
            return;
        }
        m_ticks = internal::dateToTicks(year, month, day);
    }

    DateTime::DateTime(
        std::int32_t year,
        std::int32_t month,
        std::int32_t day,
        std::int32_t hour,
        std::int32_t minute,
        std::int32_t second) noexcept
    {
        if (!internal::isValidDate(year, month, day) || !internal::isValidTime(hour, minute, second, 0))
        {
            m_ticks = internal::constants::MIN_DATETIME_TICKS;
            return;
        }
        m_ticks = internal::dateToTicks(year, month, day) + internal::timeToTicks(hour, minute, second, 0);
    }

    DateTime::DateTime(
        std::int32_t year,
        std::int32_t month,
        std::int32_t day,
        std::int32_t hour,
        std::int32_t minute,
        std::int32_t second,
        std::int32_t millisecond) noexcept
    {
        if (!internal::isValidDate(year, month, day) || !internal::isValidTime(hour, minute, second, millisecond))
        {
            m_ticks = internal::constants::MIN_DATETIME_TICKS;
            return;
        }
        m_ticks = internal::dateToTicks(year, month, day) + internal::timeToTicks(hour, minute, second, millisecond);
    }

    std::int32_t DateTime::year() const noexcept
    {
        std::int32_t year, month, day;
        internal::dateComponentsFromTicks(m_ticks, year, month, day);

        return year;
    }

    std::int32_t DateTime::month() const noexcept
    {
        std::int32_t year, month, day;
        internal::dateComponentsFromTicks(m_ticks, year, month, day);

        return month;
    }

    std::int32_t DateTime::day() const noexcept
    {
        std::int32_t year, month, day;
        internal::dateComponentsFromTicks(m_ticks, year, month, day);

        return day;
    }

    std::int32_t DateTime::hour() const noexcept
    {
        std::int32_t hour, minute, second, millisecond;
        internal::timeComponentsFromTicks(m_ticks, hour, minute, second, millisecond);

        return hour;
    }

    std::int32_t DateTime::minute() const noexcept
    {
        std::int32_t hour, minute, second, millisecond;
        internal::timeComponentsFromTicks(m_ticks, hour, minute, second, millisecond);

        return minute;
    }

    std::int32_t DateTime::second() const noexcept
    {
        std::int32_t hour, minute, second, millisecond;
        internal::timeComponentsFromTicks(m_ticks, hour, minute, second, millisecond);

        return second;
    }

    std::int32_t DateTime::millisecond() const noexcept
    {
        std::int32_t hour, minute, second, millisecond;
        internal::timeComponentsFromTicks(m_ticks, hour, minute, second, millisecond);

        return millisecond;
    }

    std::int32_t DateTime::microsecond() const noexcept
    {
        const auto remainderTicks{ m_ticks % 10000 };

        return static_cast<std::int32_t>(remainderTicks / 10);
    }

    std::int32_t DateTime::nanosecond() const noexcept
    {
        const auto remainderTicks{ m_ticks % 10 };

        return static_cast<std::int32_t>(remainderTicks * 100);
    }

    std::int32_t DateTime::dayOfWeek() const noexcept
    {
        // January 1, 0001 was a Monday (day 1), so we need to adjust
        std::int64_t days{ m_ticks / internal::constants::TICKS_PER_DAY };

        // 0=Sunday, 6=Saturday
        return static_cast<std::int32_t>((days + 1) % 7);
    }

    std::int32_t DateTime::dayOfYear() const noexcept
    {
        std::int32_t year, month, day;
        internal::dateComponentsFromTicks(m_ticks, year, month, day);

        std::int32_t dayCount{ 0 };
        for (std::int32_t m{ 1 }; m < month; ++m)
        {
            dayCount += daysInMonth(year, m);
        }

        return dayCount + day;
    }

    DateTime DateTime::date() const noexcept
    {
        std::int64_t dayTicks{ (m_ticks / internal::constants::TICKS_PER_DAY) * internal::constants::TICKS_PER_DAY };

        return DateTime{ dayTicks };
    }

    TimeSpan DateTime::timeOfDay() const noexcept
    {
        std::int64_t timeTicks{ m_ticks % internal::constants::TICKS_PER_DAY };

        return TimeSpan{ timeTicks };
    }

    DateTime DateTime::addDays(double days) const noexcept
    {
        return *this + TimeSpan::fromDays(days);
    }

    DateTime DateTime::addHours(double hours) const noexcept
    {
        return *this + TimeSpan::fromHours(hours);
    }

    DateTime DateTime::addMinutes(double minutes) const noexcept
    {
        return *this + TimeSpan::fromMinutes(minutes);
    }

    DateTime DateTime::addSeconds(double seconds) const noexcept
    {
        return *this + TimeSpan::fromSeconds(seconds);
    }

    DateTime DateTime::addMilliseconds(double milliseconds) const noexcept
    {
        return *this + TimeSpan::fromMilliseconds(milliseconds);
    }

    DateTime DateTime::addMonths(std::int32_t months) const noexcept
    {
        auto y{ year() };
        auto m{ month() };
        const auto d{ day() };
        const auto tod{ timeOfDay() };

        m += months;
        while (m > 12)
        {
            m -= 12;
            y++;
        }
        while (m < 1)
        {
            m += 12;
            y--;
        }

        const auto adjustedDay{ std::min(d, daysInMonth(y, m)) };

        return DateTime{ y, m, adjustedDay } + tod;
    }

    DateTime DateTime::addYears(std::int32_t years) const noexcept
    {
        return addMonths(years * 12);
    }

    namespace
    {
        /** @brief Append ISO 8601 date part: YYYY-MM-DD */
        inline void appendIso8601Date(
            std::string& out, std::int32_t year, std::int32_t month, std::int32_t day) noexcept
        {
            internal::appendFourDigits(out, year);
            out += '-';
            internal::appendTwoDigits(out, month);
            out += '-';
            internal::appendTwoDigits(out, day);
        }

        /** @brief Append ISO 8601 time part: HH:mm:ss */
        inline void appendIso8601Time(
            std::string& out, std::int32_t hour, std::int32_t minute, std::int32_t second) noexcept
        {
            internal::appendTwoDigits(out, hour);
            out += ':';
            internal::appendTwoDigits(out, minute);
            out += ':';
            internal::appendTwoDigits(out, second);
        }

        /** @brief Append ISO 8601 datetime part: YYYY-MM-DDTHH:mm:ss */
        inline void appendIso8601DateTime(
            std::string& out,
            std::int32_t year,
            std::int32_t month,
            std::int32_t day,
            std::int32_t hour,
            std::int32_t minute,
            std::int32_t second) noexcept
        {
            appendIso8601Date(out, year, month, day);
            out += 'T';
            appendIso8601Time(out, hour, minute, second);
        }

        /** @brief Append zero-padded fractional seconds with specific precision */
        inline void appendFractionalSeconds(
            std::string& out, std::int32_t fractionalValue, std::size_t bufferSize) noexcept
        {
            char fracBuffer[8];
            fracBuffer[0] = '.';

            const auto ptr = std::to_chars(fracBuffer + 1, fracBuffer + bufferSize, fractionalValue).ptr;
            const auto fracLen = ptr - fracBuffer;
            const auto paddingNeeded = bufferSize - fracLen;

            if (paddingNeeded > 0)
            {
                std::memmove(fracBuffer + 1 + paddingNeeded, fracBuffer + 1, fracLen - 1);
                std::memset(fracBuffer + 1, '0', paddingNeeded);
            }

            out.append(fracBuffer, bufferSize);
        }

        /** @brief Append fractional seconds with trimmed trailing zeros */
        inline void appendFractionalSecondsTrimmed(std::string& out, std::int32_t fractionalTicks) noexcept
        {
            if (fractionalTicks > 0)
            {
                char fracBuffer[8];
                fracBuffer[0] = '.';
                const auto ptr = std::to_chars(fracBuffer + 1, fracBuffer + 8, fractionalTicks).ptr;
                auto fracLen = ptr - fracBuffer;
                const auto paddingNeeded = 8 - fracLen;

                if (paddingNeeded > 0)
                {
                    std::memmove(fracBuffer + 1 + paddingNeeded, fracBuffer + 1, fracLen - 1);
                    std::memset(fracBuffer + 1, '0', paddingNeeded);
                    fracLen = 8;
                }

                while (fracLen > 2 && fracBuffer[fracLen - 1] == '0')
                {
                    --fracLen;
                }

                out.append(fracBuffer, static_cast<std::size_t>(fracLen));
            }
            else
            {
                out += ".0";
            }
        }

        /** @brief Format ISO 8601 with UTC indicator: YYYY-MM-DDTHH:mm:ssZ */
        inline void formatIso8601(
            std::string& out,
            std::int32_t y,
            std::int32_t mon,
            std::int32_t d,
            std::int32_t h,
            std::int32_t min,
            std::int32_t s) noexcept
        {
            appendIso8601DateTime(out, y, mon, d, h, min, s);
            out += 'Z';
        }

        /** @brief Format ISO 8601 with precise fractional seconds: YYYY-MM-DDTHH:mm:ss.1234567Z */
        inline void formatIso8601Precise(
            std::string& out,
            std::int32_t y,
            std::int32_t mon,
            std::int32_t d,
            std::int32_t h,
            std::int32_t min,
            std::int32_t s,
            std::int64_t ticks) noexcept
        {
            const std::int32_t fractionalTicks{ static_cast<std::int32_t>(
                ticks % internal::constants::TICKS_PER_SECOND) };
            appendIso8601DateTime(out, y, mon, d, h, min, s);
            appendFractionalSeconds(out, fractionalTicks, 8);
            out += 'Z';
        }

        /** @brief Format ISO 8601 with trimmed fractional seconds: YYYY-MM-DDTHH:mm:ss.f+Z */
        inline void formatIso8601PreciseTrimmed(
            std::string& out,
            std::int32_t y,
            std::int32_t mon,
            std::int32_t d,
            std::int32_t h,
            std::int32_t min,
            std::int32_t s,
            std::int64_t ticks) noexcept
        {
            const std::int32_t fractionalTicks{ static_cast<std::int32_t>(
                ticks % internal::constants::TICKS_PER_SECOND) };
            appendIso8601DateTime(out, y, mon, d, h, min, s);
            appendFractionalSecondsTrimmed(out, fractionalTicks);
            out += 'Z';
        }

        /** @brief Format ISO 8601 with milliseconds: YYYY-MM-DDTHH:mm:ss.123Z */
        inline void formatIso8601Millis(
            std::string& out,
            std::int32_t y,
            std::int32_t mon,
            std::int32_t d,
            std::int32_t h,
            std::int32_t min,
            std::int32_t s,
            std::int64_t ticks) noexcept
        {
            const std::int32_t fractionalTicks{ static_cast<std::int32_t>(
                ticks % internal::constants::TICKS_PER_SECOND) };
            const std::int32_t milliseconds{ static_cast<std::int32_t>(
                fractionalTicks / internal::constants::TICKS_PER_MILLISECOND) };
            appendIso8601DateTime(out, y, mon, d, h, min, s);
            appendFractionalSeconds(out, milliseconds, 4);
            out += 'Z';
        }

        /** @brief Format ISO 8601 with microseconds: YYYY-MM-DDTHH:mm:ss.123456Z */
        inline void formatIso8601Micros(
            std::string& out,
            std::int32_t y,
            std::int32_t mon,
            std::int32_t d,
            std::int32_t h,
            std::int32_t min,
            std::int32_t s,
            std::int64_t ticks) noexcept
        {
            const std::int32_t fractionalTicks{ static_cast<std::int32_t>(
                ticks % internal::constants::TICKS_PER_SECOND) };
            const std::int32_t microseconds{ static_cast<std::int32_t>(
                fractionalTicks / internal::constants::TICKS_PER_MICROSECOND) };
            appendIso8601DateTime(out, y, mon, d, h, min, s);
            appendFractionalSeconds(out, microseconds, 7);
            out += 'Z';
        }

        /** @brief Format ISO 8601 extended with UTC offset: YYYY-MM-DDTHH:mm:ss+00:00 */
        inline void formatIso8601Extended(
            std::string& out,
            std::int32_t y,
            std::int32_t mon,
            std::int32_t d,
            std::int32_t h,
            std::int32_t min,
            std::int32_t s) noexcept
        {
            appendIso8601DateTime(out, y, mon, d, h, min, s);
            out += "+00:00";
        }

        /** @brief Format ISO 8601 basic (compact): YYYYMMDDTHHMMSSZ */
        inline void formatIso8601Basic(
            std::string& out,
            std::int32_t y,
            std::int32_t mon,
            std::int32_t d,
            std::int32_t h,
            std::int32_t min,
            std::int32_t s) noexcept
        {
            internal::appendFourDigits(out, y);
            internal::appendTwoDigits(out, mon);
            internal::appendTwoDigits(out, d);
            out += 'T';
            internal::appendTwoDigits(out, h);
            internal::appendTwoDigits(out, min);
            internal::appendTwoDigits(out, s);
            out += 'Z';
        }
    } // namespace

    std::string DateTime::toString(Format format) const
    {
        std::int32_t y, mon, d, h, min, s, ms;
        internal::dateComponentsFromTicks(m_ticks, y, mon, d);
        internal::timeComponentsFromTicks(m_ticks, h, min, s, ms);

        std::string str;
        str.reserve(35);

        switch (format)
        {
            case Format::Iso8601:
                formatIso8601(str, y, mon, d, h, min, s);
                break;

            case Format::Iso8601Precise:
                formatIso8601Precise(str, y, mon, d, h, min, s, m_ticks);
                break;

            case Format::Iso8601PreciseTrimmed:
                formatIso8601PreciseTrimmed(str, y, mon, d, h, min, s, m_ticks);
                break;

            case Format::Iso8601Millis:
                formatIso8601Millis(str, y, mon, d, h, min, s, m_ticks);
                break;

            case Format::Iso8601Micros:
                formatIso8601Micros(str, y, mon, d, h, min, s, m_ticks);
                break;

            case Format::Iso8601Extended:
                formatIso8601Extended(str, y, mon, d, h, min, s);
                break;

            case Format::Iso8601Basic:
                formatIso8601Basic(str, y, mon, d, h, min, s);
                break;

            case Format::Iso8601Date:
                appendIso8601Date(str, y, mon, d);
                break;

            case Format::Iso8601Time:
                appendIso8601Time(str, h, min, s);
                break;

            case Format::UnixSeconds:
            {
                char buffer[32];
                const auto ptr = std::to_chars(buffer, buffer + 32, toEpochSeconds()).ptr;
                str.append(buffer, ptr);
                break;
            }

            case Format::UnixMilliseconds:
            {
                char buffer[32];
                const auto ptr = std::to_chars(buffer, buffer + 32, toEpochMilliseconds()).ptr;
                str.append(buffer, ptr);
                break;
            }

            default:
                return toString(Format::Iso8601);
        }

        return str;
    }

    bool DateTime::isValid() const noexcept
    {
        return m_ticks >= internal::constants::MIN_DATETIME_TICKS && m_ticks <= internal::constants::MAX_DATETIME_TICKS;
    }

    DateTime DateTime::utcNow() noexcept
    {
        return DateTime{ std::chrono::system_clock::now() };
    }

    bool DateTime::fromString(std::string_view iso8601String, DateTime& result) noexcept
    {
        // Fast empty/length check
        if (iso8601String.empty() || iso8601String.length() < 10)
        {
            return false;
        }

        // Try fast-path parser first (handles 95% of real-world cases)
        // Supports: YYYY-MM-DD, YYYY-MM-DDTHH:mm:ss, YYYY-MM-DDTHH:mm:ssZ,
        //           YYYY-MM-DDTHH:mm:ss.f, YYYY-MM-DDTHH:mm:ss.fffffffZ
        if (tryParseFastPath(iso8601String, result))
        {
            return true;
        }

        // Fallback to flexible parser for non-standard formats
        // (handles timezone offsets, variable digit counts, etc.)

        // Remove trailing 'Z' if present (for flexible parser compatibility)
        if (iso8601String.back() == 'Z')
        {
            iso8601String.remove_suffix(1);
        }

        // Remove timezone offset for DateTime parsing
        auto tzPos{ iso8601String.find_last_of("+-") };

        // Ensure it's not in date part (after position 10 = "YYYY-MM-DD")
        if (tzPos != std::string_view::npos && tzPos > 10)
        {
            iso8601String = iso8601String.substr(0, tzPos);
        }

        const char* data = iso8601String.data();
        const char* end = data + iso8601String.size();

        // Parse year (YYYY)
        if (iso8601String.size() < 4)
        {
            return false;
        }

        std::int32_t year{ 0 };
        auto [ptr1, ec1] = std::from_chars(data, data + 4, year);
        if (ec1 != std::errc{} || ptr1 != data + 4)
        {
            return false;
        }

        // Expect '-'
        if (ptr1 >= end || *ptr1 != '-')
        {
            return false;
        }
        ++ptr1; // Skip '-'

        // Parse month (MM or M)
        std::int32_t month{ 0 };
        auto dashPos = iso8601String.find('-', 5); // Find second dash after "YYYY-"
        if (dashPos == std::string_view::npos)
        {
            return false;
        }

        auto [ptr2, ec2] = std::from_chars(ptr1, data + dashPos, month);
        if (ec2 != std::errc{})
        {
            return false;
        }

        // Expect '-'
        if (ptr2 >= end || *ptr2 != '-')
        {
            return false;
        }
        ++ptr2; // Skip '-'

        // Parse day (DD or D)
        std::int32_t day{ 0 };
        const char* dayEnd = ptr2;
        while (dayEnd < end && *dayEnd >= '0' && *dayEnd <= '9')
        {
            ++dayEnd;
        }

        auto [ptr3, ec3] = std::from_chars(ptr2, dayEnd, day);
        if (ec3 != std::errc{})
        {
            return false;
        }

        // Time part is optional
        std::int32_t hour{ 0 }, minute{ 0 }, second{ 0 };
        std::int32_t fractionalTicks{ 0 };

        if (ptr3 < end && *ptr3 == 'T')
        {
            ++ptr3; // Skip 'T'

            // Parse hour (HH or H)
            const char* hourEnd = ptr3;
            while (hourEnd < end && *hourEnd >= '0' && *hourEnd <= '9')
            {
                ++hourEnd;
            }

            auto [ptr4, ec4] = std::from_chars(ptr3, hourEnd, hour);
            if (ec4 != std::errc{})
            {
                return false;
            }

            // Expect ':'
            if (ptr4 >= end || *ptr4 != ':')
            {
                return false;
            }
            ++ptr4; // Skip ':'

            // Parse minute (MM or M)
            const char* minEnd = ptr4;
            while (minEnd < end && *minEnd >= '0' && *minEnd <= '9')
            {
                ++minEnd;
            }

            auto [ptr5, ec5] = std::from_chars(ptr4, minEnd, minute);
            if (ec5 != std::errc{})
            {
                return false;
            }

            // Expect ':'
            if (ptr5 >= end || *ptr5 != ':')
            {
                return false;
            }
            ++ptr5; // Skip ':'

            // Parse second (SS or S)
            const char* secEnd = ptr5;
            while (secEnd < end && *secEnd >= '0' && *secEnd <= '9')
            {
                ++secEnd;
            }

            auto [ptr6, ec6] = std::from_chars(ptr5, secEnd, second);
            if (ec6 != std::errc{})
            {
                return false;
            }

            // Parse fractional seconds if present
            if (ptr6 < end && *ptr6 == '.')
            {
                ++ptr6; // Skip '.'

                // Count fractional digits (max 7 for 100ns precision)
                const char* fracStart = ptr6;
                const char* fracEnd = fracStart;
                std::int32_t fractionDigits{ 0 };

                while (fracEnd < end && *fracEnd >= '0' && *fracEnd <= '9' && fractionDigits < 7)
                {
                    ++fracEnd;
                    ++fractionDigits;
                }

                if (fractionDigits > 0)
                {
                    std::int32_t fractionValue{ 0 };
                    auto [ptrF, ecF] = std::from_chars(fracStart, fracEnd, fractionValue);
                    if (ecF != std::errc{} || ptrF != fracEnd)
                    {
                        return false;
                    }

                    // Pad to 7 digits (convert to 100ns ticks)
                    while (fractionDigits < 7)
                    {
                        fractionValue *= 10;
                        ++fractionDigits;
                    }

                    fractionalTicks = fractionValue;
                }
            }
        }

        // Validate components
        if (!internal::isValidDate(year, month, day) || !internal::isValidTime(hour, minute, second, 0))
        {
            return false;
        }

        // Calculate ticks
        std::int64_t ticks{ internal::dateToTicks(year, month, day) + internal::timeToTicks(hour, minute, second, 0) +
                            fractionalTicks };

        result = DateTime{ ticks };

        return true;
    }

    std::optional<DateTime> DateTime::fromString(std::string_view iso8601String) noexcept
    {
        DateTime result;
        if (fromString(iso8601String, result))
        {
            return result;
        }
        return std::nullopt;
    }

    std::chrono::system_clock::time_point DateTime::toChrono() const noexcept
    {
        // Clamp to chrono-safe range
        std::int64_t safeTicks{ m_ticks };
        if (safeTicks < internal::MIN_CHRONO_SAFE_TICKS)
        {
            safeTicks = internal::MIN_CHRONO_SAFE_TICKS;
        }
        else if (safeTicks > internal::MAX_CHRONO_SAFE_TICKS)
        {
            safeTicks = internal::MAX_CHRONO_SAFE_TICKS;
        }

        // Calculate duration since Unix epoch in 100-nanosecond ticks
        std::int64_t ticksSinceEpoch{ safeTicks - internal::constants::UNIX_EPOCH_TICKS };

        // Convert to std::chrono duration (100ns precision)
        using ticks_duration = std::chrono::duration<std::int64_t, std::ratio<1, 10000000>>;
        auto duration{ ticks_duration{ ticksSinceEpoch } };

        return std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            std::chrono::system_clock::time_point{} + duration);
    }

    DateTime DateTime::fromChrono(const std::chrono::system_clock::time_point& timePoint) noexcept
    {
        return DateTime{ timePoint };
    }

    std::ostream& operator<<(std::ostream& os, const DateTime& dateTime)
    {
        os << dateTime.toString(DateTime::Format::Iso8601);

        return os;
    }

    std::istream& operator>>(std::istream& is, DateTime& dateTime)
    {
        std::string str;
        is >> str;
        if (!DateTime::fromString(str, dateTime))
        {
            is.setstate(std::ios::failbit);
        }

        return is;
    }
} // namespace dnv::vista::sdk
