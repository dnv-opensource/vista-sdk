/**
 * @file datachannellistlookup.cpp
 * @brief Nanobench performance benchmark for DataChannelList lookup operations
 * @details Benchmarks lookup by short_id and by LocalId on a loaded DataChannelList.
 */

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <dnv/vista/sdk/core/LocalId.h>
#include <dnv/vista/sdk/transport/datachannel/DataChannel.h>
#include <dnv/vista/sdk/transport/serialization/json/datachannel/Extensions.h>

#include <SchemasRegistry.h>

#include <stdexcept>
#include <string>

namespace dnv::vista::sdk::benchmarks
{
    namespace jdc = transport::serialization::json::datachannel;

    int run()
    {
        const auto* resource = dnv::vista::sdk::schemas::find("DataChannelList.sample.compact.json");
        if (!resource || resource->size == 0)
        {
            throw std::runtime_error{ "Failed to load DataChannelList.sample.compact.json" };
        }

        std::string json{ resource->str() };

        auto dtoOpt = jdc::fromJsonString(json);
        if (!dtoOpt.has_value())
        {
            throw std::runtime_error{ "Failed to parse DataChannelList.sample.compact.json" };
        }

        auto package = jdc::toDomain(*dtoOpt);
        const auto& dcList = package.dataChannelList();

        std::string shortId;
        std::optional<LocalId> localId;

        for (const auto& dc : dcList)
        {
            if (dc.dataChannelId().shortId().has_value() && shortId.empty())
            {
                shortId = *dc.dataChannelId().shortId();
            }
            if (!localId.has_value())
            {
                localId = dc.dataChannelId().localId();
            }
            if (!shortId.empty() && localId.has_value())
            {
                break;
            }
        }

        if (shortId.empty())
        {
            throw std::runtime_error{ "No data channel with short_id found in sample" };
        }

        ankerl::nanobench::Bench bench;
        bench.title("DataChannelListLookup").warmup(10000).minEpochIterations(10000000);

        bench.run("ByShortId", [&] {
            auto result = dcList.from(shortId);
            ankerl::nanobench::doNotOptimizeAway(result);
        });

        bench.run("ByLocalId", [&] {
            auto result = dcList.from(*localId);
            ankerl::nanobench::doNotOptimizeAway(result);
        });

        return 0;
    }
} // namespace dnv::vista::sdk::benchmarks

int main()
{
    return dnv::vista::sdk::benchmarks::run();
}
