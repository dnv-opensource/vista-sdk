/**
 * @file Crc32.h
 * @brief CRC-32 computation per ISO 3309 / ITU-T V.42
 * @details Lookup-table implementation using polynomial 0xEDB88320 (reflected form).
 *          Used by the gzip decompressor to verify the trailer checksum (RFC 1952 sect. 2.3.1).
 */

#pragma once

#include <array>
#include <cstdint>
#include <span>

namespace dnv::vista::sdk::compression
{
    namespace detail
    {
        // CRC-32 IEEE 802.3 polynomial (0x04C11DB7) in reflected (LSB-first) form
        inline constexpr uint32_t CRC32_POLYNOMIAL = 0xEDB88320u;

        constexpr std::array<uint32_t, 256> buildCrc32Table() noexcept
        {
            std::array<uint32_t, 256> table{};

            for (uint32_t n = 0; n < 256; ++n)
            {
                uint32_t c = n;
                for (int k = 0; k < 8; ++k)
                {
                    if (c & 1u)
                    {
                        c = CRC32_POLYNOMIAL ^ (c >> 1);
                    }
                    else
                    {
                        c >>= 1;
                    }
                }
                table[n] = c;
            }

            return table;
        }

        inline constexpr auto CRC32_TABLE = buildCrc32Table();
    } // namespace detail

    /**
     * @brief Update a running CRC-32 with a span of bytes
     * @param crc  Current CRC value (pass 0xFFFFFFFF to start)
     * @param data Input bytes
     * @return Updated CRC (XOR with 0xFFFFFFFF to finalise)
     */
    [[nodiscard]] inline constexpr uint32_t crc32Update(uint32_t crc, std::span<const uint8_t> data) noexcept
    {
        for (const uint8_t b : data)
        {
            // reflected-bit table lookup: mix low byte of crc with input, shift out processed byte
            crc = detail::CRC32_TABLE[(crc ^ b) & 0xFFu] ^ (crc >> 8);
        }
        return crc;
    }

    /**
     * @brief Compute CRC-32 of a complete byte span
     * @param data Input bytes
     * @return CRC-32 value
     */
    [[nodiscard]] inline constexpr uint32_t crc32(std::span<const uint8_t> data) noexcept
    {
        // pre-condition and post-inversion are the standard CRC-32 framing (RFC 1952 sect. 2.3.1)
        return crc32Update(0xFFFFFFFFu, data) ^ 0xFFFFFFFFu;
    }
} // namespace dnv::vista::sdk::compression
