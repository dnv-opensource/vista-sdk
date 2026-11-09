#include "dnv/vista/sdk/types/datetime/DateTimeOffset.h"

#include "TimeZoneOffsetCache.h"
#include "Utils.h"

#include <charconv>
#include <cstring>
#include <istream>
#include <stdexcept>

namespace dnv::vista::sdk
{
    namespace internal
    {
        /** @brief Validate timezone offset is within valid range (±14:00:00) */
        static constexpr bool isValidOffset(const TimeSpan& offset) noexcept
        {
            // Use integer tick comparison to avoid floating-point precision issues
            const auto offsetTicks{ offset.ticks() };

            // ±14 hours = ±840 minutes = ±50,400 seconds = ±504,000,000,000 ticks
            constexpr std::int64_t MAX_OFFSET_TICKS{ static_cast<std::int64_t>(internal::constants::HOURS_PER_DAY) *
                                                     internal::constants::SECONDS_PER_HOUR *
                                                     internal::constants::TICKS_PER_SECOND };

            return offsetTicks >= -MAX_OFFSET_TICKS && offsetTicks <= MAX_OFFSET_TICKS;
        }

        /** @brief Append timezone offset */
        static void appendOffset(
            StringBuilder& sb, std::int32_t offsetMinutes, DateTime::Format format = DateTime::Format::Iso8601) noexcept
        {
            if (offsetMinutes == 0 && format != DateTime::Format::Iso8601Extended)
            {
                sb += 'Z';
                return;
            }

            const auto absMinutes{ std::abs(offsetMinutes) };
            const auto offsetHours{ absMinutes / internal::constants::MINUTES_PER_HOUR };
            const auto offsetMins{ absMinutes % internal::constants::MINUTES_PER_HOUR };

            sb += (offsetMinutes >= 0 ? '+' : '-');
            internal::appendTwoDigits(sb, offsetHours);
            sb += ':';
            internal::appendTwoDigits(sb, offsetMins);
        }

        /** @brief Format ISO 8601 basic (compact) format with offset */
        static void formatIso8601Basic(StringBuilder& sb, const DateTimeOffset& dto)
        {
            const auto ticks{ dto.dateTime().ticks() };
            std::int32_t y, mon, d, h, min, sec, ms;
            internal::dateComponentsFromTicks(ticks, y, mon, d);
            internal::timeComponentsFromTicks(ticks, h, min, sec, ms);

            // Compact format without separators: YYYYMMDDTHHMMSS±HHMM
            internal::appendFourDigits(sb, y);
            internal::appendTwoDigits(sb, mon);
            internal::appendTwoDigits(sb, d);
            sb += 'T';
            internal::appendTwoDigits(sb, h);
            internal::appendTwoDigits(sb, min);
            internal::appendTwoDigits(sb, sec);

            // Offset in compact format (±HHMM)
            const auto offsetMinutes{ dto.totalOffsetMinutes() };
            const auto absMinutes{ std::abs(offsetMinutes) };
            const auto offsetHours{ absMinutes / internal::constants::MINUTES_PER_HOUR };
            const auto offsetMins{ absMinutes % internal::constants::MINUTES_PER_HOUR };

            sb += (offsetMinutes >= 0 ? '+' : '-');
            internal::appendTwoDigits(sb, offsetHours);
            internal::appendTwoDigits(sb, offsetMins);
        }

