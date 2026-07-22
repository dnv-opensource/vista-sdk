/**
 * @file TimeSpan.h
 * @brief Time interval with 100-nanosecond precision and ISO 8601 duration support
 * @note Represents durations, not absolute points in time. Use DateTime or DateTimeOffset for those
 * @note Follows ISO 8601 duration format (@c P[n]DT[n]H[n]M[n]S). Used as the @c TimeSpan type
 *       in ISO 19848 TimeSeriesData (Table 25)
 */

#pragma once

#include "dnv/vista/sdk/utils/StringBuilder.h"

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
     * @brief Time interval in 100-nanosecond ticks
     *
     * Range: ±10,675,199 days (approximately ±29,247 years)
     * ISO 8601 duration format: @c P[n]Y[n]M[n]DT[n]H[n]M[n]S
     */
    class TimeSpan final
    {
    public:
        /**
         * @brief Construct from ticks (100-nanosecond units)
         * @param ticks Number of 100-nanosecond intervals for this TimeSpan (defaults to 0)
         */
        inline explicit constexpr TimeSpan(std::int64_t ticks = 0) noexcept;

        /**
         * @brief Construct from std::chrono duration
         * @param duration System clock duration to convert from
         */
        inline explicit constexpr TimeSpan(std::chrono::system_clock::duration duration) noexcept;

        /**
         * @brief Parse from ISO 8601 string
         * @param iso8601String ISO 8601 formatted string to parse
         */
        explicit TimeSpan(std::string_view iso8601String);

        TimeSpan(const TimeSpan&) = default;
        TimeSpan(TimeSpan&&) noexcept = default;
        ~TimeSpan() = default;

        TimeSpan& operator=(const TimeSpan&) = default;
        TimeSpan& operator=(TimeSpan&&) noexcept = default;

        inline constexpr std::strong_ordering operator<=>(const TimeSpan& other) const noexcept;
        inline constexpr bool operator==(const TimeSpan& other) const noexcept;

        inline constexpr TimeSpan operator+(const TimeSpan& other) const noexcept;
        inline constexpr TimeSpan operator-(const TimeSpan& other) const noexcept;
        inline constexpr TimeSpan operator-() const noexcept;
        inline constexpr TimeSpan& operator+=(const TimeSpan& other) noexcept;
        inline constexpr TimeSpan& operator-=(const TimeSpan& other) noexcept;

        [[nodiscard]] inline TimeSpan operator*(double multiplier) const noexcept;
        [[nodiscard]] inline TimeSpan operator/(double divisor) const noexcept;

        /// @brief Ratio of this duration to @p other (how many times @p other fits in this)
        [[nodiscard]] inline double operator/(const TimeSpan& other) const noexcept;

        friend inline TimeSpan operator*(double multiplier, const TimeSpan& ts) noexcept;

        [[nodiscard]] inline double days() const noexcept;                  ///< Total days (fractional)
        [[nodiscard]] inline double hours() const noexcept;                 ///< Total hours (fractional)
        [[nodiscard]] inline double minutes() const noexcept;               ///< Total minutes (fractional)
        [[nodiscard]] inline double seconds() const noexcept;               ///< Total seconds (fractional)
        [[nodiscard]] inline double milliseconds() const noexcept;          ///< Total milliseconds (fractional)
        [[nodiscard]] inline double microseconds() const noexcept;          ///< Total microseconds (fractional)
        [[nodiscard]] inline double nanoseconds() const noexcept;           ///< Total nanoseconds (fractional)
        [[nodiscard]] inline constexpr std::int64_t ticks() const noexcept; ///< Duration in 100-nanosecond ticks

        [[nodiscard]] std::string toString() const; ///< ISO 8601 duration string (e.g., @c "PT1H30M45S")

        /**
         * @brief Append ISO 8601 duration string to a reusable buffer
         * @param builder Buffer to append the formatted string to
         */
        void toString(StringBuilder& builder) const;

        /**
         * @brief Parse a TimeSpan from a string safely without throwing exceptions
         * @details Supports the following formats:
         *          - ISO 8601 duration (standard): "PT1H30M45S", "P1DT2H", "-PT45S", "PT0.5S"
         *          - Numeric seconds (convenience): "123.45", "-60" (interpreted as seconds)
         * @param iso8601DurationString The string to parse
         * @param result Reference to store the parsed TimeSpan if successful
         * @return true if parsing succeeded, false otherwise
         *
         * @par Format Requirements
         * ISO 8601 format must start with 'P' and contain at least one component (H, M, S, or D)
         * Numeric format accepts only digits, decimal point, and optional leading minus sign
         * Empty strings and invalid formats return false
         */
        [[nodiscard]] static bool fromString(std::string_view iso8601DurationString, TimeSpan& result) noexcept;

        /**
         * @brief Parse ISO 8601 duration string and return optional TimeSpan
         * @note For error details, use the two-parameter overload
         */
        [[nodiscard]] static std::optional<TimeSpan> fromString(std::string_view iso8601DurationString) noexcept;

        /**
         * @brief Convert to std::chrono::system_clock::duration
         * @details Converts this TimeSpan to a std::chrono duration type
         *          Values outside the representable range of std::chrono::system_clock::duration
         *          will be clamped to the nearest representable value
         * @return A system_clock::duration representing this TimeSpan
         */
        [[nodiscard]] std::chrono::system_clock::duration toChrono() const noexcept;

        /**
         * @brief Create TimeSpan from std::chrono::system_clock::duration
         * @details Converts a std::chrono duration to a TimeSpan
         *          Values outside the representable range of TimeSpan
         *          will be clamped to the nearest valid TimeSpan value
         * @param duration The system_clock duration to convert
         * @return A TimeSpan representing the same time duration
         */
        [[nodiscard]] static TimeSpan fromChrono(const std::chrono::system_clock::duration& duration) noexcept;

        [[nodiscard]] inline static constexpr TimeSpan fromDays(double days) noexcept;
        [[nodiscard]] inline static constexpr TimeSpan fromHours(double hours) noexcept;
        [[nodiscard]] inline static constexpr TimeSpan fromMinutes(double minutes) noexcept;
        [[nodiscard]] inline static constexpr TimeSpan fromSeconds(double seconds) noexcept;
        [[nodiscard]] inline static constexpr TimeSpan fromMilliseconds(double milliseconds) noexcept;
        [[nodiscard]] inline static constexpr TimeSpan fromMicroseconds(double microseconds) noexcept;
        [[nodiscard]] inline static constexpr TimeSpan fromTicks(std::int64_t ticks) noexcept;

    private:
        std::int64_t m_ticks; ///< Duration in 100-nanosecond ticks
    };

    std::ostream& operator<<(std::ostream& os, const TimeSpan& timeSpan);
    std::istream& operator>>(std::istream& is, TimeSpan& timeSpan);
} // namespace dnv::vista::sdk

namespace std
{
    template <>
    struct formatter<dnv::vista::sdk::TimeSpan>
    {
        constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

        auto format(const dnv::vista::sdk::TimeSpan& ts, std::format_context& ctx) const
        {
            return format_to(ctx.out(), "{}", ts.toString());
        }
    };
} // namespace std

#include "dnv/vista/sdk/detail/types/datetime/TimeSpan.inl"
