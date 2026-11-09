#include <stdexcept>

#include "Constants.h"

namespace dnv::vista::sdk
{
    inline constexpr DateTime::DateTime(std::int64_t ticks) noexcept
        : m_ticks{ ticks }
    {}

    inline DateTime::DateTime(std::chrono::system_clock::time_point timePoint) noexcept
    {
        auto duration{ timePoint.time_since_epoch() };

        using ticks_duration = std::chrono::duration<std::int64_t, std::ratio<1, 10000000>>;
        auto ticks{ std::chrono::duration_cast<ticks_duration>(duration) };

        m_ticks = internal::constants::UNIX_EPOCH_TICKS + ticks.count();

        if (m_ticks < internal::constants::MIN_DATETIME_TICKS)
        {
            m_ticks = internal::constants::MIN_DATETIME_TICKS;
        }
        else if (m_ticks > internal::constants::MAX_DATETIME_TICKS)
        {
            m_ticks = internal::constants::MAX_DATETIME_TICKS;
        }
    }

    inline DateTime::DateTime(std::string_view iso8601String)
    {
        DateTime result;
        if (!fromString(iso8601String, result))
        {
            throw std::invalid_argument{ "Invalid ISO 8601 DateTime string" };
        }
        m_ticks = result.m_ticks;
    }

    inline DateTime::DateTime(const char* iso8601String)
        : DateTime{ std::string_view{ iso8601String } }
    {}

    inline constexpr DateTime::DateTime() noexcept
        : m_ticks{ internal::constants::MIN_DATETIME_TICKS }
    {}

    inline constexpr std::strong_ordering DateTime::operator<=>(const DateTime& other) const noexcept
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

    inline constexpr bool DateTime::operator==(const DateTime& other) const noexcept
    {
        return m_ticks == other.m_ticks;
    }

    inline constexpr DateTime DateTime::operator+(const TimeSpan& duration) const noexcept
    {
        return DateTime{ m_ticks + duration.ticks() };
    }

    inline constexpr DateTime DateTime::operator-(const TimeSpan& duration) const noexcept
    {
        return DateTime{ m_ticks - duration.ticks() };
    }

    inline constexpr TimeSpan DateTime::operator-(const DateTime& other) const noexcept
    {
        return TimeSpan{ m_ticks - other.m_ticks };
    }

    inline constexpr DateTime& DateTime::operator+=(const TimeSpan& duration) noexcept
    {
        m_ticks += duration.ticks();

        return *this;
    }

    inline constexpr DateTime& DateTime::operator-=(const TimeSpan& duration) noexcept
    {
        m_ticks -= duration.ticks();

        return *this;
    }

    inline constexpr std::int64_t DateTime::ticks() const noexcept
    {
        return m_ticks;
    }

    inline constexpr std::int64_t DateTime::toEpochSeconds() const noexcept
    {
        return (m_ticks - internal::constants::UNIX_EPOCH_TICKS) / internal::constants::TICKS_PER_SECOND;
    }

    inline constexpr std::int64_t DateTime::toEpochMilliseconds() const noexcept
    {
        return (m_ticks - internal::constants::UNIX_EPOCH_TICKS) / internal::constants::TICKS_PER_MILLISECOND;
    }

    inline constexpr bool DateTime::isLeapYear(std::int32_t year) noexcept
    {
        return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    }

    inline constexpr std::int32_t DateTime::daysInMonth(std::int32_t year, std::int32_t month) noexcept
    {
        if (month < 1 || month > 12)
        {
            return 0;
        }

        if (month == 2 && isLeapYear(year))
        {
            return 29;
        }

        switch (month)
        {
            case 1:
            {
                return 31;
            }
            case 2:
            {
                return 28; // Non-leap year February
            }
            case 3:
            {
                return 31;
            }
            case 4:
            {
                return 30;
            }
            case 5:
            {
                return 31;
            }
            case 6:
            {
                return 30;
            }
            case 7:
            {
                return 31;
            }
            case 8:
            {
                return 31;
            }
            case 9:
            {
                return 30;
            }
            case 10:
            {
                return 31;
            }
            case 11:
            {
                return 30;
            }
            case 12:
            {
                return 31;
            }
            default:
            {
                return 0;
            }
        }
    }

    inline constexpr DateTime DateTime::min() noexcept
    {
        return DateTime{ internal::constants::MIN_DATETIME_TICKS };
    }

    inline constexpr DateTime DateTime::max() noexcept
    {
        return DateTime{ internal::constants::MAX_DATETIME_TICKS };
    }

    inline constexpr DateTime DateTime::epoch() noexcept
    {
        return DateTime{ internal::constants::UNIX_EPOCH_TICKS };
    }

    inline constexpr DateTime DateTime::fromEpochSeconds(std::int64_t seconds) noexcept
    {
        std::int64_t ticks{ internal::constants::UNIX_EPOCH_TICKS + (seconds * internal::constants::TICKS_PER_SECOND) };

        return DateTime{ ticks };
    }

    inline constexpr DateTime DateTime::fromEpochMilliseconds(std::int64_t milliseconds) noexcept
    {
        std::int64_t ticks{ internal::constants::UNIX_EPOCH_TICKS +
                            (milliseconds * internal::constants::TICKS_PER_MILLISECOND) };

        return DateTime{ ticks };
    }
} // namespace dnv::vista::sdk