        /** @brief Format ISO 8601 datetime with offset */
        static void formatIso8601(StringBuilder& sb, const DateTimeOffset& dto, DateTime::Format format)
        {
            const auto ticks{ dto.dateTime().ticks() };
            std::int32_t y, mon, d, h, min, sec, ms;
            internal::dateComponentsFromTicks(ticks, y, mon, d);
            internal::timeComponentsFromTicks(ticks, h, min, sec, ms);

            // Date part: YYYY-MM-DD
            internal::appendFourDigits(sb, y);
            sb += '-';
            internal::appendTwoDigits(sb, mon);
            sb += '-';
            internal::appendTwoDigits(sb, d);
            sb += 'T';

            // Time part: HH:mm:ss
            internal::appendTwoDigits(sb, h);
            sb += ':';
            internal::appendTwoDigits(sb, min);
            sb += ':';
            internal::appendTwoDigits(sb, sec);

            // Add fractional seconds for extended formats
            if (format == DateTime::Format::Iso8601Precise)
            {
                const std::int64_t fractionalTicks = ticks % internal::constants::TICKS_PER_SECOND;
                char fracBuffer[8];
                fracBuffer[0] = '.';

                // Format 7-digit fractional part with zero padding
                const auto ptr = std::to_chars(fracBuffer + 1, fracBuffer + 8, fractionalTicks).ptr;
                const auto fracLen = ptr - fracBuffer;
                const auto paddingNeeded = 8 - fracLen;

                // Shift digits right and add leading zeros if needed
                if (paddingNeeded > 0)
                {
                    std::memmove(fracBuffer + 1 + paddingNeeded, fracBuffer + 1, fracLen - 1);
                    std::memset(fracBuffer + 1, '0', paddingNeeded);
                }

                sb.append(std::string_view{ fracBuffer, 8 });
            }
            else if (format == DateTime::Format::Iso8601PreciseTrimmed)
            {
                const std::int64_t fractionalTicks = ticks % internal::constants::TICKS_PER_SECOND;

                if (fractionalTicks > 0)
                {
                    char fracBuffer[8];
                    fracBuffer[0] = '.';

                    // Format 7-digit fractional part with zero padding
                    const auto ptr = std::to_chars(fracBuffer + 1, fracBuffer + 8, fractionalTicks).ptr;
                    auto fracLen = ptr - fracBuffer;
                    const auto paddingNeeded = 8 - fracLen;

                    // Shift digits right and add leading zeros if needed
                    if (paddingNeeded > 0)
                    {
                        std::memmove(fracBuffer + 1 + paddingNeeded, fracBuffer + 1, fracLen - 1);
                        std::memset(fracBuffer + 1, '0', paddingNeeded);
                        fracLen = 8;
                    }

                    // Trim trailing zeros
                    while (fracLen > 2 && fracBuffer[fracLen - 1] == '0')
                    {
                        --fracLen;
                    }

                    sb.append(std::string_view{ fracBuffer, static_cast<std::size_t>(fracLen) });
                }
                else
                {
                    sb.append(".0");
                }
            }
            else if (format == DateTime::Format::Iso8601Millis)
            {
                const std::int64_t fractionalTicks = ticks % internal::constants::TICKS_PER_SECOND;
                const std::int32_t milliseconds =
                    static_cast<std::int32_t>(fractionalTicks / internal::constants::TICKS_PER_MILLISECOND);

                char fracBuffer[4];
                fracBuffer[0] = '.';

                // Format 3-digit milliseconds with zero padding
                const auto ptr = std::to_chars(fracBuffer + 1, fracBuffer + 4, milliseconds).ptr;
                const auto fracLen = ptr - fracBuffer;
                const auto paddingNeeded = 4 - fracLen;

                // Shift digits right and add leading zeros if needed
                if (paddingNeeded > 0)
                {
                    std::memmove(fracBuffer + 1 + paddingNeeded, fracBuffer + 1, fracLen - 1);
                    std::memset(fracBuffer + 1, '0', paddingNeeded);
                }

                sb.append(std::string_view{ fracBuffer, 4 });
            }
            else if (format == DateTime::Format::Iso8601Micros)
            {
                const std::int64_t fractionalTicks = ticks % internal::constants::TICKS_PER_SECOND;
                const std::int32_t microseconds =
                    static_cast<std::int32_t>(fractionalTicks / internal::constants::TICKS_PER_MICROSECOND);

                char fracBuffer[7];
                fracBuffer[0] = '.';

                // Format 6-digit microseconds with zero padding
                const auto ptr = std::to_chars(fracBuffer + 1, fracBuffer + 7, microseconds).ptr;
                const auto fracLen = ptr - fracBuffer;
                const auto paddingNeeded = 7 - fracLen;

                // Shift digits right and add leading zeros if needed
                if (paddingNeeded > 0)
                {
                    std::memmove(fracBuffer + 1 + paddingNeeded, fracBuffer + 1, fracLen - 1);
                    std::memset(fracBuffer + 1, '0', paddingNeeded);
                }

                sb.append(std::string_view{ fracBuffer, 7 });
            }

            // Offset part
            appendOffset(sb, dto.totalOffsetMinutes(), format);
        }

