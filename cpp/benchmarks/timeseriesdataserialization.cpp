/**
 * @file timeseriesdataserialization.cpp
 * @brief Nanobench benchmark for TimeSeriesData JSON serialization performance
 */

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <dnv/vista/sdk/transport/serialization/json/timeseries/Extensions.h>
#include <dnv/vista/sdk/transport/timeseries/TimeSeriesData.h>

#include <SchemasRegistry.h>

#include <stdexcept>
#include <string>

namespace dnv::vista::sdk::benchmarks
{
    namespace jts = transport::serialization::json::timeseries;

    int run()
    {
        const auto* resource = dnv::vista::sdk::schemas::find("TimeSeriesData.sample.json");
        if (!resource || resource->size == 0)
        {
            throw std::runtime_error{ "Failed to load TimeSeriesData.sample.json" };
        }

        std::string json{ resource->str() };

        auto dtoOpt = jts::fromJsonString(json);
        if (!dtoOpt.has_value())
        {
            throw std::runtime_error{ "Failed to parse TimeSeriesData.sample.json" };
        }

        auto package = jts::toDomain(*dtoOpt);

        auto warmup = jts::toJsonString(package);
        (void)warmup;

        ankerl::nanobench::Bench bench;
        bench.title("TimeSeriesDataSerialization").warmup(1000).minEpochIterations(10000);

        bench.run("Serialize", [&] {
            auto result = jts::toJsonString(package);
            ankerl::nanobench::doNotOptimizeAway(result);
        });

        auto serialized = jts::toJsonString(package);
        bench.run("Deserialize", [&] {
            auto result = jts::fromJsonString(serialized);
            ankerl::nanobench::doNotOptimizeAway(result);
        });

        return 0;
    }
} // namespace dnv::vista::sdk::benchmarks

int main()
{
    return dnv::vista::sdk::benchmarks::run();
}
