/**
 * @file ISO19848AnnexC.h
 * @brief ISO 19848 Annex C DNV constants
 */

#pragma once

#include <string_view>

namespace dnv::vista::sdk::internal::iso19848::annexC
{
    /** @brief Domain name for universal IDs (ISO 19848 Annex C) */
    constexpr std::string_view NamingEntity = "data.dnv.com";

    /** @brief Naming rule identifier (ISO 19848 Annex C) */
    constexpr std::string_view NamingRule = "dnv";

    /** @brief Naming scheme version (ISO 19848 Annex C) */
    constexpr std::string_view NamingSchemeVersion = "v2";

    /** @brief Full versioned naming rule: NamingRule-NamingSchemeVersion (ISO 19848 Annex C) */
    constexpr std::string_view VersionedNamingRule = "dnv-v2";

    /** @brief Reference documentation URL (ISO 19848 Annex C) */
    constexpr std::string_view ReferenceUrl = "https://docs.vista.dnv.com";
} // namespace dnv::vista::sdk::internal::iso19848::annexC
