#include "dnv/vista/sdk/types/datetime/TimeSpan.h"

#include "Utils.h"

#include <charconv>
#include <cstring>
#include <limits>
#include <string>

namespace dnv::vista::sdk
{
    namespace
    {
        /**
         * @brief Fast-path parser for common ISO 8601 duration formats
         * @details Handles optimized parsing for:
         *          - PT1H, PT2H30M, PT45S (simple integer components)
         *          - PT1H30M45S (all components)
         *          - PT45.5S (fractional seconds)
         * @return true if parsed successfully via fast path, false if fallback needed
         */
        [[nodiscard]] bool tryParseFastPathDuration(std::string_view str, TimeSpan& result) noexcept
        {
            if (str.length() < 3) // Minimum: "PT1H"
            {
                return false;
            }

            // Handle negative durations
            bool isNegative = false;
            if (str[0] == '-')
            {
                isNegative = true;
                str.remove_prefix(1);
                if (str.length() < 3)
                {
                    return false;
                }
            }

            // Must start with 'P'
            if (str[0] != 'P')
            {
                return false;
            }

            // Check for time part indicator 'T'
            if (str.length() < 3 || str[1] != 'T')
            {
                return false; // Fallback for date components (P1D) or complex formats
            }

            // Parse time components: PT[n]H[n]M[n]S
            str.remove_prefix(2); // Skip "PT"

            double totalSeconds = 0;
            bool foundComponent = false;

            // Parse hours (if present)
            auto hPos = str.find('H');
            if (hPos != std::string_view::npos)
            {
                double hours = 0;
                if (!internal::fastParseDecimal(str.substr(0, hPos), hours))
                {
                    return false;
                }
                totalSeconds += hours * internal::constants::SECONDS_PER_HOUR;
                str.remove_prefix(hPos + 1);
                foundComponent = true;
            }

            // Parse minutes (if present)
            auto mPos = str.find('M');
            if (mPos != std::string_view::npos)
            {
                double minutes = 0;
                if (!internal::fastParseDecimal(str.substr(0, mPos), minutes))
                {
                    return false;
                }
                totalSeconds += minutes * internal::constants::SECONDS_PER_MINUTE;
                str.remove_prefix(mPos + 1);
                foundComponent = true;
            }

            // Parse seconds (if present)
            auto sPos = str.find('S');
            if (sPos != std::string_view::npos)
            {
                double seconds = 0;
                if (!internal::fastParseDecimal(str.substr(0, sPos), seconds))
                {
                    return false;
                }
                totalSeconds += seconds;
                str.remove_prefix(sPos + 1);
                foundComponent = true;
            }

            // Must have at least one component and consumed entire string
            if (!foundComponent || !str.empty())
            {
                return false;
            }

            // Apply negative sign
            if (isNegative)
            {
                totalSeconds = -totalSeconds;
            }

            result = TimeSpan::fromSeconds(totalSeconds);
            return true;
        }
    } // namespace

    std::string TimeSpan::toString() const
    {
        StringBuilder sb;
        toString(sb);
        return sb.toString();
    }

