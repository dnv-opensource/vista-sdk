#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace dnv::vista::sdk::internal::constants
{

    //----------------------------------------------
    // Bit masks for leading zero count (binary search)
    //----------------------------------------------

    /** @brief Mask for testing upper 32 bits of 64-bit value */
    inline constexpr std::uint64_t BIT_MASK_UPPER_32{ 0xFFFFFFFF00000000ULL };

    /** @brief Mask for testing upper 16 bits of upper 32 bits */
    inline constexpr std::uint64_t BIT_MASK_UPPER_16{ 0xFFFF000000000000ULL };

    /** @brief Mask for testing upper 8 bits of upper 16 bits */
    inline constexpr std::uint64_t BIT_MASK_UPPER_8{ 0xFF00000000000000ULL };

    /** @brief Mask for testing upper 4 bits of upper 8 bits */
    inline constexpr std::uint64_t BIT_MASK_UPPER_4{ 0xF000000000000000ULL };

    /** @brief Mask for testing upper 2 bits of upper 4 bits */
    inline constexpr std::uint64_t BIT_MASK_UPPER_2{ 0xC000000000000000ULL };

    /** @brief Mask for testing most significant bit */
    inline constexpr std::uint64_t BIT_MASK_MSB{ 0x8000000000000000ULL };

    //=====================================================================
    // Int128 data type constants
    //=====================================================================

    //----------------------------------------------
    // Numeric base
    //----------------------------------------------

    /** @brief Base for decimal digit conversion */
    inline constexpr int INT128_BASE{ 10 };

    //----------------------------------------------
    // Bit indexing
    //----------------------------------------------

    /** @brief Maximum bit index for 128-bit integers (0-127) */
    inline constexpr int INT128_MAX_BIT_INDEX{ 127 };

    //----------------------------------------------
    // Overflow detection
    //----------------------------------------------

    /** @brief Maximum high 64-bit value before multiplying by 10 causes overflow (0xFFFFFFFFFFFFFFFF / 10) */
    inline constexpr std::uint64_t INT128_MUL10_OVERFLOW_THRESHOLD{ 0x1999999999999999ULL };

    /** @brief Double approximation of maximum positive Int128 value (2^127 - 1) for overflow checks */
    inline constexpr double INT128_MAX_AS_DOUBLE{ 1.7014118346046923e38 };

    /** @brief Double approximation of minimum negative Int128 value (-2^127) for overflow checks */
    inline constexpr double INT128_MIN_AS_DOUBLE{ -1.7014118346046924e38 };

    //----------------------------------------------
    // Arithmetic
    //----------------------------------------------

    /** @brief High 64 bits of maximum positive 128-bit signed integer (2^127 - 1) */
    inline constexpr std::uint64_t INT128_MAX_POSITIVE_HIGH{ 0x7FFFFFFFFFFFFFFFULL };

    /** @brief Low 64 bits of maximum positive 128-bit signed integer (2^127 - 1) */
    inline constexpr std::uint64_t INT128_MAX_POSITIVE_LOW{ 0xFFFFFFFFFFFFFFFFULL };

    /** @brief High 64 bits of minimum negative 128-bit signed integer (-2^127) */
    inline constexpr std::uint64_t INT128_MIN_NEGATIVE_HIGH{ 0x8000000000000000ULL };

    /** @brief Low 64 bits of minimum negative 128-bit signed integer (-2^127) */
    inline constexpr std::uint64_t INT128_MIN_NEGATIVE_LOW{ 0x0000000000000000ULL };

    //----------------------------------------------
    // String conversion
    //----------------------------------------------

    /** @brief Maximum number of decimal digits in a 128-bit signed integer (2^127 - 1 has 39 digits) */
    inline constexpr size_t INT128_MAX_DIGIT_COUNT{ 39UL };

    /** @brief Maximum iterations for Int128::isqrt() Heron's method */
    inline constexpr int INT128_ISQRT_MAX_ITERATIONS{ 100 };

    /** @brief Maximum iterations for Decimal::sqrt() Newton-Raphson refinement */
    inline constexpr int DECIMAL_SQRT_MAX_ITERATIONS{ 10 };

    /** @brief Maximum positive value as decimal string (2^127 - 1) */
    inline constexpr std::string_view INT128_MAX_POSITIVE_STRING{ "170141183460469231731687303715884105727" };

    /** @brief Maximum negative value as decimal string (absolute value of -2^127) */
    inline constexpr std::string_view INT128_MAX_NEGATIVE_STRING{ "170141183460469231731687303715884105728" };

    //----------------------------------------------
    // std::numeric_limits constants
    //----------------------------------------------

    /** @brief Number of value bits in Int128 (excluding sign bit) */
    inline constexpr int INT128_DIGITS{ 127 };

    /** @brief Number of decimal digits representable in Int128 (floor(127 * log10(2))) */
    inline constexpr int INT128_DIGITS10{ 38 };

    /** @brief Maximum decimal digits for Int128 (not applicable for integers) */
    inline constexpr int INT128_MAX_DIGITS10{ 0 };

    /** @brief Numeric base for Int128 (binary base-2 system) */
    inline constexpr int INT128_RADIX{ 2 };
} // namespace dnv::vista::sdk::internal::constants
