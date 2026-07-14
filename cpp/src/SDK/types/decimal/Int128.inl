#include "dnv/vista/sdk/detail/types/decimal/Constants.h"
#include "Constants.h"

#include <cmath>
#include <stdexcept>
#include <string_view>

namespace dnv::vista::sdk::internal
{
#if DNV_VISTA_SDK_HAS_NATIVE_INT128

    inline constexpr Int128::Int128() noexcept
        : m_value{ 0 }
    {}

    inline constexpr Int128::Int128(int val) noexcept
        : m_value{ val }
    {}

    inline constexpr Int128::Int128(std::int64_t val) noexcept
        : m_value{ val }
    {}

    inline constexpr Int128::Int128(std::uint32_t val) noexcept
        : m_value{ val }
    {}

    inline constexpr Int128::Int128(std::uint64_t val) noexcept
        : m_value{ val }
    {}

    inline constexpr Int128::Int128(DNV_VISTA_SDK_NATIVE_INT128 val) noexcept
        : m_value{ val }
    {}

    inline Int128::Int128(std::string_view str)
    {
        if (!fromString(str, *this))
        {
            throw std::invalid_argument{ "Invalid Int128 string format" };
        }
    }

    inline Int128::Int128(const char* scStr)
        : Int128{ std::string_view{ scStr } }
    {}

    inline constexpr Int128::Int128(std::uint64_t low, std::uint64_t high) noexcept
        : m_value{ static_cast<DNV_VISTA_SDK_NATIVE_INT128>(high) << internal::constants::BITS_PER_UINT64 | low }
    {}

    inline std::strong_ordering Int128::operator<=>(const Int128& other) const noexcept
    {
        if (m_value < other.m_value)
        {
            return std::strong_ordering::less;
        }
        if (m_value > other.m_value)
        {
            return std::strong_ordering::greater;
        }
        return std::strong_ordering::equal;
    }

    inline bool Int128::operator==(const Int128& other) const noexcept
    {
        return m_value == other.m_value;
    }

    inline bool Int128::operator==(int val) const noexcept
    {
        return m_value == val;
    }

    inline bool Int128::operator<(int val) const noexcept
    {
        return m_value < val;
    }

    inline bool Int128::operator<=(int val) const noexcept
    {
        return *this < val || *this == val;
    }

    inline bool Int128::operator>(int val) const noexcept
    {
        return m_value > val;
    }

    inline bool Int128::operator>=(int val) const noexcept
    {
        return *this > val || *this == val;
    }

    inline bool Int128::operator==(std::int64_t val) const noexcept
    {
        return m_value == val;
    }

    inline bool Int128::operator<(std::int64_t val) const noexcept
    {
        return m_value < val;
    }

    inline bool Int128::operator<=(std::int64_t val) const noexcept
    {
        return *this < val || *this == val;
    }

    inline bool Int128::operator>(std::int64_t val) const noexcept
    {
        return m_value > val;
    }

    inline bool Int128::operator>=(std::int64_t val) const noexcept
    {
        return *this > val || *this == val;
    }

    inline bool Int128::operator==(std::uint64_t val) const noexcept
    {
        return m_value >= 0 && static_cast<std::uint64_t>(m_value) == val;
    }

    inline bool Int128::operator<(std::uint64_t val) const noexcept
    {
        return m_value < 0 || static_cast<std::uint64_t>(m_value) < val;
    }

    inline bool Int128::operator<=(std::uint64_t val) const noexcept
    {
        return *this < val || *this == val;
    }

    inline bool Int128::operator>(std::uint64_t val) const noexcept
    {
        return m_value >= 0 && static_cast<std::uint64_t>(m_value) > val;
    }

    inline bool Int128::operator>=(std::uint64_t val) const noexcept
    {
        return *this > val || *this == val;
    }

    inline bool Int128::operator==(double val) const noexcept
    {
        constexpr double EPSILON = std::numeric_limits<double>::epsilon();
        return std::fabs(static_cast<double>(m_value) - val) <= EPSILON;
    }

    inline bool Int128::operator<(double val) const noexcept
    {
        return static_cast<double>(m_value) < val;
    }

    inline bool Int128::operator<=(double val) const noexcept
    {
        return *this < val || *this == val;
    }

    inline bool Int128::operator>(double val) const noexcept
    {
        return static_cast<double>(m_value) > val;
    }

    inline bool Int128::operator>=(double val) const noexcept
    {
        return *this > val || *this == val;
    }