    void TimeSpan::toString(StringBuilder& sb) const
    {
        // Handle negative durations
        bool isNegative{ m_ticks < 0 };
        if (isNegative)
        {
            sb += '-';
        }

        sb += 'P';

        const std::uint64_t absTicks{ m_ticks < 0 ? 0u - static_cast<std::uint64_t>(m_ticks)
                                                  : static_cast<std::uint64_t>(m_ticks) };
        std::uint64_t totalSeconds{ absTicks / static_cast<std::uint64_t>(internal::constants::TICKS_PER_SECOND) };

        // Break down into days, hours, minutes, seconds
        std::int32_t days{ static_cast<std::int32_t>(totalSeconds / internal::constants::SECONDS_PER_DAY) };
        std::int32_t remainingSeconds{ static_cast<std::int32_t>(totalSeconds % internal::constants::SECONDS_PER_DAY) };
        std::int32_t hours{ remainingSeconds / internal::constants::SECONDS_PER_HOUR };
        std::int32_t minutes{ (remainingSeconds % internal::constants::SECONDS_PER_HOUR) /
                              internal::constants::SECONDS_PER_MINUTE };
        std::int32_t seconds{ remainingSeconds % internal::constants::SECONDS_PER_MINUTE };

        // Output days if present
        if (days > 0)
        {
            sb.append(static_cast<std::int64_t>(days)).append('D');
        }

        // Check if we have any time components
        std::uint64_t fractionalTicks{ absTicks % static_cast<std::uint64_t>(internal::constants::TICKS_PER_SECOND) };
        bool hasTimeComponent{ hours > 0 || minutes > 0 || seconds > 0 || fractionalTicks > 0 };

        // Only output time component if present
        if (hasTimeComponent)
        {
            sb.append('T');

            if (hours > 0)
            {
                sb.append(static_cast<std::int64_t>(hours)).append('H');
            }

            if (minutes > 0)
            {
                sb.append(static_cast<std::int64_t>(minutes)).append('M');
            }

            // Include seconds with or without fractional part
            if (seconds > 0 || fractionalTicks > 0)
            {
                if (fractionalTicks > 0)
                {
                    // Format fractional seconds directly to avoid allocations
                    char fracBuffer[8];
                    std::snprintf(fracBuffer, sizeof(fracBuffer), "%07lld", static_cast<long long>(fractionalTicks));

                    // Strip trailing zeros
                    std::size_t fracLen{ 7 };
                    while (fracLen > 1 && fracBuffer[fracLen - 1] == '0')
                    {
                        --fracLen;
                    }

                    sb.append(static_cast<std::int64_t>(seconds))
                        .append('.')
                        .append(std::string_view{ fracBuffer, fracLen })
                        .append('S');
                }
                else
                {
                    sb.append(static_cast<std::int64_t>(seconds)).append('S');
                }
            }
        }
        else if (days == 0)
        {
            // No days and no time components: output PT0S for zero duration
            sb.append("T0S");
        }
    }

