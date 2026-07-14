/**
 * @file Int128.h
 * @brief Cross-platform 128-bit integer arithmetic type
 * @details Provides portable 128-bit signed integer operations with
 *          native __int128 on GCC/Clang and manual implementation on MSVC
 *
 *          Memory Layout and Internal Representation:
 *          =========================================
 *
 *          1. GCC/Clang with native __int128 support (DNV_VISTA_SDK_HAS_NATIVE_INT128=1):
 *          ┌─────────────────────────────────────────────────────────────────────┐
 *          │                           Native __int128                           │
 *          │                            (16 bytes)                               │
 *          └─────────────────────────────────────────────────────────────────────┘
 *          Bit 127                                                           Bit 0
 *
 *          2. MSVC and other compilers (DNV_VISTA_SDK_HAS_NATIVE_INT128=0):
 *          ┌──────────────────────────────────┬──────────────────────────────────┐
 *          │          upper64bits             │            lower64bits           │
 *          │       (most significant)         │         (least significant)      │
 *          │             8 bytes              │               8 bytes            │
 *          └──────────────────────────────────┴──────────────────────────────────┘
 *          Bit 127                     Bit 64 Bit 63                         Bit 0
 *
 *          Bit Layout and Sign Representation:
 *          ===================================
 *
 *          The 128-bit signed integer uses two's complement representation:
 *          ┌─┬───────────────────────────────────────────────────────────────────┐
 *          │S│                    Value Bits (127 bits)                          │
 *          └─┴───────────────────────────────────────────────────────────────────┘
 *          Bit 127                                                           Bit 0
 *
 *          Where:
 *          - S (bit 127): Sign bit (0 = positive, 1 = negative)
 *          - Bits 126-0: Magnitude in two's complement form
 *
 *          Value Range:
 *          - Minimum: -2^127   = -170,141,183,460,469,231,731,687,303,715,884,105,728
 *          - Maximum:  2^127-1 =  170,141,183,460,469,231,731,687,303,715,884,105,727
 *
 */

#pragma once

#include <array>
#include <compare>
#include <cstdint>
#include <optional>
#include <string>

/**
 * @brief Cross-platform 128-bit integer support detection
 * @details Detects native __int128 support for high-performance decimal arithmetic.
 *          - GCC/Clang: Native __int128 support since GCC 4.6+ and Clang 3.1+
 *          - MSVC: No native 128-bit support, requires manual implementation
 */
#if defined(__SIZEOF_INT128__) && !defined(_MSC_VER)
    // GCC and Clang have native __int128 support
    #define DNV_VISTA_SDK_HAS_NATIVE_INT128 1
__extension__ typedef __int128 DNV_VISTA_SDK_NATIVE_INT128;
__extension__ typedef unsigned __int128 DNV_VISTA_SDK_NATIVE_UINT128;
#else
    // MSVC and other compilers without native 128-bit support
    #define DNV_VISTA_SDK_HAS_NATIVE_INT128 0
// For manual 128-bit implementation, we'll use our custom Int128 struct
#endif

#if DNV_VISTA_SDK_HAS_NATIVE_INT128
    #define DNV_VISTA_SDK_IF_INT128(code) code
    #define DNV_VISTA_SDK_IF_NO_INT128(code)
#else
    #define DNV_VISTA_SDK_IF_INT128(code)
    #define DNV_VISTA_SDK_IF_NO_INT128(code) code
#endif

namespace dnv::vista::sdk
{
    class Decimal;
} // namespace dnv::vista::sdk

namespace dnv::vista::sdk::internal
{
    using dnv::vista::sdk::Decimal;

    /**
     * @brief Cross-platform 128-bit signed integer type
     * @details Implements portable 128-bit signed integer arithmetic with:
     *          - Native __int128 support on GCC/Clang for maximum performance
     *          - Manual implementation on MSVC using two 64-bit words
     *          - Full arithmetic and comparison operator support
     *          - Optimized for decimal arithmetic operations
     *          - Cross-platform value consistency and API compatibility
     */
    class Int128 final
    {
    public:
        inline constexpr Int128() noexcept;
        inline explicit constexpr Int128(int val) noexcept;
        inline explicit constexpr Int128(std::int64_t val) noexcept;
        inline explicit constexpr Int128(std::uint32_t val) noexcept;
        inline explicit constexpr Int128(std::uint64_t val) noexcept;
        constexpr Int128(
            std::uint64_t low, std::uint64_t high) noexcept; ///< Construct from two 64-bit words (low, high)

#if DNV_VISTA_SDK_HAS_NATIVE_INT128
        /// @brief Construct from native @c __int128 (GCC/Clang only).
        explicit constexpr Int128(DNV_VISTA_SDK_NATIVE_INT128 val) noexcept;
#endif

