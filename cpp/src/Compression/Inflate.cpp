/**
 * @file Inflate.cpp
 * @brief DEFLATE decompressor (RFC 1951)
 * @details Implements all three block types:
 *   - Type 00: uncompressed
 *   - Type 01: fixed Huffman codes
 *   - Type 10: dynamic Huffman codes
 */

#include "Inflate.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>
#include <vector>

namespace dnv::vista::sdk::compression
{
    namespace
    {
        //=====================================================================
        // BitReader
        //=====================================================================

        class BitReader
        {
        public:
            explicit BitReader(std::span<const uint8_t> data) noexcept
                : m_data{ data }
            {}

            /** @brief Read n bits (LSB-first per RFC 1951 sect. 3.1.1) */
            uint32_t readBits(int n)
            {
                fill(n);
                const uint32_t result = static_cast<uint32_t>(m_buf & ((1u << n) - 1u));
                m_buf >>= n;
                m_available -= n;
                return result;
            }

            /**
             * @brief Peek at the next @p n bits without consuming them.
             * @details Fills as many bytes as available. Remaining high bits are zero.
             */
            uint32_t peekBits(int n)
            {
                fillSoft(n);
                return static_cast<uint32_t>(m_buf & ((1u << n) - 1u));
            }

            /** @brief Consume n bits already peeked */
            void consumeBits(int n) noexcept
            {
                m_buf >>= n;
                m_available -= n;
            }

            /** @brief Number of bits currently buffered (see peekBits()/fillSoft()) */
            [[nodiscard]] int available() const noexcept { return m_available; }

            /** @brief Skip to the next byte boundary */
            void alignToByte() noexcept
            {
                const int rem = m_available & 7;
                if (rem)
                {
                    m_buf >>= rem;
                    m_available -= rem;
                }
            }

            /** @brief Read a little-endian 16-bit value aligned to byte boundary */
            uint16_t readU16Le()
            {
                alignToByte();
                const uint8_t lo = static_cast<uint8_t>(readBits(8));
                const uint8_t hi = static_cast<uint8_t>(readBits(8));
                return static_cast<uint16_t>(lo | (hi << 8));
            }

        private:
            void fill(int n)
            {
                while (m_available < n)
                {
                    if (m_pos >= m_data.size())
                    {
                        throw std::runtime_error{ "inflate: unexpected end of input" };
                    }
                    m_buf |= static_cast<uint64_t>(m_data[m_pos++]) << m_available;
                    m_available += 8;
                }
            }

            // Like fill() but stops at end of input instead of throwing.
            // Used by peekBits() so that the last Huffman code in a block
            // (end-of-block symbol 256, typically 7 bits) can be decoded
            // even when fewer than kFastBits bytes remain in the stream.
            void fillSoft(int n)
            {
                while (m_available < n && m_pos < m_data.size())
                {
                    m_buf |= static_cast<uint64_t>(m_data[m_pos++]) << m_available;
                    m_available += 8;
                }
            }

            std::span<const uint8_t> m_data;
            uint64_t m_buf{ 0 };
            int m_available{ 0 };
            size_t m_pos{ 0 };
        };

        //=====================================================================
        // Huffman tree (canonical codes, RFC 1951 sect. 3.2.2)
        //=====================================================================

        static constexpr int kMaxBits = 15;

        struct HuffmanTree
        {
            // Fast lookup table: maps kFastBits-bit prefix -> {symbol, code_len}
            // Fallback for longer codes via sorted table.
            static constexpr int kFastBits = 9;
            static constexpr int kFastSize = 1 << kFastBits;
            static constexpr uint16_t kInvalid = 0xFFFF;

            struct Entry
            {
                uint16_t symbol;
                uint8_t bits;
            };

            std::array<Entry, kFastSize> fast{};
            // Slow path: sorted (code, len, symbol) triples for codes > kFastBits
            struct Slow
            {
                uint32_t code;
                uint8_t bits;
                uint16_t symbol;
            };
            std::vector<Slow> slow;

