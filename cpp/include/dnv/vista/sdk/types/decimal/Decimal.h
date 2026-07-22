/**
 * @file Decimal.h
 * @brief Cross-platform high-precision decimal arithmetic type
 * @note Implements XSD @c xs:decimal semantics (ISO 19848 Table 2): exact @c mantissa/10^scale
 *       arithmetic with no floating-point rounding
 */

#pragma once

#include "dnv/vista/sdk/utils/StringBuilder.h"

#include <array>
#include <compare>
#include <cstdint>
#include <format>
#include <limits>
#include <optional>
#include <string>
#include <string_view>

namespace dnv::vista::sdk
{
    /**
     * @brief Cross-platform 128-bit fixed-point decimal type with exact decimal arithmetic
     *
     * @details Stores values as `mantissa / 10^scale` where:
     *          - mantissa is a 96-bit unsigned integer (0 <= m < 2^96)
     *          - scale is an integer in [0, 28]
     *          - sign is stored separately in bit 31 of the flags word
     *
     *          Range: ±79,228,162,514,264,337,593,543,950,335 with up to 28 decimal places
     *
     *          IEEE 754 input: NaN and Infinity from float/double are converted to zero
     *          Division by zero throws @c std::overflow_error (no NaN/Infinity states)
     *
     *          Memory Layout and Internal Representation:
     *          ==========================================
     *
     *          1. Flags (32 bits):
     *          ┌───────────┬─────────────────────────────────────┬───────────────────────────────────────────────────┐
     *          │    Bits   │             Description             │                       Notes                       │
     *          ├───────────┼─────────────────────────────────────┼───────────────────────────────────────────────────┤
     *          │   0 - 15  │  Unused (must be zero)              │  Reserved - Required to be zero for valid format  │
     *          │  16 - 23  │  Scale (0-28)                       │  Number of decimal digits after decimal point     │
     *          │  24 - 30  │  Unused (must be zero)              │  Reserved - Required to be zero for valid format  │
     *          │  31       │  Sign (0 = positive, 1 = negative)  │  Sign bit                                         │
     *          └───────────┴─────────────────────────────────────┴───────────────────────────────────────────────────┘
     *
     *          2. Mantissa (96 bits total):
     *          ┌───────────────┬───────────┬─────────────────────────────────┐
     *          │ Mantissa Part │   Bits    │           Description           │
     *          ├───────────────┼───────────┼─────────────────────────────────┤
     *          │  mantissa[0]  │   0 - 31  │  Lower 32 bits of the mantissa  │
     *          │  mantissa[1]  │  32 - 63  │  Middle 32 bits of the mantissa │
     *          │  mantissa[2]  │  64 - 95  │  Upper 32 bits of the mantissa  │
     *          └───────────────┴───────────┴─────────────────────────────────┘
     *
     *          Complete Memory Layout (128 bits / 16 bytes):
     *          =============================================
     *
     *          ┌────────────────────┬────────────────────┬────────────────────┬────────────────────┐
     *          │     mantissa[2]    │     mantissa[1]    │     mantissa[0]    │       flags        │
     *          │  (upper 32 bits)   │  (middle 32 bits)  │  (lower 32 bits)   │   (scale + sign)   │
     *          │    bits 127 .. 96  │    bits 95 .. 64   │    bits 63 .. 32   │    bits 31 .. 0    │
     *          └────────────────────┴────────────────────┴────────────────────┴────────────────────┘
     *
     *          The 96-bit mantissa represents an unsigned integer from 0 to 2^96-1
     *          The sign is stored separately in bit 31 of the flags word
     *
     * @note This is exact fixed-point decimal arithmetic, NOT IEEE 754 decimal128
     *       Use the string constructor for values requiring more than ~15 significant digits
     */
    class Decimal final
    {
        friend class std::numeric_limits<Decimal>;

