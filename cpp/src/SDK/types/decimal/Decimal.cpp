#include "dnv/vista/sdk/types/decimal/Decimal.h"

#include "dnv/vista/sdk/detail/types/decimal/Constants.h"

#include "Int128.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <istream>
#include <ostream>
#include <sstream>

namespace dnv::vista::sdk
{
    using internal::Int128;

    namespace internal
    {
        /**
         * @brief Multiply a 96-bit mantissa by 10 and add a digit (in-place)
         * @param mantissa Pointer to 3-element mantissa array
         * @param digit Digit to add (0-9)
         */
        static void multiplyMantissaBy10AndAdd(std::uint32_t* mantissa, std::uint32_t digit) noexcept
        {
            // Multiply 96-bit number by 10 using: result = mantissa * 8 + mantissa * 2
            std::uint64_t carry = 0;

            // Multiply by 10 (shift left 3 bits + shift left 1 bit = multiply by 8 + 2 = 10)
            for (int i = 0; i < internal::constants::DECIMAL_MANTISSA_ARRAY_SIZE; ++i)
            {
                std::uint64_t temp =
                    static_cast<std::uint64_t>(mantissa[i]) * internal::constants::DECIMAL_BASE + carry;
                mantissa[i] = static_cast<std::uint32_t>(temp);
                carry = temp >> internal::constants::BITS_PER_UINT32;
            }

            // Add the digit
            carry = digit;
            for (int i = 0; i < internal::constants::DECIMAL_MANTISSA_ARRAY_SIZE && carry > 0; ++i)
            {
                std::uint64_t temp = static_cast<std::uint64_t>(mantissa[i]) + carry;
                mantissa[i] = static_cast<std::uint32_t>(temp);
                carry = temp >> internal::constants::BITS_PER_UINT32;
            }
        }

        /**
         * @brief Get power of 10 as Int128 for any scale 0-28
         * @param power The power (0-28)
         * @return Int128 representing 10^power
         */
        static Int128 powerOf10(std::uint8_t power) noexcept
        {
            if (power < internal::constants::DECIMAL_POWER_TABLE_SIZE &&
                internal::constants::DECIMAL_POWERS_OF_10[power] != 0)
            {
                // Use 64-bit lookup table for powers 0-19
                return Int128{ internal::constants::DECIMAL_POWERS_OF_10[power] };
            }
            else if (power <= internal::constants::DECIMAL_EXTENDED_POWER_MAX)
            {
                // Use pre-computed 128-bit values for powers 20-28
                const auto& extended{ internal::constants::DECIMAL_EXTENDED_POWERS_OF_10
                                          [power - internal::constants::DECIMAL_EXTENDED_POWER_MIN] };
                return Int128{ extended.first, extended.second };
            }
            else
            {
                // Fallback to iterative computation for invalid powers (shouldn't happen)
                Int128 result{ 1 };
                for (std::uint8_t i{ 0 }; i < power; ++i)
                {
                    result = result * Int128{ internal::constants::DECIMAL_BASE };
                }
                return result;
            }
        }

        /**
         * @brief Extract 128-bit mantissa value from Decimal
         * @param decimal The decimal value to extract mantissa from
         * @return Int128 representation of the mantissa
         */
        static Int128 mantissaAsInt128(const Decimal& decimal) noexcept
        {
#if DNV_VISTA_SDK_HAS_NATIVE_INT128
            const auto& mantissaArray{ decimal.mantissa() };
            DNV_VISTA_SDK_NATIVE_INT128 value{ static_cast<DNV_VISTA_SDK_NATIVE_INT128>(mantissaArray[2])
                                                   << internal::constants::BITS_PER_UINT64 |
                                               static_cast<DNV_VISTA_SDK_NATIVE_INT128>(mantissaArray[1])
                                                   << internal::constants::BITS_PER_UINT32 |
                                               static_cast<DNV_VISTA_SDK_NATIVE_INT128>(mantissaArray[0]) };

            return Int128{ value };
#else
            const auto& mantissaArray{ decimal.mantissa() };
            std::uint64_t low{ static_cast<std::uint64_t>(mantissaArray[1]) << internal::constants::BITS_PER_UINT32 |
                               mantissaArray[0] };
            std::uint64_t high{ mantissaArray[2] };

            return Int128{ low, high };
#endif
        }

        /**
         * @brief Align scales of two decimals for arithmetic operations
         * @param decimal First decimal value
         * @param other Second decimal value
         * @return Pair of Int128 mantissas with aligned scales
         */
        static std::pair<Int128, Int128> alignScale(const Decimal& decimal, const Decimal& other)
        {
            Int128 left{ mantissaAsInt128(decimal) };
            Int128 right{ mantissaAsInt128(other) };

            std::uint8_t leftScale{ decimal.scale() };
            std::uint8_t rightScale{ other.scale() };

            // Optimized scaling using enhanced power-of-10 lookup with 128-bit support
            if (leftScale < rightScale)
            {
                std::uint8_t scaleDiff{ static_cast<std::uint8_t>(rightScale - leftScale) };
                left = left * powerOf10(scaleDiff);
            }
            else if (rightScale < leftScale)
            {
                std::uint8_t scaleDiff{ static_cast<std::uint8_t>(leftScale - rightScale) };
                right = right * powerOf10(scaleDiff);
            }

            return { std::move(left), std::move(right) };
        }

        /**
         * @brief Set mantissa value in Decimal from Int128
         * @param decimal The decimal to modify
         * @param value The Int128 mantissa value to set
         */
        static void setMantissa(Decimal& decimal, const Int128& value) noexcept
        {
#if DNV_VISTA_SDK_HAS_NATIVE_INT128
            // Cast to unsigned to ensure proper bit extraction
            auto unsignedValue{ static_cast<DNV_VISTA_SDK_NATIVE_UINT128>(value.toNative()) };
            auto& mantissa{ decimal.mantissa() };
            mantissa[0] = static_cast<std::uint32_t>(unsignedValue);
            mantissa[1] = static_cast<std::uint32_t>(unsignedValue >> internal::constants::BITS_PER_UINT32);
            mantissa[2] = static_cast<std::uint32_t>(unsignedValue >> internal::constants::BITS_PER_UINT64);
#else
            auto& mantissa{ decimal.mantissa() };
            std::uint64_t low{ value.toLow() };
            std::uint64_t high{ value.toHigh() };

            mantissa[0] = static_cast<std::uint32_t>(low);
            mantissa[1] = static_cast<std::uint32_t>(low >> internal::constants::BITS_PER_UINT32);
            mantissa[2] = static_cast<std::uint32_t>(high);
#endif
        }