        /**
         * @brief Construct from string (exact parsing).
         * @throws std::invalid_argument if string is not a valid integer.
         */
        inline explicit Int128(std::string_view str);

        /**
         * @brief Construct from C-string (exact parsing).
         * @throws std::invalid_argument if string is not a valid integer.
         */
        inline explicit Int128(const char* scStr);

        /**
         * @brief Construct from double-precision floating-point value
         * @param val Double value to convert
         * @details Follows C++ standard truncation behavior (like static_cast<int>(double)):
         *          - Fractional parts are truncated toward zero (42.7 -> 42, -42.7 -> -42)
         *          - NaN and infinity values are converted to zero
         *          - Values exceeding Int128 range are clamped to Int128 limits (±2^127)
         *          - Uses string conversion to preserve full double precision (53-bit mantissa)
         * @note This matches the behavior of static_cast<int>(double) for consistency
         */
        explicit Int128(double val);

        /**
         * @brief Construct from Decimal value
         * @param decimal Decimal value to convert
         * @details Follows C++ standard truncation behavior (like static_cast<int>(double)):
         *          - Fractional parts are truncated toward zero (42.7 -> 42, -42.7 -> -42)
         *          - Integer part is extracted by dividing mantissa by 10^scale
         *          - Zero values are handled efficiently
         *          - Large values within Int128 range are supported
         * @note This matches the behavior of static_cast<int>(double) for consistency,
         *       rather than throwing exceptions for fractional parts
         */
        explicit Int128(const Decimal& decimal);

        constexpr Int128(const Int128& other) noexcept = default;
        constexpr Int128(Int128&& other) noexcept = default;
        ~Int128() = default;

        constexpr Int128& operator=(const Int128& other) noexcept = default;
        constexpr Int128& operator=(Int128&& other) noexcept = default;

        inline std::strong_ordering operator<=>(const Int128& other) const noexcept;
        inline bool operator==(const Int128& other) const noexcept;

        /// @note Comparisons with @c int are exact (no precision loss).
        inline bool operator==(int val) const noexcept;
        inline bool operator<(int val) const noexcept;
        inline bool operator<=(int val) const noexcept;
        inline bool operator>(int val) const noexcept;
        inline bool operator>=(int val) const noexcept;

        /// @note Comparisons with @c int64_t are exact (no precision loss).
        inline bool operator==(std::int64_t val) const noexcept;
        inline bool operator<(std::int64_t val) const noexcept;
        inline bool operator<=(std::int64_t val) const noexcept;
        inline bool operator>(std::int64_t val) const noexcept;
        inline bool operator>=(std::int64_t val) const noexcept;

        /// @note Comparisons with @c uint64_t are exact (no precision loss).
        inline bool operator==(std::uint64_t val) const noexcept;
        inline bool operator<(std::uint64_t val) const noexcept;
        inline bool operator<=(std::uint64_t val) const noexcept;
        inline bool operator>(std::uint64_t val) const noexcept;
        inline bool operator>=(std::uint64_t val) const noexcept;

        /// @note Comparisons with @c double are subject to ~15-17 digit precision limitations.
#if !DNV_VISTA_SDK_HAS_NATIVE_INT128
        bool operator==(double val) const noexcept;
        bool operator<(double val) const noexcept;
        inline bool operator<=(double val) const noexcept;
        bool operator>(double val) const noexcept;
        inline bool operator>=(double val) const noexcept;
#else
        inline bool operator==(double val) const noexcept;
        inline bool operator<(double val) const noexcept;
        inline bool operator<=(double val) const noexcept;
        inline bool operator>(double val) const noexcept;
        inline bool operator>=(double val) const noexcept;
#endif

        /// @note Decimal must have no fractional part to compare equal.
        bool operator==(const Decimal& val) const noexcept;
        bool operator<(const Decimal& val) const noexcept;
        inline bool operator<=(const Decimal& val) const noexcept { return *this < val || *this == val; }
        inline bool operator>(const Decimal& val) const noexcept { return !(*this <= val); }
        inline bool operator>=(const Decimal& val) const noexcept { return !(*this < val); }

