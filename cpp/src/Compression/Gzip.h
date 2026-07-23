/**
 * @file Gzip.h
 * @brief Gzip decompressor (RFC 1952)
 */

#pragma once

#include <dnv/vista/sdk/Export.h>

#include <cstdint>
#include <span>
#include <vector>

namespace dnv::vista::sdk::compression
{
    /**
     * @brief Decompress a gzip-formatted byte stream (RFC 1952)
     * @param input  Compressed input (must start with gzip magic 0x1f 0x8b)
     * @return Decompressed bytes
     * @throws std::runtime_error on malformed or corrupt input
     */
    [[nodiscard]] DNV_VISTA_SDK_CPP_API std::vector<uint8_t> gunzip(std::span<const uint8_t> input);
} // namespace dnv::vista::sdk::compression