            void build(const uint8_t* lengths, int count)
            {
                // Count codes per length (RFC 1951 sect. 3.2.2 step 1)
                std::array<int, kMaxBits + 1> blCount{};
                for (int i = 0; i < count; ++i)
                {
                    if (lengths[i])
                    {
                        ++blCount[lengths[i]];
                    }
                }

                // First code per length (step 2)
                std::array<uint32_t, kMaxBits + 1> nextCode{};
                uint32_t code = 0;
                for (int bits = 1; bits <= kMaxBits; ++bits)
                {
                    code = (code + blCount[bits - 1]) << 1;
                    nextCode[bits] = code;
                }

                // Assign codes (step 3), build lookup tables
                for (auto& e : fast)
                {
                    e = { kInvalid, 0 };
                }
                slow.clear();

                for (int sym = 0; sym < count; ++sym)
                {
                    const int len = lengths[sym];
                    if (len == 0)
                    {
                        continue;
                    }

                    const uint32_t c = nextCode[len]++;

                    if (len <= kFastBits)
                    {
                        // Fill all kFastSize entries whose low `len` bits match c
                        const uint32_t reversed = reverseBits(c, len);
                        const int step = 1 << len;
                        for (int idx = static_cast<int>(reversed); idx < kFastSize; idx += step)
                        {
                            fast[idx] = { static_cast<uint16_t>(sym), static_cast<uint8_t>(len) };
                        }
                    }
                    else
                    {
                        slow.push_back({ reverseBits(c, len), static_cast<uint8_t>(len), static_cast<uint16_t>(sym) });
                    }
                }

                // Sort by (bits, code) so the slow-path linear scan can break early
                std::sort(slow.begin(), slow.end(), [](const Slow& a, const Slow& b) {
                    return a.bits < b.bits || (a.bits == b.bits && a.code < b.code);
                });
            }

            /** @brief Decode one Huffman symbol, consuming exactly the right number of bits */
            uint16_t decode2(BitReader& br) const
            {
                // Peek kFastBits bits from the stream (LSB-first = bit-reversed canonical).
                const uint32_t peek = br.peekBits(kFastBits);
                const auto& e = fast[peek];
                if (e.bits != 0 && br.available() >= e.bits)
                {
                    br.consumeBits(e.bits);
                    return e.symbol;
                }

                // Slow path: codes longer than kFastBits bits.
                if (br.available() < kFastBits)
                {
                    throw std::runtime_error{ "inflate: invalid Huffman code" };
                }

                // Read one bit at a time, accumulating the LSB-first stream value,
                // and compare against the reversed canonical codes stored in slow[].
                uint32_t lsb = peek;
                br.consumeBits(kFastBits);
                for (int len = kFastBits + 1; len <= kMaxBits; ++len)
                {
                    lsb |= br.readBits(1) << (len - 1);
                    for (const auto& s : slow)
                    {
                        if (s.bits == static_cast<uint8_t>(len) && s.code == lsb)
                        {
                            return s.symbol;
                        }
                        if (s.bits > static_cast<uint8_t>(len))
                        {
                            break;
                        }
                    }
                }
                throw std::runtime_error{ "inflate: invalid Huffman code" };
            }

        private:
            static uint32_t reverseBits(uint32_t v, int n) noexcept
            {
                uint32_t r = 0;
                for (int i = 0; i < n; ++i)
                {
                    r = (r << 1) | (v & 1u);
                    v >>= 1;
                }
                return r;
            }
        };

        //=====================================================================
        // Fixed Huffman trees (RFC 1951 sect. 3.2.6)
        //=====================================================================

