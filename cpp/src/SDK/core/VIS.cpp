#include "dnv/vista/sdk/core/VIS.h"

#include "EmbeddedResources/EmbeddedResources.h"

#include "internal/VersionedCache.h"
#include "dto/CodebooksDto.h"
#include "VisVersionsExtensions.h"

namespace dnv::vista::sdk
{
    namespace
    {
        using internal::getOrLoad;
        using internal::VersionedCache;

        VersionedCache<VisVersion, Codebooks>& codebooksCache()
        {
            static VersionedCache<VisVersion, Codebooks> cache;
            return cache;
        }
    } // namespace

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
} // namespace dnv::vista::sdk
