/**
 * @file gmodload.cpp
 * @brief Nanobench performance benchmark for Gmod singleton cache access
 * @details Measures the cost of retrieving an already-loaded Gmod from the VIS singleton cache.
 *          The Gmod JSON is loaded once at first access. Subsequent calls return the cached instance.
 *          This benchmark targets the cache-hit path, not JSON parsing.
 */

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <dnv/VistaSDK.h>

namespace dnv::vista::sdk::benchmarks
{
    int run()
    {
        {
            const auto& warmup = VIS::instance().gmod(VisVersion::v3_7a);
            ankerl::nanobench::doNotOptimizeAway(&warmup);
        }

        ankerl::nanobench::Bench bench;
        bench.title("GmodCacheAccess").warmup(10000).minEpochIterations(1000000);

        bench.run("GmodCacheAccess_v3_4a", [&] {
            const auto& gmod = VIS::instance().gmod(VisVersion::v3_4a);
            ankerl::nanobench::doNotOptimizeAway(&gmod);
        });

        bench.run("GmodCacheAccess_v3_7a", [&] {
            const auto& gmod = VIS::instance().gmod(VisVersion::v3_7a);
            ankerl::nanobench::doNotOptimizeAway(&gmod);
        });

        return 0;
    }
} // namespace dnv::vista::sdk::benchmarks

int main()
{
    return dnv::vista::sdk::benchmarks::run();
}
