/**
 * @file DateTime.h
 * @brief Cross-platform DateTime type with 100-nanosecond precision and ISO 8601 support
 * @note DateTime always represents UTC. For timezone-aware operations, use DateTimeOffset instead
 * @note Follows ISO 8601-1 date/time semantics (ISO 19848 Table 8 @c DateTime type)
 *       Internal precision is 100-nanosecond ticks
 */

#pragma once

#include "TimeSpan.h"

#include <chrono>
#include <compare>
#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <string_view>

namespace dnv::vista::sdk
{
    /**
     * @brief Cross-platform DateTime type with 100-nanosecond precision
     *
     * Range: January 1, 0001 to December 31, 9999. No timezone storage, always UTC
     * For timezone-aware values, use DateTimeOffset
     */
    class DateTime final
    {
    public:
        /**
         * @brief DateTime string format options
         */
        enum class Format : std::uint8_t
        {
            /** @brief ISO 8601 with seconds precision: "2024-01-01T12:00:00Z" */
            Iso8601,

            /** @brief ISO 8601 with full tick precision (7 decimal digits): "2024-01-01T12:00:00.1234567Z" */
            Iso8601Precise,

            /** @brief ISO 8601 with trimmed trailing zeros in fractions: "2024-01-01T12:00:00.1234Z" */
            Iso8601PreciseTrimmed,

            /** @brief ISO 8601 with millisecond precision (3 decimal digits): "2024-01-01T12:00:00.123Z" */
            Iso8601Millis,

            /** @brief ISO 8601 with microsecond precision (6 decimal digits): "2024-01-01T12:00:00.123456Z" */
            Iso8601Micros,

            /** @brief ISO 8601 with numeric UTC offset (forces +HH:MM notation, never Z): "2024-01-01T12:00:00+00:00"
             */
            Iso8601Extended,

            /** @brief ISO 8601 compact form without separators: "20240101T120000Z" */
            Iso8601Basic,

            /** @brief ISO 8601 date only: "2024-01-01" */
            Iso8601Date,

            /** @brief ISO 8601 time only: "12:00:00" */
            Iso8601Time,

            /** @brief Unix epoch seconds (integer): "1704110400" */
            UnixSeconds,

            /** @brief Unix epoch milliseconds (integer): "1704110400000" */
            UnixMilliseconds
        };

        /** @brief Constructs a zero-initialized DateTime */
        constexpr DateTime() noexcept;

        /**
         * @brief Construct from tick count (100-nanosecond units since year 1)
         * @param ticks Number of 100-nanosecond intervals since January 1, 0001 UTC
         */
        explicit constexpr inline DateTime(std::int64_t ticks) noexcept;

        /**
         * @brief Construct from system clock time point
         * @param timePoint System clock time point to convert from
         */
        inline explicit DateTime(std::chrono::system_clock::time_point timePoint) noexcept;

        /**
         * @brief Construct from date components
         * @param year Year component (1-9999)
         * @param month Month component (1-12)
         * @param day Day component (1-31)
         */
        DateTime(std::int32_t year, std::int32_t month, std::int32_t day) noexcept;

        /**
         * @brief Construct from date and time components
         * @param year Year component (1-9999)
         * @param month Month component (1-12)
         * @param day Day component (1-31)
         * @param hour Hour component (0-23)
         * @param minute Minute component (0-59)
         * @param second Second component (0-59)
         */
        DateTime(
            std::int32_t year,
            std::int32_t month,
            std::int32_t day,
            std::int32_t hour,
            std::int32_t minute,
            std::int32_t second) noexcept;

        /**
         * @brief Construct from date and time components with milliseconds
         * @param year Year component (1-9999)
         * @param month Month component (1-12)
         * @param day Day component (1-31)
         * @param hour Hour component (0-23)
         * @param minute Minute component (0-59)
         * @param second Second component (0-59)
         * @param millisecond Millisecond component (0-999)
         */
        DateTime(
            std::int32_t year,
            std::int32_t month,
            std::int32_t day,
            std::int32_t hour,
            std::int32_t minute,
            std::int32_t second,
            std::int32_t millisecond) noexcept;