    inline Int128 Int128::operator+(const Int128& other) const noexcept
    {
        using Unsigned = DNV_VISTA_SDK_NATIVE_UINT128;
        return Int128{ static_cast<DNV_VISTA_SDK_NATIVE_INT128>(
            static_cast<Unsigned>(m_value) + static_cast<Unsigned>(other.m_value)) };
    }

    inline Int128 Int128::operator-(const Int128& other) const noexcept
    {
        using Unsigned = DNV_VISTA_SDK_NATIVE_UINT128;
        return Int128{ static_cast<DNV_VISTA_SDK_NATIVE_INT128>(
            static_cast<Unsigned>(m_value) - static_cast<Unsigned>(other.m_value)) };
    }

    inline Int128 Int128::operator*(const Int128& other) const noexcept
    {
        using Unsigned = DNV_VISTA_SDK_NATIVE_UINT128;
        return Int128{ static_cast<DNV_VISTA_SDK_NATIVE_INT128>(
            static_cast<Unsigned>(m_value) * static_cast<Unsigned>(other.m_value)) };
    }

    inline Int128 Int128::operator/(const Int128& other) const
    {
        if (other.m_value == 0)
        {
            throw std::overflow_error{ "Division by zero" };
        }

        constexpr DNV_VISTA_SDK_NATIVE_INT128 int128Min = static_cast<DNV_VISTA_SDK_NATIVE_INT128>(1)
                                                          << (2 * constants::BITS_PER_UINT64 - 1);
        if (m_value == int128Min && other.m_value == static_cast<DNV_VISTA_SDK_NATIVE_INT128>(-1))
        {
            return *this;
        }

        return Int128{ m_value / other.m_value };
    }

    inline Int128 Int128::operator%(const Int128& other) const
    {
        if (other.m_value == 0)
        {
            throw std::overflow_error{ "Division by zero" };
        }

        return Int128{ m_value % other.m_value };
    }

    inline Int128 Int128::operator-() const noexcept
    {
        using Unsigned = DNV_VISTA_SDK_NATIVE_UINT128;
        return Int128{ static_cast<DNV_VISTA_SDK_NATIVE_INT128>(0 - static_cast<Unsigned>(m_value)) };
    }

    inline Int128 Int128::abs() const noexcept
    {
        return m_value < 0 ? -*this : *this;
    }

    inline std::uint64_t Int128::toLow() const noexcept
    {
        return static_cast<std::uint64_t>(m_value);
    }

    inline std::uint64_t Int128::toHigh() const noexcept
    {
        return static_cast<std::uint64_t>(m_value >> internal::constants::BITS_PER_UINT64);
    }

    inline DNV_VISTA_SDK_NATIVE_INT128 Int128::toNative() const noexcept
    {
        return m_value;
    }

    inline std::array<std::int32_t, 4> Int128::toBits() const noexcept
    {
        std::array<std::int32_t, 4> bits{};
        std::uint64_t low = toLow();
        std::uint64_t high = toHigh();
        bits[0] = static_cast<std::int32_t>(low);
        bits[1] = static_cast<std::int32_t>(low >> internal::constants::BITS_PER_UINT32);
        bits[2] = static_cast<std::int32_t>(high);
        bits[3] = static_cast<std::int32_t>(high >> internal::constants::BITS_PER_UINT32);
        return bits;
    }

#else

    inline constexpr Int128::Int128() noexcept
        : m_layout{ 0, 0 }
    {}

    inline constexpr Int128::Int128(int val) noexcept
        : m_layout{ static_cast<std::uint64_t>(val), (val < 0) ? static_cast<std::uint64_t>(-1) : 0 }
    {}

    inline constexpr Int128::Int128(std::int64_t val) noexcept
        : m_layout{ static_cast<std::uint64_t>(val), (val < 0) ? static_cast<std::uint64_t>(-1) : 0 }
    {}

    inline constexpr Int128::Int128(std::uint32_t val) noexcept
        : m_layout{ val, 0 }
    {}

    inline constexpr Int128::Int128(std::uint64_t val) noexcept
        : m_layout{ val, 0 }
    {}

    inline Int128::Int128(std::string_view str)
    {
        if (!fromString(str, *this))
        {
            throw std::invalid_argument{ "Invalid Int128 string format" };
        }
    }

    inline Int128::Int128(const char* scStr)
        : Int128{ std::string_view{ scStr } }
    {}

