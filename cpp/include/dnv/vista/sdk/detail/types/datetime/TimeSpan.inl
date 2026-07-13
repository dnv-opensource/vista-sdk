#include <cmath>
#include <limits>
#include <stdexcept>

#include "Constants.h"

namespace dnv::vista::sdk
{
    inline constexpr TimeSpan::TimeSpan(std::int64_t ticks) noexcept
        : m_ticks{ ticks }
    {}

    inline constexpr TimeSpan::TimeSpan(std::chrono::system_clock::duration duration) noexcept
        : m_ticks{ std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() / 100 }
    {}

    inline TimeSpan::TimeSpan(std::string_view iso8601String)
    {
        TimeSpan result;
        if (!fromString(iso8601String, result))
        {
            throw std::invalid_argument{ "Invalid ISO 8601 duration string format" };
        }
        m_ticks = result.m_ticks;
    }

    inline constexpr std::strong_ordering TimeSpan::operator<=>(const TimeSpan& other) const noexcept
    {
        if (m_ticks < other.m_ticks)
        {
            return std::strong_ordering::less;
        }
        if (m_ticks > other.m_ticks)
        {
            return std::strong_ordering::greater;
        }

        return std::strong_ordering::equal;
    }

    inline constexpr bool TimeSpan::operator==(const TimeSpan& other) const noexcept
    {
        return m_ticks == other.m_ticks;
    }

    inline constexpr TimeSpan TimeSpan::operator+(const TimeSpan& other) const noexcept
    {
        return TimeSpan{ m_ticks + other.m_ticks };
    }

    inline constexpr TimeSpan TimeSpan::operator-(const TimeSpan& other) const noexcept
    {
        return TimeSpan{ m_ticks - other.m_ticks };
    }

    inline constexpr TimeSpan TimeSpan::operator-() const noexcept
    {
        return TimeSpan{ -m_ticks };
    }

    inline constexpr TimeSpan& TimeSpan::operator+=(const TimeSpan& other) noexcept
    {
        m_ticks += other.m_ticks;

        return *this;
    }

    inline constexpr TimeSpan& TimeSpan::operator-=(const TimeSpan& other) noexcept
    {
        m_ticks -= other.m_ticks;

        return *this;
    }

    inline TimeSpan TimeSpan::operator*(double multiplier) const noexcept
    {
        return TimeSpan{ static_cast<std::int64_t>(std::round(static_cast<double>(m_ticks) * multiplier)) };
    }

    inline TimeSpan TimeSpan::operator/(double divisor) const noexcept
    {
        const double result = std::round(static_cast<double>(m_ticks) / divisor);
        if (!std::isfinite(result))
        {
            return TimeSpan{ result > 0 ? std::numeric_limits<std::int64_t>::max()
                                        : std::numeric_limits<std::int64_t>::min() };
        }
        return TimeSpan{ static_cast<std::int64_t>(result) };
    }

    inline double TimeSpan::operator/(const TimeSpan& other) const noexcept
    {
        return static_cast<double>(m_ticks) / static_cast<double>(other.m_ticks);
    }

    [[nodiscard]] inline TimeSpan operator*(double multiplier, const TimeSpan& ts) noexcept
    {
        return ts * multiplier;
    }

    inline double TimeSpan::days() const noexcept
    {
        return static_cast<double>(m_ticks) / internal::constants::TICKS_PER_DAY;
    }

    inline double TimeSpan::hours() const noexcept
    {
        return static_cast<double>(m_ticks) / internal::constants::TICKS_PER_HOUR;
    }

    inline double TimeSpan::minutes() const noexcept
    {
        return static_cast<double>(m_ticks) / internal::constants::TICKS_PER_MINUTE;
    }

    inline double TimeSpan::seconds() const noexcept
    {
        return static_cast<double>(m_ticks) / internal::constants::TICKS_PER_SECOND;
    }

    inline double TimeSpan::milliseconds() const noexcept
    {
        return static_cast<double>(m_ticks) / internal::constants::TICKS_PER_MILLISECOND;
    }

    inline double TimeSpan::microseconds() const noexcept
    {
        return static_cast<double>(m_ticks) / internal::constants::TICKS_PER_MICROSECOND;
    }

    inline double TimeSpan::nanoseconds() const noexcept
    {
        return static_cast<double>(m_ticks) * internal::constants::NANOSECONDS_PER_TICK;
    }

    inline constexpr std::int64_t TimeSpan::ticks() const noexcept
    {
        return m_ticks;
    }

