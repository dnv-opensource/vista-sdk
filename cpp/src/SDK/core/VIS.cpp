#include "dnv/vista/sdk/core/VIS.h"

#include "EmbeddedResources/EmbeddedResources.h"

#include "internal/versioning/GmodVersioning.h"
#include "internal/VersionedCache.h"
#include "dto/CodebooksDto.h"
#include "dto/GmodDto.h"
#include "dto/LocationsDto.h"
#include "VisVersionsExtensions.h"

namespace dnv::vista::sdk
{
    namespace
    {
        using internal::getOrLoad;
        using internal::VersionedCache;

        VersionedCache<VisVersion, Gmod>& gmodCache()
        {
            static VersionedCache<VisVersion, Gmod> cache;
            return cache;
        }

        VersionedCache<VisVersion, Codebooks>& codebooksCache()
        {
            static VersionedCache<VisVersion, Codebooks> cache;
            return cache;
        }

        VersionedCache<VisVersion, Locations>& locationsCache()
        {
            static VersionedCache<VisVersion, Locations> cache;
            return cache;
        }

        template <typename T>
        struct LazySingleton
        {
            std::shared_mutex mutex;
            std::unique_ptr<T> value;
        };

        template <typename T, typename InitFn>
        const T& getOrInit(LazySingleton<T>& singleton, InitFn init)
        {
            {
                std::shared_lock lock(singleton.mutex);
                if (singleton.value)
                {
                    return *singleton.value;
                }
            }

            std::unique_lock lock(singleton.mutex);

            if (singleton.value)
            {
                return *singleton.value;
            }

            singleton.value = init();

            return *singleton.value;
        }

        LazySingleton<internal::GmodVersioning>& gmodVersioningSingleton()
        {
            static LazySingleton<internal::GmodVersioning> singleton;
            return singleton;
        }
    } // namespace

    namespace internal
    {
        const GmodVersioning& gmodVersioning()
        {
            return getOrInit(gmodVersioningSingleton(), []() {
                auto dto = EmbeddedResources::gmodVersioning();

                if (!dto.has_value())
                {
                    throw std::invalid_argument{ "Invalid state" };
                }

                return std::make_unique<GmodVersioning>(*dto);
            });
        }
    } // namespace internal

    const VIS& VIS::instance()
    {
        static VIS instance;

        return instance;
    }

    std::span<const VisVersion> VIS::versions() const noexcept
    {
        static constexpr auto versions = VisVersions::all();

        return versions;
    }

    VisVersion VIS::latest() const noexcept
    {
        return VisVersions::latest();
    }

    const Gmod& VIS::gmod(VisVersion visVersion) const
    {
        return getOrLoad(gmodCache(), visVersion, [](VisVersion version) {
            auto versionStr = VisVersions::toString(version);
            auto dto = EmbeddedResources::gmod(versionStr);

            if (!dto.has_value())
            {
                throw std::out_of_range{ "Gmod not available for version: " + std::string{ versionStr } };
            }

            return Gmod{ version, *dto };
        });
    }

    const Codebooks& VIS::codebooks(VisVersion visVersion) const
    {
        return getOrLoad(codebooksCache(), visVersion, [](VisVersion version) {
            auto versionStr = VisVersions::toString(version);
            auto dto = EmbeddedResources::codebooks(versionStr);

            if (!dto.has_value())
            {
                throw std::out_of_range{ "Codebooks not available for version: " + std::string{ versionStr } };
            }

            return Codebooks{ version, *dto };
        });
    }

    const Locations& VIS::locations(VisVersion visVersion) const
    {
        return getOrLoad(locationsCache(), visVersion, [](VisVersion version) {
            auto versionStr = VisVersions::toString(version);
            auto dto = EmbeddedResources::locations(versionStr);

            if (!dto.has_value())
            {
                throw std::out_of_range{ "Locations not available for version: " + std::string{ versionStr } };
            }

            return Locations{ version, *dto };
        });
    }

    std::optional<GmodNode> VIS::convertNode(
        VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion) const
    {
        return internal::gmodVersioning().convertNode(sourceVersion, sourceNode, targetVersion);
    }

    std::optional<GmodNode> VIS::convertNode(
        const GmodNode& sourceNode, VisVersion targetVersion, [[maybe_unused]] const GmodNode* sourceParent) const
    {
        return convertNode(sourceNode.version(), sourceNode, targetVersion);
    }

    std::optional<GmodPath> VIS::convertPath(
        VisVersion sourceVersion, const GmodPath& sourcePath, VisVersion targetVersion) const
    {
        return internal::gmodVersioning().convertPath(sourceVersion, sourcePath, targetVersion);
    }

    std::optional<GmodPath> VIS::convertPath(const GmodPath& sourcePath, VisVersion targetVersion) const
    {
        return convertPath(sourcePath.version(), sourcePath, targetVersion);
    }

    std::optional<LocalIdBuilder> VIS::convertLocalId(
        const LocalIdBuilder& sourceLocalId, VisVersion targetVersion) const
    {
        return internal::gmodVersioning().convertLocalId(sourceLocalId, targetVersion);
    }

    std::optional<LocalId> VIS::convertLocalId(const LocalId& sourceLocalId, VisVersion targetVersion) const
    {
        return internal::gmodVersioning().convertLocalId(sourceLocalId, targetVersion);
    }

    std::unordered_map<VisVersion, const Gmod&> VIS::gmodsMap(const std::span<const VisVersion>& visVersions) const
    {
        for (const auto& version : visVersions)
        {
            auto versionStr = VisVersions::toString(version);
            if (versionStr.empty())
            {
                throw std::invalid_argument{ "Invalid VIS version provided" };
            }
        }

        std::unordered_set<VisVersion> uniqueVersions(visVersions.begin(), visVersions.end());

        std::unordered_map<VisVersion, const Gmod&> result;
        result.reserve(uniqueVersions.size());

        for (const auto& version : uniqueVersions)
        {
            result.emplace(version, gmod(version));
        }

        return result;
    }

    std::unordered_map<VisVersion, const Codebooks&> VIS::codebooksMap(
        const std::span<const VisVersion>& visVersions) const
    {
        for (const auto& version : visVersions)
        {
            auto versionStr = VisVersions::toString(version);
            if (versionStr.empty())
            {
                throw std::invalid_argument{ "Invalid VIS version provided" };
            }
        }

        std::unordered_set<VisVersion> uniqueVersions(visVersions.begin(), visVersions.end());

        std::unordered_map<VisVersion, const Codebooks&> result;
        result.reserve(uniqueVersions.size());

        for (const auto& version : uniqueVersions)
        {
            result.emplace(version, codebooks(version));
        }

        return result;
    }

    std::unordered_map<VisVersion, const Locations&> VIS::locationsMap(
        const std::span<const VisVersion>& visVersions) const
    {
        for (const auto& version : visVersions)
        {
            auto versionStr = VisVersions::toString(version);
            if (versionStr.empty())
            {
                throw std::invalid_argument{ "Invalid VIS version provided" };
            }
        }

        std::unordered_set<VisVersion> uniqueVersions(visVersions.begin(), visVersions.end());

        std::unordered_map<VisVersion, const Locations&> result;
        result.reserve(uniqueVersions.size());

        for (const auto& version : uniqueVersions)
        {
            result.emplace(version, locations(version));
        }

        return result;
    }
} // namespace dnv::vista::sdk