        /**
         * @brief Divide decimal mantissa by power of 10
         * @param decimal The decimal to modify
         * @param power The power of 10 to divide by (0-28)
         */
        static void divideByPowerOf10(Decimal& decimal, std::uint8_t power)
        {
            Int128 mantissa{ mantissaAsInt128(decimal) };

            // Use enhanced power-of-10 lookup with full 128-bit support
            mantissa = mantissa / powerOf10(power);

            setMantissa(decimal, mantissa);
        }

        /**
         * @brief Normalize decimal by removing trailing zeros and reducing scale
         * @param decimal The decimal to normalize
         */
        static void normalize(Decimal& decimal) noexcept
        {
            // Remove trailing zeros and reduce scale
            while (decimal.scale() > 0 &&
                   (mantissaAsInt128(decimal) % Int128{ internal::constants::DECIMAL_BASE }) == Int128{ 0 })
            {
                divideByPowerOf10(decimal, 1U);
                std::uint8_t currentScale{ decimal.scale() };
                std::uint32_t currentFlags = decimal.flags();
                std::uint32_t newFlags =
                    (currentFlags & ~internal::constants::DECIMAL_SCALE_MASK) |
                    (static_cast<std::uint32_t>(currentScale - 1U) << internal::constants::DECIMAL_SCALE_SHIFT);
                decimal.flags() = newFlags;
            }
        }

        /**
         * @brief Determine if rounding up is needed for ToNearest mode (Banker's rounding)
         */
        static bool shouldRoundUpToNearest(
            const Int128& roundingDigit,
            const Int128& mantissa,
            const Int128& divisor,
            std::uint8_t digitsToRemove,
            const Decimal& result) noexcept
        {
            if (roundingDigit.toLow() > internal::constants::DECIMAL_ROUNDING_THRESHOLD)
            {
                return true; // > 5: always round away from zero
            }
            else if (roundingDigit.toLow() == internal::constants::DECIMAL_ROUNDING_THRESHOLD)
            {
                // == 5: check if there are any non-zero digits after this one
                bool hasRemainingFraction{ false };
                if (digitsToRemove > 1U)
                {
                    Int128 remainderDivisor{ divisor };
                    Int128 remainder{ mantissa % remainderDivisor };
                    Int128 roundingDigitContribution{ roundingDigit *
                                                      (divisor / Int128{ internal::constants::DECIMAL_BASE }) };
                    hasRemainingFraction = (remainder != roundingDigitContribution);
                }

                if (hasRemainingFraction)
                {
                    return true; // Ties away from zero when there's additional fractional part
                }
                else
                {
                    // Exact tie: round to even
                    Int128 resultMantissa{ mantissaAsInt128(result) };
                    bool isEven{ (resultMantissa % Int128{ 2 }) == Int128{ 0 } };
                    return !isEven; // Round up if currently odd
                }
            }
            return false;
        }

        /**
         * @brief Determine if rounding up is needed for ToNearestTiesAway mode
         */
        static bool shouldRoundUpToNearestTiesAway(const Int128& roundingDigit) noexcept
        {
            return (roundingDigit.toLow() >= internal::constants::DECIMAL_ROUNDING_THRESHOLD);
        }

        /**
         * @brief Determine if rounding up is needed for ToPositiveInfinity mode (Ceiling)
         */
        static bool shouldRoundUpToPositiveInfinity(
            const Int128& mantissa, std::uint8_t digitsToRemove, bool isNegative) noexcept
        {
            if (isNegative)
            {
                return false; // Negative numbers round toward zero for ceiling
            }

            // Check if ANY fractional digits exist
            if (digitsToRemove > 0)
            {
                Int128 fractionalDivisor{ powerOf10(digitsToRemove) };
                Int128 fractionalPart{ mantissa % fractionalDivisor };
                return fractionalPart != Int128{};
            }
            return false;
        }

        /**
         * @brief Determine if rounding up is needed for ToNegativeInfinity mode (Floor)
         */
        static bool shouldRoundUpToNegativeInfinity(
            const Int128& mantissa, std::uint8_t digitsToRemove, bool isNegative) noexcept
        {
            if (!isNegative)
            {
                return false; // Positive numbers round toward zero for floor
            }

            // Check if ANY fractional digits exist
            if (digitsToRemove > 0)
            {
                Int128 fractionalDivisor{ powerOf10(digitsToRemove) };
                Int128 fractionalPart{ mantissa % fractionalDivisor };
                return fractionalPart != Int128{};
            }
            return false;
        }

        /**
         * @brief Fast-path helper: Parse small decimals using native 64-bit arithmetic
         * @details Handles common cases like prices, percentages, etc. with 4-8× speedup
         * @param str String view to parse
         * @param result Decimal to store the result
         * @return true if fast-path succeeded, false if slow path needed
         */
        bool tryParseFastPath(std::string_view str, Decimal& result) noexcept
        {
            if (str.empty() || str.length() > 28) // Max 28 significant digits for Decimal
            {
                return false;
            }

            // Handle sign
            bool negative = false;
            size_t pos = 0;

            if (str[0] == '-')
            {
                negative = true;
                pos = 1;
            }
            else if (str[0] == '+')
            {
                pos = 1;
            }

            // Need at least one character after sign
            if (pos >= str.length())
            {
                return false;
            }

            // Quick scan: validate format and find decimal point
            size_t decimalPos = std::string_view::npos;
            size_t digitCount = 0;

            for (size_t i = pos; i < str.length(); ++i)
            {
                char c = str[i];
                if (c == '.')
                {
                    if (decimalPos != std::string_view::npos)
                    {
                        return false; // Multiple decimal points
                    }
                    decimalPos = i;
                }
                else if (c >= '0' && c <= '9')
                {
                    digitCount++;
                }
                else
                {
                    return false; // Invalid character
                }
            }

            if (digitCount == 0)
            {
                return false; // No digits
            }

            // Fast-path 1: Integer values that fit in 64-bit (most common case)
            if (decimalPos == std::string_view::npos && digitCount <= 19)
            {
                std::uint64_t value = 0;
                for (size_t i = pos; i < str.length(); ++i)
                {
                    value = value * 10 + static_cast<std::uint64_t>(str[i] - '0');
                }

                // Construct Decimal using internal helpers
                result = Decimal{};
                setMantissa(result, Int128{ value });

                if (negative)
                {
                    result.flags() |= internal::constants::DECIMAL_SIGN_MASK;
                }

                return true;
            }

            // Fast-path 2: Small decimals that fit in 64-bit mantissa
            if (decimalPos != std::string_view::npos && digitCount <= 19)
            {
                std::uint8_t scale = static_cast<std::uint8_t>(str.length() - decimalPos - 1);

                if (scale > internal::constants::DECIMAL_MAXIMUM_PLACES)
                {
                    return false; // Exceeds max precision - use slow path
                }

                std::uint64_t value = 0;
                for (size_t i = pos; i < str.length(); ++i)
                {
                    if (str[i] != '.')
                    {
                        value = value * 10 + static_cast<std::uint64_t>(str[i] - '0');
                    }
                }

                // Remove trailing zeros and adjust scale
                while (scale > 0 && value % 10 == 0)
                {
                    value /= 10;
                    scale--;
                }

                // Construct Decimal using internal helpers
                result = Decimal{};
                setMantissa(result, Int128{ value });
                result.flags() = static_cast<std::uint32_t>(scale) << internal::constants::DECIMAL_SCALE_SHIFT;

                if (negative)
                {
                    result.flags() |= internal::constants::DECIMAL_SIGN_MASK;
                }

                return true;
            }

            return false; // Use slow path for larger numbers
        }
    } // namespace internal

