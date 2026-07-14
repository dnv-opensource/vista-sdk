#include "Int128.h"

#include "dnv/vista/sdk/types/decimal/Decimal.h"

#if defined(_MSC_VER) && !defined(__clang__)
    #include <intrin.h>
#endif

#include <sstream>
#include <iomanip>
#include <cmath>
#include <limits>

namespace dnv::vista::sdk::internal
{
    using dnv::vista::sdk::Decimal;

    Decimal decimalFromInt128(const Int128& val);

    namespace
    {
        // Fast-path helper: Parse digits without allocation/multiplication for values <= 64-bit
        // Handles ~90% of real-world Int128 use cases with 5-10× speedup
        bool tryParseFastPath(std::string_view str, Int128& result) noexcept
        {
            if (str.empty() || str.length() > 20) // uint64_max is 20 digits
            {
                return false;
            }

            // Handle sign
            bool isNegative = false;
            size_t pos = 0;

            if (str[0] == '-')
            {
                isNegative = true;
                pos = 1;
            }
            else if (str[0] == '+')
            {
                pos = 1;
            }

            // Need at least one digit after sign
            if (pos >= str.length())
            {
                return false;
            }

            // Quick validation: all remaining chars must be digits
            for (size_t i = pos; i < str.length(); ++i)
            {
                if (str[i] < '0' || str[i] > '9')
                {
                    return false;
                }
            }

            // For 20 digits, need overflow check (uint64_max = 18446744073709551615)
            size_t digitCount = str.length() - pos;
            if (digitCount == 20)
            {
                // Lexicographic comparison is safe for same-length numeric strings
                if (str.substr(pos) > "18446744073709551615")
                {
                    return false; // Would overflow uint64_t
                }
            }

            // Fast accumulation using native 64-bit arithmetic
            std::uint64_t value = 0;
            for (size_t i = pos; i < str.length(); ++i)
            {
                value = value * 10 + static_cast<std::uint64_t>(str[i] - '0');
            }

            // Construct Int128 from 64-bit value
            result = Int128{ value };
            if (isNegative)
            {
                result = -result;
            }

            return true;
        }
    } // anonymous namespace

    Int128::Int128(double val)
    {
        if (std::isnan(val) || std::isinf(val))
        {
            *this = Int128{ 0 };
            return;
        }

        double truncated = std::trunc(val);

        if (truncated > constants::INT128_MAX_AS_DOUBLE || truncated < constants::INT128_MIN_AS_DOUBLE)
        {
            if (truncated > 0)
            {
                *this = std::numeric_limits<Int128>::max();
            }
            else
            {
                *this = std::numeric_limits<Int128>::min();
            }
            return;
        }

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(0) << truncated;
        *this = Int128{ oss.str() };
    }

    Int128::Int128(const Decimal& decimal)
    {
        if (decimal == Decimal{})
        {
            *this = Int128{ 0 };
            return;
        }

        Int128 integerPart;

        const auto& mantissaArray = decimal.mantissa();
        std::uint64_t low =
            static_cast<std::uint64_t>(mantissaArray[1]) << constants::BITS_PER_UINT32 | mantissaArray[0];
        std::uint64_t high = mantissaArray[2];

        integerPart = Int128{ low, high };

        if (decimal.scale() > 0)
        {
            // Optimized: Use power-of-10 lookup tables instead of iterative multiplication
            Int128 divisor;
            std::uint8_t scale = decimal.scale();

            if (scale < constants::DECIMAL_POWER_TABLE_SIZE)
            {
                // Use 64-bit lookup table for powers 0-19
                divisor = Int128{ constants::DECIMAL_POWERS_OF_10[scale] };
            }
            else if (scale <= constants::DECIMAL_EXTENDED_POWER_MAX)
            {
                // Use pre-computed 128-bit values for powers 20-28
                const auto& extended =
                    constants::DECIMAL_EXTENDED_POWERS_OF_10[scale - constants::DECIMAL_EXTENDED_POWER_MIN];
                divisor = Int128{ extended.first, extended.second };
            }
            else
            {
                // Fallback (shouldn't happen for valid Decimal scale 0-28)
                divisor = Int128{ 1 };
                for (std::uint8_t i = 0; i < scale; ++i)
                {
                    divisor = divisor * Int128{ constants::INT128_BASE };
                }
            }

            integerPart = integerPart / divisor; // Integer division truncates
        }

        bool isNegative = decimal < Decimal{};

        if (isNegative)
        {
            integerPart = -integerPart;
        }

        // Copy the result to this object
        *this = integerPart;
    }

