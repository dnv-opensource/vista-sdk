#include <stdexcept>

#include "Constants.h"

namespace dnv::vista::sdk
{
    inline constexpr DateTimeOffset::DateTimeOffset(const DateTime& dateTime, const TimeSpan& offset) noexcept
        : m_dateTime{ dateTime },
          m_offset{ offset }
    {}

    inline constexpr DateTimeOffset::DateTimeOffset(std::int64_t ticks, const TimeSpan& offset) noexcept
        : m_dateTime{ ticks },
          m_offset{ offset }
    {}

    inline DateTimeOffset::DateTimeOffset(
        std::int32_t year, std::int32_t month, std::int32_t day, const TimeSpan& offset) noexcept
        : m_dateTime{ year, month, day },
          m_offset{ offset }
    {}

    inline DateTimeOffset::DateTimeOffset(
        std::int32_t year,
        std::int32_t month,
        std::int32_t day,
        std::int32_t hour,
        std::int32_t minute,
        std::int32_t second,
        const TimeSpan& offset) noexcept
        : m_dateTime{ year, month, day, hour, minute, second },
          m_offset{ offset }
    {}

    inline DateTimeOffset::DateTimeOffset(
        std::int32_t year,
        std::int32_t month,
        std::int32_t day,
        std::int32_t hour,
        std::int32_t minute,
        std::int32_t second,
        std::int32_t millisecond,
        const TimeSpan& offset) noexcept
        : m_dateTime{ year, month, day, hour, minute, second, millisecond },
          m_offset{ offset }
    {}

    inline DateTimeOffset::DateTimeOffset(
        std::int32_t year,
        std::int32_t month,
        std::int32_t day,
        std::int32_t hour,
        std::int32_t minute,
        std::int32_t second,
        std::int32_t millisecond,
        std::int32_t microsecond,
        const TimeSpan& offset) noexcept
        : m_dateTime{ year, month, day, hour, minute, second, millisecond },
          m_offset{ offset }
    {
        // Add microseconds to the DateTime (1 microsecond = 10 ticks)
        m_dateTime += TimeSpan{ microsecond * 10 };
    }

    inline DateTimeOffset::DateTimeOffset(
        std::chrono::system_clock::time_point timePoint, std::chrono::seconds offset) noexcept
        : m_dateTime{ timePoint + offset },
          m_offset{ TimeSpan::fromSeconds(static_cast<double>(offset.count())) }
    {}

    inline DateTimeOffset::DateTimeOffset(std::string_view iso8601String)
        : m_dateTime{ DateTime::min() },
          m_offset{ 0 }
    {
        if (!fromString(iso8601String, *this))
        {
            throw std::invalid_argument{ "Invalid ISO 8601 DateTimeOffset string format" };
        }
    }

    inline DateTimeOffset::DateTimeOffset(const char* iso8601String)
        : DateTimeOffset{ std::string_view{ iso8601String } }
    {}

    inline constexpr DateTimeOffset::DateTimeOffset() noexcept
        : m_dateTime{ DateTime::min() },
          m_offset{ 0 }
    {}

    inline constexpr std::strong_ordering DateTimeOffset::operator<=>(const DateTimeOffset& other) const noexcept
    {
        const auto thisUtc = utcTicks();
        const auto otherUtc = other.utcTicks();

        if (thisUtc < otherUtc)
        {
            return std::strong_ordering::less;
        }
        if (thisUtc > otherUtc)
        {
            return std::strong_ordering::greater;
        }

        return std::strong_ordering::equal;
    }

    inline constexpr bool DateTimeOffset::operator==(const DateTimeOffset& other) const noexcept
    {
        return utcTicks() == other.utcTicks();
    }

    inline constexpr DateTimeOffset DateTimeOffset::operator+(const TimeSpan& duration) const noexcept
    {
        return DateTimeOffset{ m_dateTime + duration, m_offset };
    }

    inline constexpr DateTimeOffset DateTimeOffset::operator-(const TimeSpan& duration) const noexcept
    {
        return DateTimeOffset{ m_dateTime - duration, m_offset };
    }

