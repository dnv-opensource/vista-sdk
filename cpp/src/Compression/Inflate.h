/**
 * @file Inflate.h
 * @brief DEFLATE decompressor (RFC 1951)
 */

#pragma once

#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

namespace dnv::vista::sdk::compression
{
    /**
     * @brief Decompress a DEFLATE-compressed byte stream (RFC 1951)
     * @param input  Compressed input bytes
     * @param sizeHint  Expected output size (optional, used to pre-reserve)
     * @return Decompressed bytes
     * @throws std::runtime_error on malformed input
     */
    [[nodiscard]] std::vector<uint8_t> inflate(std::span<const uint8_t> input, size_t sizeHint = 0);
} // namespace dnv::vista::sdk::compression