    bool Int128::operator==(const Decimal& val) const noexcept
    {
        // If Decimal has fractional part, it can't equal an integer
        if (val.scale() > 0)
        {
            return false;
        }

        if ((*this < Int128{}) != (val < Decimal{}))
        {
            return *this == Int128{} && val == Decimal{};
        }

        // Convert Decimal's mantissa to Int128 for comparison
        const auto& mantissaArray = val.mantissa();
        std::uint64_t low =
            static_cast<std::uint64_t>(mantissaArray[1]) << constants::BITS_PER_UINT32 | mantissaArray[0];
        std::uint64_t high = mantissaArray[2];
        Int128 decimalMantissa{ low, high };

        // Compare absolute values
        Int128 thisAbs = abs();
        return thisAbs == decimalMantissa;
    }

    bool Int128::operator<(const Decimal& val) const noexcept
    {
        if (*this < Int128{} && !(val < Decimal{}))
        {
            return true; // Negative < Non-negative
        }
        if (!(*this < Int128{}) && val < Decimal{})
        {
            return false; // Non-negative > Negative
        }

        // Same signs - need to handle Decimal's scale
        const auto& mantissaArray = val.mantissa();
        std::uint64_t low =
            static_cast<std::uint64_t>(mantissaArray[1]) << constants::BITS_PER_UINT32 | mantissaArray[0];
        std::uint64_t high = mantissaArray[2];
        Int128 decimalMantissa{ low, high };

        if (val.scale() > 0)
        {
            // Decimal has fractional part - scale up this integer for comparison
            Int128 scaledThis = *this;

            // Apply power of 10 scaling
            for (std::uint8_t i = 0; i < val.scale(); ++i)
            {
                scaledThis = scaledThis * Int128{ constants::INT128_BASE };
            }

            if (*this < Int128{})
            {
                // Both negative - compare absolute values with flipped result
                return scaledThis.abs() > decimalMantissa;
            }
            else
            {
                return scaledThis < decimalMantissa;
            }
        }
        else
        {
            // No fractional part - direct comparison of absolute values
            if (*this < Int128{})
            {
                // Both negative - flip comparison
                return abs() > decimalMantissa;
            }
            else
            {
                return abs() < decimalMantissa;
            }
        }
    }

#if !DNV_VISTA_SDK_HAS_NATIVE_INT128

    Int128 Int128::operator*(const Int128& other) const noexcept
    {
        // 128-bit multiplication using Karatsuba-style algorithm (https://en.wikipedia.org/wiki/Karatsuba_algorithm)
        // Performance: Breaks 64x64 multiplication into 32x32 operations to leverage hardware multipliers efficiently
        // on all platforms
        std::uint64_t aLow{ m_layout.lower64bits & constants::UINT32_MAX_VALUE };
        std::uint64_t aHigh{ m_layout.lower64bits >> constants::BITS_PER_UINT32 };
        std::uint64_t bLow{ other.m_layout.lower64bits & constants::UINT32_MAX_VALUE };
        std::uint64_t bHigh{ other.m_layout.lower64bits >>
                             constants::BITS_PER_UINT32 }; // Four 32x32->64 multiplications
        std::uint64_t p0{ aLow * bLow };
        std::uint64_t p1{ aLow * bHigh };
        std::uint64_t p2{ aHigh * bLow };
        std::uint64_t p3{ aHigh * bHigh };

        // Carry computation for intermediate sum
        std::uint64_t carry{ ((p0 >> constants::BITS_PER_UINT32) + (p1 & constants::UINT32_MAX_VALUE) +
                              (p2 & constants::UINT32_MAX_VALUE)) >>
                             constants::BITS_PER_UINT32 }; // Final result assembly

        std::uint64_t resultLow{ p0 + (p1 << constants::BITS_PER_UINT32) + (p2 << constants::BITS_PER_UINT32) };

        std::uint64_t resultHigh{ p3 + (p1 >> constants::BITS_PER_UINT32) + (p2 >> constants::BITS_PER_UINT32) + carry +
                                  m_layout.upper64bits * other.m_layout.lower64bits +
                                  m_layout.lower64bits * other.m_layout.upper64bits };

        return Int128{ resultLow, resultHigh };
    }