    inline constexpr TimeSpan DateTimeOffset::operator-(const DateTimeOffset& other) const noexcept
    {
        return TimeSpan{ utcTicks() - other.utcTicks() };
    }

    inline constexpr DateTimeOffset& DateTimeOffset::operator+=(const TimeSpan& duration) noexcept
    {
        m_dateTime += duration;

        return *this;
    }

    inline constexpr DateTimeOffset& DateTimeOffset::operator-=(const TimeSpan& duration) noexcept
    {
        m_dateTime -= duration;

        return *this;
    }

    inline constexpr const DateTime& DateTimeOffset::dateTime() const noexcept
    {
        return m_dateTime;
    }

    inline constexpr const TimeSpan& DateTimeOffset::offset() const noexcept
    {
        return m_offset;
    }

    inline constexpr std::int64_t DateTimeOffset::ticks() const noexcept
    {
        return m_dateTime.ticks();
    }

    inline constexpr std::int64_t DateTimeOffset::utcTicks() const noexcept
    {
        return m_dateTime.ticks() - m_offset.ticks();
    }

    inline std::int32_t DateTimeOffset::year() const noexcept
    {
        return m_dateTime.year();
    }

    inline std::int32_t DateTimeOffset::month() const noexcept
    {
        return m_dateTime.month();
    }

    inline std::int32_t DateTimeOffset::day() const noexcept
    {
        return m_dateTime.day();
    }

    inline std::int32_t DateTimeOffset::hour() const noexcept
    {
        return m_dateTime.hour();
    }

    inline std::int32_t DateTimeOffset::minute() const noexcept
    {
        return m_dateTime.minute();
    }

    inline std::int32_t DateTimeOffset::second() const noexcept
    {
        return m_dateTime.second();
    }

    inline std::int32_t DateTimeOffset::millisecond() const noexcept
    {
        return m_dateTime.millisecond();
    }

    inline std::int32_t DateTimeOffset::microsecond() const noexcept
    {
        const auto remainderTicks{ m_dateTime.ticks() % 10000 };

        return static_cast<std::int32_t>(remainderTicks / 10);
    }

    inline std::int32_t DateTimeOffset::nanosecond() const noexcept
    {
        const auto remainderTicks{ m_dateTime.ticks() % 10 };

        return static_cast<std::int32_t>(remainderTicks * 100);
    }

    inline std::int32_t DateTimeOffset::dayOfWeek() const noexcept
    {
        return m_dateTime.dayOfWeek();
    }

    inline std::int32_t DateTimeOffset::dayOfYear() const noexcept
    {
        return m_dateTime.dayOfYear();
    }

    inline std::int32_t DateTimeOffset::totalOffsetMinutes() const noexcept
    {
        return static_cast<std::int32_t>(m_offset.minutes());
    }

    inline std::int64_t DateTimeOffset::toEpochSeconds() const noexcept
    {
        return utcDateTime().toEpochSeconds();
    }

    inline std::int64_t DateTimeOffset::toEpochMilliseconds() const noexcept
    {
        return utcDateTime().toEpochMilliseconds();
    }

    inline TimeSpan DateTimeOffset::timeOfDay() const noexcept
    {
        return m_dateTime.timeOfDay();
    }

    inline DateTimeOffset DateTimeOffset::add(const TimeSpan& value) const noexcept
    {
        return *this + value;
    }

    inline TimeSpan DateTimeOffset::subtract(const DateTimeOffset& value) const noexcept
    {
        return *this - value;
    }

    inline DateTimeOffset DateTimeOffset::subtract(const TimeSpan& value) const noexcept
    {
        return *this - value;
    }

    inline DateTimeOffset DateTimeOffset::addTicks(std::int64_t ticks) const noexcept
    {
        return *this + TimeSpan{ ticks };
    }

    inline bool DateTimeOffset::equals(const DateTimeOffset& other) const noexcept
    {
        return *this == other;
    }

    inline bool DateTimeOffset::equalsExact(const DateTimeOffset& other) const noexcept
    {
        return m_dateTime == other.m_dateTime && m_offset == other.m_offset;
    }
} // namespace dnv::vista::sdk
