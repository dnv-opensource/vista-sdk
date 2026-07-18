/**
 * @file gmodtraversal.cpp
 * @brief Nanobench Gmod tree traversal performance benchmark testing full tree iteration
 */

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <dnv/vista/sdk/core/VIS.h>

#include <vector>

namespace dnv::vista::sdk::benchmarks
{
    int run()
    {
        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(VisVersion::v3_4a);

        ankerl::nanobench::Bench bench;
        bench.title("GmodTraversal").warmup(3).minEpochIterations(5);

        bench.run("FullTraversal", [&] {
            std::size_t count = 0;
            gmod.traverse(
                [&count]([[maybe_unused]] const TraversalPath& parents, [[maybe_unused]] const GmodNode& node)
                    -> TraversalHandlerResult {
                    ++count;
                    return TraversalHandlerResult::Continue;
                });
            ankerl::nanobench::doNotOptimizeAway(count);
        });

        return 0;
    }
} // namespace dnv::vista::sdk::benchmarks

int main()
{
    return dnv::vista::sdk::benchmarks::run();
}