        /** @brief Format date only */
        static void formatDateOnly(StringBuilder& sb, const DateTimeOffset& dto)
        {
            std::int32_t y, mon, d;
            internal::dateComponentsFromTicks(dto.dateTime().ticks(), y, mon, d);

            internal::appendFourDigits(sb, y);
            sb += '-';
            internal::appendTwoDigits(sb, mon);
            sb += '-';
            internal::appendTwoDigits(sb, d);
        }

        /** @brief Format time only with offset */
        static void formatTimeOnly(StringBuilder& sb, const DateTimeOffset& dto)
        {
            std::int32_t h, min, sec, ms;
            internal::timeComponentsFromTicks(dto.dateTime().ticks(), h, min, sec, ms);

            internal::appendTwoDigits(sb, h);
            sb += ':';
            internal::appendTwoDigits(sb, min);
            sb += ':';
            internal::appendTwoDigits(sb, sec);

            appendOffset(sb, dto.totalOffsetMinutes());
        }
    } // namespace internal

    namespace
    {
        /**
         * @brief Fast-path parser for standard ISO 8601 formats with timezone offset
         * @details Handles the most common formats:
         *          - "YYYY-MM-DDTHH:mm:ss+HH:MM" (25 chars)
         *          - "YYYY-MM-DDTHH:mm:ssZ" (20 chars)
         *          - "YYYY-MM-DDTHH:mm:ss.f+HH:MM" (26-32 chars)
         * @return true if parsed successfully via fast path, false if fallback needed
         */
        [[nodiscard]] bool tryParseFastPathOffset(std::string_view str, DateTimeOffset& result) noexcept
        {
            const std::size_t len = str.length();

            // Minimum length: "YYYY-MM-DDTHH:mm:ssZ" (20 chars)
            if (len < 20)
            {
                return false;
            }

            const char* data = str.data();

            // Validate fixed separators and digit positions for date/time part
            if (data[4] != '-' || data[7] != '-' || data[10] != 'T' || data[13] != ':' || data[16] != ':' ||
                !internal::areDigits(data, 4) || !internal::areDigits(data + 5, 2) ||
                !internal::areDigits(data + 8, 2) || !internal::areDigits(data + 11, 2) ||
                !internal::areDigits(data + 14, 2) || !internal::areDigits(data + 17, 2))
            {
                return false;
            }

            // Parse date/time components
            const std::int32_t year = internal::parse4Digits(data);
            const std::int32_t month = internal::parse2Digits(data + 5);
            const std::int32_t day = internal::parse2Digits(data + 8);
            const std::int32_t hour = internal::parse2Digits(data + 11);
            const std::int32_t minute = internal::parse2Digits(data + 14);
            const std::int32_t second = internal::parse2Digits(data + 17);

            // Validate components (basic range checks)
            if (month < 1 || month > 12 || day < 1 || day > 31 || hour < 0 || hour > 23 || minute < 0 || minute > 59 ||
                second < 0 || second > 59)
            {
                return false;
            }

            // Additional day-of-month validation
            if (day > DateTime::daysInMonth(year, month))
            {
                return false;
            }

            std::int32_t fractionalTicks = 0;
            std::size_t pos = 19;

            // Handle fractional seconds if present
            if (data[19] == '.')
            {
                pos = 20; // Start after '.'

                // Parse up to 7 fractional digits
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

                // Pad to 7 digits
                while (fractionDigits < 7)
                {
                    fractionValue *= 10;
                    ++fractionDigits;
                }

                fractionalTicks = fractionValue;

                // Skip remaining fractional digits
                while (pos < len && string::isDigit(data[pos]))
                {
                    ++pos;
                }
            }

            // Parse timezone offset
            TimeSpan offset{ 0 };

            if (pos >= len)
            {
                return false; // Missing timezone indicator
            }

            if (data[pos] == 'Z')
            {
                // UTC indicator
                offset = TimeSpan{ 0 };
                ++pos;
            }
            else if (data[pos] == '+' || data[pos] == '-')
            {
                const bool isNegative = (data[pos] == '-');
                ++pos;

                // Need at least 5 chars for "+HH:MM"
                if (len - pos < 5)
                {
                    return false;
                }

                // Fast-path: expect "+HH:MM" or "-HH:MM" format (5 chars: HH:MM)
                if (pos + 5 == len && data[pos + 2] == ':' && internal::areDigits(data + pos, 2) &&
                    internal::areDigits(data + pos + 3, 2))
                {
                    const std::int32_t offsetHours = internal::parse2Digits(data + pos);
                    const std::int32_t offsetMinutes = internal::parse2Digits(data + pos + 3);

                    // Validate offset range (±00:00 to ±14:00)
                    if (offsetHours < 0 || offsetHours > 14 || offsetMinutes < 0 || offsetMinutes > 59)
                    {
                        return false;
                    }
                    if (offsetHours == 14 && offsetMinutes > 0)
                    {
                        return false; // Max is ±14:00
                    }

                    std::int32_t totalOffsetMinutes = offsetHours * 60 + offsetMinutes;
                    if (isNegative)
                    {
                        totalOffsetMinutes = -totalOffsetMinutes;
                    }

                    offset = TimeSpan::fromMinutes(totalOffsetMinutes);
                    pos += 5;
                }
                else
                {
                    // Non-standard offset format - use fallback
                    return false;
                }
            }
            else
            {
                // Invalid timezone indicator
                return false;
            }

            // Should have consumed entire string
            if (pos != len)
            {
                return false;
            }

            // Build DateTime and DateTimeOffset
            // First construct a DateTime with date and time components (fractionalTicks converted to milliseconds)
            const std::int32_t milliseconds =
                static_cast<std::int32_t>(fractionalTicks / internal::constants::TICKS_PER_MILLISECOND);
            DateTime dateTime{ year, month, day, hour, minute, second, milliseconds };

            // Add remaining sub-millisecond ticks (100ns precision)
            const std::int32_t remainingTicks =
                static_cast<std::int32_t>(fractionalTicks % internal::constants::TICKS_PER_MILLISECOND);
            if (remainingTicks > 0)
            {
                dateTime = DateTime{ dateTime.ticks() + remainingTicks };
            }

            result = DateTimeOffset{ dateTime, offset };
            return true;
        }
    } // namespace

