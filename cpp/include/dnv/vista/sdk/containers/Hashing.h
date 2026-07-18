/**
 * @file Hashing.h
 * @brief CRC32-C string hashing and seed mixing utilities for hash containers
 */

#pragma once

#if (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
    #include <cpuid.h>
#endif

#if defined(_MSC_VER)
    #include <intrin.h>
    #include <nmmintrin.h>
#endif

#include <cstdint>
#include <string>
#include <string_view>

namespace dnv::vista::sdk::internal
{

    inline constexpr std::uint32_t FNV1A_OFFSET_BASIS32 = 0x811C9DC5u;
    inline constexpr std::uint64_t SEED_MIX_MULTIPLIER = 0x2545F4914F6CDD1DuLL; ///< Thomas Wang

    inline constexpr std::uint32_t crc32cSoft(std::uint32_t hash, std::uint8_t ch) noexcept
    {
        constexpr std::uint32_t polynomial = 0x82F63B78u;
        std::uint32_t crc = hash ^ ch;
        for (int i = 0; i < 8; ++i)
        {
            crc = (crc >> 1) ^ ((crc & 1u) ? polynomial : 0u);
        }
        return crc;
    }

    inline std::uint32_t crc32cStep(std::uint32_t hash, std::uint8_t ch) noexcept
    {
#if defined(__SSE4_2__) || (defined(_MSC_VER) && defined(__AVX__))
    #if defined(__GNUC__) || defined(__clang__)
        return __builtin_ia32_crc32qi(hash, ch);
    #elif defined(_MSC_VER)
        return _mm_crc32_u8(hash, ch);
    #endif
#else
        return crc32cSoft(hash, ch);
#endif
    }

    inline std::uint32_t hashString(std::string_view key) noexcept
    {
        if (key.empty())
        {
            return 0u;
        }

        std::uint32_t h = FNV1A_OFFSET_BASIS32;
        for (const char c : key)
        {
            h = crc32cStep(h, static_cast<std::uint8_t>(c));
        }
        return h;
    }

    inline constexpr std::uint32_t seedMix(std::uint32_t seed, std::uint32_t hash, std::uint64_t size) noexcept
    {
        // xorshift mix (Marsaglia) to disperse bits before Fibonacci hashing reduces to [0, size)
        std::uint32_t x = seed + hash;
        x ^= x >> 12;
        x ^= x << 25;
        x ^= x >> 27;

        return static_cast<std::uint32_t>((static_cast<std::uint64_t>(x) * SEED_MIX_MULTIPLIER) & (size - 1));
    }
} // namespace dnv::vista::sdk::internal