        HuffmanTree buildFixedLitLen()
        {
            std::array<uint8_t, 288> lengths{};
            for (int i = 0; i <= 143; ++i)
            {
                lengths[i] = 8;
            }
            for (int i = 144; i <= 255; ++i)
            {
                lengths[i] = 9;
            }
            for (int i = 256; i <= 279; ++i)
            {
                lengths[i] = 7;
            }
            for (int i = 280; i <= 287; ++i)
            {
                lengths[i] = 8;
            }
            HuffmanTree t;
            t.build(lengths.data(), 288);
            return t;
        }

        HuffmanTree buildFixedDist()
        {
            std::array<uint8_t, 32> lengths{};
            for (int i = 0; i < 32; ++i)
            {
                lengths[i] = 5;
            }
            HuffmanTree t;
            t.build(lengths.data(), 32);
            return t;
        }

        //=====================================================================
        // Length / distance decode tables (RFC 1951 sect. 3.2.5)
        //=====================================================================

        // Length: base values and extra bits for codes 257-285
        static constexpr std::array<uint16_t, 29> kLengthBase = { 3,  4,  5,  6,   7,   8,   9,   10,  11, 13,
                                                                  15, 17, 19, 23,  27,  31,  35,  43,  51, 59,
                                                                  67, 83, 99, 115, 131, 163, 195, 227, 258 };
        static constexpr std::array<uint8_t, 29> kLengthExtra = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2,
                                                                  2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0 };

        // Distance: base values and extra bits for codes 0-29
        static constexpr std::array<uint16_t, 30> kDistBase = { 1,    2,    3,    4,     5,     7,    9,    13,
                                                                17,   25,   33,   49,    65,    97,   129,  193,
                                                                257,  385,  513,  769,   1025,  1537, 2049, 3073,
                                                                4097, 6145, 8193, 12289, 16385, 24577 };
        static constexpr std::array<uint8_t, 30> kDistExtra = { 0, 0, 0, 0, 1, 1, 2, 2,  3,  3,  4,  4,  5,  5,  6,
                                                                6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13 };

        // Code-length alphabet order (RFC 1951 sect. 3.2.7)
        static constexpr std::array<uint8_t, 19> kClenOrder = { 16, 17, 18, 0, 8,  7, 9,  6, 10, 5,
                                                                11, 4,  12, 3, 13, 2, 14, 1, 15 };

        //=====================================================================
        // Decompress one block of literals/back-refs
        //=====================================================================

        void inflateBlock(BitReader& br, const HuffmanTree& litLen, const HuffmanTree& dist, std::vector<uint8_t>& out)
        {
            for (;;)
            {
                const uint16_t sym = litLen.decode2(br);

                if (sym < 256)
                {
                    out.push_back(static_cast<uint8_t>(sym));
                }
                else if (sym == 256)
                {
                    break; // end of block
                }
                else
                {
                    // Back-reference: decode length
                    const int lengthIdx = sym - 257;
                    if (lengthIdx >= 29)
                    {
                        throw std::runtime_error{ "inflate: invalid length symbol" };
                    }
                    const uint32_t length = kLengthBase[lengthIdx] + br.readBits(kLengthExtra[lengthIdx]);

                    // Decode distance
                    const uint16_t distSym = dist.decode2(br);
                    if (distSym >= 30)
                    {
                        throw std::runtime_error{ "inflate: invalid distance symbol" };
                    }
                    const uint32_t distance = kDistBase[distSym] + br.readBits(kDistExtra[distSym]);

                    if (distance > out.size())
                    {
                        throw std::runtime_error{ "inflate: distance exceeds output" };
                    }

                    // Copy byte-by-byte to handle overlapping references
                    const size_t start = out.size() - distance;
                    for (uint32_t i = 0; i < length; ++i)
                    {
                        out.push_back(out[start + i]);
                    }
                }
            }
        }

        //=====================================================================
        // Dynamic Huffman code tables (RFC 1951 sect. 3.2.7)
        //=====================================================================