    DateTimeOffset::DateTimeOffset(const DateTime& dateTime) noexcept
        : m_dateTime{ dateTime },
          m_offset{ internal::systemTimezoneOffset(dateTime) }
    {}

    DateTime DateTimeOffset::utcDateTime() const noexcept
    {
        return DateTime{ utcTicks() };
    }

    DateTime DateTimeOffset::localDateTime() const noexcept
    {
        return m_dateTime;
    }

    DateTimeOffset DateTimeOffset::date() const noexcept
    {
        return DateTimeOffset{ m_dateTime.date(), m_offset };
    }

    DateTimeOffset DateTimeOffset::toOffset(const TimeSpan& newOffset) const noexcept
    {
        // Convert to UTC, then apply new offset
        const auto utcTime{ utcDateTime() };

        return DateTimeOffset{ utcTime + newOffset, newOffset };
    }

    DateTimeOffset DateTimeOffset::toUniversalTime() const noexcept
    {
        return DateTimeOffset{ utcDateTime(), TimeSpan{ 0 } };
    }

    DateTimeOffset DateTimeOffset::toLocalTime() const noexcept
    {
        const auto utcTime{ utcDateTime() };
        const auto localOffset{ internal::systemTimezoneOffset(utcTime) };

        return DateTimeOffset{ utcTime + localOffset, localOffset };
    }

    std::int64_t DateTimeOffset::toFILETIME() const noexcept
    {
        return utcTicks() - internal::constants::MICROSOFT_FILETIME_EPOCH_TICKS;
    }

    DateTimeOffset DateTimeOffset::addDays(double days) const noexcept
    {
        return DateTimeOffset{ m_dateTime + TimeSpan::fromDays(days), m_offset };
    }

    DateTimeOffset DateTimeOffset::addHours(double hours) const noexcept
    {
        return DateTimeOffset{ m_dateTime + TimeSpan::fromHours(hours), m_offset };
    }

