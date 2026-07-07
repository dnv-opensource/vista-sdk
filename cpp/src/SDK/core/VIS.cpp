#include "dnv/vista/sdk/core/VIS.h"

#include "VisVersionsExtensions.h"

namespace dnv::vista::sdk
{
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
} // namespace dnv::vista::sdk
