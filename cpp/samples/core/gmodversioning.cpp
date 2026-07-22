/**
 * @file gmodversioning.cpp
 * @brief Demonstrates usage of vista-sdk VIS versioning API
 * @details This sample shows how to convert Gmod nodes, paths, and LocalIds
 *          between different VIS versions using the VIS singleton API
 */

#include <dnv/VistaSDK.h>

#include <iomanip>
#include <iostream>
#include <string>

int main()
{
    using namespace dnv::vista::sdk;

    std::cout << "=== vista-sdk VIS Versioning Sample ===\n\n";

    {
        std::cout << "1. VIS: Available versions\n";
        std::cout << "----------------------------\n";

        const auto& vis = VIS::instance();

        std::cout << "Latest VIS version: " << VisVersions::toString(vis.latest()) << "\n";
        std::cout << "\nAll available versions:\n";
        for (const auto& version : vis.versions())
        {
            std::cout << "  - " << VisVersions::toString(version) << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "2. VIS::convertNode: Converting individual Gmod nodes\n";
        std::cout << "-------------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& sourceGmod = vis.gmod(VisVersion::v3_4a);

        // Simple node code change
        const auto& sourceNode1 = sourceGmod["323.5"];
        auto targetNode1 = vis.convertNode(VisVersion::v3_4a, sourceNode1, VisVersion::v3_6a);

        std::cout << "Converting node '323.5' from v3.4a to v3.6a:\n";
        std::cout << "  Source: " << sourceNode1.code() << " (" << sourceNode1.metadata().name() << ")\n";
        if (targetNode1.has_value())
        {
            std::cout << "  Target: " << targetNode1->code() << " (" << targetNode1->metadata().name() << ")\n";
        }

        // Node with bigger change
        const auto& sourceNode2 = sourceGmod["412.72"];
        auto targetNode2 = vis.convertNode(VisVersion::v3_4a, sourceNode2, vis.latest());

        std::cout << "\nConverting node '412.72' from v3.4a to " << VisVersions::toString(vis.latest()) << ":\n";
        std::cout << "  Source: " << sourceNode2.code() << " (" << sourceNode2.metadata().name() << ")\n";
        if (targetNode2.has_value())
        {
            std::cout << "  Target: " << targetNode2->code() << " (" << targetNode2->metadata().name() << ")\n";
        }

        // Node that stays the same
        const auto& sourceNode3 = sourceGmod["1014.211"];
        auto targetNode3 = vis.convertNode(VisVersion::v3_4a, sourceNode3, VisVersion::v3_6a);

        std::cout << "\nConverting node '1014.211' from v3.4a to v3.6a:\n";
        std::cout << "  Source: " << sourceNode3.code() << "\n";
        if (targetNode3.has_value())
        {
            std::cout << "  Target: " << targetNode3->code() << " (unchanged)\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "3. VIS::convertPath: Converting Gmod paths (simple code changes)\n";
        std::cout << "-------------------------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& sourceGmod = vis.gmod(VisVersion::v3_4a);
        const auto& sourceLocations = vis.locations(VisVersion::v3_4a);

        // Simple path with code change
        auto sourcePath1 = GmodPath::fromShortPath("323.51/H362.1", sourceGmod, sourceLocations);
        if (sourcePath1.has_value())
        {
            auto targetPath1 = vis.convertPath(VisVersion::v3_4a, *sourcePath1, VisVersion::v3_6a);

            std::cout << "Converting path '323.51/H362.1' from v3.4a to v3.6a:\n";
            std::cout << "  Source: " << sourcePath1->toString() << "\n";
            if (targetPath1.has_value())
            {
                std::cout << "  Target: " << targetPath1->toString() << "\n";
            }
        }

        // Multi-segment path
        auto sourcePath2 = GmodPath::fromShortPath("511.11/C101.663i/C663.5/CS6d", sourceGmod, sourceLocations);
        if (sourcePath2.has_value())
        {
            auto targetPath2 = vis.convertPath(VisVersion::v3_4a, *sourcePath2, VisVersion::v3_6a);

            std::cout << "\nConverting path '511.11/C101.663i/C663.5/CS6d' from v3.4a to v3.6a:\n";
            std::cout << "  Source: " << sourcePath2->toString() << "\n";
            if (targetPath2.has_value())
            {
                std::cout << "  Target: " << targetPath2->toString() << "\n";
            }
        }

        std::cout << "\n";
    }

    {
        std::cout << "4. VIS::convertPath: Converting paths with depth changes\n";
        std::cout << "-----------------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& sourceGmod = vis.gmod(VisVersion::v3_4a);
        const auto& targetGmod = vis.gmod(VisVersion::v3_6a);
        const auto& sourceLocations = vis.locations(VisVersion::v3_4a);
        const auto& targetLocations = vis.locations(VisVersion::v3_6a);

        // Path where intermediate node is added (depth increases)
        auto sourcePath = GmodPath::fromShortPath("511.331/C221", sourceGmod, sourceLocations);

        if (sourcePath.has_value())
        {
            std::cout << "Converting path '511.331/C221' from v3.4a to v3.6a:\n";
            std::cout << "  Source: " << sourcePath->toString() << " (depth: " << sourcePath->length() << ")\n";

            auto targetPath = vis.convertPath(VisVersion::v3_4a, *sourcePath, VisVersion::v3_6a);

            if (targetPath.has_value())
            {
                std::cout << "  Target: " << targetPath->toString() << " (depth: " << targetPath->length() << ")\n";
                std::cout << "  Note: Depth changed from " << sourcePath->length() << " to " << targetPath->length()
                          << " nodes\n";

                // Verify against expected
                auto expected = GmodPath::fromShortPath("511.31/C121.31/C221", targetGmod, targetLocations);
                if (expected.has_value() && *targetPath == *expected)
                {
                    std::cout << "  OK: Conversion matches expected path\n";
                }
            }
            else
            {
                std::cout << "  ERROR: Conversion failed\n";
            }
        }

        std::cout << "\n";
    }

    {
        std::cout << "5. VIS::convertPath: Converting paths with locations\n";
        std::cout << "-------------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& sourceGmod = vis.gmod(VisVersion::v3_7a);
        const auto& sourceLocations = vis.locations(VisVersion::v3_7a);

        // Path with location that is preserved during conversion
        auto sourcePath = GmodPath::fromShortPath("691.811i-A/H101.11-1", sourceGmod, sourceLocations);

        if (sourcePath.has_value())
        {
            std::cout << "Converting path '691.811i-A/H101.11-1' from v3.7a to v3.9a:\n";
            std::cout << "  Source: " << sourcePath->toString() << "\n";
            std::cout << "  Source has location: " << std::boolalpha << sourcePath->node().location().has_value()
                      << "\n";

            auto targetPath = vis.convertPath(VisVersion::v3_7a, *sourcePath, VisVersion::v3_9a);

            if (targetPath.has_value())
            {
                std::cout << "  Target: " << targetPath->toString() << "\n";
                std::cout << "  Target has location: " << std::boolalpha << targetPath->node().location().has_value()
                          << "\n";

                if (sourcePath->node().location().has_value() && targetPath->node().location().has_value())
                {
                    std::cout << "  OK: Location preserved: " << targetPath->node().location()->value() << "\n";
                }
            }
        }

        std::cout << "\n";
    }

    {
        std::cout << "6. VIS::convertPath: Converting to latest version\n";
        std::cout << "----------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& sourceGmod = vis.gmod(VisVersion::v3_4a);
        const auto& sourceLocations = vis.locations(VisVersion::v3_4a);

        // Convert multiple paths to latest version
        std::vector<std::string> pathStrings = { "411.1/C101.72/I101",
                                                 "1012.21/C1147.221/C1051.7/C101.22",
                                                 "632.32i/S110.2/C111.42/G203.31/S90.5/C401" };

        std::cout << "Converting paths from v3.4a to latest (" << VisVersions::toString(vis.latest()) << "):\n";

        for (const auto& pathStr : pathStrings)
        {
            auto sourcePath = GmodPath::fromShortPath(pathStr, sourceGmod, sourceLocations);
            if (sourcePath.has_value())
            {
                auto targetPath = vis.convertPath(*sourcePath, vis.latest());
                std::cout << "  " << std::setw(45) << std::left << pathStr;
                if (targetPath.has_value())
                {
                    std::cout << " -> " << targetPath->toString() << "\n";
                }
                else
                {
                    std::cout << " -> FAILED\n";
                }
            }
        }

        std::cout << "\n";
    }

    {
        std::cout << "7. VIS::convertLocalId: Converting LocalIds between versions\n";
        std::cout << "---------------------------------------------------------------\n";

        const auto& vis = VIS::instance();

        // Parse a LocalId in v3.4a
        std::string sourceLocalIdStr = "/dnv-v2/vis-3-4a/411.1/C101/sec/411.1/C101.64i/S201/meta/cnt-condensate";
        auto sourceLocalId = LocalId::fromString(sourceLocalIdStr);

        if (sourceLocalId.has_value())
        {
            std::cout << "Source LocalId (v3.4a):\n";
            std::cout << "  " << sourceLocalId->toString() << "\n";

            // Convert to v3.5a
            auto targetLocalId = vis.convertLocalId(*sourceLocalId, VisVersion::v3_5a);

            if (targetLocalId.has_value())
            {
                std::cout << "\nTarget LocalId (v3.5a):\n";
                std::cout << "  " << targetLocalId->toString() << "\n";
                std::cout << "\n  Note: 'C101.64i' converted to 'C101.64'\n";
            }
        }

        // More complex LocalId conversion
        std::string sourceLocalIdStr2 =
            "/dnv-v2/vis-3-4a/411.1/C101.64i-1/S201.1/C151.2/S110/meta/cnt-hydraulic.oil/state-running";
        auto sourceLocalId2 = LocalId::fromString(sourceLocalIdStr2);

        if (sourceLocalId2.has_value())
        {
            std::cout << "\nSource LocalId (v3.4a):\n";
            std::cout << "  " << sourceLocalId2->toString() << "\n";

            // Convert to v3.9a
            auto targetLocalId2 = vis.convertLocalId(*sourceLocalId2, VisVersion::v3_9a);

            if (targetLocalId2.has_value())
            {
                std::cout << "\nTarget LocalId (v3.9a):\n";
                std::cout << "  " << targetLocalId2->toString() << "\n";
            }
        }

        std::cout << "\n";
    }

    {
        std::cout << "8. VIS batch methods: Loading multiple versions at once\n";
        std::cout << "----------------------------------------------------------\n";

        const auto& vis = VIS::instance();

        auto versions = vis.versions();

        std::cout << "Loading Gmods for all versions (" << VisVersions::toString(versions.front()) << " -> "
                  << VisVersions::toString(versions.back()) << "):\n";

        // Load all Gmods in one call (with validation and deduplication)
        auto gmodMap = vis.gmodsMap(versions);
        std::cout << "  Loaded " << gmodMap.size() << " Gmod(s)\n";

        // Load all Codebooks in one call
        auto codebooksMap = vis.codebooksMap(versions);
        std::cout << "  Loaded " << codebooksMap.size() << " Codebooks\n";

        // Load all Locations in one call
        auto locationsMap = vis.locationsMap(versions);
        std::cout << "  Loaded " << locationsMap.size() << " Locations\n";

        // Demonstrate usage: count unique nodes in each version
        std::cout << "\nUnique node count per version:\n";
        for (const auto& version : versions)
        {
            const auto& gmod = gmodMap.at(version);
            size_t nodeCount = 0;
            for ([[maybe_unused]] const auto& [code, node] : gmod)
            {
                ++nodeCount;
            }
            std::cout << "  v" << VisVersions::toString(version) << ": " << nodeCount << " nodes\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "9. VIS::convertPath: Chained conversion through all versions\n";
        std::cout << "---------------------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& versions = vis.versions();

        constexpr std::string_view startPath = "511.36/I101";
        const VisVersion startVersion = VisVersion::v3_4a;

        std::cout << "Tracing '" << startPath << "' from " << VisVersions::toString(startVersion)
                  << " through all versions:\n\n";

        auto currentPath = GmodPath::fromShortPath(startPath, vis.gmod(startVersion), vis.locations(startVersion));
        if (!currentPath.has_value())
        {
            std::cout << "  ERROR: Failed to parse source path\n";
        }
        else
        {
            size_t maxLen = 0;
            for (const auto& v : versions)
            {
                maxLen = std::max(maxLen, VisVersions::toString(v).size());
            }
            const int colWidth = static_cast<int>(maxLen);

            std::cout << "  " << std::setw(colWidth) << std::left << VisVersions::toString(startVersion) << ": "
                      << currentPath->toString() << "\n";

            for (size_t i = 1; i < versions.size(); ++i)
            {
                const VisVersion sourceVersion = versions[i - 1];
                const VisVersion targetVersion = versions[i];

                auto nextPath = vis.convertPath(sourceVersion, *currentPath, targetVersion);
                std::cout << "  " << std::setw(colWidth) << std::left << VisVersions::toString(targetVersion) << ": ";

                if (nextPath.has_value())
                {
                    std::cout << nextPath->toString() << "\n";
                    currentPath = std::move(nextPath);
                }
                else
                {
                    std::cout << "(conversion failed)\n";
                    break;
                }
            }
        }

        std::cout << "\n";
    }

    return 0;
}
