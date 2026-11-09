#include "Gzip.h"

#include "Crc32.h"
#include "Inflate.h"

#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

namespace dnv::vista::sdk::compression
{
    namespace
    {
        // Gzip header magic and constants (RFC 1952 sect. 2.3)
        static constexpr uint8_t kId1 = 0x1f;
        static constexpr uint8_t kId2 = 0x8b;
        static constexpr uint8_t kCmDeflate = 8;

        // FLG bits (RFC 1952 sect. 2.3.1)
        static constexpr uint8_t kFlagFEXTRA = 1u << 2;
        static constexpr uint8_t kFlagFNAME = 1u << 3;
        static constexpr uint8_t kFlagFCOMMENT = 1u << 4;
        static constexpr uint8_t kFlagFHCRC = 1u << 1;

        uint8_t readByte(std::span<const uint8_t> data, size_t& pos)
        {
            if (pos >= data.size())
            {
                throw std::runtime_error{ "gunzip: truncated header" };
            }
            return data[pos++];
        }

        uint16_t readU16Le(std::span<const uint8_t> data, size_t& pos)
        {
            const uint8_t lo = readByte(data, pos);
            const uint8_t hi = readByte(data, pos);
            return static_cast<uint16_t>(lo | (hi << 8));
        }

        uint32_t readU32Le(std::span<const uint8_t> data, size_t& pos)
        {
            uint32_t v = 0;
            for (int i = 0; i < 4; ++i)
            {
                v |= static_cast<uint32_t>(readByte(data, pos)) << (i * 8);
            }
            return v;
        }

        /** @brief Skip a null-terminated string (FNAME or FCOMMENT field) */
        void skipNullTerminated(std::span<const uint8_t> data, size_t& pos)
        {
            while (readByte(data, pos) != 0)
            {
            }
        }
    } // namespace

    std::vector<uint8_t> gunzip(std::span<const uint8_t> input)
    {
        size_t pos = 0;

        // Fixed 10-byte header (RFC 1952 sect. 2.3.1)
        if (input.size() < 18) // 10 header + 4 CRC32 + 4 ISIZE minimum
        {
            throw std::runtime_error{ "gunzip: input too short" };
        }

        const uint8_t id1 = readByte(input, pos);
        const uint8_t id2 = readByte(input, pos);
        if (id1 != kId1 || id2 != kId2)
        {
            throw std::runtime_error{ "gunzip: not a gzip stream (bad magic)" };
        }

        const uint8_t cm = readByte(input, pos);
        if (cm != kCmDeflate)
        {
            throw std::runtime_error{ "gunzip: unsupported compression method" };
        }

        const uint8_t flg = readByte(input, pos);
        pos += 4; // MTIME (4 bytes)
        pos += 1; // XFL   (1 byte)
        pos += 1; // OS    (1 byte)

        // Optional fields
        if (flg & kFlagFEXTRA)
        {
            const uint16_t xlen = readU16Le(input, pos);
            if (pos + xlen > input.size())
            {
                throw std::runtime_error{ "gunzip: FEXTRA field extends beyond input" };
            }
            pos += xlen;
        }
        if (flg & kFlagFNAME)
        {
            skipNullTerminated(input, pos);
        }
        if (flg & kFlagFCOMMENT)
        {
            skipNullTerminated(input, pos);
        }
        if (flg & kFlagFHCRC)
        {
            pos += 2; // CRC16 of header - we skip verification
        }

        if (pos >= input.size())
        {
            throw std::runtime_error{ "gunzip: no compressed data after header" };
        }

        // DEFLATE payload: everything except the 8-byte trailer
        if (input.size() < pos + 8)
        {
            throw std::runtime_error{ "gunzip: truncated - missing trailer" };
        }
        const size_t payloadSize = input.size() - pos - 8;
        const auto payload = input.subspan(pos, payloadSize);
        pos += payloadSize;

        // Size hint: ISIZE is at trailer offset +4, stored mod 2^32
        const uint32_t expectedCrc = readU32Le(input, pos);
        const uint32_t expectedSize = readU32Le(input, pos);

        // Decompress
        const size_t sizeHint = expectedSize ? static_cast<size_t>(expectedSize) : payloadSize * 4;
        auto out = inflate(payload, sizeHint);

        // Verify CRC-32 (RFC 1952 sect. 2.3.1)
        const uint32_t actualCrc = crc32({ out.data(), out.size() });
        if (actualCrc != expectedCrc)
        {
            throw std::runtime_error{ "gunzip: CRC-32 mismatch - data corrupt" };
        }

        // Verify ISIZE (mod 2^32)
        if (static_cast<uint32_t>(out.size()) != expectedSize)
        {
            throw std::runtime_error{ "gunzip: ISIZE mismatch" };
        }

        return out;
    }
} // namespace dnv::vista::sdk::compression
