/**
 * @file DateTimeOffset.h
 * @brief DateTime paired with a fixed UTC offset
 * @details Provides timezone-aware datetime operations with 100-nanosecond precision,
 *          ISO 8601 parsing/formatting with timezone offsets, and cross-platform compatibility
 *          Supports date range from January 1, 0001 to December 31, 9999 with offset ±14:00:00
 *
 * @note This implementation includes timezone offset support. For UTC-only operations,
 *       use the DateTime class instead
 * @note Follows ISO 8601-1 with UTC offset. Serializes as @c Z when offset is zero,
 *       per ISO 19848 timestamp requirements
 */

#pragma once

#include "dnv/vista/sdk/utils/StringBuilder.h"
#include "DateTime.h"
#include "TimeSpan.h"

#include <compare>
#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <string_view>

namespace dnv::vista::sdk
{
    /**
     * @brief DateTime paired with a fixed UTC offset
     *
     * Composed of a @c DateTime value (local time) and a @c TimeSpan offset from UTC
     * Range: January 1, 0001 to December 31, 9999, offset ±14:00:00
     */
    class DateTimeOffset final
    {
    public:
        /** @brief Constructs a zero-initialized DateTimeOffset */
        inline constexpr DateTimeOffset() noexcept;

        /**
         * @brief Construct from DateTime and offset
         * @param dateTime The DateTime component (local time)
         * @param offset The timezone offset from UTC
         */
        inline constexpr DateTimeOffset(const DateTime& dateTime, const TimeSpan& offset) noexcept;

        /**
         * @brief Construct from DateTime (assumes local timezone offset)
         * @param dateTime The DateTime to use (local timezone offset will be determined automatically)
         */
        explicit DateTimeOffset(const DateTime& dateTime) noexcept;

        /**
         * @brief Construct from tick count and offset
         * @param ticks Number of 100-nanosecond intervals since January 1, 0001 (local time)
         * @param offset The timezone offset from UTC
         */
        inline constexpr DateTimeOffset(std::int64_t ticks, const TimeSpan& offset) noexcept;

        /**
         * @brief Construct from date components and offset
         * @param year Year component (1-9999)
         * @param month Month component (1-12)
         * @param day Day component (1-31)
         * @param offset The timezone offset from UTC
         */
        inline DateTimeOffset(std::int32_t year, std::int32_t month, std::int32_t day, const TimeSpan& offset) noexcept;

        /**
         * @brief Construct from date and time components and offset
         * @param year Year component (1-9999)
         * @param month Month component (1-12)
         * @param day Day component (1-31)
         * @param hour Hour component (0-23)
         * @param minute Minute component (0-59)
         * @param second Second component (0-59)
         * @param offset The timezone offset from UTC
         */
        inline DateTimeOffset(
            std::int32_t year,
            std::int32_t month,
            std::int32_t day,
            std::int32_t hour,
            std::int32_t minute,
            std::int32_t second,
            const TimeSpan& offset) noexcept;

        /**
         * @brief Construct from date and time components with milliseconds and offset
         * @param year Year component (1-9999)
         * @param month Month component (1-12)
         * @param day Day component (1-31)
         * @param hour Hour component (0-23)
         * @param minute Minute component (0-59)
         * @param second Second component (0-59)
         * @param millisecond Millisecond component (0-999)
         * @param offset The timezone offset from UTC
         */
        inline DateTimeOffset(
            std::int32_t year,
            std::int32_t month,
            std::int32_t day,
            std::int32_t hour,
            std::int32_t minute,
            std::int32_t second,
            std::int32_t millisecond,
            const TimeSpan& offset) noexcept;

        /**
         * @brief Construct from date and time components with microseconds and offset
         * @param year Year component (1-9999)
         * @param month Month component (1-12)
         * @param day Day component (1-31)
         * @param hour Hour component (0-23)
         * @param minute Minute component (0-59)
         * @param second Second component (0-59)
         * @param millisecond Millisecond component (0-999)
         * @param microsecond Microsecond component (0-999)
         * @param offset The timezone offset from UTC
         */
        inline DateTimeOffset(
            std::int32_t year,
            std::int32_t month,
            std::int32_t day,
            std::int32_t hour,
            std::int32_t minute,
            std::int32_t second,
            std::int32_t millisecond,
            std::int32_t microsecond,
            const TimeSpan& offset) noexcept;

        /**
         * @brief Parse from ISO 8601 string with timezone offset
         * @param iso8601String ISO 8601 formatted string with timezone offset to parse
         */
        inline explicit DateTimeOffset(std::string_view iso8601String);

        /**
         * @brief Parse from C-string (convenience for string literals)
         * @param iso8601String ISO 8601 formatted C-string with timezone offset to parse
         */
        inline explicit DateTimeOffset(const char* iso8601String);

        DateTimeOffset(const DateTimeOffset&) = default;
        DateTimeOffset(DateTimeOffset&&) noexcept = default;
        ~DateTimeOffset() = default;

        DateTimeOffset& operator=(const DateTimeOffset&) = default;
        DateTimeOffset& operator=(DateTimeOffset&&) noexcept = default;

