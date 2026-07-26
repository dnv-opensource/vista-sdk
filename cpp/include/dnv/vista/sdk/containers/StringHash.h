/**
 * @file StringHash.h
 * @brief Transparent string hasher for heterogeneous lookup in standard hash containers
 */

#pragma once

#include "Hashing.h"

namespace dnv::vista::sdk
{
    struct StringHash
    {
        using is_transparent = void;
        size_t operator()(std::string_view sv) const noexcept { return internal::hashString(sv); }
    };
} // namespace dnv::vista::sdk
