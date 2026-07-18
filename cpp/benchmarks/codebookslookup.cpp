/**
 * @file codebookslookup.cpp
 * @brief Nanobench performance benchmark for Codebooks lookup operations
 * @details Compares std::unordered_map, std::map, and Codebooks class lookup performance.
 */

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <dnv/vista/sdk/core/VIS.h>

#include <array>
#include <map>
#include <unordered_map>

namespace dnv::vista::sdk::benchmarks
{
    int run()
    {
        const auto& vis = VIS::instance();
        const auto& codebooks = vis.codebooks(VisVersion::v3_7a);

        static constexpr std::array<CodebookName, 11> allNames = { CodebookName::Quantity,
                                                                   CodebookName::Content,
                                                                   CodebookName::Calculation,
                                                                   CodebookName::State,
                                                                   CodebookName::Command,
                                                                   CodebookName::Type,
                                                                   CodebookName::FunctionalServices,
                                                                   CodebookName::MaintenanceCategory,
                                                                   CodebookName::ActivityType,
                                                                   CodebookName::Position,
                                                                   CodebookName::Detail };

        std::unordered_map<CodebookName, Codebook> unorderedMap;
        unorderedMap.reserve(allNames.size());
        std::map<CodebookName, Codebook> map;

        for (auto name : allNames)
        {
            unorderedMap.emplace(name, codebooks[name]);
            map.emplace(name, codebooks[name]);
        }

        ankerl::nanobench::Rng rng;

        ankerl::nanobench::Bench bench;
        bench.title("CodebooksLookup").warmup(10000).minEpochIterations(150000000);

        bench.run("StdUnorderedMap", [&] {
            auto a = unorderedMap.find(CodebookName::Quantity);
            auto b = unorderedMap.find(CodebookName::Type);
            auto c = unorderedMap.find(CodebookName::Detail);
            ankerl::nanobench::doNotOptimizeAway(a);
            ankerl::nanobench::doNotOptimizeAway(b);
            ankerl::nanobench::doNotOptimizeAway(c);
        });

        bench.run("StdMap", [&] {
            auto a = map.find(CodebookName::Quantity);
            auto b = map.find(CodebookName::Type);
            auto c = map.find(CodebookName::Detail);
            ankerl::nanobench::doNotOptimizeAway(a);
            ankerl::nanobench::doNotOptimizeAway(b);
            ankerl::nanobench::doNotOptimizeAway(c);
        });

        bench.run("Codebooks", [&] {
            const auto& a = codebooks[CodebookName::Quantity];
            const auto& b = codebooks[CodebookName::Type];
            const auto& c = codebooks[CodebookName::Detail];
            ankerl::nanobench::doNotOptimizeAway(&a);
            ankerl::nanobench::doNotOptimizeAway(&b);
            ankerl::nanobench::doNotOptimizeAway(&c);
        });

        return 0;
    }
} // namespace dnv::vista::sdk::benchmarks

int main()
{
    return dnv::vista::sdk::benchmarks::run();
}