    namespace internal
    {
        inline constexpr std::int64_t saturatingTicksCast(double ticks) noexcept
        {
            constexpr double INT64_MIN_AS_DOUBLE = static_cast<double>(std::numeric_limits<std::int64_t>::min());
            constexpr double INT64_MAX_AS_DOUBLE = static_cast<double>(std::numeric_limits<std::int64_t>::max());

            if (ticks <= INT64_MIN_AS_DOUBLE)
            {
                return std::numeric_limits<std::int64_t>::min();
            }
            if (ticks >= INT64_MAX_AS_DOUBLE)
            {
                return std::numeric_limits<std::int64_t>::max();
            }
            return static_cast<std::int64_t>(ticks);
        }
    } // namespace internal

    inline constexpr TimeSpan TimeSpan::fromDays(double days) noexcept
    {
        return TimeSpan{ internal::saturatingTicksCast(days * internal::constants::TICKS_PER_DAY) };
    }

    inline constexpr TimeSpan TimeSpan::fromHours(double hours) noexcept
    {
        return TimeSpan{ internal::saturatingTicksCast(hours * internal::constants::TICKS_PER_HOUR) };
    }

    inline constexpr TimeSpan TimeSpan::fromMinutes(double minutes) noexcept
    {
        return TimeSpan{ internal::saturatingTicksCast(minutes * internal::constants::TICKS_PER_MINUTE) };
    }

    inline constexpr TimeSpan TimeSpan::fromSeconds(double seconds) noexcept
    {
        return TimeSpan{ internal::saturatingTicksCast(seconds * internal::constants::TICKS_PER_SECOND) };
    }

    inline constexpr TimeSpan TimeSpan::fromMilliseconds(double milliseconds) noexcept
    {
        return TimeSpan{ internal::saturatingTicksCast(milliseconds * internal::constants::TICKS_PER_MILLISECOND) };
    }

    inline constexpr TimeSpan TimeSpan::fromMicroseconds(double microseconds) noexcept
    {
        return TimeSpan{ internal::saturatingTicksCast(
            std::round(microseconds * internal::constants::TICKS_PER_MICROSECOND)) };
    }

    inline constexpr TimeSpan TimeSpan::fromTicks(std::int64_t ticks) noexcept
    {
        return TimeSpan{ ticks };
    }

    namespace literals
    {
        inline constexpr TimeSpan operator""_d(long double days) noexcept
        {
            return TimeSpan::fromDays(static_cast<double>(days));
        }

        inline constexpr TimeSpan operator""_d(unsigned long long days) noexcept
        {
            return TimeSpan::fromDays(static_cast<double>(days));
        }

        inline constexpr TimeSpan operator""_h(long double hours) noexcept
        {
            return TimeSpan::fromHours(static_cast<double>(hours));
        }

        inline constexpr TimeSpan operator""_h(unsigned long long hours) noexcept
        {
            return TimeSpan::fromHours(static_cast<double>(hours));
        }

        inline constexpr TimeSpan operator""_min(long double minutes) noexcept
        {
            return TimeSpan::fromMinutes(static_cast<double>(minutes));
        }

        inline constexpr TimeSpan operator""_min(unsigned long long minutes) noexcept
        {
            return TimeSpan::fromMinutes(static_cast<double>(minutes));
        }

        inline constexpr TimeSpan operator""_s(long double seconds) noexcept
        {
            return TimeSpan::fromSeconds(static_cast<double>(seconds));
        }

        inline constexpr TimeSpan operator""_s(unsigned long long seconds) noexcept
        {
            return TimeSpan::fromSeconds(static_cast<double>(seconds));
        }

        inline constexpr TimeSpan operator""_ms(long double milliseconds) noexcept
        {
            return TimeSpan::fromMilliseconds(static_cast<double>(milliseconds));
        }

        inline constexpr TimeSpan operator""_ms(unsigned long long milliseconds) noexcept
        {
            return TimeSpan::fromMilliseconds(static_cast<double>(milliseconds));
        }

        inline constexpr TimeSpan operator""_us(long double microseconds) noexcept
        {
            return TimeSpan::fromMicroseconds(static_cast<double>(microseconds));
        }

        inline constexpr TimeSpan operator""_us(unsigned long long microseconds) noexcept
        {
            return TimeSpan::fromMicroseconds(static_cast<double>(microseconds));
        }

        inline constexpr TimeSpan operator""_ns(long double nanoseconds) noexcept
        {
            return TimeSpan::fromTicks(static_cast<std::int64_t>(std::round(static_cast<double>(nanoseconds) / 100.0)));
        }

        inline constexpr TimeSpan operator""_ns(unsigned long long nanoseconds) noexcept
        {
            return TimeSpan::fromTicks(static_cast<std::int64_t>(nanoseconds / 100ULL));
        }
    } // namespace literals
} // namespace dnv::vista::sdk
