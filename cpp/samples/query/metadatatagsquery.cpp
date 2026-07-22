/**
 * @file metadatatagsquery.cpp
 * @brief Demonstrates the vista-sdk MetadataTagsQuery API
 * @details Shows how to build queries to match LocalIds by their metadata tags,
 *          using both subset matching (at least these tags) and exact matching (only these tags).
 */

#include <dnv/VistaSDK.h>

#include <iostream>
#include <string>

int main()
{
    using namespace dnv::vista::sdk;

    std::cout << "=== vista-sdk MetadataTagsQuery Sample ===\n\n";

    {
        std::cout << "1. MetadataTagsQuery: Matching LocalIds by metadata tags\n";
        std::cout << "-----------------------------------------------------------\n";

        // Build a query for LocalIds with quantity='temperature'
        auto query = MetadataTagsQueryBuilder::create().withTag(CodebookName::Quantity, "temperature").build();

        // Test against some LocalIds
        auto localId1 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas");
        auto localId2 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.63/meta/qty-pressure/cnt-air");

        if (localId1.has_value() && localId2.has_value())
        {
            std::cout << "Query: Match LocalIds with qty-temperature tag\n\n";
            std::cout << "Test LocalId 1: " << localId1->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*localId1) << "\n\n";

            std::cout << "Test LocalId 2: " << localId2->toString() << "\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*localId2) << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "2. MetadataTagsQuery: Multiple tags with AND semantics\n";
        std::cout << "---------------------------------------------------------\n";

        // Build query requiring BOTH tags
        auto query = MetadataTagsQueryBuilder::create()
                         .withTag(CodebookName::Quantity, "volume")
                         .withTag(CodebookName::Content, "cargo")
                         .build();

        auto localId1 =
            LocalId::fromString("/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo/pos~percentage");
        auto localId2 = LocalId::fromString("/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume");

        if (localId1.has_value() && localId2.has_value())
        {
            std::cout << "Query: Match LocalIds with qty-volume AND cnt-cargo\n\n";
            std::cout << "Test LocalId 1: " << localId1->toString() << "\n";
            std::cout << "  Has qty-volume: true, Has cnt-cargo: true\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*localId1) << "\n\n";

            std::cout << "Test LocalId 2: " << localId2->toString() << "\n";
            std::cout << "  Has qty-volume: true, Has cnt-cargo: false\n";
            std::cout << "  Matches: " << std::boolalpha << query.match(*localId2) << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "3. MetadataTagsQuery: Subset vs Exact matching\n";
        std::cout << "-------------------------------------------------\n";

        // Subset matching (default): LocalId can have EXTRA tags beyond those in query
        auto subsetQuery = MetadataTagsQueryBuilder::create()
                               .withTag(CodebookName::Quantity, "pressure")
                               .withAllowOtherTags(true) // Allow extra tags (this is default)
                               .build();

        // Exact matching: LocalId must have EXACTLY the tags in query (no more, no less)
        auto exactQuery = MetadataTagsQueryBuilder::create()
                              .withTag(CodebookName::Quantity, "pressure")
                              .withAllowOtherTags(false) // No extra tags allowed
                              .build();

        auto localId1 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.63/meta/qty-pressure");
        auto localId2 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.63/meta/qty-pressure/cnt-air");

        if (localId1.has_value() && localId2.has_value())
        {
            std::cout << "Query: Match LocalIds with qty-pressure\n\n";

            std::cout << "Test LocalId 1 (only qty-pressure): " << localId1->toString() << "\n";
            std::cout << "  Subset query matches: " << std::boolalpha << subsetQuery.match(*localId1) << "\n";
            std::cout << "  Exact query matches: " << std::boolalpha << exactQuery.match(*localId1) << "\n\n";

            std::cout << "Test LocalId 2 (qty-pressure + cnt-air): " << localId2->toString() << "\n";
            std::cout << "  Subset query matches: " << std::boolalpha << subsetQuery.match(*localId2)
                      << " (allows extra tags)\n";
            std::cout << "  Exact query matches: " << std::boolalpha << exactQuery.match(*localId2)
                      << " (no extra tags allowed)\n";
        }

        std::cout << "\n";
    }

    return 0;
}