    DateTimeOffset DateTimeOffset::addMilliseconds(double milliseconds) const noexcept
    {
        return DateTimeOffset{ m_dateTime + TimeSpan::fromMilliseconds(milliseconds), m_offset };
    }

    DateTimeOffset DateTimeOffset::addMinutes(double minutes) const noexcept
    {
        return DateTimeOffset{ m_dateTime + TimeSpan::fromMinutes(minutes), m_offset };
    }

    DateTimeOffset DateTimeOffset::addMonths(std::int32_t months) const noexcept
    {
        // Extract date components
        auto year{ m_dateTime.year() };
        auto month{ m_dateTime.month() };
        const auto day{ m_dateTime.day() };
        const auto timeOfDay{ m_dateTime.timeOfDay() };

        // Add months with proper year overflow handling
        month += months;
        while (month > 12)
        {
            month -= 12;
            year++;
        }
        while (month < 1)
        {
            month += 12;
            year--;
        }

        // Handle day overflow (e.g., Jan 31 + 1 month = Feb 28/29)
        const auto daysInTargetMonth{ DateTime::daysInMonth(year, month) };
        const auto adjustedDay{ std::min(day, daysInTargetMonth) };

        // Create new DateTime and add back the time of day
        const auto newDate{ DateTime{ year, month, adjustedDay } + timeOfDay };

        return DateTimeOffset{ newDate, m_offset };
    }

    DateTimeOffset DateTimeOffset::addSeconds(double seconds) const noexcept
    {
        return DateTimeOffset{ m_dateTime + TimeSpan::fromSeconds(seconds), m_offset };
    }

    DateTimeOffset DateTimeOffset::addYears(std::int32_t years) const noexcept
    {
        return addMonths(years * 12);
    }

    std::string DateTimeOffset::toString(DateTime::Format format) const
    {
        StringBuilder sb;
        toString(sb, format);
        return sb.toString();
    }

    void DateTimeOffset::toString(StringBuilder& builder, DateTime::Format format) const
    {
        switch (format)
        {
            case DateTime::Format::Iso8601:
            case DateTime::Format::Iso8601Precise:
            case DateTime::Format::Iso8601PreciseTrimmed:
            case DateTime::Format::Iso8601Millis:
            case DateTime::Format::Iso8601Micros:
            case DateTime::Format::Iso8601Extended:
            {
                internal::formatIso8601(builder, *this, format);
                break;
            }
            case DateTime::Format::Iso8601Basic:
            {
                internal::formatIso8601Basic(builder, *this);
                break;
            }
            case DateTime::Format::Iso8601Date:
            {
                internal::formatDateOnly(builder, *this);
                break;
            }
            case DateTime::Format::Iso8601Time:
            {
                internal::formatTimeOnly(builder, *this);
                break;
            }
            case DateTime::Format::UnixSeconds:
            {
                char buffer[32];
                const auto ptr = std::to_chars(buffer, buffer + 32, toEpochSeconds()).ptr;
                builder.append(std::string_view{ buffer, static_cast<std::size_t>(ptr - buffer) });
                break;
            }
            case DateTime::Format::UnixMilliseconds:
            {
                char buffer[32];
                const auto ptr = std::to_chars(buffer, buffer + 32, toEpochMilliseconds()).ptr;
                builder.append(std::string_view{ buffer, static_cast<std::size_t>(ptr - buffer) });
                break;
            }
            default:
            {
                toString(builder, DateTime::Format::Iso8601);
                break;
            }
        }
    }

    bool DateTimeOffset::isValid() const noexcept
    {
        return m_dateTime.isValid() && internal::isValidOffset(m_offset) &&
               utcTicks() >= internal::constants::MIN_DATETIME_TICKS &&
               utcTicks() <= internal::constants::MAX_DATETIME_TICKS;
    }

    DateTimeOffset DateTimeOffset::now() noexcept
    {
        const auto utcNow{ DateTime::utcNow() };
        const auto localOffset{ internal::systemTimezoneOffset(utcNow) };

        return DateTimeOffset{ utcNow + localOffset, localOffset };
    }

