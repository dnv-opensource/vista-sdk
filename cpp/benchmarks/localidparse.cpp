/**
 * @file localidparse.cpp
 * @brief Nanobench performance benchmark for LocalId parsing operations
 * @details Benchmarks simple and complex LocalId string parsing.
 */

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <dnv/VistaSDK.h>

#include <string_view>

namespace dnv::vista::sdk::benchmarks
{
    int run()
    {
        static constexpr std::string_view simpleLocalId = "/dnv-v2/vis-3-4a/751/I101/meta/state-common.alarm";

        static constexpr std::string_view complexLocalId =
            "/dnv-v2/vis-3-4a/1036.11/S90.3/S61/sec/1036.13i-1/C662.1/C661/meta/state-auto.control/detail-blow.off";

        ankerl::nanobench::Bench bench;
        bench.title("LocalIdParse").warmup(1000).minEpochIterations(10000);

        bench.run("Simple", [&] {
            auto result = LocalId::fromString(simpleLocalId);
            ankerl::nanobench::doNotOptimizeAway(result);
        });

        bench.run("Complex", [&] {
            auto result = LocalId::fromString(complexLocalId);
            ankerl::nanobench::doNotOptimizeAway(result);
        });

        return 0;
    }
} // namespace dnv::vista::sdk::benchmarks

int main()
{
    return dnv::vista::sdk::benchmarks::run();
}