    Decimal::Decimal(float value) noexcept
        : m_layout{ 0, { { 0, 0, 0 } } }
    {
        if (std::isnan(value) || std::isinf(value) || value == 0.0f)
        {
            return;
        }

        char buffer[64];

        float absValue = value < 0 ? -value : value;
        int precision = (absValue > 0 && absValue < 1e-6f) ? 10 : 4;

        auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value, std::chars_format::fixed, precision);

        if (ec != std::errc{})
        {
            return; // Zero on error
        }

        // Parse the string directly (inline, avoiding fromString overhead)
        const char* p = buffer;
        const char* end = ptr;

        // Handle sign
        bool negative = false;
        if (*p == '-')
        {
            negative = true;
            ++p;
        }
        else if (*p == '+')
        {
            ++p;
        }

        const char* decimalPoint = static_cast<const char*>(std::memchr(p, '.', static_cast<std::size_t>(end - p)));

        std::uint8_t scale = 0;
        if (decimalPoint)
        {
            scale = static_cast<std::uint8_t>(end - decimalPoint - 1);

            while (scale > 0 && *(end - 1) == '0')
            {
                --scale;
                --end;
            }
        }

        // Parse digits into the mantissa
        while (p < end)
        {
            if (*p == '.')
            {
                ++p;
                continue;
            }

            if (*p >= '0' && *p <= '9')
            {
                internal::multiplyMantissaBy10AndAdd(m_layout.mantissa.data(), static_cast<std::uint32_t>(*p - '0'));
            }
            ++p;
        }

        m_layout.flags = (scale << internal::constants::DECIMAL_SCALE_SHIFT);
        if (negative)
        {
            m_layout.flags |= internal::constants::DECIMAL_SIGN_MASK;
        }