        /**
         * @brief Parse from ISO 8601 string
         * @param iso8601String ISO 8601 formatted string to parse
         */
        inline explicit DateTime(std::string_view iso8601String);

        /**
         * @brief Parse from C-string (convenience for string literals)
         * @param iso8601String ISO 8601 formatted C-string to parse
         */
        inline explicit DateTime(const char* iso8601String);

        DateTime(const DateTime&) = default;
        DateTime(DateTime&&) noexcept = default;
        ~DateTime() = default;

        DateTime& operator=(const DateTime&) = default;
        DateTime& operator=(DateTime&&) noexcept = default;

        inline constexpr std::strong_ordering operator<=>(const DateTime& other) const noexcept;
        inline constexpr bool operator==(const DateTime& other) const noexcept;

        inline constexpr DateTime operator+(const TimeSpan& duration) const noexcept;
        inline constexpr DateTime operator-(const TimeSpan& duration) const noexcept;
        inline constexpr TimeSpan operator-(const DateTime& other) const noexcept;
        inline constexpr DateTime& operator+=(const TimeSpan& duration) noexcept;
        inline constexpr DateTime& operator-=(const TimeSpan& duration) noexcept;

        [[nodiscard]] std::int32_t year() const noexcept;        ///< Year component (1-9999)
        [[nodiscard]] std::int32_t month() const noexcept;       ///< Month component (1-12)
        [[nodiscard]] std::int32_t day() const noexcept;         ///< Day component (1-31)
        [[nodiscard]] std::int32_t hour() const noexcept;        ///< Hour component (0-23)
        [[nodiscard]] std::int32_t minute() const noexcept;      ///< Minute component (0-59)
        [[nodiscard]] std::int32_t second() const noexcept;      ///< Second component (0-59)
        [[nodiscard]] std::int32_t millisecond() const noexcept; ///< Millisecond component (0-999)
        [[nodiscard]] std::int32_t microsecond() const noexcept; ///< Microsecond component (0-999)

        /**
         * @brief Get nanosecond component (0-900, in 100ns increments)
         * @return The nanosecond component of this DateTime (0, 100, 200, ..., 900)
         * @note Due to 100-nanosecond tick precision, only values 0-900 in 100ns increments are possible
         */
        [[nodiscard]] std::int32_t nanosecond() const noexcept;

        [[nodiscard]] constexpr std::int64_t ticks()
            const noexcept;                                    ///< 100-nanosecond intervals since January 1, 0001 UTC
        [[nodiscard]] std::int32_t dayOfWeek() const noexcept; ///< Day of week (0=Sunday, 6=Saturday)
        [[nodiscard]] std::int32_t dayOfYear() const noexcept; ///< Day of year (1-366)
        [[nodiscard]] inline constexpr std::int64_t toEpochSeconds() const noexcept; ///< Seconds since Unix epoch
        [[nodiscard]] inline constexpr std::int64_t toEpochMilliseconds()
            const noexcept;                                ///< Milliseconds since Unix epoch
        [[nodiscard]] DateTime date() const noexcept;      ///< Date component with time set to 00:00:00
        [[nodiscard]] TimeSpan timeOfDay() const noexcept; ///< Elapsed time since midnight

        [[nodiscard]] DateTime addDays(double days) const noexcept;
        [[nodiscard]] DateTime addHours(double hours) const noexcept;
        [[nodiscard]] DateTime addMinutes(double minutes) const noexcept;
        [[nodiscard]] DateTime addSeconds(double seconds) const noexcept;
        [[nodiscard]] DateTime addMilliseconds(double milliseconds) const noexcept;

        /**
         * @brief Add months to this DateTime
         * @param months Number of months to add (may be negative)
         * @note If the resulting day exceeds the days in the target month, it is clamped
         *       (e.g., Jan 31 + 1 month = Feb 28/29)
         * @return New DateTime shifted by the specified number of months
         */
        [[nodiscard]] DateTime addMonths(std::int32_t months) const noexcept;