    public:
        /**
         * @brief Rounding modes for decimal arithmetic operations
         * @details Defines how rounding should be performed when reducing decimal precision
         *          Each mode follows industry-standard rounding semantics:
         *
         *          - ToNearest (banker's rounding): Round to nearest value, ties round to even digit
         *            Examples: 2.5->2, 3.5->4, 4.5->4, 5.5->6 (minimizes cumulative rounding bias)
         *
         *          - ToNearestTiesAway (standard rounding): Round to nearest value, ties away from zero
         *            Examples: 2.5->3, 3.5->4, -2.5->-3, -3.5->-4 (traditional "round half up" for positives)
         *
         *          - ToZero (truncation): Round towards zero, discarding fractional part
         *            Examples: 2.9->2, -2.9->-2, 3.1->3, -3.1->-3
         *
         *          - ToPositiveInfinity (ceiling): Round towards positive infinity
         *            Examples: 2.1->3, -2.9->-2, 3.0->3
         *
         *          - ToNegativeInfinity (floor): Round towards negative infinity
         *            Examples: 2.9->2, -2.1->-3, 3.0->3
         */
        enum class RoundingMode : std::uint8_t
        {
            ToNearest = 0,      ///< Round to nearest, ties to even (banker's rounding)
            ToNearestTiesAway,  ///< Round to nearest, ties away from zero (standard rounding)
            ToZero,             ///< Round towards zero (truncate)
            ToPositiveInfinity, ///< Round towards +∞ (ceiling)
            ToNegativeInfinity  ///< Round towards -∞ (floor)
        };

        inline constexpr Decimal() noexcept;

        /**
         * @brief Construct from @c float. NaN/Infinity become zero
         * @param value Float value to convert
         * @note Limited to ~6-7 significant digits. Use string constructor for exact fractional values
         */
        explicit Decimal(float value) noexcept;

        /**
         * @brief Construct from @c double. NaN/Infinity become zero
         * @param value Double value to convert
         * @note Limited to ~15-17 significant digits. Use string constructor for exact fractional values
         */
        explicit Decimal(double value) noexcept;

        inline explicit Decimal(std::int32_t value) noexcept;
        inline explicit Decimal(std::int64_t value) noexcept;
        inline explicit Decimal(std::uint32_t value) noexcept;
        inline explicit Decimal(std::uint64_t value) noexcept;

        /**
         * @brief Construct from string (exact parsing)
         * @param str String representation (e.g., "123.456")
         * @throws std::invalid_argument if string is not a valid decimal
         * @see fromString() for non-throwing parsing
         */
        inline explicit Decimal(std::string_view str);

        /**
         * @brief Construct from C-string (exact parsing)
         * @param scStr Null-terminated C-string representation (e.g., "123.456")
         * @throws std::invalid_argument if string is not a valid decimal
         * @see fromString() for non-throwing parsing
         */
        inline explicit Decimal(const char* scStr);

        Decimal(const Decimal& other) noexcept = default;
        Decimal(Decimal&& other) noexcept = default;
        ~Decimal() = default;

        Decimal& operator=(const Decimal& other) noexcept = default;
        Decimal& operator=(Decimal&& other) noexcept = default;
        std::strong_ordering operator<=>(const Decimal& other) const noexcept;
        bool operator==(const Decimal& other) const noexcept;

        /// @note Comparisons with @c float are subject to ~6-7 digit precision limitations
        inline bool operator==(float val) const noexcept;
        inline bool operator<(float val) const noexcept;
        inline bool operator<=(float val) const noexcept;
        inline bool operator>(float val) const noexcept;
        inline bool operator>=(float val) const noexcept;

        /// @note Comparisons with @c double are subject to ~15-17 digit precision limitations
        inline bool operator==(double val) const noexcept;
        inline bool operator<(double val) const noexcept;
        inline bool operator<=(double val) const noexcept;
        inline bool operator>(double val) const noexcept;
        inline bool operator>=(double val) const noexcept;

        inline bool operator==(std::int64_t val) const noexcept;
        inline bool operator<(std::int64_t val) const noexcept;
        inline bool operator<=(std::int64_t val) const noexcept;
        inline bool operator>(std::int64_t val) const noexcept;
        inline bool operator>=(std::int64_t val) const noexcept;

