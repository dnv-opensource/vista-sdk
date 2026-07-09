/**
 * @file VIS.h
 * @brief Central entry point for the Vista SDK
 * @details Provides singleton access to VIS version management
 */

#pragma once

#include "Codebooks.h"
#include "Locations.h"
#include "VisVersions.h"

#include <span>

namespace dnv::vista::sdk
{
    /**
     * @brief VIS singleton registry
     * @details Central entry point for the Vista SDK. Uses Meyer's singleton pattern
     *          for thread-safe lazy initialization
     */
    class VIS final
    {
    private:
        VIS() = default;
        VIS(const VIS&) = delete;
        VIS(VIS&&) noexcept = delete;

    public:
        ~VIS() = default;

        VIS& operator=(const VIS&) = delete;
        VIS& operator=(VIS&&) noexcept = delete;

        /**
         * @brief Get the singleton instance
         * @return Reference to the singleton VIS instance
         */
        static const VIS& instance();

        /**
         * @brief Get all available VIS versions
         * @return Span over all supported VIS versions, in ascending order
         */
        std::span<const VisVersion> versions() const noexcept;

        /**
         * @brief Get the latest VIS version
         * @return The most recent VIS version
         */
        VisVersion latest() const noexcept;

        /**
         * @brief Get codebooks for a specific VIS version
         * @param visVersion The VIS version for which to retrieve codebooks
         * @return Reference to the Codebooks container for the requested version
         * @throws std::out_of_range if the requested version is not available
         * @details The returned reference is stable for the lifetime of the VIS singleton
         */
        const Codebooks& codebooks(VisVersion visVersion) const;

        /**
         * @brief Get locations for a specific VIS version
         * @param visVersion The VIS version for which to retrieve locations
         * @return Reference to the Locations container for the requested version
         * @throws std::out_of_range if the requested version is not available
         * @details The returned reference is stable for the lifetime of the VIS singleton
         */
        const Locations& locations(VisVersion visVersion) const;
    };
} // namespace dnv::vista::sdk