    DateTimeOffset DateTimeOffset::utcNow() noexcept
    {
        return DateTimeOffset{ DateTime::utcNow(), TimeSpan{ 0 } };
    }

    DateTimeOffset DateTimeOffset::today() noexcept
    {
        // Get current local time, then extract just the date part at midnight
        const auto localNow{ DateTimeOffset::now() };

        // Extract local date components
        const auto year{ localNow.year() };
        const auto month{ localNow.month() };
        const auto day{ localNow.day() };

        // Create local midnight for today with the same offset
        return DateTimeOffset{ year, month, day, 0, 0, 0, localNow.offset() };
    }

    DateTimeOffset DateTimeOffset::min() noexcept
    {
        return DateTimeOffset{ DateTime::min(), TimeSpan{ 0 } };
    }

    DateTimeOffset DateTimeOffset::max() noexcept
    {
        return DateTimeOffset{ DateTime::max(), TimeSpan{ 0 } };
    }

    DateTimeOffset DateTimeOffset::epoch() noexcept
    {
        return DateTimeOffset{ DateTime::epoch(), TimeSpan{ 0 } };
    }

    bool DateTimeOffset::fromString(std::string_view iso8601String, DateTimeOffset& result) noexcept
    {
        // Try fast-path for standard ISO 8601 with timezone offset
        if (tryParseFastPathOffset(iso8601String, result))
        {
            return true;
        }

        // Fallback to flexible parser for non-standard formats
        /*
            ISO 8601 compliant parser supporting:
            - Local time without timezone (valid but ambiguous per ISO 8601)
            - UTC indicator: Z
            - Timezone offsets: ±HH:MM (extended), ±HHMM (basic), ±HH (basic)
            - Maximum offset: ±14:00 (±840 minutes)

            Note: ISO 8601 allows local time without timezone designator, though it's
            ambiguous for cross-timezone communication. When no timezone is provided,
            offset defaults to zero (treated as unspecified/local time).
        */

        // Work directly with string_view to avoid allocations
        TimeSpan offset{ 0 };
        DateTime dateTime;
        std::string_view dateTimeStr{ iso8601String };

        // Find timezone indicator - search from right to avoid matching negative years
        std::size_t offsetPos{ std::string_view::npos };
        for (std::size_t i{ iso8601String.length() }; i > 10; --i)
        {
            // Skip date part (YYYY-MM-DD = 10 chars)
            const char ch{ iso8601String[i - 1] };
            if (ch == 'Z' || ch == '+' || ch == '-')
            {
                offsetPos = i - 1;
                break;
            }
        }

        if (offsetPos != std::string_view::npos)
        {
            // Validate no double signs (e.g., "+-", "-+", "++", "--")
            const char prevChar{ iso8601String[offsetPos - 1] };
            if (prevChar == '+' || prevChar == '-')
            {
                return false; // Reject double sign patterns
            }

            if (iso8601String[offsetPos] == 'Z')
            {
                // UTC indicator
                offset = TimeSpan{ 0 };
                dateTimeStr = iso8601String.substr(0, offsetPos);
            }
            else
            {
                // Parse offset: supports +HH:MM, +HHMM, +HH formats
                std::string_view offsetStr{ iso8601String.substr(offsetPos) };
                dateTimeStr = iso8601String.substr(0, offsetPos);

                // Minimum: +H or -H (at least 2 chars: sign + digit)
                if (offsetStr.length() < 2)
                {
                    return false;
                }

                const bool isNegative{ offsetStr[0] == '-' };
                const std::string_view numericPart{ offsetStr.substr(1) };

                std::int32_t hours{ 0 };
                std::int32_t minutes{ 0 };

                const auto colonPos{ numericPart.find(':') };
                if (colonPos != std::string_view::npos)
                {
                    // Format: +HH:MM or +H:MM
                    if (colonPos == 0 || colonPos >= numericPart.length() - 1)
                    {
                        return false;
                    }

                    const std::string_view hoursStr{ numericPart.substr(0, colonPos) };
                    const std::string_view minutesStr{ numericPart.substr(colonPos + 1) };

                    auto [ptrH, ecH] = std::from_chars(hoursStr.data(), hoursStr.data() + hoursStr.size(), hours);
                    auto [ptrM, ecM] =
                        std::from_chars(minutesStr.data(), minutesStr.data() + minutesStr.size(), minutes);

                    if (ecH != std::errc{} || ecM != std::errc{} || ptrH != hoursStr.data() + hoursStr.size() ||
                        ptrM != minutesStr.data() + minutesStr.size())
                    {
                        return false; // Invalid numeric format
                    }
                }
                else if (numericPart.length() == 4)
                {
                    // Format: +HHMM
                    auto [ptrH, ecH] = std::from_chars(numericPart.data(), numericPart.data() + 2, hours);
                    auto [ptrM, ecM] = std::from_chars(numericPart.data() + 2, numericPart.data() + 4, minutes);

                    if (ecH != std::errc{} || ecM != std::errc{} || ptrH != numericPart.data() + 2 ||
                        ptrM != numericPart.data() + 4)
                    {
                        return false; // Invalid numeric format
                    }
                }
                else if (numericPart.length() == 2 || numericPart.length() == 1)
                {
                    // Format: +HH or +H
                    auto [ptr, ec] =
                        std::from_chars(numericPart.data(), numericPart.data() + numericPart.length(), hours);
                    if (ec != std::errc{} || ptr != numericPart.data() + numericPart.length())
                    {
                        return false; // Invalid numeric format
                    }
                    minutes = 0;
                }
                else
                {
                    return false; // Invalid format
                }

                // Validate offset components - ISO 8601 allows ±14:00 maximum
                // Hours must be 0-14, minutes 0-59
                // Special case: if hours == 14, minutes must be 0 (max is exactly ±14:00)
                if (hours < 0 || hours > 14 || minutes < 0 || minutes > 59)
                {
                    return false; // Out of range
                }

                if (hours == 14 && minutes > 0)
                {
                    return false; // Maximum offset is exactly ±14:00, not ±14:01+
                }

                const auto totalMinutes{ hours * internal::constants::MINUTES_PER_HOUR + minutes };
                offset = TimeSpan::fromMinutes(isNegative ? -totalMinutes : totalMinutes);
            }
        }

        // Parse the datetime part
        if (DateTime::fromString(dateTimeStr, dateTime))
        {
            result = DateTimeOffset{ dateTime, offset };
            return true;
        }

        return false;
    }