        inline bool operator==(std::uint64_t val) const noexcept;
        inline bool operator<(std::uint64_t val) const noexcept;
        inline bool operator<=(std::uint64_t val) const noexcept;
        inline bool operator>(std::uint64_t val) const noexcept;
        inline bool operator>=(std::uint64_t val) const noexcept;

        inline bool operator==(std::int32_t val) const noexcept;
        inline bool operator<(std::int32_t val) const noexcept;
        inline bool operator<=(std::int32_t val) const noexcept;
        inline bool operator>(std::int32_t val) const noexcept;
        inline bool operator>=(std::int32_t val) const noexcept;

        Decimal operator+(const Decimal& other) const;
        inline Decimal operator-(const Decimal& other) const;
        Decimal operator*(const Decimal& other) const;

        /**
         * @brief Division operator
         * @param other Divisor
         * @return Result of division
         * @throws std::overflow_error if divisor is zero (no NaN/Infinity representation)
         */
        Decimal operator/(const Decimal& other) const;

        inline Decimal& operator+=(const Decimal& other);
        inline Decimal& operator-=(const Decimal& other);
        inline Decimal& operator*=(const Decimal& other);

        /**
         * @brief Division assignment operator
         * @param other Divisor
         * @return Reference to this after division
         * @throws std::overflow_error if divisor is zero (no NaN/Infinity representation)
         */
        inline Decimal& operator/=(const Decimal& other);

        inline Decimal operator-() const noexcept;

        /**
         * @brief Number of decimal places in the stored value (0-28)
         */
        [[nodiscard]] std::uint8_t scale() const noexcept;

        [[nodiscard]] inline const std::uint32_t& flags() const noexcept;
        [[nodiscard]] inline std::uint32_t& flags() noexcept;
        [[nodiscard]] inline const std::array<std::uint32_t, 3>& mantissa() const noexcept;
        [[nodiscard]] inline std::array<std::uint32_t, 3>& mantissa() noexcept;

        [[nodiscard]] inline Decimal abs() const noexcept;
        [[nodiscard]] inline Decimal ceil() const noexcept;
        [[nodiscard]] inline Decimal floor() const noexcept;

        /**
         * @brief Round decimal to specified precision using configurable rounding mode
         * @param decimalsPlacesCount Number of decimal places to round to (default: 0 for integer rounding)
         * @param mode Rounding mode to apply (default: RoundingMode::ToNearest for banker's rounding)
         * @return Decimal value rounded to the specified precision
         */
        [[nodiscard]] Decimal round(
            std::int32_t decimalsPlacesCount = 0, RoundingMode mode = RoundingMode::ToNearest) const noexcept;

        /**
         * @brief Compute square root
         * @throws std::domain_error if called on a negative value
         */
        [[nodiscard]] Decimal sqrt() const;

        [[nodiscard]] inline Decimal trunc() const noexcept;

        /**
         * @brief Parse string to decimal with error handling
         * @param str String to parse (e.g., "123.45", "-0.001", "42")
         * @param result Output decimal value
         * @return true if parsing succeeded, false otherwise
         * @details Parses string representations of decimal numbers with comprehensive format support
         *          Supported input formats:
         *          - Integers: "123", "-456"
         *          - Decimals: "123.45", "-0.001"
         *          - Scientific notation: "1.23e5", "-4.56E-3"
         *          - Leading/trailing whitespace is trimmed
         *          - Positive signs are optional: "+123.45" -> 123.45
         *          - Maximum precision: 28 significant decimal digits
         * @note Range: ±79,228,162,514,264,337,593,543,950,335 with up to 28 decimal places
         * @note For convenience wrapper with std::optional return, use the single-parameter overload
         */
        [[nodiscard]] static bool fromString(std::string_view str, Decimal& result) noexcept;

        /**
         * @brief Parse string to decimal, returning @c std::nullopt on failure
         * @param str String to parse
         * @note For error details, use the two-parameter overload
         */
        [[nodiscard]] static std::optional<Decimal> fromString(std::string_view str) noexcept;