        internal::normalize(*this);
    }

    Decimal::Decimal(double value) noexcept
        : m_layout{ 0, { { 0, 0, 0 } } }
    {
        if (std::isnan(value) || std::isinf(value) || value == 0.0)
        {
            return;
        }

        // Convert double to string using std::to_chars.
        // Use fixed format to avoid scientific notation which would require
        // additional parsing logic. This ensures clean decimal representation
        // while avoiding binary representation artifacts
        char buffer[64];

        auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value, std::chars_format::fixed);

        if (ec != std::errc{})
        {
            return; // Zero on error
        }

        // Parse the string directly (inline, avoiding fromString overhead)
        const char* p = buffer;
        const char* end = ptr;

        // Handle sign
        bool negative = false;
        if (*p == '-')
        {
            negative = true;
            ++p;
        }
        else if (*p == '+')
        {
            ++p;
        }

        // Parse digits and track decimal point
        const char* decimalPoint = nullptr;
        while (p < end)
        {
            if (*p == '.')
            {
                decimalPoint = p;
                ++p;
                continue;
            }

            if (*p >= '0' && *p <= '9')
            {
                internal::multiplyMantissaBy10AndAdd(m_layout.mantissa.data(), static_cast<std::uint32_t>(*p - '0'));
            }
            ++p;
        }

        // Calculate scale (digits after decimal point)
        std::uint8_t scale = 0;
        if (decimalPoint)
        {
            scale = static_cast<std::uint8_t>(end - decimalPoint - 1);
        }

        m_layout.flags = (scale << internal::constants::DECIMAL_SCALE_SHIFT);
        if (negative)
        {
            m_layout.flags |= internal::constants::DECIMAL_SIGN_MASK;
        }

        internal::normalize(*this);
    }

    namespace internal
    {
        Decimal decimalFromInt128(const Int128& val)
        {
            Decimal result;
            if (val == Int128{})
            {
                return result;
            }

            bool isNegative = val < Int128{};

            if (val == std::numeric_limits<Int128>::min())
            {
                throw std::overflow_error{ "Int128 value exceeds Decimal range (±79228162514264337593543950335)" };
            }

            Int128 absoluteValue = val.abs();
            if (isNegative)
            {
                result.flags() |= internal::constants::DECIMAL_SIGN_MASK;
            }

            std::uint64_t high64 = absoluteValue.toHigh();
            if (high64 > internal::constants::UINT32_MAX_VALUE)
            {
                throw std::overflow_error{ "Int128 value exceeds Decimal range (±79228162514264337593543950335)" };
            }

            setMantissa(result, absoluteValue);
            return result;
        }
    } // namespace internal

    std::strong_ordering Decimal::operator<=>(const Decimal& other) const noexcept
    {
        // Compare signs first by checking flags directly (avoid recursion)
        bool thisNeg = (m_layout.flags & internal::constants::DECIMAL_SIGN_MASK) != 0;
        bool otherNeg = (other.m_layout.flags & internal::constants::DECIMAL_SIGN_MASK) != 0;

        if (thisNeg != otherNeg)
        {
            // Different signs: negative < positive
            return thisNeg ? std::strong_ordering::less : std::strong_ordering::greater;
        }

        // Same sign: compare aligned mantissas
        auto [leftMantissa, rightMantissa] = internal::alignScale(*this, other);

        // For both negative, reverse the comparison
        if (thisNeg)
        {
            if (leftMantissa > rightMantissa)
            {
                return std::strong_ordering::less;
            }
            if (leftMantissa < rightMantissa)
            {
                return std::strong_ordering::greater;
            }
            return std::strong_ordering::equal;
        }

        // Both positive or both zero
        if (leftMantissa < rightMantissa)
        {
            return std::strong_ordering::less;
        }
        if (leftMantissa > rightMantissa)
        {
            return std::strong_ordering::greater;
        }
        return std::strong_ordering::equal;
    }

    bool Decimal::operator==(const Decimal& other) const noexcept
    {
        // Check both zero by inspecting mantissa directly
        bool thisZero = m_layout.mantissa[0] == 0 && m_layout.mantissa[1] == 0 && m_layout.mantissa[2] == 0;
        bool otherZero =
            other.m_layout.mantissa[0] == 0 && other.m_layout.mantissa[1] == 0 && other.m_layout.mantissa[2] == 0;

        if (thisZero && otherZero)
        {
            return true;
        }

        // Check sign mismatch by inspecting flags directly
        bool thisNeg = (m_layout.flags & internal::constants::DECIMAL_SIGN_MASK) != 0;
        bool otherNeg = (other.m_layout.flags & internal::constants::DECIMAL_SIGN_MASK) != 0;

        if (thisNeg != otherNeg)
        {
            return false;
        }

        auto [left, right] = internal::alignScale(*this, other);

        return left == right;
    }

    Decimal Decimal::operator+(const Decimal& other) const
    {
        if (*this == Decimal{})
        {
            return other;
        }
        if (other == Decimal{})
        {
            return *this;
        }

        Decimal result;
        auto [left, right]{ internal::alignScale(*this, other) };

        // Start with scale only (sign bit must be set explicitly below)
        result.m_layout.flags = static_cast<std::uint32_t>(std::max(scale(), other.scale()))
                                << internal::constants::DECIMAL_SCALE_SHIFT;

        const bool thisNeg{ *this < Decimal{} };
        const bool otherNeg{ other < Decimal{} };

        if (thisNeg == otherNeg)
        {
            // Same sign: mantissas add, result keeps that sign
            internal::setMantissa(result, left + right);
            if (thisNeg)
            {
                result.m_layout.flags |= internal::constants::DECIMAL_SIGN_MASK;
            }
        }
        else
        {
            // Different signs: subtract smaller from larger, sign follows the larger
            if (left > right)
            {
                internal::setMantissa(result, left - right);
                if (thisNeg)
                {
                    result.m_layout.flags |= internal::constants::DECIMAL_SIGN_MASK;
                }
            }
            else
            {
                internal::setMantissa(result, right - left);
                if (otherNeg)
                {
                    result.m_layout.flags |= internal::constants::DECIMAL_SIGN_MASK;
                }
            }
        }

        internal::normalize(result);

        return result;
    }

    Decimal Decimal::operator*(const Decimal& other) const
    {
        if (*this == Decimal{} || other == Decimal{})
        {
            return Decimal{};
        }

        Decimal result;
        Int128 left{ internal::mantissaAsInt128(*this) };
        Int128 right{ internal::mantissaAsInt128(other) };

        // Combine scales
        std::uint8_t newScale{ static_cast<std::uint8_t>(scale() + other.scale()) };

        // If combined scale would exceed maximum, pre-scale operands to prevent Int128 overflow
        // Example: pi(scale=28) × e(scale=28) = scale 56, we need to reduce by 28
        if (newScale > internal::constants::DECIMAL_MAXIMUM_PLACES)
        {
            std::uint8_t excessScale = newScale - internal::constants::DECIMAL_MAXIMUM_PLACES;

            // Divide the excess scale evenly between operands to minimize precision loss
            std::uint8_t leftReduction = excessScale / 2;
            std::uint8_t rightReduction = excessScale - leftReduction;

            if (leftReduction > 0)
            {
                Int128 leftDivisor = internal::powerOf10(leftReduction);
                Int128 leftHalf = leftDivisor / Int128{ 2 };
                left = (left + leftHalf) / leftDivisor;
            }

            if (rightReduction > 0)
            {
                Int128 rightDivisor = internal::powerOf10(rightReduction);
                Int128 rightHalf = rightDivisor / Int128{ 2 };
                right = (right + rightHalf) / rightDivisor;
            }

            newScale = internal::constants::DECIMAL_MAXIMUM_PLACES;
        } // Calculate the product mantissa (now safe from overflow)
        Int128 productMantissa{ left * right };

        // Check if the mantissa fits in 96 bits (max value: 2^96 - 1)
        const Int128 max96bit{ internal::constants::DECIMAL_96BIT_MAX_LOW,
                               internal::constants::DECIMAL_96BIT_MAX_HIGH };

        // If mantissa still exceeds 96 bits, reduce precision iteratively
        while (productMantissa > max96bit && newScale > 0)
        {
            // Use rounding when dividing to minimize precision loss
            productMantissa = (productMantissa + Int128{ 5 }) / Int128{ internal::constants::DECIMAL_BASE };
            newScale--;
        }

        // Safety check
        if (productMantissa > max96bit)
        {
            // Number is too large for Decimal representation even at scale 0
            // Keep the most significant digits
            while (productMantissa > max96bit)
            {
                productMantissa = productMantissa / Int128{ internal::constants::DECIMAL_BASE };
            }
        }

        // Now store the properly scaled mantissa
        internal::setMantissa(result, productMantissa);

        result.m_layout.flags = (static_cast<std::uint32_t>(newScale) << internal::constants::DECIMAL_SCALE_SHIFT);

        // Combine signs
        if ((*this < Decimal{}) != (other < Decimal{}))
        {
            result.m_layout.flags |= internal::constants::DECIMAL_SIGN_MASK;
        }

        internal::normalize(result);

        return result;
    }

    Decimal Decimal::operator/(const Decimal& other) const
    {
        if (other == Decimal{})
        {
            throw std::overflow_error{ "Division by zero" };
        }

        if (*this == Decimal{})
        {
            return Decimal{};
        }

        Decimal result;
        Int128 dividend{ internal::mantissaAsInt128(*this) };
        Int128 divisor{ internal::mantissaAsInt128(other) };

        // Scale adjustment for division:
        // If dividend has scale d and divisor has scale s,
        // we want result with enough precision
        //
        // The mathematical identity is:
        // (dividend / 10^d) / (divisor / 10^s) = (dividend * 10^s) / (divisor * 10^d)
        //
        // To get precision p in the result, we compute:
        // (dividend * 10^(s+p)) / divisor, with result scale = d + p

        std::int32_t dividendScale = static_cast<std::int32_t>(scale());
        std::int32_t divisorScale = static_cast<std::int32_t>(other.scale());

        // Target precision: match the maximum input precision plus extra
        std::int32_t targetPrecision =
            std::max(dividendScale, divisorScale) + internal::constants::DECIMAL_DIVISION_EXTRA_PRECISION;

        // Limit to maximum Decimal places
        if (targetPrecision > internal::constants::DECIMAL_MAXIMUM_PLACES)
        {
            targetPrecision = internal::constants::DECIMAL_MAXIMUM_PLACES;
        }

        // Scale up dividend to achieve target precision
        // To get result with scale result_scale, we need:
        // result_mantissa / 10^result_scale = (dividend_mantissa / 10^dividend_scale) / (divisor_mantissa /
        // 10^divisor_scale) Which means: result_mantissa = (dividend_mantissa * 10^(divisor_scale + result_scale -
        // dividend_scale)) / divisor_mantissa
        std::int32_t scaleUpBy = divisorScale + targetPrecision - dividendScale;

        // If the overflow-avoidance breaks below stop scaling up early (fewer than scaleUpBy
        // iterations completed), the resulting `i + dividendScale - divisorScale` can be negative:
        // the dividend wasn't scaled up enough to even reach scale 0 for the requested precision.
        // A negative scale isn't representable, so to still report the value at scale 0, the
        // quotient's magnitude must be multiplied by 10^|negative amount| - clamping the scale to
        // 0 without this compensation would silently under-report the result by that same factor.
        // (Precision is necessarily reduced in this fallback case since the missing digits were
        // never computed - this only recovers the correct order of magnitude, matching what a
        // scale-0 Decimal can represent, not full precision.)
        std::int32_t extraQuotientMultiplications = 0;

        if (scaleUpBy > 0)
        {
            for (std::int32_t i = 0; i < scaleUpBy; ++i)
            {
                // Check if scaling would cause overflow BEFORE we multiply
                if (dividend.toHigh() > internal::constants::INT128_MUL10_OVERFLOW_THRESHOLD)
                {
                    // Can't scale more without overflow
                    targetPrecision = i + dividendScale - divisorScale;
                    if (targetPrecision < 0)
                    {
                        extraQuotientMultiplications = -targetPrecision;
                        targetPrecision = 0;
                    }
                    break;
                }
                Int128 newDividend = dividend * Int128{ internal::constants::DECIMAL_BASE };
                // Double-check we didn't overflow (result should be positive and bigger)
                if (newDividend < dividend)
                {
                    targetPrecision = i + dividendScale - divisorScale;
                    if (targetPrecision < 0)
                    {
                        extraQuotientMultiplications = -targetPrecision;
                        targetPrecision = 0;
                    }
                    break;
                }
                dividend = newDividend;
            }
        }
        else if (scaleUpBy < 0)
        {
            // Need to scale down - shouldn't happen with our target precision logic
            // Just use the dividend as-is
            targetPrecision = dividendScale - divisorScale;
            if (targetPrecision < 0)
            {
                extraQuotientMultiplications = -targetPrecision;
                targetPrecision = 0;
            }
        }

        Int128 quotientMantissa = dividend / divisor;

        // Compensate for the scale-up shortfall by multiplying the quotient's magnitude by the
        // missing factors of 10, so `mantissa / 10^scale` (scale 0 here) still represents the
        // correct order of magnitude instead of being silently too small.
        for (std::int32_t i = 0; i < extraQuotientMultiplications; ++i)
        {
            quotientMantissa = quotientMantissa * Int128{ internal::constants::DECIMAL_BASE };
        }

        // Check if quotient fits in 96 bits
        // A 96-bit value has toHigh() <= 0xFFFFFFFF (only lower 32 bits of high word can be set)
        while (quotientMantissa.toHigh() > 0xFFFFFFFFULL && targetPrecision > 0)
        {
            quotientMantissa = quotientMantissa / Int128{ internal::constants::DECIMAL_BASE };
            targetPrecision--;
        }

        internal::setMantissa(result, quotientMantissa);
        result.m_layout.flags =
            (static_cast<std::uint32_t>(targetPrecision) << internal::constants::DECIMAL_SCALE_SHIFT);

        // Combine signs
        if ((*this < Decimal{}) != (other < Decimal{}))
        {
            result.m_layout.flags |= internal::constants::DECIMAL_SIGN_MASK;
        }

        internal::normalize(result);

        return result;
    }

    Decimal Decimal::round(std::int32_t decimalsPlacesCount, RoundingMode mode) const noexcept
    {
        if (decimalsPlacesCount < 0)
        {
            decimalsPlacesCount = 0;
        }

        bool thisZero = m_layout.mantissa[0] == 0 && m_layout.mantissa[1] == 0 && m_layout.mantissa[2] == 0;
        if (decimalsPlacesCount >= static_cast<std::int32_t>(scale()) || thisZero)
        {
            return *this;
        }

        Decimal result{ *this };
        std::uint8_t currentScale{ scale() };
        std::uint8_t targetScale{ static_cast<std::uint8_t>(decimalsPlacesCount) };
        std::uint8_t digitsToRemove{ static_cast<std::uint8_t>(currentScale - targetScale) };

        // Get the digit that determines rounding direction
        Int128 mantissa{ internal::mantissaAsInt128(*this) };
        Int128 divisor{ 1 };
        if (digitsToRemove > 1U)
        {
            std::uint8_t divisorPowers = static_cast<std::uint8_t>(digitsToRemove - 1U);
            for (std::uint8_t i{ 0 }; i < divisorPowers; ++i)
            {
                divisor = divisor * Int128{ internal::constants::DECIMAL_BASE };
            }
        }

        Int128 roundingDigit{ (mantissa / divisor) % Int128{ internal::constants::DECIMAL_BASE } };

        // Perform truncation to target scale
        for (std::uint8_t i = 0; i < digitsToRemove; ++i)
        {
            internal::divideByPowerOf10(result, 1U);
        }

        result.m_layout.flags = (result.m_layout.flags & ~internal::constants::DECIMAL_SCALE_MASK) |
                                (static_cast<std::uint32_t>(targetScale) << internal::constants::DECIMAL_SCALE_SHIFT);

        // Determine if we should round up based on the rounding mode
        bool shouldRoundUp{ false };

        switch (mode)
        {
            case RoundingMode::ToNearest:
            {
                shouldRoundUp =
                    internal::shouldRoundUpToNearest(roundingDigit, mantissa, divisor, digitsToRemove, result);
                break;
            }
            case RoundingMode::ToNearestTiesAway:
            {
                shouldRoundUp = internal::shouldRoundUpToNearestTiesAway(roundingDigit);
                break;
            }
            case RoundingMode::ToZero:
            {
                shouldRoundUp = false; // Truncate (never round up)
                break;
            }
            case RoundingMode::ToPositiveInfinity:
            {
                bool thisNeg = (m_layout.flags & internal::constants::DECIMAL_SIGN_MASK) != 0;
                shouldRoundUp = internal::shouldRoundUpToPositiveInfinity(mantissa, digitsToRemove, thisNeg);
                break;
            }
            case RoundingMode::ToNegativeInfinity:
            {
                bool thisNeg = (m_layout.flags & internal::constants::DECIMAL_SIGN_MASK) != 0;
                shouldRoundUp = internal::shouldRoundUpToNegativeInfinity(mantissa, digitsToRemove, thisNeg);
                break;
            }
        }

        // Apply rounding adjustment
        if (shouldRoundUp)
        {
            Int128 resultMantissa{ internal::mantissaAsInt128(result) };
            // Rounding up always means incrementing the (unsigned) mantissa - for negative
            // numbers this increases the magnitude (e.g. -123 -> -124), for positive numbers
            // it increases the value directly (e.g. 123 -> 124). Same operation either way.
            resultMantissa = resultMantissa + Int128{ 1 };
            internal::setMantissa(result, resultMantissa);
        }

        return result;
    }

    Decimal Decimal::sqrt() const
    {
        // Check for negative input
        bool isNegative = (m_layout.flags & internal::constants::DECIMAL_SIGN_MASK) != 0;
        if (isNegative)
        {
            throw std::domain_error{ "Cannot compute square root of negative number" };
        }

        // Handle zero
        if (*this == 0)
        {
            return Decimal{ 0 };
        }

        // Handle one
        Decimal one{ 1 };
        if (*this == one)
        {
            return one;
        }

        // Check if this is a perfect square by converting to Int128 and using isqrt
        // This gives exact results for perfect squares
        // For scaled values (e.g., 2.25 = 225/100), scale up to integer, check if perfect square
        const auto& mantissaArray = mantissa();
        Int128 mantissaValue;

#if DNV_VISTA_SDK_HAS_NATIVE_INT128
        DNV_VISTA_SDK_NATIVE_INT128 nativeVal =
            static_cast<DNV_VISTA_SDK_NATIVE_INT128>(mantissaArray[2]) << internal::constants::BITS_PER_UINT64 |
            static_cast<DNV_VISTA_SDK_NATIVE_INT128>(mantissaArray[1]) << internal::constants::BITS_PER_UINT32 |
            static_cast<DNV_VISTA_SDK_NATIVE_INT128>(mantissaArray[0]);
        mantissaValue = Int128{ nativeVal };
#else
        std::uint64_t low =
            static_cast<std::uint64_t>(mantissaArray[1]) << internal::constants::BITS_PER_UINT32 | mantissaArray[0];
        std::uint64_t high = mantissaArray[2];
        mantissaValue = Int128{ low, high };
#endif

        // Check if mantissa is a perfect square
        Int128 intSqrt = mantissaValue.isqrt();
        Int128 squared = intSqrt * intSqrt;

        if (squared == mantissaValue)
        {
            // Mantissa is a perfect square
            // sqrt(mantissa * 10^-scale) = sqrt(mantissa) * 10^(-scale/2)
            // If scale is even, we get an exact result
            std::uint8_t currentScale = scale();

            if (currentScale % 2 == 0)
            {
                // Even scale - exact result
                // Construct result directly with correct scale
                std::uint8_t targetScale = currentScale / 2;

                // Convert intSqrt to Decimal with target scale
                Decimal result{ internal::decimalFromInt128(intSqrt) };

                // Decimal from Int128 has scale 0, we need to set it to targetScale
                // This means we divide by 10^targetScale
                if (targetScale > 0)
                {
                    Decimal divisor{ internal::decimalFromInt128(internal::powerOf10(targetScale)) };
                    result = result / divisor;
                }

                return result;
            }
        }

        // Newton-Raphson method: x_{n+1} = 0.5 * (x_n + value/x_n)
        // Use magnitude-based initial guess for much faster convergence
        // Convert to double to estimate magnitude, then use as starting point
        double valueApprox = toDouble();
        double guessApprox = std::sqrt(std::abs(valueApprox));
        Decimal x{ guessApprox };

        // Ensure initial guess is at least 1 for very small numbers
        if (x < one)
        {
            x = one;
        }

        Decimal two{ 2 };
        // Use a more practical epsilon based on the precision we can actually achieve
        Decimal epsilon{ internal::constants::DECIMAL_SQRT_EPSILON };

        // Iterate until convergence (just a few iterations to refine double precision to Decimal precision)
        Decimal xNew = x;
        Decimal xPrev = x;
        for (int i = 0; i < internal::constants::DECIMAL_SQRT_MAX_ITERATIONS; ++i)
        {
            Decimal quotient = *this / x;
            Decimal sum = x + quotient;
            xNew = sum / two; // Use division instead of multiplication

            // Check convergence: if difference is tiny, we're done
            Decimal diff = (xNew > x) ? (xNew - x) : (x - xNew);

            if (diff < epsilon)
            {
                return xNew;
            }

            // Check if xNew equals x (no progress being made)
            if (xNew == x)
            {
                return xNew;
            }

            // Check if we're oscillating between two values
            if (i > 0 && xNew == xPrev)
            {
                // Oscillating - return the value closest to the true sqrt
                // Since we're oscillating, both x and xNew are very close to the answer
                return xNew;
            }

            xPrev = x;
            x = xNew;
        }

        // Return the last computed value even if we didn't fully converge
        return xNew;
    }

    bool Decimal::fromString(std::string_view str, Decimal& result) noexcept
    {
        try
        {
            // Fast-path: Handle common cases with native 64-bit arithmetic
            if (internal::tryParseFastPath(str, result))
            {
                return true;
            }

            result = Decimal{};

            if (str.empty())
            {
                return false;
            }

            // Handle sign
            bool negative{ false };
            size_t pos{ 0 };
            if (str[0] == '-')
            {
                negative = true;
                pos = 1;
            }
            else if (str[0] == '+')
            {
                pos = 1;
            }

            // Check if we have at least one character after sign
            if (pos >= str.length())
            {
                return false;
            }

            // Find decimal point and validate there's only one
            size_t decimalPos{ std::string_view::npos };
            std::uint8_t currentScale{ 0 };
            size_t decimalCount{ 0 };

            for (size_t i{ pos }; i < str.length(); ++i)
            {
                if (str[i] == '.')
                {
                    decimalCount++;
                    if (decimalCount > 1)
                    {
                        return false;
                    }

                    decimalPos = i;
                }
            }

            if (decimalPos != std::string_view::npos)
            {
                currentScale = static_cast<std::uint8_t>(str.length() - decimalPos - 1);
                if (currentScale > internal::constants::DECIMAL_MAXIMUM_PLACES)
                {
                    currentScale = internal::constants::DECIMAL_MAXIMUM_PLACES;
                }
            }

            // Optimized digit accumulation
            Int128 mantissaValue;
            const Int128 ten{ internal::constants::DECIMAL_BASE };
            bool hasDigits{ false };
            std::uint8_t significantDigits{ 0 };
            std::uint8_t decimalDigitsProcessed{ 0 };

            for (size_t i{ pos }; i < str.length(); ++i)
            {
                if (str[i] == '.')
                {
                    continue;
                }

                if (str[i] < '0' || str[i] > '9')
                {
                    // Invalid character
                    return false;
                }

                hasDigits = true;
                std::uint64_t digit{ static_cast<std::uint64_t>(str[i] - '0') };

                if (significantDigits >= internal::constants::DECIMAL_MAX_SIGNIFICANT_DIGITS)
                {
                    // Truncate excess digits - adjust scale based on actual decimal digits processed
                    if (decimalPos != std::string_view::npos)
                    {
                        currentScale = decimalDigitsProcessed;
                    }
                    break;
                }

                // Count significant digits (skip leading zeros only before decimal point)
                if (digit != 0 || mantissaValue != Int128{ 0 } ||
                    (decimalPos != std::string_view::npos && i > decimalPos))
                {
                    significantDigits++;
                }

                // Count decimal digits processed
                if (decimalPos != std::string_view::npos && i > decimalPos)
                {
                    decimalDigitsProcessed++;
                }

                // Safe to accumulate this digit
                mantissaValue = mantissaValue * ten + Int128{ digit };
            }

            // Ensure we have at least one digit (prevents parsing ".", "+", "-", etc.)
            if (!hasDigits)
            {
                return false;
            }

            // Check if mantissa fits in our 96-bit storage
            if (mantissaValue.toHigh() > internal::constants::UINT32_MAX_VALUE)
            {
                // Value too large - truncate excess precision to fit
                while (mantissaValue.toHigh() > internal::constants::UINT32_MAX_VALUE && currentScale > 0)
                {
                    mantissaValue = mantissaValue / Int128{ internal::constants::DECIMAL_BASE };
                    --currentScale;
                }

                // If still too large after removing all decimal places, truncate the integer part to fit
                while (mantissaValue.toHigh() > internal::constants::UINT32_MAX_VALUE)
                {
                    mantissaValue = mantissaValue / Int128{ internal::constants::DECIMAL_BASE };
                }
            }

            // Set result
            if (negative)
            {
                result.m_layout.flags |= internal::constants::DECIMAL_SIGN_MASK;
            }

            result.m_layout.flags |=
                (static_cast<std::uint32_t>(currentScale) << internal::constants::DECIMAL_SCALE_SHIFT);

            // Store the 96-bit mantissa
            std::uint64_t low{ mantissaValue.toLow() };
            std::uint64_t high{ mantissaValue.toHigh() };

            result.m_layout.mantissa[0] = static_cast<std::uint32_t>(low);
            result.m_layout.mantissa[1] = static_cast<std::uint32_t>(low >> internal::constants::BITS_PER_UINT32);
            result.m_layout.mantissa[2] = static_cast<std::uint32_t>(high);

            // Normalize to remove trailing zeros
            internal::normalize(result);

            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    std::optional<Decimal> Decimal::fromString(std::string_view str) noexcept
    {
        Decimal result;
        if (fromString(str, result))
        {
            return result;
        }
        return std::nullopt;
    }

    double Decimal::toDouble() const noexcept
    {
        Int128 mantissa{ internal::mantissaAsInt128(*this) };

        double result;
#if DNV_VISTA_SDK_HAS_NATIVE_INT128
        result = static_cast<double>(mantissa.toNative());
#else
        // Convert 128-bit to double (approximate)
        result = static_cast<double>(mantissa.toHigh()) *
                     (internal::constants::BIT_MASK_ONE << internal::constants::BITS_PER_UINT32) *
                     (internal::constants::BIT_MASK_ONE << internal::constants::BITS_PER_UINT32) +
                 static_cast<double>(mantissa.toLow());
#endif
        // Apply scale (using single division to avoid cumulative rounding errors)
        std::uint8_t currentScale = scale();
        if (currentScale > 0)
        {
            result /= internal::constants::DOUBLE_POWERS_OF_10[currentScale];
        }

        // Apply sign
        if ((m_layout.flags & internal::constants::DECIMAL_SIGN_MASK) != 0)
        {
            result = -result;
        }

        return result;
    }

    std::string Decimal::toString() const
    {
        StringBuilder sb;
        toString(sb);
        return sb.toString();
    }

    void Decimal::toString(StringBuilder& builder) const
    {
        if (m_layout.mantissa[0] == 0 && m_layout.mantissa[1] == 0 && m_layout.mantissa[2] == 0)
        {
            builder += '0';
            return;
        }

        Int128 mantissa{ internal::mantissaAsInt128(*this).abs() };
        std::uint8_t currentScale{ scale() };

        // Optimized digit extraction with fast division
        std::array<char, internal::constants::DECIMAL_MAX_STRING_LENGTH> digits;
        size_t digitCount{ 0 };

#if DNV_VISTA_SDK_HAS_NATIVE_INT128
        // Fast path for values that fit in 64-bit
        if (mantissa.toNative() <= UINT64_MAX)
        {
            std::uint64_t value{ static_cast<std::uint64_t>(mantissa.toNative()) };
            while (value > 0 && digitCount < digits.size())
            {
                digits[digitCount++] = static_cast<char>('0' + (value % internal::constants::DECIMAL_BASE));
                value /= internal::constants::DECIMAL_BASE;
            }
        }
        else
        {
            // Full 128-bit extraction
            while (mantissa != 0 && digitCount < digits.size())
            {
                digits[digitCount++] =
                    static_cast<char>('0' + (mantissa.toNative() % internal::constants::DECIMAL_BASE));
                mantissa = Int128{ mantissa.toNative() / internal::constants::DECIMAL_BASE };
            }
        }
#else
        if (mantissa.toHigh() == 0)
        {
            // Fast 64-bit path
            std::uint64_t value{ mantissa.toLow() };
            while (value > 0 && digitCount < digits.size())
            {
                digits[digitCount++] = static_cast<char>('0' + (value % internal::constants::DECIMAL_BASE));
                value /= internal::constants::DECIMAL_BASE;
            }
        }
        else
        {
            // Manual 128-bit extraction
            while (mantissa != Int128{} && digitCount < digits.size())
            {
                if (mantissa.toHigh() == 0)
                {
                    // Switched to 64-bit range
                    std::uint64_t value{ mantissa.toLow() };
                    while (value > 0 && digitCount < digits.size())
                    {
                        digits[digitCount++] = static_cast<char>('0' + (value % internal::constants::DECIMAL_BASE));
                        value /= internal::constants::DECIMAL_BASE;
                    }
                    break;
                }
                // Use full 128-bit value for modulo operation, not just low 64 bits
                Int128 remainder{ mantissa % Int128{ internal::constants::DECIMAL_BASE } };
                digits[digitCount++] = static_cast<char>('0' + remainder.toLow());

                mantissa = mantissa / Int128{ internal::constants::DECIMAL_BASE };
            }
        }
#endif

        if (digitCount == 0)
        {
            digitCount = 1;
            digits[0] = '0';
        }

        // Handle sign
        if ((m_layout.flags & internal::constants::DECIMAL_SIGN_MASK) != 0)
        {
            builder += '-';
        }

        // Apply decimal point formatting
        if (currentScale > 0)
        {
            if (currentScale >= digitCount)
            {
                // Need leading zeros: "0.00123"
                builder += '0';
                builder += '.';

                // Add leading zeros
                for (size_t i = 0; i < currentScale - digitCount; ++i)
                {
                    builder += '0';
                }

                // Add digits in reverse order
                for (size_t i = digitCount; i > 0; --i)
                {
                    builder += digits[i - 1];
                }
            }
            else
            {
                // Add integer part (reverse order)
                for (size_t i = digitCount; i > currentScale; --i)
                {
                    builder += digits[i - 1];
                }

                builder += '.';

                // Add fractional part (reverse order)
                for (size_t i = currentScale; i > 0; --i)
                {
                    builder += digits[i - 1];
                }
            }
        }
        else
        {
            // No decimal point, just add digits in reverse order
            for (size_t i = digitCount; i > 0; --i)
            {
                builder += digits[i - 1];
            }
        }
    }

    std::array<std::int32_t, 4> Decimal::toBits() const noexcept
    {
        std::array<std::int32_t, 4> bits{};

        // First three elements are the 96-bit mantissa
        bits[0] = static_cast<std::int32_t>(m_layout.mantissa[0]);
        bits[1] = static_cast<std::int32_t>(m_layout.mantissa[1]);
        bits[2] = static_cast<std::int32_t>(m_layout.mantissa[2]);

        // Fourth element contains scale and sign information
        bits[3] = static_cast<std::int32_t>(m_layout.flags);

        return bits;
    }

    std::uint8_t Decimal::decimalPlacesCount() const noexcept
    {
        // If the value is zero, it has 0 decimal places
        if (m_layout.mantissa[0] == 0 && m_layout.mantissa[1] == 0 && m_layout.mantissa[2] == 0)
        {
            return 0;
        }

        // Get the current scale
        std::uint8_t currentScale = scale();

        // If scale is 0, it's an integer - no decimal places
        if (currentScale == 0)
        {
            return 0;
        }

        // Convert mantissa to Int128 for proper arithmetic
        const auto& mantissaArray = mantissa();
#if DNV_VISTA_SDK_HAS_NATIVE_INT128
        DNV_VISTA_SDK_NATIVE_INT128 mantissaValue{ static_cast<DNV_VISTA_SDK_NATIVE_INT128>(mantissaArray[2])
                                                       << internal::constants::BITS_PER_UINT64 |
                                                   static_cast<DNV_VISTA_SDK_NATIVE_INT128>(mantissaArray[1])
                                                       << internal::constants::BITS_PER_UINT32 |
                                                   static_cast<DNV_VISTA_SDK_NATIVE_INT128>(mantissaArray[0]) };
        Int128 mantissa128{ mantissaValue };
#else
        std::uint64_t low{ static_cast<std::uint64_t>(mantissaArray[1]) << internal::constants::BITS_PER_UINT32 |
                           mantissaArray[0] };
        std::uint64_t high{ mantissaArray[2] };
        Int128 mantissa128{ low, high };
#endif

        std::uint8_t trailingZeros = 0;
        Int128 ten{ internal::constants::DECIMAL_BASE };

        // Count trailing zeros by testing divisibility by 10 iteratively
        while (trailingZeros < currentScale)
        {
            // If there's a remainder, we can't divide evenly by 10
            if (mantissa128 % ten != Int128{ 0 })
            {
                break;
            }

            // Continue testing with the next power of 10
            mantissa128 = mantissa128 / ten;
            trailingZeros++;
        }

        return currentScale - trailingZeros;
    }

    std::uint32_t Decimal::totalDigitsCount() const
    {
        const auto str = toString();
        const auto digitCount = static_cast<std::uint32_t>(
            std::count_if(str.begin(), str.end(), [](char c) { return c != '.' && c != '-'; }));
        return digitCount == 0 ? 1 : digitCount;
    }

    std::ostream& operator<<(std::ostream& os, const Decimal& decimal)
    {
        // Check if std::fixed is set with specific precision
        if ((os.flags() & std::ios_base::fixed) && os.precision() >= 0)
        {
            // Format with minimum decimal places based on stream precision
            std::string str{ decimal.toString() };

            // Find decimal point
            size_t dot_pos{ str.find('.') };

            if (dot_pos == std::string::npos)
            {
                // No decimal point - add it with required precision
                str += '.';
                for (std::streamsize i = 0; i < os.precision(); ++i)
                {
                    str += '0';
                }
            }
            else
            {
                // Has decimal point - pad to required precision
                size_t current_decimals{ str.length() - dot_pos - 1 };
                if (static_cast<std::streamsize>(current_decimals) < os.precision())
                {
                    for (std::streamsize i = static_cast<std::streamsize>(current_decimals); i < os.precision(); ++i)
                    {
                        str += '0';
                    }
                }
            }

            return os << str;
        }

        // Default behavior: use toString() as-is
        return os << decimal.toString();
    }

    std::istream& operator>>(std::istream& is, Decimal& decimal)
    {
        std::string str;
        is >> str;

        if (!Decimal::fromString(str, decimal))
        {
            is.setstate(std::ios::failbit);
        }

        return is;
    }
} // namespace dnv::vista::sdk
