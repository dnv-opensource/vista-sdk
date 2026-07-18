/**
 * @file gmodlookup.cpp
 * @brief Nanobench performance benchmark for Gmod node lookup operations
 * @details Compares lookup performance: std::unordered_map vs Gmod class.
 */

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <dnv/vista/sdk/core/VIS.h>

#include <string_view>
#include <unordered_map>

namespace dnv::vista::sdk::benchmarks
{
    int run()
    {
        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(VisVersion::v3_7a);

        std::unordered_map<std::string, const GmodNode*> unorderedMap;
        unorderedMap.reserve(10000);
        for (const auto& [code, node] : gmod)
        {
            unorderedMap[code] = &node;
        }

        ankerl::nanobench::Bench bench;
        bench.title("GmodLookup").warmup(10000).minEpochIterations(1000000);

        bench.run("StdUnorderedMap", [&] {
            bool result =
                unorderedMap.find("VE") != unorderedMap.end() && unorderedMap.find("400a") != unorderedMap.end() &&
                unorderedMap.find("400") != unorderedMap.end() && unorderedMap.find("H346.11112") != unorderedMap.end();
            ankerl::nanobench::doNotOptimizeAway(result);
        });

        bench.run("Gmod", [&] {
            bool result = gmod.node("VE").has_value() && gmod.node("400a").has_value() &&
                          gmod.node("400").has_value() && gmod.node("H346.11112").has_value();
            ankerl::nanobench::doNotOptimizeAway(result);
        });

        return 0;
    }
} // namespace dnv::vista::sdk::benchmarks

int main()
{
    return dnv::vista::sdk::benchmarks::run();
}