        void readDynamicTrees(BitReader& br, HuffmanTree& litLen, HuffmanTree& dist)
        {
            const int hlit = static_cast<int>(br.readBits(5)) + 257;
            const int hdist = static_cast<int>(br.readBits(5)) + 1;
            const int hclen = static_cast<int>(br.readBits(4)) + 4;

            // Read code-length alphabet lengths
            std::array<uint8_t, 19> clenLengths{};
            for (int i = 0; i < hclen; ++i)
            {
                clenLengths[kClenOrder[i]] = static_cast<uint8_t>(br.readBits(3));
            }

            HuffmanTree clenTree;
            clenTree.build(clenLengths.data(), 19);

            // Decode literal/length + distance code lengths
            const int total = hlit + hdist;
            std::vector<uint8_t> lengths(static_cast<size_t>(total), 0);

            for (int i = 0; i < total;)
            {
                const uint16_t sym = clenTree.decode2(br);

                if (sym <= 15)
                {
                    lengths[i++] = static_cast<uint8_t>(sym);
                }
                else if (sym == 16)
                {
                    if (i == 0)
                    {
                        throw std::runtime_error{ "inflate: repeat with no previous length" };
                    }
                    const int rep = static_cast<int>(br.readBits(2)) + 3;
                    const uint8_t prev = lengths[i - 1];
                    for (int k = 0; k < rep && i < total; ++k, ++i)
                    {
                        lengths[i] = prev;
                    }
                }
                else if (sym == 17)
                {
                    const int rep = static_cast<int>(br.readBits(3)) + 3;
                    for (int k = 0; k < rep && i < total; ++k, ++i)
                    {
                        lengths[i] = 0;
                    }
                }
                else if (sym == 18)
                {
                    const int rep = static_cast<int>(br.readBits(7)) + 11;
                    for (int k = 0; k < rep && i < total; ++k, ++i)
                    {
                        lengths[i] = 0;
                    }
                }
                else
                {
                    throw std::runtime_error{ "inflate: invalid code-length symbol" };
                }
            }

            litLen.build(lengths.data(), hlit);
            dist.build(lengths.data() + hlit, hdist);
        }
    } // namespace

    //=========================================================================
    // Public API
    //=========================================================================

    std::vector<uint8_t> inflate(std::span<const uint8_t> input, size_t sizeHint)
    {
        BitReader br{ input };
        std::vector<uint8_t> out;
        if (sizeHint)
        {
            out.reserve(sizeHint);
        }

        // Lazily built fixed trees (shared across blocks)
        static const HuffmanTree kFixedLitLen = buildFixedLitLen();
        static const HuffmanTree kFixedDist = buildFixedDist();

        for (;;)
        {
            const uint32_t bfinal = br.readBits(1);
            const uint32_t btype = br.readBits(2);

            if (btype == 0b00)
            {
                // Uncompressed block (RFC 1951 sect. 3.2.4)
                br.alignToByte();
                const uint16_t len = br.readU16Le();
                const uint16_t nlen = br.readU16Le();
                if ((len ^ nlen) != 0xFFFFu)
                {
                    throw std::runtime_error{ "inflate: bad uncompressed block length" };
                }
                out.reserve(out.size() + len);
                for (uint16_t i = 0; i < len; ++i)
                {
                    out.push_back(static_cast<uint8_t>(br.readBits(8)));
                }
            }
            else if (btype == 0b01)
            {
                // Fixed Huffman codes (RFC 1951 sect. 3.2.6)
                inflateBlock(br, kFixedLitLen, kFixedDist, out);
            }
            else if (btype == 0b10)
            {
                // Dynamic Huffman codes (RFC 1951 sect. 3.2.7)
                HuffmanTree litLen, dist;
                readDynamicTrees(br, litLen, dist);
                inflateBlock(br, litLen, dist, out);
            }
            else
            {
                throw std::runtime_error{ "inflate: reserved block type 11" };
            }

            if (bfinal)
            {
                break;
            }
        }

        return out;
    }
} // namespace dnv::vista::sdk::compression
