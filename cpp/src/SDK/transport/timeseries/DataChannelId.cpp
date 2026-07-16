#include "dnv/vista/sdk/transport/timeseries/DataChannelId.h"

#include "dnv/vista/sdk/core/LocalId.h"

namespace dnv::vista::sdk::transport::timeseries
{
    std::optional<DataChannelId> DataChannelId::fromString(std::string_view value) noexcept
    try
    {
        if (value.empty())
        {
            return std::nullopt;
        }

        if (auto localId = LocalId::fromString(value))
        {
            return DataChannelId(std::move(*localId));
        }

        return DataChannelId(std::string{ value });
    }
    catch (const std::bad_alloc&)
    {
        return std::nullopt;
    }
} // namespace dnv::vista::sdk::transport::timeseries