    inline constexpr Int128::Int128(std::uint64_t low, std::uint64_t high) noexcept
        : m_layout{ low, high }
    {}

    inline std::strong_ordering Int128::operator<=>(const Int128& other) const noexcept
    {
        bool thisNeg = static_cast<std::int64_t>(m_layout.upper64bits) < 0;
        bool otherNeg = static_cast<std::int64_t>(other.m_layout.upper64bits) < 0;

        if (thisNeg && !otherNeg)
        {
            return std::strong_ordering::less;
        }
        if (!thisNeg && otherNeg)
        {
            return std::strong_ordering::greater;
        }

        if (m_layout.upper64bits < other.m_layout.upper64bits)
        {
            return std::strong_ordering::less;
        }
        if (m_layout.upper64bits > other.m_layout.upper64bits)
        {
            return std::strong_ordering::greater;
        }

        if (m_layout.lower64bits < other.m_layout.lower64bits)
        {
            return std::strong_ordering::less;
        }
        if (m_layout.lower64bits > other.m_layout.lower64bits)
        {
            return std::strong_ordering::greater;
        }

        return std::strong_ordering::equal;
    }

#endif

#if DNV_VISTA_SDK_HAS_NATIVE_INT128

    inline Int128& Int128::operator+=(const Int128& other) noexcept
    {
        using Unsigned = DNV_VISTA_SDK_NATIVE_UINT128;
        m_value = static_cast<DNV_VISTA_SDK_NATIVE_INT128>(
            static_cast<Unsigned>(m_value) + static_cast<Unsigned>(other.m_value));
        return *this;
    }

    inline Int128& Int128::operator-=(const Int128& other) noexcept
    {
        using Unsigned = DNV_VISTA_SDK_NATIVE_UINT128;
        m_value = static_cast<DNV_VISTA_SDK_NATIVE_INT128>(
            static_cast<Unsigned>(m_value) - static_cast<Unsigned>(other.m_value));
        return *this;
    }

    inline Int128& Int128::operator*=(const Int128& other) noexcept
    {
        using Unsigned = DNV_VISTA_SDK_NATIVE_UINT128;
        m_value = static_cast<DNV_VISTA_SDK_NATIVE_INT128>(
            static_cast<Unsigned>(m_value) * static_cast<Unsigned>(other.m_value));
        return *this;
    }

    inline Int128& Int128::operator/=(const Int128& other)
    {
        if (other.m_value == 0)
        {
            throw std::overflow_error{ "Division by zero" };
        }
        m_value /= other.m_value;
        return *this;
    }

    inline Int128& Int128::operator%=(const Int128& other)
    {
        if (other.m_value == 0)
        {
            throw std::overflow_error{ "Division by zero" };
        }
        m_value %= other.m_value;
        return *this;
    }
#else
    Int128 Int128::operator+(const Int128& other) const noexcept
    {
        // 128-bit addition with carry propagation
        std::uint64_t resultLow{ m_layout.lower64bits + other.m_layout.lower64bits };
        std::uint64_t carry{ (resultLow < m_layout.lower64bits) ? internal::constants::BIT_MASK_ONE
                                                                : internal::constants::BIT_MASK_ZERO };
        std::uint64_t resultHigh{ m_layout.upper64bits + other.m_layout.upper64bits + carry };
        return Int128{ resultLow, resultHigh };
    }

    Int128 Int128::operator-(const Int128& other) const noexcept
    {
        // 128-bit subtraction with borrow propagation
        std::uint64_t resultLow{ m_layout.lower64bits - other.m_layout.lower64bits };
        std::uint64_t borrow{ (m_layout.lower64bits < other.m_layout.lower64bits)
                                  ? internal::constants::BIT_MASK_ONE
                                  : internal::constants::BIT_MASK_ZERO };
        std::uint64_t resultHigh{ m_layout.upper64bits - other.m_layout.upper64bits - borrow };
        return Int128{ resultLow, resultHigh };
    }

    inline Int128& Int128::operator+=(const Int128& other) noexcept
    {
        *this = *this + other;
        return *this;
    }

    inline Int128& Int128::operator-=(const Int128& other) noexcept
    {
        *this = *this - other;
        return *this;
    }

    inline Int128& Int128::operator*=(const Int128& other) noexcept
    {
        *this = *this * other;
        return *this;
    }

    inline Int128& Int128::operator/=(const Int128& other)
    {
        *this = *this / other;
        return *this;
    }

