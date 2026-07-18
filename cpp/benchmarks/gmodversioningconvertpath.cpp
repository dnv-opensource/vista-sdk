/**
 * @file gmodversioningconvertpath.cpp
 * @brief Nanobench Gmod version conversion performance benchmark testing path conversion between VIS versions
 */

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <dnv/vista/sdk/core/ParsingErrors.h>
#include <dnv/vista/sdk/core/VIS.h>

#include <stdexcept>

namespace dnv::vista::sdk::benchmarks
{
    int run()
    {
        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(VisVersion::v3_4a);
        const auto& locations = vis.locations(VisVersion::v3_4a);

        ParsingErrors errors;

        auto pathNoLoc = GmodPath::fromShortPath("411.1/C101.72/I101", gmod, locations, errors);
        if (!pathNoLoc.has_value())
        {
            throw std::runtime_error{ "Failed to parse path without location" };
        }

        auto pathWithLoc = GmodPath::fromShortPath("612.21-1/C701.13/S93", gmod, locations, errors);
        if (!pathWithLoc.has_value())
        {
            throw std::runtime_error{ "Failed to parse path with location" };
        }

        ankerl::nanobench::Bench bench;
        bench.title("GmodVersioningConvertPath").warmup(1000).minEpochIterations(10000);

        bench.run("ConvertPath", [&] {
            auto result = vis.convertPath(VisVersion::v3_4a, *pathNoLoc, VisVersion::v3_5a);
            ankerl::nanobench::doNotOptimizeAway(result);
        });

        bench.run("ConvertPathWithLocation", [&] {
            auto result = vis.convertPath(VisVersion::v3_4a, *pathWithLoc, VisVersion::v3_5a);
            ankerl::nanobench::doNotOptimizeAway(result);
        });

        return 0;
    }
} // namespace dnv::vista::sdk::benchmarks

int main()
{
    return dnv::vista::sdk::benchmarks::run();
}
