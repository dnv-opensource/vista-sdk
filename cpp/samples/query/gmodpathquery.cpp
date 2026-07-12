/**
 * @file gmodpathquery.cpp
 * @brief Demonstrates the vista-sdk GmodPathQuery API
 * @details Shows how to build queries to match GmodPaths based on nodes and locations,
 *          using both path-based queries (from an existing path) and node-based queries (from scratch).
 */

#include <dnv/vista/sdk/core/VIS.h>
#include <dnv/vista/sdk/query/GmodPathQuery.h>

#include <iostream>
#include <string>

int main()
{
    using namespace dnv::vista::sdk;

    std::cout << "=== vista-sdk GmodPathQuery Sample ===\n\n";

    {
        std::cout << "1. GmodPathQuery: Matching paths without locations\n";
        std::cout << "-----------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto version = vis.latest();
        const auto& gmod = vis.gmod(version);
        const auto& locations = vis.locations(version);

        auto basePath = GmodPath::fromShortPath("411.1/C101.31", gmod, locations);
        auto path1 = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations);
        auto path2 = GmodPath::fromShortPath("411.1/C101.31-5", gmod, locations);
        auto path3 = GmodPath::fromShortPath("411.1/C101.63", gmod, locations);

        if (basePath.has_value() && path1.has_value() && path2.has_value() && path3.has_value())
        {
            auto query = GmodPathQueryBuilder::from(*basePath).withoutLocations().build();

            std::cout << "Query: Match '411.1/C101.31' ignoring locations\n\n";

            std::cout << "Test path 1: " << path1->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*path1) << "\n\n";

            std::cout << "Test path 2: " << path2->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*path2) << "\n\n";

            std::cout << "Test path 3: " << path3->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*path3) << " (different node)\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "2. GmodPathQuery: Matching specific nodes (Nodes builder)\n";
        std::cout << "-------------------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto version = vis.latest();
        const auto& gmod = vis.gmod(version);
        const auto& locations = vis.locations(version);

        auto node411Opt = gmod.node("411.1");
        auto nodeC101Opt = gmod.node("C101");
        auto path1 = GmodPath::fromShortPath("411.1-1P/C101.31-2", gmod, locations);
        auto path2 = GmodPath::fromShortPath("411.1-2P/C101.63", gmod, locations);
        auto path3 = GmodPath::fromShortPath("511.11/C104", gmod, locations);

        if (node411Opt.has_value() && nodeC101Opt.has_value() && path1.has_value() && path2.has_value() &&
            path3.has_value())
        {
            const auto* node411 = *node411Opt;
            const auto* nodeC101 = *nodeC101Opt;

            auto query = GmodPathQueryBuilder::create().withNode(*node411, true).withNode(*nodeC101, true).build();

            std::cout << "Query: Match paths containing nodes '411.1' AND 'C101' (any locations)\n\n";

            std::cout << "Test path 1: " << path1->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*path1) << "\n\n";

            std::cout << "Test path 2: " << path2->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*path2) << "\n\n";

            std::cout << "Test path 3: " << path3->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*path3) << " (different first node)\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "3. GmodPathQuery: withAnyNodeBefore - match any parent\n";
        std::cout << "--------------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto version = vis.latest();
        const auto& gmod = vis.gmod(version);
        const auto& locations = vis.locations(version);

        // Match any path ending with S206, regardless of parent nodes
        auto basePath = GmodPath::fromShortPath("411.1/C101.63/S206", gmod, locations);
        auto path1 = GmodPath::fromShortPath("411.1-1/C101.63/S206", gmod, locations);
        auto path2 = GmodPath::fromShortPath("411.1-2/C101.63/S206", gmod, locations);
        auto path3 = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations);

        if (basePath.has_value() && path1.has_value() && path2.has_value() && path3.has_value())
        {
            auto query = GmodPathQueryBuilder::from(*basePath)
                             .withoutLocations()
                             .withAnyNodeBefore([](const auto& nodes) -> const GmodNode* {
                                 auto it = nodes.find("S206");
                                 return it != nodes.end() ? it->second : nullptr;
                             })
                             .build();

            std::cout << "Query: Match any path ending with S206 (ignore parents)\n\n";

            std::cout << "Test path 1: " << path1->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*path1) << "\n\n";

            std::cout << "Test path 2: " << path2->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*path2) << "\n\n";

            std::cout << "Test path 3: " << path3->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*path3) << " (no S206 node)\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "4. GmodPathQuery: withAnyNodeAfter - match any children\n";
        std::cout << "-----------------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto version = vis.latest();
        const auto& gmod = vis.gmod(version);
        const auto& locations = vis.locations(version);

        // Match any path starting with 411.1, regardless of what comes after
        auto basePath = GmodPath::fromShortPath("411.1/C101.31", gmod, locations);
        auto path1 = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations);
        auto path2 = GmodPath::fromShortPath("411.1/C101.63/S206", gmod, locations);
        auto path3 = GmodPath::fromShortPath("511.11/C104", gmod, locations);

        if (basePath.has_value() && path1.has_value() && path2.has_value() && path3.has_value())
        {
            auto query = GmodPathQueryBuilder::from(*basePath)
                             .withoutLocations()
                             .withAnyNodeAfter([](const auto& nodes) -> const GmodNode* {
                                 auto it = nodes.find("411.1");
                                 return it != nodes.end() ? it->second : nullptr;
                             })
                             .build();

            std::cout << "Query: Match any path starting with '411.1' (ignore children)\n\n";

            std::cout << "Test path 1: " << path1->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*path1) << "\n\n";

            std::cout << "Test path 2: " << path2->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*path2) << "\n\n";

            std::cout << "Test path 3: " << path3->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*path3) << " (different first node)\n";
        }

        std::cout << "\n";
    }

    return 0;
}
