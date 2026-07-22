/**
 * @file gmodpath.cpp
 * @brief Demonstrates usage of vista-sdk GmodPath API
 * @details This sample shows how to parse Gmod paths (short and full format),
 *          navigate parsed paths, handle individualization, and work with path errors
 */

#include <dnv/VistaSDK.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

int main()
{
    using namespace dnv::vista::sdk;

    std::cout << "=== vista-sdk GmodPath Sample ===\n\n";

    {
        std::cout << "1. GmodPath: Parsing short paths\n";
        std::cout << "-----------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());
        const auto& locations = vis.locations(vis.latest());

        // Parse simple short path
        auto path1 = GmodPath::fromShortPath("411.1", gmod, locations);
        if (path1.has_value())
        {
            std::cout << "Parsed: \"411.1\"\n";
            std::cout << "  Node     : " << path1->node().code() << "\n";
            std::cout << "  Full path: " << path1->toFullPathString() << "\n";
        }

        // Parse path with location
        auto path2 = GmodPath::fromShortPath("411.1-1P", gmod, locations);
        if (path2.has_value())
        {
            std::cout << "\nParsed: \"411.1-1P\"\n";
            std::cout << "  Node     : " << path2->node().code() << "\n";
            if (path2->node().location().has_value())
            {
                std::cout << "  Location : " << path2->node().location()->value() << "\n";
            }
            std::cout << "  Full path: " << path2->toFullPathString() << "\n";
        }

        // Parse multi-segment path
        auto path3 = GmodPath::fromShortPath("612.21-1/C701.13/S93", gmod, locations);
        if (path3.has_value())
        {
            std::cout << "\nParsed: \"612.21-1/C701.13/S93\"\n";
            std::cout << "  Final node : " << path3->node().code() << "\n";
            std::cout << "  Path length: " << path3->length() << " nodes\n";
            std::cout << "  Full path  : " << path3->toFullPathString() << "\n";
        }

        // Parse multi-segment path with errors
        ParsingErrors errors;
        auto path4 = GmodPath::fromShortPath("C101.63-2P/S206.22", gmod, locations, errors);
        if (path4.has_value())
        {
            std::cout << "\nParsed: \"C101.63-2P/S206.22\"\n";
            std::cout << "  Final node : " << path4->node().code() << "\n";
            std::cout << "  Path length: " << path4->length() << " nodes\n";
            std::cout << "  Full path  : " << path4->toFullPathString() << "\n";
        }
        else if (errors.hasErrors())
        {
            std::cout << "\nFailed to parse \"C101.63-2P/S206.22\":\n";
            for (const auto& error : errors)
            {
                std::cout << "  - " << error.message << "\n";
            }
        }

        std::cout << "\n";
    }

    {
        std::cout << "2. GmodPath: Parsing full paths\n";
        std::cout << "----------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());
        const auto& locations = vis.locations(vis.latest());

        // Full path starting from root (VE)
        auto path = GmodPath::fromFullPath("VE/400a/410/411/411i/411.1-1P", gmod, locations);
        if (path.has_value())
        {
            std::cout << "Parsed full path: \"VE/400a/410/411/411i/411.1-1P\"\n";
            std::cout << "  Final node : " << path->node().code() << "\n";
            std::cout << "  Path length: " << path->length() << " nodes\n";
            if (path->node().location().has_value())
            {
                std::cout << "  Location   : " << path->node().location()->value() << "\n";
            }
        }

        // Complex full path
        auto path2 = GmodPath::fromFullPath(
            "VE/600a/610/612/612.2/612.2i-1/612.21-1/CS10/C701/C701.1/C701.13/S93", gmod, locations);
        if (path2.has_value())
        {
            std::cout << "\nParsed: \"VE/600a/610/612/612.2/612.2i-1/612.21-1/CS10/C701/C701.1/C701.13/S93\"\n";
            std::cout << "  Final node : " << path2->node().code() << "\n";
            std::cout << "  Path length: " << path2->length() << " nodes\n";
            std::cout << "  Full path  : " << path2->toFullPathString() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "3. GmodPath: Invalid paths with error handling\n";
        std::cout << "-------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());
        const auto& locations = vis.locations(vis.latest());

        std::vector<std::string> invalidPaths = {
            "",                 // Empty path
            "INVALID",          // Non-existent node
            "411.1-XYZ",        // Invalid location
            "VE/INVALID/411.1", // Invalid node in full path
        };

        for (const auto& pathStr : invalidPaths)
        {
            ParsingErrors errors;
            auto path = GmodPath::fromShortPath(pathStr, gmod, locations, errors);

            std::cout << "Path: \"" << pathStr << "\"\n";
            if (path.has_value())
            {
                std::cout << "  Valid (unexpected!)\n";
            }
            else
            {
                std::cout << "  Invalid - Errors:\n";
                for (const auto& error : errors)
                {
                    std::cout << "    - " << error.message << "\n";
                }
            }
            std::cout << "\n";
        }
    }

    {
        std::cout << "4. GmodPath: Navigating path nodes\n";
        std::cout << "-------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());
        const auto& locations = vis.locations(vis.latest());

        auto path = GmodPath::fromFullPath(
            "VE/600a/610/612/612.2/612.2i-1/612.21-1/CS10/C701/C701.1/C701.13/S93", gmod, locations);
        if (path.has_value())
        {
            std::cout << "Path: " << path->toFullPathString() << "\n";
            std::cout << "\nAll nodes in path:\n";

            // Iterate through parent nodes
            size_t index = 0;
            for (const auto& parent : path->parents())
            {
                std::string nodeStr(parent.code());
                if (parent.location().has_value())
                {
                    nodeStr += "-" + parent.location()->value();
                }
                std::cout << "  [" << std::setw(2) << std::right << index++ << "] " << std::setw(12) << std::left
                          << nodeStr;
                std::cout << " (" << parent.metadata().name() << ")\n";
            }

            // Show the final target node
            const auto& node = path->node();
            std::string nodeStr(node.code());
            if (node.location().has_value())
            {
                nodeStr += "-" + node.location()->value();
            }
            std::cout << "  [" << std::setw(2) << std::right << index << "] " << std::setw(12) << std::left << nodeStr;
            std::cout << " (" << node.metadata().name() << ")\n";

            std::cout << "\nFinal node:\n";
            std::cout << "  Code    : " << path->node().code() << "\n";
            std::cout << "  Name    : " << path->node().metadata().name() << "\n";
            std::cout << "  Category: " << path->node().metadata().category() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "5. GmodPath: Individualization\n";
        std::cout << "--------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());
        const auto& locations = vis.locations(vis.latest());

        // Parse path that can be individualized (411i and 411.1 are in the same individualizable set)
        auto path = GmodPath::fromShortPath("411.1/C101.62/S205", gmod, locations);
        if (path.has_value())
        {
            std::cout << "Path: " << path->toString() << "\n";
            std::cout << "Is individualizable: " << std::boolalpha << path->isIndividualizable() << "\n";

            auto sets = path->individualizableSets();
            std::cout << "Individualizable sets: " << sets.size() << "\n";

            for (size_t i = 0; i < sets.size(); ++i)
            {
                const auto& set = sets[i];
                std::cout << "\nSet " << (i + 1) << ":\n";
                std::cout << "  Nodes  : " << set.toString() << "\n";
                std::cout << "  Indices: ";
                for (int idx : set.nodeIndices())
                {
                    std::cout << idx << " ";
                }
                std::cout << "\n";
            }

            // Parse the same path individualized (location applied)
            auto indPath = GmodPath::fromShortPath("411.1-1/C101.62/S205", gmod, locations);
            if (indPath.has_value())
            {
                std::cout << "\nIndividualized: " << indPath->toString() << "\n";
                std::cout << "Full path     : " << indPath->toFullPathString() << "\n";
            }
        }

        std::cout << "\n";
    }

    {
        std::cout << "6. GmodPath: Conversion between formats\n";
        std::cout << "------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());
        const auto& locations = vis.locations(vis.latest());

        // Parse short path
        auto shortPath = GmodPath::fromShortPath("411.1-1P", gmod, locations);
        if (shortPath.has_value())
        {
            std::cout << "Short path:" << shortPath->toString() << "\n";
            std::cout << "Full path : " << shortPath->toFullPathString() << "\n";
        }

        // Parse full path
        auto fullPath = GmodPath::fromFullPath("VE/400a/410/411/411i/411.1-1P", gmod, locations);
        if (fullPath.has_value())
        {
            std::cout << "\nFull path : " << fullPath->toFullPathString() << "\n";
            std::cout << "Short path: " << fullPath->toString() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "7. GmodPath: Comparison and equality\n";
        std::cout << "---------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());
        const auto& locations = vis.locations(vis.latest());

        auto path1 = GmodPath::fromShortPath("411.1-1P", gmod, locations);
        auto path2 = GmodPath::fromFullPath("VE/400a/410/411/411i/411.1-1P", gmod, locations);
        auto path3 = GmodPath::fromShortPath("411.1-2P", gmod, locations);

        std::cout << std::boolalpha;
        if (path1.has_value() && path2.has_value() && path3.has_value())
        {
            std::cout << "Path 1: " << path1->toString() << "\n";
            std::cout << "Path 2: " << path2->toString() << "\n";
            std::cout << "Path 3: " << path3->toString() << "\n\n";

            std::cout << "Path 1 == Path 2? " << (*path1 == *path2) << "\n";
            std::cout << "Path 1 == Path 3? " << (*path1 == *path3) << "\n";
            std::cout << "Path 1 != Path 3? " << (*path1 != *path3) << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "8. GmodPath: Working with product types/selections\n";
        std::cout << "-----------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());
        const auto& locations = vis.locations(vis.latest());

        // Example 1: Node with product selection
        auto path1 = GmodPath::fromShortPath("411.1-1P", gmod, locations);
        if (path1.has_value())
        {
            const auto& node = path1->node();
            std::cout << "Path                 : " << path1->toString() << "\n";
            std::cout << "Node code            : " << node.code() << "\n";
            std::cout << "Is product selection : " << std::boolalpha << node.isProductSelection() << "\n";

            if (node.productSelection().has_value())
            {
                std::cout << "Has product selection: " << (*node.productSelection())->code() << "\n";
            }
        }

        // Example 2: Product node (from the boiler path)
        auto path2 = GmodPath::fromShortPath("612.21-1/C701.13/S93", gmod, locations);
        if (path2.has_value())
        {
            const auto& node = path2->node();
            std::cout << "\nPath      : " << path2->toString() << "\n";
            std::cout << "Node code : " << node.code() << " (" << node.metadata().name() << ")\n";
            std::cout << "Category  : " << node.metadata().category() << "\n";
            std::cout << "Is product: " << std::boolalpha << (node.metadata().category() == "PRODUCT") << "\n";
        }

        std::cout << "\n";
    }

    return 0;
}
