/**
 * @file localidquery.cpp
 * @brief Demonstrates the vista-sdk LocalIdQuery API
 * @details Shows how to build queries to match LocalId instances by primary/secondary items
 *          and metadata tags, using GmodPathQuery and MetadataTagsQuery as building blocks.
 */

#include <dnv/vista/sdk/core/VIS.h>
#include <dnv/vista/sdk/query/LocalIdQuery.h>

#include <iostream>
#include <string>

int main()
{
    using namespace dnv::vista::sdk;

    std::cout << "=== vista-sdk LocalIdQuery Sample ===\n\n";

    {
        std::cout << "1. LocalIdQuery: Matching by primary item\n";
        std::cout << "--------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto version = vis.latest();
        const auto& gmod = vis.gmod(version);
        const auto& locations = vis.locations(version);

        // Build query for specific primary item (ignoring locations)
        auto primaryPath = GmodPath::fromShortPath("411.1/C101.31", gmod, locations);
        auto localId1 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature");
        auto localId2 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31-5/meta/qty-pressure");
        auto localId3 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.63/meta/qty-temperature");

        if (primaryPath.has_value() && localId1.has_value() && localId2.has_value() && localId3.has_value())
        {
            auto pathQuery = GmodPathQueryBuilder::from(*primaryPath).withoutLocations().build();

            auto query = LocalIdQueryBuilder::create().withPrimaryItem(pathQuery).build();

            std::cout << "Query: Match LocalIds with primary item '411.1/C101.31' (any location)\n\n";

            std::cout << "Test LocalId 1: " << localId1->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*localId1) << "\n\n";

            std::cout << "Test LocalId 2: " << localId2->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*localId2) << "\n\n";

            std::cout << "Test LocalId 3: " << localId3->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*localId3) << " (different node)\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "2. LocalIdQuery: Combined query (primary item + tags)\n";
        std::cout << "-------------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto version = vis.latest();
        const auto& gmod = vis.gmod(version);

        auto node411Opt = gmod.node("411.1");
        auto nodeC101Opt = gmod.node("C101");
        auto localId1 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature");
        auto localId2 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-pressure");
        auto localId3 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.63/meta/qty-temperature");

        if (node411Opt.has_value() && nodeC101Opt.has_value() && localId1.has_value() && localId2.has_value() &&
            localId3.has_value())
        {
            const auto* node411 = *node411Opt;
            const auto* nodeC101 = *nodeC101Opt;

            // Build path query
            auto pathQuery = GmodPathQueryBuilder::create().withNode(*node411, true).withNode(*nodeC101, true).build();

            // Build tags query
            auto tagsQuery = MetadataTagsQueryBuilder::create().withTag(CodebookName::Quantity, "temperature").build();

            // Combine in LocalIdQuery
            auto query = LocalIdQueryBuilder::create().withPrimaryItem(pathQuery).withTags(tagsQuery).build();

            std::cout << "Query: Match LocalIds with primary='411.1/C101' AND qty-temperature\n\n";

            std::cout << "Test LocalId 1: " << localId1->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*localId1) << " (has both)\n\n";

            std::cout << "Test LocalId 2: " << localId2->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*localId2) << " (missing tag)\n\n";

            std::cout << "Test LocalId 3: " << localId3->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*localId3)
                      << " (C101.63 is a subnode of C101)\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "3. LocalIdQuery: Secondary item requirement\n";
        std::cout << "----------------------------------------------\n";

        auto localId1 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature");
        auto localId2 = LocalId::fromString("/dnv-v2/vis-3-4a/621.21/S90/sec/411.1/C101/meta/qty-mass");

        if (localId1.has_value() && localId2.has_value())
        {
            // Query requiring NO secondary item
            auto query1 = LocalIdQueryBuilder::create().withoutSecondaryItem().build();

            std::cout << "Query: Match LocalIds WITHOUT secondary item\n\n";
            std::cout << "Test LocalId 1 (no secondary): " << localId1->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query1.match(*localId1) << "\n\n";

            std::cout << "Test LocalId 2 (has secondary): " << localId2->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query1.match(*localId2) << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "4. Building queries from existing LocalIds\n";
        std::cout << "---------------------------------------------\n";

        // Parse a reference LocalId
        auto referenceLocalId =
            LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet");

        // Test against various LocalIds
        auto localId1 =
            LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet");
        auto localId2 =
            LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31-5/meta/qty-temperature/cnt-exhaust.gas/pos-inlet");
        auto localId3 =
            LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-pressure/cnt-exhaust.gas/pos-inlet");

        if (referenceLocalId.has_value() && localId1.has_value() && localId2.has_value() && localId3.has_value())
        {
            // Build query that matches this exact LocalId
            auto query = LocalIdQueryBuilder::from(*referenceLocalId).build();

            std::cout << "Reference LocalId: " << referenceLocalId->toString() << "\n\n";
            std::cout << "Query: Match exact copy of reference LocalId\n\n";

            std::cout << "Test LocalId 1 (exact match): \n  Matches: " << std::boolalpha << query.match(*localId1)
                      << "\n\n";
            std::cout << "Test LocalId 2 (different location): \n  Matches: " << std::boolalpha
                      << query.match(*localId2) << "\n\n";
            std::cout << "Test LocalId 3 (different tag): \n  Matches: " << std::boolalpha << query.match(*localId3)
                      << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "5. LocalIdQuery: withPrimaryItem using Nodes builder lambda\n";
        std::cout << "--------------------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto version = vis.latest();
        const auto& gmod = vis.gmod(version);

        auto node411Opt = gmod.node("411.1");
        auto nodeC101Opt = gmod.node("C101");
        auto localId1 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature");
        auto localId2 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.63/meta/qty-pressure");
        auto localId3 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C102.31/meta/qty-temperature");

        if (node411Opt.has_value() && nodeC101Opt.has_value() && localId1.has_value() && localId2.has_value() &&
            localId3.has_value())
        {
            const auto* node411 = *node411Opt;
            const auto* nodeC101 = *nodeC101Opt;

            // Build base nodes query (can be reused)
            const auto baseNodes = GmodPathQueryBuilder::create().withNode(*node411, true).withNode(*nodeC101, true);

            // Build final query using const& copy (demonstrates reusability)
            auto query = LocalIdQueryBuilder::create().withPrimaryItem(baseNodes.build()).build();

            std::cout << "Query: Match LocalIds with primary containing nodes 411.1 AND C101 (any locations)\n\n";

            std::cout << "Test LocalId 1: " << localId1->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*localId1) << "\n\n";

            std::cout << "Test LocalId 2: " << localId2->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*localId2) << "\n\n";

            std::cout << "Test LocalId 3: " << localId3->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*localId3) << " (wrong second node)\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "6. LocalIdQuery: withPrimaryItem using base path + Path builder lambda\n";
        std::cout << "------------------------------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto version = vis.latest();
        const auto& gmod = vis.gmod(version);
        const auto& locations = vis.locations(version);

        auto basePath = GmodPath::fromShortPath("411.1/C101.31", gmod, locations);
        auto localId1 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature");
        auto localId2 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31-5/meta/qty-pressure");
        auto localId3 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.63/meta/qty-temperature");

        if (basePath.has_value() && localId1.has_value() && localId2.has_value() && localId3.has_value())
        {
            auto query =
                LocalIdQueryBuilder::create()
                    .withPrimaryItem(
                        *basePath,
                        [](const GmodPathQueryBuilder::Path& builder) { return builder.withoutLocations().build(); })
                    .build();

            std::cout << "Query: Match LocalIds with primary '411.1/C101.31' (any location) using Path lambda\n\n";

            std::cout << "Test LocalId 1: " << localId1->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*localId1) << "\n\n";

            std::cout << "Test LocalId 2: " << localId2->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*localId2) << "\n\n";

            std::cout << "Test LocalId 3: " << localId3->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*localId3) << " (different node)\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "7. LocalIdQuery: withAnyNodeBefore usage\n";
        std::cout << "--------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto version = vis.latest();
        const auto& gmod = vis.gmod(version);
        const auto& locations = vis.locations(version);

        auto nodeC101Opt = gmod.node("C101");
        auto basePath = GmodPath::fromShortPath("411.1/C101.31", gmod, locations);
        auto localId1 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature");
        auto localId2 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/E202.1/E31/meta/qty-pressure");
        auto localId3 = LocalId::fromString("/dnv-v2/vis-3-4a/621.21/S90/meta/qty-temperature");

        if (nodeC101Opt.has_value() && basePath.has_value() && localId1.has_value() && localId2.has_value() &&
            localId3.has_value())
        {
            const auto* nodeC101 = *nodeC101Opt;

            auto query = LocalIdQueryBuilder::create()
                             .withPrimaryItem(
                                 *basePath,
                                 [nodeC101](const GmodPathQueryBuilder::Path& builder) {
                                     return builder
                                         .withAnyNodeBefore(
                                             [nodeC101](const std::unordered_map<std::string, const GmodNode*>&)
                                                 -> const GmodNode* { return nodeC101; })
                                         .withoutLocations()
                                         .build();
                                 })
                             .build();

            std::cout << "Query: Match LocalIds with any node before C101 (ignoring nodes before C101)\n\n";

            std::cout << "Test LocalId 1 (411.1): " << localId1->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*localId1) << "\n\n";

            std::cout << "Test LocalId 2 (411.1/E202.1): " << localId2->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*localId2) << " (no C101)\n\n";

            std::cout << "Test LocalId 3 (621.21): " << localId3->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*localId3) << " (no C101)\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "8. LocalIdQuery: accumulating tags via withTags configure-callback\n";
        std::cout << "---------------------------------------------------------------------\n";

        auto localId1 =
            LocalId::fromString("/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened");
        auto localId2 = LocalId::fromString("/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water");

        if (localId1.has_value() && localId2.has_value())
        {
            auto query = LocalIdQueryBuilder::create()
                             .withTags([](MetadataTagsQueryBuilder& b) {
                                 return b.withTag(CodebookName::Content, "sea.water").build();
                             })
                             .withTags([](MetadataTagsQueryBuilder& b) {
                                 return b.withTag(CodebookName::State, "opened").build();
                             })
                             .build();

            std::cout << "Query: Match LocalIds with cnt-sea.water AND state-opened\n\n";

            std::cout << "Test LocalId 1 (both tags): " << localId1->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*localId1) << "\n\n";

            std::cout << "Test LocalId 2 (only cnt-sea.water): " << localId2->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*localId2) << "\n";
        }

        std::cout << "\n";
    }

    return 0;
}