    inline Int128& Int128::operator%=(const Int128& other)
    {
        if (other == Int128{})
        {
            throw std::overflow_error{ "Division by zero" };
        }
        *this = *this % other;
        return *this;
    }

    inline Int128 Int128::operator%(const Int128& other) const
    {
        if (other == Int128{})
        {
            throw std::overflow_error{ "Division by zero" };
        }

        // Performance optimization: Fast modulo for 64-bit values
        if (m_layout.upper64bits == 0 && other.m_layout.upper64bits == 0)
        {
            // Both fit in 64-bit - use native modulo
            return Int128{ m_layout.lower64bits % other.m_layout.lower64bits, 0 };
        }

        Int128 quotient{ *this / other };

        return *this - (quotient * other);
    }

    inline Int128 Int128::operator-() const noexcept
    {
        // Two's complement negation
        Int128 result{ Int128{ ~m_layout.lower64bits, ~m_layout.upper64bits } };

        return Int128{ result + Int128{ 1 } };
    }

    inline bool Int128::operator==(const Int128& other) const noexcept
    {
        return m_layout.lower64bits == other.m_layout.lower64bits && m_layout.upper64bits == other.m_layout.upper64bits;
    }

    inline bool Int128::operator==(int val) const noexcept
    {
        return *this == static_cast<std::int64_t>(val);
    }

    inline bool Int128::operator<(int val) const noexcept
    {
        return *this < static_cast<std::int64_t>(val);
    }

    inline bool Int128::operator<=(int val) const noexcept
    {
        return *this <= static_cast<std::int64_t>(val);
    }

    inline bool Int128::operator>(int val) const noexcept
    {
        return *this > static_cast<std::int64_t>(val);
    }

    inline bool Int128::operator>=(int val) const noexcept
    {
        return *this >= static_cast<std::int64_t>(val);
    }

    inline bool Int128::operator==(std::int64_t val) const noexcept
    {
        // For negative values, upper64bits should be all 1s (sign extension)
        // For positive values, upper64bits should be 0
        std::uint64_t expected_upper = (val < 0) ? static_cast<std::uint64_t>(-1) : 0;

        return m_layout.upper64bits == expected_upper && m_layout.lower64bits == static_cast<std::uint64_t>(val);
    }

    inline bool Int128::operator<(std::int64_t val) const noexcept
    {
        // For negative values, upper64bits should be all 1s (sign extension)
        // For positive values, upper64bits should be 0
        std::uint64_t expected_upper = (val < 0) ? static_cast<std::uint64_t>(-1) : 0;

        // Compare as signed values for correct ordering
        if (m_layout.upper64bits != expected_upper)
        {
            return static_cast<std::int64_t>(m_layout.upper64bits) < static_cast<std::int64_t>(expected_upper);
        }
        return m_layout.lower64bits < static_cast<std::uint64_t>(val);
    }

    inline bool Int128::operator<=(std::int64_t val) const noexcept
    {
        return *this < val || *this == val;
    }

    inline bool Int128::operator>(std::int64_t val) const noexcept
    {
        // For negative values, upper64bits should be all 1s (sign extension)
        // For positive values, upper64bits should be 0
        std::uint64_t expected_upper = (val < 0) ? static_cast<std::uint64_t>(-1) : 0;

        // Compare as signed values for correct ordering
        if (m_layout.upper64bits != expected_upper)
        {
            return static_cast<std::int64_t>(m_layout.upper64bits) > static_cast<std::int64_t>(expected_upper);
        }
        return m_layout.lower64bits > static_cast<std::uint64_t>(val);
    }

    inline bool Int128::operator>=(std::int64_t val) const noexcept
    {
        return *this > val || *this == val;
    }

    inline bool Int128::operator==(std::uint64_t val) const noexcept
    {
        // For unsigned comparison, this Int128 must be non-negative
        return m_layout.upper64bits == 0 && m_layout.lower64bits == val;
    }

    inline bool Int128::operator<(std::uint64_t val) const noexcept
    {
        // If this is negative, it's always less than any positive uint64_t
        if (*this < Int128{})
        {
            return true;
        }

        // If upper bits are non-zero, this is definitely >= 2^64, so greater than any uint64_t
        if (m_layout.upper64bits != 0)
        {
            return false;
        }

        // Compare lower bits
        return m_layout.lower64bits < val;
    }