        /**
         * @brief Add years to this DateTime
         * @param years Number of years to add (may be negative)
         * @note Equivalent to addMonths(years * 12)
         * @return New DateTime shifted by the specified number of years
         */
        [[nodiscard]] DateTime addYears(std::int32_t years) const noexcept;

        [[nodiscard]] bool isValid() const noexcept;
        [[nodiscard]] inline static constexpr bool isLeapYear(std::int32_t year) noexcept;

        /**
         * @brief Get days in month for given year and month
         * @param year The year to check (1-9999)
         * @param month The month to check (1-12)
         * @return Number of days in the specified month (28-31)
         */
        [[nodiscard]] inline static constexpr std::int32_t daysInMonth(std::int32_t year, std::int32_t month) noexcept;

        /**
         * @brief Get current UTC time
         * @return DateTime representing the current UTC date and time
         * @note For local time, use DateTimeOffset::now() instead
         */
        [[nodiscard]] static DateTime utcNow() noexcept;

        [[nodiscard]] inline static constexpr DateTime min() noexcept;   ///< January 1, 0001 00:00:00.0000000 UTC
        [[nodiscard]] inline static constexpr DateTime max() noexcept;   ///< December 31, 9999 23:59:59.9999999 UTC
        [[nodiscard]] inline static constexpr DateTime epoch() noexcept; ///< January 1, 1970 00:00:00 UTC

        /**
         * @brief Parse ISO 8601 string safely without throwing exceptions
         * @param iso8601String The ISO 8601 formatted string to parse
         * @param result Reference to store the parsed DateTime if successful
         * @return true if parsing succeeded, false otherwise
         */
        [[nodiscard]] static bool fromString(std::string_view iso8601String, DateTime& result) noexcept;

        /**
         * @brief Parse ISO 8601 string and return optional DateTime
         * @param iso8601String The ISO 8601 formatted string to parse
         * @return std::optional<DateTime> containing the parsed value if successful, std::nullopt otherwise
         */
        [[nodiscard]] static std::optional<DateTime> fromString(std::string_view iso8601String) noexcept;

        [[nodiscard]] inline static constexpr DateTime fromEpochSeconds(std::int64_t seconds) noexcept;
        [[nodiscard]] inline static constexpr DateTime fromEpochMilliseconds(std::int64_t milliseconds) noexcept;

        /**
         * @brief Convert to string using specified format
         * @param format The format to use for string conversion
         * @return String representation using the specified format
         */
        [[nodiscard]] std::string toString(Format format = Format::Iso8601) const;

        /**
         * @brief Convert to std::chrono::system_clock::time_point
         * @details Values outside the representable range of std::chrono::system_clock
         *          (approximately years 1677-2262 on 64-bit Linux systems) will be
         *          clamped to the nearest representable value. This means extreme dates
         *          (near year 1 or year 9999) cannot round-trip through chrono
         * @return A system_clock::time_point representing this DateTime
         */
        [[nodiscard]] std::chrono::system_clock::time_point toChrono() const noexcept;

        /**
         * @brief Create DateTime from std::chrono::system_clock::time_point
         * @details Values outside the representable range of DateTime
         *          will be clamped to the nearest valid DateTime value
         * @param timePoint The system_clock time point to convert
         * @return A DateTime representing the same instant in time
         */
        [[nodiscard]] static DateTime fromChrono(const std::chrono::system_clock::time_point& timePoint) noexcept;

    private:
        std::int64_t m_ticks; ///< 100-nanosecond ticks since January 1, 0001 UTC
    };

    std::ostream& operator<<(std::ostream& os, const DateTime& dateTime);
    std::istream& operator>>(std::istream& is, DateTime& dateTime);
} // namespace dnv::vista::sdk

namespace std
{
    template <>
    struct formatter<dnv::vista::sdk::DateTime>
    {
        constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

        auto format(const dnv::vista::sdk::DateTime& dt, std::format_context& ctx) const
        {
            return format_to(ctx.out(), "{}", dt.toString(dnv::vista::sdk::DateTime::Format::Iso8601));
        }
    };
} // namespace std

#include "dnv/vista/sdk/detail/types/datetime/DateTime.inl"
