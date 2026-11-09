/**
 * @file codebookmethods.cpp
 * @brief Nanobench performance benchmark for Codebook's own lookup/validation methods
 */

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <dnv/VistaSDK.h>

namespace dnv::vista::sdk::benchmarks
{
    int run()
    {
        const auto& vis = VIS::instance();
        const auto& codebooks = vis.codebooks(VisVersion::v3_11a);

        const auto& quantity = codebooks[CodebookName::Quantity]; // 247 standard values, 1 group
        const auto& state = codebooks[CodebookName::State];       // 148 standard values, 59 groups
        const auto& position = codebooks[CodebookName::Position]; // 35 standard values, 12 groups

        ankerl::nanobench::Bench bench;
        bench.title("CodebookMethods").warmup(10000).minEpochIterations(5000000);

        bench.run("Quantity_hasStandardValue_hit", [&] {
            bool a = quantity.hasStandardValue("temperature");
            ankerl::nanobench::doNotOptimizeAway(a);
        });

        bench.run("Quantity_hasStandardValue_miss", [&] {
            bool a = quantity.hasStandardValue("not_a_real_quantity");
            ankerl::nanobench::doNotOptimizeAway(a);
        });

        bench.run("Quantity_createTag_standard", [&] {
            auto a = quantity.createTag("temperature");
            ankerl::nanobench::doNotOptimizeAway(a);
        });

        bench.run("Quantity_createTag_custom", [&] {
            auto a = quantity.createTag("custom_measurement");
            ankerl::nanobench::doNotOptimizeAway(a);
        });

        bench.run("State_hasGroup_hit", [&] {
            bool a = state.hasGroup("Running");
            ankerl::nanobench::doNotOptimizeAway(a);
        });

        bench.run("State_hasGroup_miss", [&] {
            bool a = state.hasGroup("NotARealGroup");
            ankerl::nanobench::doNotOptimizeAway(a);
        });

        bench.run("Position_validatePosition_simple", [&] {
            auto a = position.validatePosition("centre");
            ankerl::nanobench::doNotOptimizeAway(a);
        });

        bench.run("Position_validatePosition_composite", [&] {
            auto a = position.validatePosition("centre-starboard-2");
            ankerl::nanobench::doNotOptimizeAway(a);
        });

        return 0;
    }
} // namespace dnv::vista::sdk::benchmarks

int main()
{
    return dnv::vista::sdk::benchmarks::run();
}