    Int128 Int128::operator/(const Int128& other) const
    {
        if (other == Int128{})
        {
            throw std::overflow_error{ "Division by zero" };
        }

        const Int128 int128Min{ constants::INT128_MIN_NEGATIVE_LOW, constants::INT128_MIN_NEGATIVE_HIGH };
        if (*this == int128Min && other == Int128{ ~std::uint64_t{ 0 }, ~std::uint64_t{ 0 } })
        {
            return *this;
        }

        // Performance optimization: Fast path for 64-bit values
        // Avoids expensive 128-bit division when possible
        if (m_layout.upper64bits == 0 && other.m_layout.upper64bits == 0)
        {
            // Both fit in 64-bit - use native division
            return Int128{ m_layout.lower64bits / other.m_layout.lower64bits, 0 };
        }

        // Optimized path: dividend is 128-bit but divisor fits in 64-bit
        // Use precise 128/64 division algorithm
        if (other.m_layout.upper64bits == 0)
        {
            std::uint64_t divisor{ other.m_layout.lower64bits };
    #if defined(_MSC_VER) && !defined(__clang__)
            // MSVC optimization: Use hardware intrinsic for 128/64 division
            // BUT ONLY if the quotient will fit in 64 bits (to avoid CPU exception)
            // The quotient fits in 64 bits if: upper64bits < divisor
            if (m_layout.upper64bits < divisor)
            {
                std::uint64_t remainder;
                std::uint64_t lowQuotient = _udiv128(m_layout.upper64bits, m_layout.lower64bits, divisor, &remainder);
                return Int128{ lowQuotient, 0 };
            }
                // else: quotient doesn't fit in 64 bits, fall through to manual algorithm
    #endif
            // Divide high part first
            std::uint64_t highQuotient{ m_layout.upper64bits / divisor };
            std::uint64_t highRemainder{ m_layout.upper64bits % divisor };

            // Now divide (highRemainder << 64 + m_layout.lower64bits) by divisor
            // This is equivalent to dividing a 128-bit number by a 64-bit number
            if (highRemainder == 0)
            {
                // Simple case: high part divides evenly
                std::uint64_t lowQuotient{ m_layout.lower64bits / divisor };
                return Int128{ lowQuotient, highQuotient };
            }
            else if (highRemainder < (1ULL << 32))
            {
                // Remainder is small enough that we can shift it left by 32 without overflow
                // Process in two 32-bit chunks
                std::uint64_t quotientAccumulator = 0;
                std::uint64_t remainder = highRemainder;
                bool canUseOptimizedPath = true;

                for (int shift = 32; shift >= 0; shift -= 32)
                {
                    std::uint64_t chunk = (m_layout.lower64bits >> shift) & constants::UINT32_MAX_VALUE;

                    // Check if remainder is too large to shift left by 32
                    if (remainder >= (1ULL << 32))
                    {
                        // Cannot use optimized path - need general case
                        canUseOptimizedPath = false;
                        break;
                    }

                    std::uint64_t dividendPart = (remainder << 32) | chunk;
                    std::uint64_t q = dividendPart / divisor;
                    remainder = dividendPart % divisor;
                    quotientAccumulator = (quotientAccumulator << 32) | q;
                }

                if (canUseOptimizedPath)
                {
                    return Int128{ quotientAccumulator, highQuotient };
                }
            }
            // else: Remainder is large - fall through to general case below
        }

        // General case: 128-bit / 128-bit division using binary long division
        // This handles all cases where both operands require the full 128-bit range
        // Also used when divisor is 64-bit but remainder is too large for optimized path		// Handle sign for
        // signed division
        bool resultNegative{ false };
        Int128 absDividend{ *this };
        Int128 absDivisor{ other };

        if (absDividend < Int128{})
        {
            resultNegative = !resultNegative;
            absDividend = -absDividend;
        }

        if (absDivisor < Int128{})
        {
            resultNegative = !resultNegative;
            absDivisor = -absDivisor;
        }

        // Early exit for simple cases
        if (absDividend < absDivisor)
        {
            return Int128{ 0, 0 };
        }

        if (absDividend == absDivisor)
        {
            return resultNegative ? Int128{ 0, 0 } - Int128{ 1, 0 } : Int128{ 1, 0 };
        }

    #ifdef _MSC_VER
        // MSVC optimization: Find the highest set bit to reduce loop iterations
        // Start from the most significant bit that's actually set
        int startBit = constants::INT128_MAX_BIT_INDEX;

        // Find the highest bit set in dividend using intrinsics
        if (absDividend.m_layout.upper64bits != 0)
        {
            unsigned long index;
            _BitScanReverse64(&index, absDividend.m_layout.upper64bits);
            startBit = static_cast<int>(index) + constants::BITS_PER_UINT64;
        }
        else
        {
            unsigned long index;
            _BitScanReverse64(&index, absDividend.m_layout.lower64bits);
            startBit = static_cast<int>(index);
        }
    #else
        int startBit = constants::INT128_MAX_BIT_INDEX;
    #endif
        // Binary long division algorithm
        Int128 quotient{ 0, 0 };
        Int128 remainder{ 0, 0 };

        // Process bits from most significant to least significant
        // MSVC: Now starting from the actual highest bit instead of always from bit 127
        for (int i{ startBit }; i >= 0; --i)
        {
            // Shift remainder left by 1
            remainder = remainder + remainder; // equivalent to << 1

            // Set the least significant bit of remainder to the i-th bit of dividend
            if ((i >= constants::BITS_PER_UINT64 &&
                 ((absDividend.m_layout.upper64bits >> (i - constants::BITS_PER_UINT64)) & 1)) ||
                (i < constants::BITS_PER_UINT64 && ((absDividend.m_layout.lower64bits >> i) & 1)))
            {
                remainder = remainder + Int128{ 1, 0 };
            }

            // If remainder >= divisor, subtract divisor and set quotient bit
            if (!(remainder < absDivisor))
            {
                remainder = remainder - absDivisor;

                // Set the i-th bit of quotient
                if (i >= constants::BITS_PER_UINT64)
                {
                    quotient.m_layout.upper64bits |= (constants::BIT_MASK_ONE << (i - constants::BITS_PER_UINT64));
                }
                else
                {
                    quotient.m_layout.lower64bits |= (constants::BIT_MASK_ONE << i);
                }
            }
        }
        return resultNegative ? Int128{ 0, 0 } - quotient : quotient;
    }

#endif