        inline constexpr std::strong_ordering operator<=>(const DateTimeOffset& other) const noexcept;
        inline constexpr bool operator==(const DateTimeOffset& other) const noexcept;

        inline constexpr DateTimeOffset operator+(const TimeSpan& duration) const noexcept;
        inline constexpr DateTimeOffset operator-(const TimeSpan& duration) const noexcept;

        inline constexpr TimeSpan operator-(const DateTimeOffset& other) const noexcept;
        inline constexpr DateTimeOffset& operator+=(const TimeSpan& duration) noexcept;
        inline constexpr DateTimeOffset& operator-=(const TimeSpan& duration) noexcept;

        [[nodiscard]] inline constexpr const DateTime& dateTime() const noexcept; ///< Local DateTime component
        [[nodiscard]] inline constexpr const TimeSpan& offset() const noexcept;   ///< UTC offset
        [[nodiscard]] DateTime utcDateTime() const noexcept;                      ///< Equivalent UTC DateTime
        [[nodiscard]] DateTime localDateTime() const noexcept;                    ///< Equivalent local DateTime
        [[nodiscard]] inline constexpr std::int64_t ticks() const noexcept;       ///< Local time as 100ns ticks
        [[nodiscard]] inline constexpr std::int64_t utcTicks() const noexcept;    ///< UTC time as 100ns ticks

        [[nodiscard]] inline std::int32_t year() const noexcept;        ///< Year component (1-9999)
        [[nodiscard]] inline std::int32_t month() const noexcept;       ///< Month component (1-12)
        [[nodiscard]] inline std::int32_t day() const noexcept;         ///< Day component (1-31)
        [[nodiscard]] inline std::int32_t hour() const noexcept;        ///< Hour component (0-23)
        [[nodiscard]] inline std::int32_t minute() const noexcept;      ///< Minute component (0-59)
        [[nodiscard]] inline std::int32_t second() const noexcept;      ///< Second component (0-59)
        [[nodiscard]] inline std::int32_t millisecond() const noexcept; ///< Millisecond component (0-999)
        [[nodiscard]] inline std::int32_t microsecond() const noexcept; ///< Microsecond component (0-999)

        /**
         * @brief Nanosecond component in 100ns increments (0, 100, 200, ..., 900)
         * @note Due to 100-nanosecond tick precision, only multiples of 100 are possible
         */
        [[nodiscard]] inline std::int32_t nanosecond() const noexcept;

        [[nodiscard]] inline std::int32_t dayOfWeek() const noexcept; ///< Day of week (0=Sunday, 6=Saturday)
        [[nodiscard]] inline std::int32_t dayOfYear() const noexcept; ///< Day of year (1-366)

        /**
         * @brief Get offset in total minutes
         * @return The total minutes offset from UTC (positive for East, negative for West)
         */
        [[nodiscard]] inline std::int32_t totalOffsetMinutes() const noexcept;

        [[nodiscard]] inline std::int64_t toEpochSeconds() const noexcept;      ///< Seconds since Unix epoch
        [[nodiscard]] inline std::int64_t toEpochMilliseconds() const noexcept; ///< Milliseconds since Unix epoch
        [[nodiscard]] DateTimeOffset date() const noexcept;       ///< Date component with time set to 00:00:00
        [[nodiscard]] inline TimeSpan timeOfDay() const noexcept; ///< Elapsed time since midnight

        /**
         * @brief Convert to specified offset
         * @param newOffset The new timezone offset to convert to
         * @return DateTimeOffset representing the same instant in time with the specified offset
         */
        [[nodiscard]] DateTimeOffset toOffset(const TimeSpan& newOffset) const noexcept;

        /**
         * @brief Convert to UTC (offset = 00:00:00)
         * @return DateTimeOffset representing the same instant in UTC (offset = 00:00:00)
         */
        [[nodiscard]] DateTimeOffset toUniversalTime() const noexcept;

        /**
         * @brief Convert to local time (system timezone)
         * @return DateTimeOffset representing the same instant in local timezone
         */
        [[nodiscard]] DateTimeOffset toLocalTime() const noexcept;

        /**
         * @brief Convert to Windows FILETIME format
         * @details Works on any platform - performs mathematical epoch conversion only
         *          Useful for interoperability with Windows-originated data on any system
         * @return 100-nanosecond intervals since January 1, 1601 UTC
         */
        [[nodiscard]] std::int64_t toFILETIME() const noexcept;

        [[nodiscard]] inline DateTimeOffset add(const TimeSpan& value) const noexcept;
        [[nodiscard]] DateTimeOffset addDays(double days) const noexcept;
        [[nodiscard]] DateTimeOffset addHours(double hours) const noexcept;
        [[nodiscard]] DateTimeOffset addMilliseconds(double milliseconds) const noexcept;
        [[nodiscard]] DateTimeOffset addMinutes(double minutes) const noexcept;
        [[nodiscard]] DateTimeOffset addMonths(std::int32_t months) const noexcept;
        [[nodiscard]] DateTimeOffset addSeconds(double seconds) const noexcept;
        [[nodiscard]] inline DateTimeOffset addTicks(std::int64_t ticks) const noexcept;
        [[nodiscard]] DateTimeOffset addYears(std::int32_t years) const noexcept;