    std::optional<DateTimeOffset> DateTimeOffset::fromString(std::string_view iso8601String) noexcept
    {
        DateTimeOffset result;
        if (fromString(iso8601String, result))
        {
            return result;
        }
        return std::nullopt;
    }

    DateTimeOffset DateTimeOffset::fromEpochSeconds(std::int64_t seconds) noexcept
    {
        return DateTimeOffset{ DateTime::fromEpochSeconds(seconds), TimeSpan{ 0 } };
    }

    DateTimeOffset DateTimeOffset::fromEpochMilliseconds(std::int64_t milliseconds) noexcept
    {
        return DateTimeOffset{ DateTime::fromEpochMilliseconds(milliseconds), TimeSpan{ 0 } };
    }

    DateTimeOffset DateTimeOffset::fromFILETIME(std::int64_t FILETIME) noexcept
    {
        const std::int64_t ticks{ FILETIME + internal::constants::MICROSOFT_FILETIME_EPOCH_TICKS };
        return DateTimeOffset{ DateTime{ ticks }, TimeSpan{ 0 } };
    }

    std::chrono::system_clock::time_point DateTimeOffset::toChrono() const noexcept
    {
        return utcDateTime().toChrono();
    }

    DateTimeOffset DateTimeOffset::fromChrono(
        const std::chrono::system_clock::time_point& timePoint, std::chrono::seconds offset) noexcept
    {
        return DateTimeOffset{ timePoint, offset };
    }

    std::ostream& operator<<(std::ostream& os, const DateTimeOffset& dateTimeOffset)
    {
        return os << dateTimeOffset.toString();
    }

    std::istream& operator>>(std::istream& is, DateTimeOffset& dateTimeOffset)
    {
        std::string str;
        is >> str;

        if (!DateTimeOffset::fromString(str, dateTimeOffset))
        {
            is.setstate(std::ios::failbit);
        }

        return is;
    }
} // namespace dnv::vista::sdk