        /**
         * @brief Convert to double (may lose precision)
         * @return Double representation
         * @note Conversion may lose precision beyond ~15-17 significant digits
         * @note For exact precision, use toString() method instead
         */
        [[nodiscard]] double toDouble() const noexcept;

        /**
         * @brief Convert to string with exact precision
         * @return String representation
         */
        [[nodiscard]] std::string toString() const;

        /**
         * @brief Append string representation with exact precision to a reusable buffer
         * @param builder Buffer to append the formatted string to
         */
        void toString(StringBuilder& builder) const;

        /**
         * @brief Get internal 32-bit representation
         * @return Array of 4 32-bit integers representing the decimal
         */
        [[nodiscard]] std::array<std::int32_t, 4> toBits() const noexcept;

        /**
         * @brief Count actual decimal places (excluding trailing zeros)
         * @return Number of significant decimal places (0-28)
         * @details Examples:
         *          - Decimal("123.4500") returns 2 (trailing zeros ignored)
         *          - Decimal("123.000") returns 0 (integer value)
         *          - Decimal("0.001") returns 3
         */
        [[nodiscard]] std::uint8_t decimalPlacesCount() const noexcept;

        /**
         * @brief Count total significant decimal digits (excluding sign and decimal point)
         * @return Number of significant digits, at least 1 (a value of 0 counts as 1 digit)
         * @details Since toString() always renders a normalized value (no leading/trailing
         *          zeros), this is simply the digit count of toString() with the decimal point
         *          (and leading '-' if present) excluded. Examples:
         *          - Decimal("123.400") returns 4 (normalizes to "123.4")
         *          - Decimal("042") returns 2 (normalizes to "42")
         *          - Decimal("0") returns 1
         *          - Decimal("-123.45") returns 5
         */
        [[nodiscard]] std::uint32_t totalDigitsCount() const;

    private:
        /** @brief Internal storage layout for 128-bit decimal representation */
        struct Layout
        {
            std::uint32_t flags;                   ///< Scale (bits 16-23) + Sign (bit 31)
            std::array<std::uint32_t, 3> mantissa; ///< 96-bit mantissa (3 x 32-bit)
        } m_layout;
    };

    [[nodiscard]] inline Decimal abs(const Decimal& value) noexcept
    {
        return value.abs();
    }

    /**
     * @brief Compute square root
     * @throws std::domain_error if @p value is negative
     */
    [[nodiscard]] inline Decimal sqrt(const Decimal& value)
    {
        return value.sqrt();
    }

    [[nodiscard]] inline Decimal ceil(const Decimal& value) noexcept
    {
        return value.ceil();
    }
    [[nodiscard]] inline Decimal floor(const Decimal& value) noexcept
    {
        return value.floor();
    }
    [[nodiscard]] inline Decimal trunc(const Decimal& value) noexcept
    {
        return value.trunc();
    }

    /**
     * @brief Round @p value to @p decimalsPlacesCount decimal places
     * @param value Decimal value to round
     * @param decimalsPlacesCount Target decimal places (default: 0)
     * @param mode Rounding mode (default: @c RoundingMode::ToNearest)
     */
    [[nodiscard]] inline Decimal round(
        const Decimal& value,
        std::int32_t decimalsPlacesCount = 0,
        Decimal::RoundingMode mode = Decimal::RoundingMode::ToNearest) noexcept
    {
        return value.round(decimalsPlacesCount, mode);
    }

    std::ostream& operator<<(std::ostream& os, const Decimal& decimal);
    std::istream& operator>>(std::istream& is, Decimal& decimal);
} // namespace dnv::vista::sdk

namespace std
{
    template <>
    struct formatter<dnv::vista::sdk::Decimal>
    {
        constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

        auto format(const dnv::vista::sdk::Decimal& d, std::format_context& ctx) const
        {
            return format_to(ctx.out(), "{}", d.toString());
        }
    };
} // namespace std

#include "dnv/vista/sdk/detail/types/decimal/Decimal.inl"
