/**
 * @file VIS.h
 * @brief Central entry point for the Vista SDK
 * @details Provides singleton access to VIS version management
 */

#pragma once

#include <dnv/vista/sdk/Export.h>

#include "Codebooks.h"
#include "Gmod.h"
#include "GmodPath.h"
#include "LocalIdBuilder.h"
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
        static DNV_VISTA_SDK_CPP_API const VIS& instance();

        /**
         * @brief Get all available VIS versions
         * @return Span over all supported VIS versions, in ascending order
         */
        DNV_VISTA_SDK_CPP_API std::span<const VisVersion> versions() const noexcept;

        /**
         * @brief Get the latest VIS version
         * @return The most recent VIS version
         */
        DNV_VISTA_SDK_CPP_API VisVersion latest() const noexcept;

        /**
         * @brief Get Gmod for a specific VIS version
         * @param visVersion The VIS version for which to retrieve the Gmod
         * @return Reference to a `Gmod` container for the requested version
         * @throws std::out_of_range if the requested version is not available
         * @details The returned reference is stable for the lifetime of the VIS singleton
         */
        DNV_VISTA_SDK_CPP_API const Gmod& gmod(VisVersion visVersion) const;

        /**
         * @brief Get codebooks for a specific VIS version
         * @param visVersion The VIS version for which to retrieve codebooks
         * @return Reference to the Codebooks container for the requested version
         * @throws std::out_of_range if the requested version is not available
         * @details The returned reference is stable for the lifetime of the VIS singleton
         */
        DNV_VISTA_SDK_CPP_API const Codebooks& codebooks(VisVersion visVersion) const;

        /**
         * @brief Get locations for a specific VIS version
         * @param visVersion The VIS version for which to retrieve locations
         * @return Reference to the Locations container for the requested version
         * @throws std::out_of_range if the requested version is not available
         * @details The returned reference is stable for the lifetime of the VIS singleton
         */
        DNV_VISTA_SDK_CPP_API const Locations& locations(VisVersion visVersion) const;

        /**
         * @brief Convert a Gmod node from one VIS version to another
         * @param sourceVersion The VIS version of the source node
         * @param sourceNode The Gmod node to convert
         * @param targetVersion The target VIS version
         * @return The converted node in the target version, or std::nullopt if conversion failed
         * @details Converts a Gmod node across VIS versions by applying versioning transformations
         *          step-by-step from source to target version. The location information is preserved
         *          during conversion when possible
         */
        DNV_VISTA_SDK_CPP_API std::optional<GmodNode> convertNode(
            VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion) const;

        /**
         * @brief Convert a Gmod node to a target VIS version (convenience overload)
         * @param sourceNode The Gmod node to convert (contains source version)
         * @param targetVersion The target VIS version
         * @param sourceParent Optional parent node for context (currently unused)
         * @return The converted node in the target version, or std::nullopt if conversion failed
         * @details Convenience method that extracts the source version from the node and delegates
         *          to the full convertNode() method
         */
        DNV_VISTA_SDK_CPP_API std::optional<GmodNode> convertNode(
            const GmodNode& sourceNode, VisVersion targetVersion, const GmodNode* sourceParent = nullptr) const;

        /**
         * @brief Convert a Gmod path from one VIS version to another
         * @param sourceVersion The VIS version of the source path
         * @param sourcePath The Gmod path to convert
         * @param targetVersion The target VIS version
         * @return The converted path in the target version, or std::nullopt if conversion failed
         * @details Converts a Gmod path across VIS versions by applying node conversions to each
         *          element in the path. If any node in the path cannot be converted, the entire
         *          conversion fails
         */
        DNV_VISTA_SDK_CPP_API std::optional<GmodPath> convertPath(
            VisVersion sourceVersion, const GmodPath& sourcePath, VisVersion targetVersion) const;

        /**
         * @brief Convert a Gmod path to a target VIS version (convenience overload)
         * @param sourcePath The Gmod path to convert (contains source version)
         * @param targetVersion The target VIS version
         * @return The converted path in the target version, or std::nullopt if conversion failed
         * @details Convenience method that extracts the source version from the path and delegates
         *          to the full convertPath() method
         */
        DNV_VISTA_SDK_CPP_API std::optional<GmodPath> convertPath(
            const GmodPath& sourcePath, VisVersion targetVersion) const;

        /**
         * @brief Convert a LocalIdBuilder from one VIS version to another
         * @param sourceLocalId The LocalIdBuilder to convert
         * @param targetVersion The target VIS version
         * @return The converted LocalIdBuilder in the target version, or std::nullopt if conversion failed
         * @details Converts a LocalIdBuilder by converting its primary and secondary item paths to the
         *          target version. If either path conversion fails, the entire conversion fails
         * @throws std::invalid_argument if the source LocalIdBuilder doesn't have a VIS version
         */
        DNV_VISTA_SDK_CPP_API std::optional<LocalIdBuilder> convertLocalId(
            const LocalIdBuilder& sourceLocalId, VisVersion targetVersion) const;

        /**
         * @brief Convert a LocalId from one VIS version to another
         * @param sourceLocalId The LocalId to convert
         * @param targetVersion The target VIS version
         * @return The converted LocalId in the target version, or std::nullopt if conversion failed
         * @details Converts a LocalId by first converting it to a LocalIdBuilder, applying version
         *          conversion, and then building the final LocalId. Preserves all metadata and quantity
         *          information during conversion
         */
        DNV_VISTA_SDK_CPP_API std::optional<LocalId> convertLocalId(
            const LocalId& sourceLocalId, VisVersion targetVersion) const;

        /**
         * @brief Get multiple Gmod instances for specified VIS versions
         * @param visVersions Vector of VIS versions to retrieve Gmods for
         * @return Unordered map containing Gmod references keyed by VIS version
         * @throws std::out_of_range if any requested version is not available
         * @details Efficiently loads multiple Gmod versions in a single call. Useful for
         *          operations that need to work with multiple VIS versions simultaneously,
         *          such as cross-version comparison or batch conversion operations
         *          Each returned reference points to an internal cached object owned by the VIS singleton
         */
        DNV_VISTA_SDK_CPP_API std::unordered_map<VisVersion, const Gmod&> gmodsMap(
            const std::span<const VisVersion>& visVersions) const;

        /**
         * @brief Get multiple Codebooks instances for specified VIS versions
         * @param visVersions Vector of VIS versions to retrieve Codebooks for
         * @return Unordered map containing Codebooks references keyed by VIS version
         * @throws std::out_of_range if any requested version is not available
         * @details Efficiently loads multiple Codebooks versions in a single call. Useful for
         *          operations that need to work with multiple VIS versions simultaneously
         *          Each returned reference points to an internal cached object owned by the VIS singleton
         */
        DNV_VISTA_SDK_CPP_API std::unordered_map<VisVersion, const Codebooks&> codebooksMap(
            const std::span<const VisVersion>& visVersions) const;

        /**
         * @brief Get multiple Locations instances for specified VIS versions
         * @param visVersions Vector of VIS versions to retrieve Locations for
         * @return Unordered map containing Locations references keyed by VIS version
         * @throws std::out_of_range if any requested version is not available
         * @details Efficiently loads multiple Locations versions in a single call. Useful for
         *          operations that need to work with multiple VIS versions simultaneously
         *          Each returned reference points to an internal cached object owned by the VIS singleton
         */
        DNV_VISTA_SDK_CPP_API std::unordered_map<VisVersion, const Locations&> locationsMap(
            const std::span<const VisVersion>& visVersions) const;
    };
} // namespace dnv::vista::sdk
