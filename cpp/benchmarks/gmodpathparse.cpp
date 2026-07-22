/**
 * @file gmodpathparse.cpp
 * @brief Nanobench performance benchmark for GmodPath parsing operations
 * @details Benchmarks short path and full path parsing with and without location individualization.
 *          Paths are rotated across a pool each iteration to avoid branch predictor bias.
 */

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <dnv/VistaSDK.h>

#include <array>
#include <string_view>

namespace dnv::vista::sdk::benchmarks
{
    int run()
    {
        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(VisVersion::v3_4a);
        const auto& locations = vis.locations(VisVersion::v3_4a);

        static constexpr std::array<std::string_view, 6> shortPaths = { "411.1/C101.72/I101", "511/C101.63/S90",
                                                                        "411.1/C101.63/S90",  "621.21/C101.72/I101",
                                                                        "511/C101.72/I101",   "411.1/C101.31/I101" };

        static constexpr std::array<std::string_view, 6> fullPaths = {
            "VE/400a/410/411/411i/411.1/CS1/C101/C101.7/C101.72/I101",
            "VE/500a/510/511/CS1/C101/C101.6/C101.63/S90",
            "VE/400a/410/411/411i/411.1/CS1/C101/C101.6/C101.63/S90",
            "VE/600a/620/621/621.2/621.2i/621.21/CS1/C101/C101.7/C101.72/I101",
            "VE/500a/510/511/CS1/C101/C101.7/C101.72/I101",
            "VE/400a/410/411/411i/411.1/CS1/C101/C101.3/C101.31/I101"
        };

        static constexpr std::array<std::string_view, 4> shortPathsIndividualized = {
            "612.21-1/C701.13/S93", "612.21-2/C701.13/S93", "612.21-1/C701.13/S90", "612.21-2/C701.13/S90"
        };

        static constexpr std::array<std::string_view, 4> fullPathsIndividualized = {
            "VE/600a/610/612/612.2/612.2i/612.21-1/CS10/C701/C701.1/C701.13/S93",
            "VE/600a/610/612/612.2/612.2i/612.21-2/CS10/C701/C701.1/C701.13/S93",
            "VE/600a/610/612/612.2/612.2i/612.21-1/CS10/C701/C701.1/C701.13/S90",
            "VE/600a/610/612/612.2/612.2i/612.21-2/CS10/C701/C701.1/C701.13/S90"
        };

        ankerl::nanobench::Rng rng;

        ankerl::nanobench::Bench bench;
        bench.title("GmodPathParse").warmup(1000).minEpochIterations(10000);

        bench.run("FromShortPath", [&] {
            auto result = GmodPath::fromShortPath(
                shortPaths[rng.bounded(static_cast<uint32_t>(shortPaths.size()))], gmod, locations);
            ankerl::nanobench::doNotOptimizeAway(result);
        });

        bench.run("FromFullPath", [&] {
            auto result = GmodPath::fromFullPath(
                fullPaths[rng.bounded(static_cast<uint32_t>(fullPaths.size()))], gmod, locations);

            ankerl::nanobench::doNotOptimizeAway(result);
        });

        bench.run("FromShortPathIndividualized", [&] {
            auto result = GmodPath::fromShortPath(
                shortPathsIndividualized[rng.bounded(static_cast<uint32_t>(shortPathsIndividualized.size()))],
                gmod,
                locations);
            ankerl::nanobench::doNotOptimizeAway(result);
        });

        bench.run("FromFullPathIndividualized", [&] {
            auto result = GmodPath::fromFullPath(
                fullPathsIndividualized[rng.bounded(static_cast<uint32_t>(fullPathsIndividualized.size()))],
                gmod,
                locations);
            ankerl::nanobench::doNotOptimizeAway(result);
        });

        return 0;
    }
} // namespace dnv::vista::sdk::benchmarks

int main()
{
    return dnv::vista::sdk::benchmarks::run();
}