    bool TimeSpan::fromString(std::string_view iso8601DurationString, TimeSpan& result) noexcept
    {
        if (iso8601DurationString.empty())
        {
            return false;
        }

        // Try fast-path for common ISO 8601 duration formats (PT1H30M, PT45S, etc.)
        if (tryParseFastPathDuration(iso8601DurationString, result))
        {
            return true;
        }

        // Handle numeric seconds format (convenience)
        if (iso8601DurationString.find_first_not_of("0123456789.-") == std::string_view::npos)
        {
            double seconds{};

            if (internal::fastParseDecimal(iso8601DurationString, seconds))
            {
                result = TimeSpan::fromSeconds(seconds);
                return true;
            }
            return false;
        }

        // Check for negative sign
        bool isNegative{ false };
        std::string_view parseStr{ iso8601DurationString };
        if (!parseStr.empty() && parseStr[0] == '-')
        {
            isNegative = true;
            parseStr = parseStr.substr(1);
        }

        if (parseStr.length() > 1 && parseStr[0] == 'P')
        {
            double days{ 0.0 };
            double totalSeconds{ 0.0 };
            bool foundComponent{ false };

            // Parse days component (P[n]D)
            auto tPos{ parseStr.find('T') };

            // Parse date part (before T, or entire string if no T)
            auto datePart{ tPos != std::string_view::npos ? parseStr.substr(1, tPos - 1) : parseStr.substr(1) };
            if (!datePart.empty())
            {
                auto dPos{ datePart.find('D') };
                if (dPos != std::string_view::npos)
                {
                    auto dayStr{ datePart.substr(0, dPos) };

                    if (internal::fastParseDecimal(dayStr, days))
                    {
                        foundComponent = true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }

            // Parse time part after T (if present)
            if (tPos != std::string_view::npos)
            {
                auto timePart{ parseStr.substr(tPos + 1) };
                auto originalTimePart{ timePart }; // Keep original for position tracking

                // Must have at least one component after T (not just "PT")
                if (timePart.empty())
                {
                    return false;
                }

                // Find all component positions in the original string for order validation
                auto hPos{ originalTimePart.find('H') };
                auto mPos{ originalTimePart.find('M') };
                auto sPos{ originalTimePart.find('S') };

                // Detect duplicate components
                if (hPos != std::string_view::npos && originalTimePart.find('H', hPos + 1) != std::string_view::npos)
                {
                    return false; // Duplicate hours
                }
                if (mPos != std::string_view::npos && originalTimePart.find('M', mPos + 1) != std::string_view::npos)
                {
                    return false; // Duplicate minutes
                }
                if (sPos != std::string_view::npos && originalTimePart.find('S', sPos + 1) != std::string_view::npos)
                {
                    return false; // Duplicate seconds
                }

                // Validate component order (H before M before S)
                if (hPos != std::string_view::npos && mPos != std::string_view::npos && hPos > mPos)
                {
                    return false; // Minutes before hours
                }
                if (hPos != std::string_view::npos && sPos != std::string_view::npos && hPos > sPos)
                {
                    return false; // Seconds before hours
                }
                if (mPos != std::string_view::npos && sPos != std::string_view::npos && mPos > sPos)
                {
                    return false; // Seconds before minutes
                }

                // Parse hours
                if (hPos != std::string_view::npos)
                {
                    auto hourStr{ timePart.substr(0, hPos) };
                    double hours{};

                    if (internal::fastParseDecimal(hourStr, hours))
                    {
                        totalSeconds += hours * static_cast<double>(internal::constants::SECONDS_PER_HOUR);
                        timePart = timePart.substr(hPos + 1);
                        foundComponent = true;
                    }
                    else
                    {
                        return false;
                    }
                }

                // Parse minutes (recalculate position in current timePart)
                mPos = timePart.find('M');
                if (mPos != std::string::npos)
                {
                    auto minuteStr{ timePart.substr(0, mPos) };
                    double minutes{};

                    if (internal::fastParseDecimal(minuteStr, minutes))
                    {
                        totalSeconds += minutes * static_cast<double>(internal::constants::SECONDS_PER_MINUTE);
                        timePart = timePart.substr(mPos + 1);
                        foundComponent = true;
                    }
                    else
                    {
                        return false;
                    }
                }

                // Parse seconds (recalculate position in current timePart)
                sPos = timePart.find('S');
                if (sPos != std::string::npos)
                {
                    auto secondStr{ timePart.substr(0, sPos) };
                    double seconds{};

                    if (internal::fastParseDecimal(secondStr, seconds))
                    {
                        totalSeconds += seconds;
                        foundComponent = true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }

            // Must have parsed at least one valid component (D, H, M, or S)
            if (!foundComponent)
            {
                return false;
            }

            // Convert days to seconds
            totalSeconds += days * static_cast<double>(internal::constants::SECONDS_PER_DAY);

            // Apply negative sign if present
            if (isNegative)
            {
                totalSeconds = -totalSeconds;
            }

            result = TimeSpan::fromSeconds(totalSeconds);

            return true;
        }

        return false;
    }

    std::optional<TimeSpan> TimeSpan::fromString(std::string_view iso8601DurationString) noexcept
    {
        TimeSpan result;
        if (fromString(iso8601DurationString, result))
        {
            return result;
        }
        return std::nullopt;
    }

    std::chrono::system_clock::duration TimeSpan::toChrono() const noexcept
    {
        // m_ticks * 100 (ticks to nanoseconds) can overflow int64_t for ticks constructed
        // directly near INT64_MAX/MIN via fromTicks(), so the multiplication is saturated
        // rather than performed directly to avoid signed integer overflow (UB).
        constexpr std::int64_t maxTicksForNanos = std::numeric_limits<std::int64_t>::max() / 100;
        constexpr std::int64_t minTicksForNanos = std::numeric_limits<std::int64_t>::min() / 100;

        std::int64_t nanos;
        if (m_ticks > maxTicksForNanos)
        {
            nanos = std::numeric_limits<std::int64_t>::max();
        }
        else if (m_ticks < minTicksForNanos)
        {
            nanos = std::numeric_limits<std::int64_t>::min();
        }
        else
        {
            nanos = m_ticks * 100;
        }

        return std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::nanoseconds{ nanos });
    }

    TimeSpan TimeSpan::fromChrono(const std::chrono::system_clock::duration& duration) noexcept
    {
        return TimeSpan{ std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() / 100 };
    }

    std::ostream& operator<<(std::ostream& os, const TimeSpan& timeSpan)
    {
        os << timeSpan.toString();

        return os;
    }

    std::istream& operator>>(std::istream& is, TimeSpan& timeSpan)
    {
        std::string str;
        is >> str;
        if (!TimeSpan::fromString(str, timeSpan))
        {
            is.setstate(std::ios::failbit);
        }

        return is;
    }
} // namespace dnv::vista::sdk
