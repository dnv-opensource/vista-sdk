/**
 * @file datachannellistserialization.cpp
 * @brief Nanobench benchmark for DataChannelList JSON serialization performance
 */

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

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

        auto warmup = jdc::toJsonString(package);
        (void)warmup;

        ankerl::nanobench::Bench bench;
        bench.title("DataChannelListSerialization").warmup(1000).minEpochIterations(10000);

        bench.run("Serialize", [&] {
            auto result = jdc::toJsonString(package);
            ankerl::nanobench::doNotOptimizeAway(result);
        });

        auto serialized = jdc::toJsonString(package);
        bench.run("Deserialize", [&] {
            auto result = jdc::fromJsonString(serialized);
            ankerl::nanobench::doNotOptimizeAway(result);
        });

        return 0;
    }
} // namespace dnv::vista::sdk::benchmarks

int main()
{
    return dnv::vista::sdk::benchmarks::run();
}