        inline Int128 operator+(const Int128& other) const noexcept;
        inline Int128 operator-(const Int128& other) const noexcept;
        Int128 operator*(const Int128& other) const noexcept;
        inline Int128 operator-() const noexcept;

        /// @throws std::overflow_error if divisor is zero.
        Int128 operator/(const Int128& other) const;

        inline Int128& operator+=(const Int128& other) noexcept;
        inline Int128& operator-=(const Int128& other) noexcept;
        inline Int128& operator*=(const Int128& other) noexcept;

        /// @throws std::overflow_error if divisor is zero.
        inline Int128& operator/=(const Int128& other);

        /// @throws std::overflow_error if divisor is zero.
        inline Int128& operator%=(const Int128& other);

        /// @throws std::overflow_error if divisor is zero.
        inline Int128 operator%(const Int128& other) const;

        [[nodiscard]] std::uint64_t toLow() const noexcept;  ///< Lower 64 bits
        [[nodiscard]] std::uint64_t toHigh() const noexcept; ///< Upper 64 bits

#if DNV_VISTA_SDK_HAS_NATIVE_INT128
        /**
         * @brief Get native @c __int128 value (GCC/Clang only).
         * @details Provides access to the underlying native 128-bit integer for interfacing
         *          with APIs that expect @c __int128 or for compiler intrinsics.
         */
        DNV_VISTA_SDK_NATIVE_INT128 toNative() const noexcept;
#endif

        /**
         * @brief Absolute value
         * @note min() (-2^127) has no positive representable magnitude. abs(min()) wraps around
         *       and returns min() again (two's complement convention), not a positive value
         */
        [[nodiscard]] inline Int128 abs() const noexcept;

        /**
         * @brief Calculate integer square root (floor of exact square root)
         * @return Largest integer i such that i*i <= this
         * @throws std::domain_error if this is negative
         * @note Returns floor(sqrt(this)) - always rounds down
         * @note For exact precision, convert to Decimal and use Decimal::sqrt()
         */
        [[nodiscard]] Int128 isqrt() const;

        /**
         * @brief Parse 128-bit integer from string without throwing.
         * @details Supported formats: decimal only, optional leading @c -, no whitespace,
         *          no scientific notation. Range: -2^127 to 2^127-1.
         * @note For @c std::optional return, use the single-parameter overload.
         */
        [[nodiscard]] static bool fromString(std::string_view str, Int128& result) noexcept;

        /**
         * @brief Parse 128-bit integer from string, returning @c std::nullopt on failure.
         * @note For error details, use the two-parameter overload.
         */
        [[nodiscard]] static std::optional<Int128> fromString(std::string_view str) noexcept;

        [[nodiscard]] std::string toString() const;                        ///< Decimal string representation
        [[nodiscard]] std::array<std::int32_t, 4> toBits() const noexcept; ///< Internal 32-bit representation

    private:
#if DNV_VISTA_SDK_HAS_NATIVE_INT128
        DNV_VISTA_SDK_NATIVE_INT128 m_value; ///< Native 128-bit integer storage (GCC/Clang)
#else
        /** @brief Manual 128-bit storage using two 64-bit words (MSVC) */
        struct Layout
        {
            std::uint64_t lower64bits;
            std::uint64_t upper64bits;
        } m_layout;
#endif
    };

    /**
     * @brief Absolute value of @p value.
     * @details Free function for ADL. Enables @c abs(value) in generic algorithms.
     */
    [[nodiscard]] inline Int128 abs(const Int128& value) noexcept
    {
        return value.abs();
    }

    /**
     * @brief Integer square root (floor) of @p value.
     * @throws std::domain_error if @p value is negative.
     * @details Free function for ADL. Returns largest @c i such that @c i*i <= value.
     */
    [[nodiscard]] inline Int128 isqrt(const Int128& value)
    {
        return value.isqrt();
    }

    /**
     * @brief High-precision square root of @p value as @c Decimal (up to 28 digits).
     * @throws std::domain_error if @p value is negative.
     * @details Examples:
     *          - @c sqrt(Int128{4})   -> @c Decimal{"2.0"} (exact)
     *          - @c sqrt(Int128{2})   -> @c Decimal{"1.4142135623730950488..."}
     *          - @c sqrt(Int128{100}) -> @c Decimal{"10.0"} (exact)
     * @note For integer-only results, use @c isqrt().
     * @see Int128::isqrt(), Decimal::sqrt()
     */
    [[nodiscard]] Decimal sqrt(const Int128& value);
} // namespace dnv::vista::sdk::internal

#include "Int128.inl"