        [[nodiscard]] inline TimeSpan subtract(const DateTimeOffset& value) const noexcept;
        [[nodiscard]] inline DateTimeOffset subtract(const TimeSpan& value) const noexcept;

        [[nodiscard]] std::string toString(DateTime::Format format = DateTime::Format::Iso8601) const;

        /**
         * @brief Append string representation using specified format to a reusable buffer
         * @param builder Buffer to append the formatted string to
         * @param format The format to use for string conversion
         */
        void toString(StringBuilder& builder, DateTime::Format format = DateTime::Format::Iso8601) const;

        /// @brief True if both represent the same instant in UTC (offsets may differ)
        [[nodiscard]] inline bool equals(const DateTimeOffset& other) const noexcept;

        /// @brief True if both have the same local time AND the same offset
        [[nodiscard]] inline bool equalsExact(const DateTimeOffset& other) const noexcept;

        [[nodiscard]] bool isValid() const noexcept;

        [[nodiscard]] static DateTimeOffset now() noexcept;    ///< Current local time with system timezone offset
        [[nodiscard]] static DateTimeOffset utcNow() noexcept; ///< Current UTC time (offset = 00:00:00)
        [[nodiscard]] static DateTimeOffset today() noexcept;  ///< Current local date with time set to 00:00:00
        [[nodiscard]] static DateTimeOffset min() noexcept;    ///< January 1, 0001 00:00:00.0000000, zero offset
        [[nodiscard]] static DateTimeOffset max() noexcept;    ///< December 31, 9999 23:59:59.9999999, zero offset
        [[nodiscard]] static DateTimeOffset epoch() noexcept;  ///< January 1, 1970 00:00:00 UTC, zero offset

        /**
         * @brief Parse ISO 8601 string with timezone offset safely without throwing exceptions
         * @param iso8601String The ISO 8601 formatted string with timezone offset to parse
         * @param result Reference to store the parsed DateTimeOffset if successful
         * @return true if parsing succeeded, false otherwise
         */
        [[nodiscard]] static bool fromString(std::string_view iso8601String, DateTimeOffset& result) noexcept;

        /**
         * @brief Parse ISO 8601 string with timezone offset, returning @c std::nullopt on failure
         * @note For error details, use the two-parameter overload
         */
        [[nodiscard]] static std::optional<DateTimeOffset> fromString(std::string_view iso8601String) noexcept;

        [[nodiscard]] static DateTimeOffset fromEpochSeconds(std::int64_t seconds) noexcept;
        [[nodiscard]] static DateTimeOffset fromEpochMilliseconds(std::int64_t milliseconds) noexcept;

        /**
         * @brief Create DateTimeOffset from Windows FILETIME format
         * @details Works on any platform - performs mathematical epoch conversion only
         *          Useful for processing Windows-originated data on any system
         * @param FILETIME 100-nanosecond intervals since January 1, 1601 UTC
         * @return DateTimeOffset representing the same instant in UTC (offset = 00:00:00)
         */
        [[nodiscard]] static DateTimeOffset fromFILETIME(std::int64_t FILETIME) noexcept;

        /**
         * @brief Construct from std::chrono::system_clock::time_point and optional UTC offset
         * @param timePoint The UTC time point
         * @param offset The timezone offset from UTC (default: zero)
         */
        inline explicit DateTimeOffset(
            std::chrono::system_clock::time_point timePoint,
            std::chrono::seconds offset = std::chrono::seconds{ 0 }) noexcept;

        /**
         * @brief Convert to std::chrono::system_clock::time_point (UTC)
         * @details Returns the UTC instant. The offset is not preserved.
         */
        [[nodiscard]] std::chrono::system_clock::time_point toChrono() const noexcept;

        /**
         * @brief Create DateTimeOffset from std::chrono::system_clock::time_point and optional UTC offset
         * @param timePoint The UTC time point
         * @param offset The timezone offset from UTC (default: zero)
         */
        [[nodiscard]] static DateTimeOffset fromChrono(
            const std::chrono::system_clock::time_point& timePoint,
            std::chrono::seconds offset = std::chrono::seconds{ 0 }) noexcept;

    private:
        DateTime m_dateTime; ///< Local date and time
        TimeSpan m_offset;   ///< Offset from UTC
    };

    std::ostream& operator<<(std::ostream& os, const DateTimeOffset& dateTimeOffset);
    std::istream& operator>>(std::istream& is, DateTimeOffset& dateTimeOffset);
} // namespace dnv::vista::sdk

namespace std
{
    template <>
    struct formatter<dnv::vista::sdk::DateTimeOffset>
    {
        constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

        auto format(const dnv::vista::sdk::DateTimeOffset& dto, std::format_context& ctx) const
        {
            return format_to(ctx.out(), "{}", dto.toString());
        }
    };
} // namespace std

#include "dnv/vista/sdk/detail/types/datetime/DateTimeOffset.inl"