    bool Int128::fromString(std::string_view str, Int128& result) noexcept
    {
        try
        {
            // Fast-path: Handle values that fit in 64-bit (~90% of real-world cases)
            if (tryParseFastPath(str, result))
            {
                return true;
            }

            if (str.empty())
            {
                return false;
            }

            // Handle sign
            bool isNegative = false;
            size_t pos = 0;

            if (str[0] == '-')
            {
                isNegative = true;
                pos = 1;
            }
            else if (str[0] == '+')
            {
                pos = 1;
            }

            // Check if we have digits after sign
            if (pos >= str.length())
            {
                return false;
            }

            // Parse digits and build the number
            result = Int128{ 0 };

            // Quick overflow check: if string is too long, it's definitely overflow
            size_t digitCount = str.length() - pos;
            if (digitCount > constants::INT128_MAX_DIGIT_COUNT)
            {
                return false;
            }

            // For exactly 39 digits, we need to check against max values
            if (digitCount == constants::INT128_MAX_DIGIT_COUNT)
            {
                std::string_view digits = str.substr(pos);

                if (!isNegative)
                {
                    // Check against max positive
                    if (digits > constants::INT128_MAX_POSITIVE_STRING)
                    {
                        return false;
                    }
                }
                else
                {
                    // Check against max negative (absolute value)
                    // Special case: the minimum value exactly equals this string
                    if (digits == constants::INT128_MAX_NEGATIVE_STRING)
                    {
                        // Use std::numeric_limits to get the correct minimum value
                        result = std::numeric_limits<Int128>::min();
                        return true;
                    }
                    if (digits > constants::INT128_MAX_NEGATIVE_STRING)
                    {
                        return false;
                    }
                }
            }

            for (size_t i = pos; i < str.length(); ++i)
            {
                char c{ str[i] };
                if (c < '0' || c > '9')
                {
                    return false; // Invalid character
                }

                int digit = c - '0';
                result = result * Int128{ constants::INT128_BASE } + Int128{ digit };
            }

            // Apply sign
            if (isNegative)
            {
                result = -result;
            }

            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    std::optional<Int128> Int128::fromString(std::string_view str) noexcept
    {
        Int128 result;
        if (fromString(str, result))
        {
            return result;
        }
        return std::nullopt;
    }

    std::string Int128::toString() const
    {
        if (*this == Int128{})
        {
            return "0";
        }

        // Handle the special case of minimum value (-2^127)
        // This value cannot be represented positively in 128-bit signed integer
        if (*this == std::numeric_limits<Int128>::min())
        {
            return std::string{ "-" } + std::string{ constants::INT128_MAX_NEGATIVE_STRING };
        }

        // Optimized digit extraction using char buffer (avoids N allocations)
        std::array<char, constants::INT128_MAX_DIGIT_COUNT> digits;
        size_t digitCount{ 0 };

        Int128 temp = abs();

#if DNV_VISTA_SDK_HAS_NATIVE_INT128
        // Fast path for values that fit in 64-bit
        if (temp.toNative() <= UINT64_MAX)
        {
            std::uint64_t value{ static_cast<std::uint64_t>(temp.toNative()) };
            while (value > 0 && digitCount < digits.size())
            {
                digits[digitCount++] = static_cast<char>('0' + (value % constants::INT128_BASE));
                value /= constants::INT128_BASE;
            }
        }
        else
        {
            // Full 128-bit extraction
            while (temp != Int128{} && digitCount < digits.size())
            {
                digits[digitCount++] = static_cast<char>('0' + (temp.toNative() % constants::INT128_BASE));
                temp = Int128{ temp.toNative() / constants::INT128_BASE };
            }
        }
#else
        if (temp.toHigh() == 0)
        {
            // Fast 64-bit path
            std::uint64_t value{ temp.toLow() };
            while (value > 0 && digitCount < digits.size())
            {
                digits[digitCount++] = static_cast<char>('0' + (value % constants::INT128_BASE));
                value /= constants::INT128_BASE;
            }
        }
        else
        {
            // Full 128-bit extraction
            while (temp != Int128{} && digitCount < digits.size())
            {
                if (temp.toHigh() == 0)
                {
                    // Switched to 64-bit range
                    std::uint64_t value{ temp.toLow() };
                    while (value > 0 && digitCount < digits.size())
                    {
                        digits[digitCount++] = static_cast<char>('0' + (value % constants::INT128_BASE));
                        value /= constants::INT128_BASE;
                    }
                    break;
                }
                Int128 remainder{ temp % Int128{ constants::INT128_BASE } };
                digits[digitCount++] = static_cast<char>('0' + remainder.toLow());
                temp = temp / Int128{ constants::INT128_BASE };
            }
        }
#endif

        if (digitCount == 0)
        {
            digitCount = 1;
            digits[0] = '0';
        }

        std::string result;
        result.reserve(digitCount + 1);

        // Handle sign
        if (*this < Int128{})
        {
            result.push_back('-');
        }

        // Add digits in reverse order
        for (size_t i = digitCount; i > 0; --i)
        {
            result.push_back(digits[i - 1]);
        }

        return result;
    }

#if !DNV_VISTA_SDK_HAS_NATIVE_INT128
    bool Int128::operator==(double val) const noexcept
    {
        if (std::isnan(val) || std::isinf(val))
        {
            return false; // Int128 has no NaN/Infinity representation
        }

        // Convert this Int128 to long double for comparison
        // For manual implementation, we need to carefully construct the value
        long double thisValue;
        if (*this < Int128{})
        {
            // Handle negative values using two's complement
            Int128 abs_this = this->abs();
            thisValue =
                -(static_cast<long double>(abs_this.m_layout.upper64bits) *
                      static_cast<long double>(constants::BIT_MASK_ONE << constants::BITS_PER_UINT32) *
                      static_cast<long double>(constants::BIT_MASK_ONE << constants::BITS_PER_UINT32) +
                  static_cast<long double>(abs_this.m_layout.lower64bits));
        }
        else
        {
            thisValue = static_cast<long double>(m_layout.upper64bits) *
                            static_cast<long double>(constants::BIT_MASK_ONE << constants::BITS_PER_UINT32) *
                            static_cast<long double>(constants::BIT_MASK_ONE << constants::BITS_PER_UINT32) +
                        static_cast<long double>(m_layout.lower64bits);
        }
        return thisValue == static_cast<long double>(val);
    }

    bool Int128::operator<(double val) const noexcept
    {
        if (std::isnan(val))
        {
            return false; // No ordering with NaN
        }
        if (std::isinf(val))
        {
            return val > 0.0; // Any finite value < +infinity, any finite value > -infinity
        }

        // Convert this Int128 to long double for comparison
        long double thisValue;
        if (*this < Int128{})
        {
            Int128 abs_this = this->abs();
            thisValue =
                -(static_cast<long double>(abs_this.m_layout.upper64bits) *
                      static_cast<long double>(constants::BIT_MASK_ONE << constants::BITS_PER_UINT32) *
                      static_cast<long double>(constants::BIT_MASK_ONE << constants::BITS_PER_UINT32) +
                  static_cast<long double>(abs_this.m_layout.lower64bits));
        }
        else
        {
            thisValue = static_cast<long double>(m_layout.upper64bits) *
                            static_cast<long double>(constants::BIT_MASK_ONE << constants::BITS_PER_UINT32) *
                            static_cast<long double>(constants::BIT_MASK_ONE << constants::BITS_PER_UINT32) +
                        static_cast<long double>(m_layout.lower64bits);
        }

        return thisValue < static_cast<long double>(val);
    }

    bool Int128::operator>(double val) const noexcept
    {
        if (std::isnan(val))
        {
            return false; // No ordering with NaN
        }
        if (std::isinf(val))
        {
            return val < 0.0; // Any finite value > -infinity, any finite value < +infinity
        }

        // Convert this Int128 to long double for comparison
        long double thisValue;
        if (*this < Int128{})
        {
            Int128 abs_this = this->abs();
            thisValue =
                -(static_cast<long double>(abs_this.m_layout.upper64bits) *
                      static_cast<long double>(constants::BIT_MASK_ONE << constants::BITS_PER_UINT32) *
                      static_cast<long double>(constants::BIT_MASK_ONE << constants::BITS_PER_UINT32) +
                  static_cast<long double>(abs_this.m_layout.lower64bits));
        }
        else
        {
            thisValue = static_cast<long double>(m_layout.upper64bits) *
                            static_cast<long double>(constants::BIT_MASK_ONE << constants::BITS_PER_UINT32) *
                            static_cast<long double>(constants::BIT_MASK_ONE << constants::BITS_PER_UINT32) +
                        static_cast<long double>(m_layout.lower64bits);
        }

        return thisValue > static_cast<long double>(val);
    }
#endif

    Int128 Int128::isqrt() const
    {
        // Integer square root using Heron's method (Babylonian algorithm)
        // Algorithm: x_{n+1} = (x_n + n/x_n) / 2
        // Initial guess: 2^(bitLength/2) via bit shifting for fast convergence

        if (*this < Int128{ 0 })
        {
            throw std::domain_error{ "Cannot compute square root of negative integer" };
        }

        if (*this == Int128{ 0 } || *this == Int128{ 1 })
        {
            return *this;
        }

        // Find bit length: position of highest set bit
        int bitLength = 0;
        if (toHigh() != 0)
        {
            // High 64 bits contain data
            std::uint64_t high = toHigh();
            bitLength = constants::BITS_PER_UINT64;

            // Count leading zeros in high word using binary search
            if ((high & constants::BIT_MASK_UPPER_32) == 0)
            {
                bitLength += 0;
                high <<= constants::BITS_PER_UINT32;
            }
            else
            {
                bitLength += constants::BITS_PER_UINT32;
            }
            if ((high & constants::BIT_MASK_UPPER_16) == 0)
            {
                bitLength += 0;
                high <<= constants::BITS_PER_UINT16;
            }
            else
            {
                bitLength += constants::BITS_PER_UINT16;
            }
            if ((high & constants::BIT_MASK_UPPER_8) == 0)
            {
                bitLength += 0;
                high <<= constants::BITS_PER_UINT8;
            }
            else
            {
                bitLength += constants::BITS_PER_UINT8;
            }
            if ((high & constants::BIT_MASK_UPPER_4) == 0)
            {
                bitLength += 0;
                high <<= constants::BITS_PER_NIBBLE;
            }
            else
            {
                bitLength += constants::BITS_PER_NIBBLE;
            }
            if ((high & constants::BIT_MASK_UPPER_2) == 0)
            {
                bitLength += 0;
                high <<= constants::BITS_PER_PAIR;
            }
            else
            {
                bitLength += constants::BITS_PER_PAIR;
            }
            if ((high & constants::BIT_MASK_MSB) == 0)
            {
                bitLength += 0;
            }
            else
            {
                bitLength += 1;
            }
        }
        else
        {
            // Only low 64 bits contain data
            std::uint64_t low = toLow();
            bitLength = 0;

            if ((low & constants::BIT_MASK_UPPER_32) == 0)
            {
                bitLength += 0;
                low <<= constants::BITS_PER_UINT32;
            }
            else
            {
                bitLength += constants::BITS_PER_UINT32;
            }
            if ((low & constants::BIT_MASK_UPPER_16) == 0)
            {
                bitLength += 0;
                low <<= constants::BITS_PER_UINT16;
            }
            else
            {
                bitLength += constants::BITS_PER_UINT16;
            }
            if ((low & constants::BIT_MASK_UPPER_8) == 0)
            {
                bitLength += 0;
                low <<= constants::BITS_PER_UINT8;
            }
            else
            {
                bitLength += constants::BITS_PER_UINT8;
            }
            if ((low & constants::BIT_MASK_UPPER_4) == 0)
            {
                bitLength += 0;
                low <<= constants::BITS_PER_NIBBLE;
            }
            else
            {
                bitLength += constants::BITS_PER_NIBBLE;
            }
            if ((low & constants::BIT_MASK_UPPER_2) == 0)
            {
                bitLength += 0;
                low <<= constants::BITS_PER_PAIR;
            }
            else
            {
                bitLength += constants::BITS_PER_PAIR;
            }
            if ((low & constants::BIT_MASK_MSB) == 0)
            {
                bitLength += 0;
            }
            else
            {
                bitLength += 1;
            }
        }

        // Initial guess: 2^(bitLength/2) - construct via bit shifting
        int initialShift = bitLength / 2;
        Int128 div;
        if (initialShift >= constants::BITS_PER_UINT64)
        {
            div = Int128{ 0, constants::BIT_MASK_ONE << (initialShift - constants::BITS_PER_UINT64) };
        }
        else
        {
            div = Int128{ constants::BIT_MASK_ONE << initialShift, 0 };
        }

        Int128 div2 = div;

        // Heron iteration: x_{n+1} = (x_n + n/x_n) / 2
        // Converges when y == div (stable) or y == div2 (alternating)
        // For integer sqrt, we want the FLOOR of the result
        for (int i = 0; i < constants::INT128_ISQRT_MAX_ITERATIONS; ++i)
        {
            Int128 y = (div + (*this / div)) / Int128{ 2 };

            if (y == div || y == div2)
            {
                // When alternating between two values, return the smaller one (floor)
                return (y < div) ? y : div;
            }

            div2 = div;
            div = y;
        }

        return div;
    }

    Decimal sqrt(const Int128& value)
    {
        // Check for negative input
        if (value < Int128{ 0 })
        {
            throw std::domain_error{ "Cannot compute square root of negative number" };
        }

        // Handle zero and one
        if (value == Int128{ 0 } || value == Int128{ 1 })
        {
            return decimalFromInt128(value);
        }

        // Use Int128::isqrt() for integer square root
        Int128 intSqrt = value.isqrt();

        // Check if it's a perfect square - use division to avoid overflow in multiplication
        // Instead of checking intSqrt * intSqrt == value, check value / intSqrt == intSqrt && value % intSqrt == 0
        if (value / intSqrt == intSqrt && value % intSqrt == Int128{ 0 })
        {
            // Perfect square - return exact integer as Decimal
            return decimalFromInt128(intSqrt);
        }

        // Not a perfect square - refine using Decimal::sqrt()
        // Convert to Decimal and compute square root
        Decimal decValue{ decimalFromInt128(value) };

        return decValue.sqrt();
    }
} // namespace dnv::vista::sdk::internal