    inline bool Int128::operator<=(std::uint64_t val) const noexcept
    {
        return *this < val || *this == val;
    }

    inline bool Int128::operator>(std::uint64_t val) const noexcept
    {
        // If this is negative, it's never greater than any positive uint64_t
        if (*this < Int128{})
        {
            return false;
        }

        // If upper bits are non-zero, this is definitely >= 2^64, so greater than any uint64_t
        if (m_layout.upper64bits != 0)
        {
            return true;
        }

        // Compare lower bits
        return m_layout.lower64bits > val;
    }

    inline bool Int128::operator>=(std::uint64_t val) const noexcept
    {
        return *this > val || *this == val;
    }

    inline bool Int128::operator<=(double val) const noexcept
    {
        return *this < val || *this == val;
    }

    inline bool Int128::operator>=(double val) const noexcept
    {
        return *this > val || *this == val;
    }

    inline Int128 Int128::abs() const noexcept
    {
        if (!(*this < Int128{}))
        {
            return *this;
        }
        return -*this;
    }

    inline std::uint64_t Int128::toLow() const noexcept
    {
        return m_layout.lower64bits;
    }

    inline std::uint64_t Int128::toHigh() const noexcept
    {
        return m_layout.upper64bits;
    }

    inline std::array<std::int32_t, 4> Int128::toBits() const noexcept
    {
        std::array<std::int32_t, 4> bits{};
        std::uint64_t low = toLow();
        std::uint64_t high = toHigh();
        bits[0] = static_cast<std::int32_t>(low);
        bits[1] = static_cast<std::int32_t>(low >> internal::constants::BITS_PER_UINT32);
        bits[2] = static_cast<std::int32_t>(high);
        bits[3] = static_cast<std::int32_t>(high >> internal::constants::BITS_PER_UINT32);
        return bits;
    }
#endif

} // namespace dnv::vista::sdk::internal

namespace std
{
    template <>
    struct numeric_limits<dnv::vista::sdk::internal::Int128>
    {
        static constexpr bool is_specialized = true;
        static constexpr bool is_signed = true;
        static constexpr bool is_integer = true;
        static constexpr bool is_exact = true;
        static constexpr bool has_infinity = false;
        static constexpr bool has_quiet_NaN = false;
        static constexpr bool has_signaling_NaN = false;
        static constexpr bool has_denorm = false;
        static constexpr bool has_denorm_loss = false;
        static constexpr bool is_bounded = true;
        static constexpr bool is_modulo = false;
        static constexpr bool traps = false;
        static constexpr bool tinyness_before = false;
        static constexpr std::float_round_style round_style = std::round_toward_zero;

        static dnv::vista::sdk::internal::Int128 min() noexcept
        {
            // Minimum value: -2^127 = -170141183460469231731687303715884105728
            return dnv::vista::sdk::internal::Int128{ dnv::vista::sdk::internal::constants::INT128_MIN_NEGATIVE_LOW,
                                                      dnv::vista::sdk::internal::constants::INT128_MIN_NEGATIVE_HIGH };
        }

        static dnv::vista::sdk::internal::Int128 max() noexcept
        {
            // Maximum value: 2^127 - 1 = 170141183460469231731687303715884105727
            return dnv::vista::sdk::internal::Int128{ dnv::vista::sdk::internal::constants::INT128_MAX_POSITIVE_LOW,
                                                      dnv::vista::sdk::internal::constants::INT128_MAX_POSITIVE_HIGH };
        }

        static dnv::vista::sdk::internal::Int128 lowest() noexcept { return min(); }

        static dnv::vista::sdk::internal::Int128 epsilon() noexcept { return dnv::vista::sdk::internal::Int128{ 0 }; }

        static constexpr int digits =
            dnv::vista::sdk::internal::constants::INT128_DIGITS; // value bits (excluding sign bit)
        static constexpr int digits10 =
            dnv::vista::sdk::internal::constants::INT128_DIGITS10; // decimal digits (floor(127 * log10(2)))
        static constexpr int max_digits10 =
            dnv::vista::sdk::internal::constants::INT128_MAX_DIGITS10;                   // not applicable for integers
        static constexpr int radix = dnv::vista::sdk::internal::constants::INT128_RADIX; // binary
        static constexpr int min_exponent = 0;
        static constexpr int min_exponent10 = 0;
        static constexpr int max_exponent = 0;
        static constexpr int max_exponent10 = 0;
    };
} // namespace std
