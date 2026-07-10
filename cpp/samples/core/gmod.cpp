/**
 * @file gmod.cpp
 * @brief Demonstrates usage of vista-sdk Gmod API
 * @details This sample shows how to access the Generic Product Model (Gmod),
 *          navigate nodes, query metadata, and work with product types/selections
 */

#include <dnv/vista/sdk/core/VIS.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

int main()
{
    using namespace dnv::vista::sdk;

    std::cout << "=== vista-sdk Gmod Sample ===\n\n";

    {
        std::cout << "1. Gmod: Accessing Gmod for a VIS version\n";
        std::cout << "--------------------------------------------\n";

        const auto& vis = VIS::instance();
        auto version = vis.latest();
        const auto& gmod = vis.gmod(version);

        std::cout << "Gmod for version   : " << VisVersions::toString(gmod.version()) << "\n";

        size_t nodeCount = 0;
        for ([[maybe_unused]] const auto& [code, node] : gmod)
        {
            nodeCount++;
        }

        std::cout << "Total nodes in Gmod: " << nodeCount << "\n";
        std::cout << "Root node code     : " << gmod.rootNode().code() << "\n";
        std::cout << "\n";
    }

    {
        std::cout << "2. GmodNode: Accessing nodes by code\n";
        std::cout << "---------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());

        // Access node using operator[]
        const auto& node1 = gmod["411.1"];
        std::cout << "Node code    : " << node1.code() << "\n";
        std::cout << "Node name    : " << node1.metadata().name() << "\n";
        std::cout << "Node category: " << node1.metadata().category() << "\n";
        std::cout << "Node type    : " << node1.metadata().type() << "\n";

        // Safe access using node
        auto node2Opt = gmod.node("C101.31");
        if (node2Opt.has_value())
        {
            std::cout << "\nFound node: " << (*node2Opt)->code() << "\n";
            std::cout << "Name      : " << (*node2Opt)->metadata().name() << "\n";
        }

        // Try invalid code
        auto invalidOpt = gmod.node("INVALID");
        std::cout << "\nInvalid code lookup: " << std::boolalpha << invalidOpt.has_value() << "\n";

        std::cout << "\n";
    }

    {
        std::cout << "3. GmodNode: Navigating the tree hierarchy\n";
        std::cout << "---------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());

        const auto& node = gmod["411.1"];

        // Parents (can have multiple in Gmod graph)
        if (!node.parents().empty())
        {
            std::cout << "Parents of " << node.code() << " (" << node.parents().size() << "):\n";
            for (const auto* parent : node.parents())
            {
                std::cout << "  - " << parent->code() << "\n";
            }
        }

        // Children
        std::cout << "\nChildren of " << node.code() << " (" << node.children().size() << "):\n";
        size_t maxDisplay = 5;
        size_t count = 0;
        for (const auto* child : node.children())
        {
            if (count++ >= maxDisplay)
            {
                std::cout << "  ... and " << (node.children().size() - maxDisplay) << " more\n";
                break;
            }
            std::cout << "  - " << child->code() << " (" << child->metadata().name() << ")\n";
        }

        // Root check
        std::cout << "\nIs root  ? " << std::boolalpha << node.isRoot() << "\n";
        std::cout << "Root node: " << gmod.rootNode().code() << "\n";
        std::cout << "Is root  ? " << std::boolalpha << gmod.rootNode().isRoot() << "\n";

        std::cout << "\n";
    }

    {
        std::cout << "4. GmodNode: Node metadata\n";
        std::cout << "----------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());

        const auto& node = gmod["S222.11"];
        const auto& metadata = node.metadata();

        std::cout << "Node                : " << node.code() << "\n";
        std::cout << "Category            : " << metadata.category() << "\n";
        std::cout << "Type                : " << metadata.type() << "\n";
        std::cout << "Name                : " << metadata.name() << "\n";

        if (metadata.definition().has_value())
        {
            std::cout << "Definition      : " << metadata.definition().value() << "\n";
        }

        if (metadata.installSubstructure().has_value())
        {
            std::cout << "Install substructure: " << metadata.installSubstructure().value() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "5. GmodNode: Product types and selections\n";
        std::cout << "--------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());

        // Product type example
        const auto& node1 = gmod["411.3"];
        std::cout << "Node             : " << node1.code() << "\n";
        std::cout << "Has product type : " << std::boolalpha << node1.productType().has_value() << "\n";
        if (auto prodTypeOpt = node1.productType())
        {
            const GmodNode* prodType = prodTypeOpt.value();
            std::cout << "Product type code: " << prodType->code() << "\n";
            std::cout << "Product type name: " << prodType->metadata().name() << "\n";
        }

        // Product selection example
        const auto& node2 = gmod["411.2"];
        std::cout << "\nNode: " << node2.code() << "\n";
        std::cout << "Has product selection : " << std::boolalpha << node2.productSelection().has_value() << "\n";
        if (auto prodSelOpt = node2.productSelection())
        {
            const GmodNode* prodSel = prodSelOpt.value();
            std::cout << "Product selection code: " << prodSel->code() << "\n";
            std::cout << "Product selection name: " << prodSel->metadata().name() << "\n";
        }

        // Check if node is product selection
        auto csNode = gmod.node("CS1");
        if (csNode.has_value())
        {
            std::cout << "\nNode: " << (*csNode)->code() << "\n";
            std::cout << "Is product selection: " << std::boolalpha << (*csNode)->isProductSelection() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "6. GmodNode: Mappability\n";
        std::cout << "--------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());

        std::vector<std::string> testCodes = {
            "VE",      // Root - not mappable
            "411",     // Mappable
            "411.1",   // Not mappable
            "C101",    // Mappable
            "C101.31", // Mappable
            "CS1"      // Product selection - not mappable
        };

        std::cout << std::boolalpha;
        for (const auto& code : testCodes)
        {
            auto nodeOpt = gmod.node(code);
            if (nodeOpt.has_value())
            {
                std::cout << "  " << std::setw(7) << std::left << code;
                std::cout << ": " << (*nodeOpt)->isMappable() << "\n";
            }
        }

        std::cout << "\n";
    }

    {
        std::cout << "7. Gmod: Iterating all nodes\n";
        std::cout << "------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());

        std::cout << "First 10 nodes:\n";
        size_t count = 0;
        for (const auto& [code, node] : gmod)
        {
            if (count++ >= 10)
            {
                break;
            }
            std::cout << "  " << std::setw(8) << std::left << code;
            std::cout << " - " << node.metadata().name() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "8. GmodNode: Finding specific node types\n";
        std::cout << "-------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());

        // Find PRODUCT TYPE nodes
        std::cout << "Sample PRODUCT TYPE nodes:\n";
        size_t count = 0;
        for (const auto& [code, node] : gmod)
        {
            if (node.metadata().category() == "PRODUCT" && node.metadata().type() == "TYPE")
            {
                if (count++ >= 5)
                {
                    break;
                }
                std::cout << "  " << std::setw(10) << std::left << code;
                std::cout << " - " << node.metadata().name() << "\n";
            }
        }

        // Find ASSET FUNCTION LEAF nodes
        std::cout << "\nSample ASSET FUNCTION LEAF nodes:\n";
        count = 0;
        for (const auto& [code, node] : gmod)
        {
            if (node.isLeafNode() && node.metadata().category() == "ASSET FUNCTION")
            {
                if (count++ >= 5)
                {
                    break;
                }
                std::cout << "  " << std::setw(10) << std::left << code;
                std::cout << " - " << node.metadata().name() << "\n";
            }
        }

        std::cout << "\n";
    }

    {
        std::cout << "9. GmodNode: Tree navigation example\n";
        std::cout << "----------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());

        // Navigate from a node to root
        const auto& node = gmod["C101.31"];
        std::cout << "Path from " << node.code() << " to root:\n";

        const GmodNode* current = &node;
        int depth = 0;
        std::unordered_set<std::string_view> visited;
        while (current != nullptr)
        {
            if (!visited.insert(current->code()).second)
            {
                std::cout << std::string(depth * 2, ' ') << "- [cycle detected at " << current->code() << "]\n";
                break;
            }

            std::cout << std::string(depth * 2, ' ') << "- " << current->code();
            std::cout << " (" << current->metadata().name() << ")\n";

            current = current->parents().empty() ? nullptr : current->parents()[0];
            ++depth;
        }

        std::cout << "\n";
    }

    {
        std::cout << "10. GmodNode: Node properties comparison\n";
        std::cout << "-------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());

        std::vector<std::string> codes = { "411.1", "C101", "CS1", "F201" };

        std::cout << std::left;
        std::cout << std::setw(12) << "Code";
        std::cout << std::setw(20) << "Category";
        std::cout << std::setw(15) << "Type";
        std::cout << std::setw(12) << "Mappable";
        std::cout << std::setw(12) << "IsLeaf";
        std::cout << "\n";
        std::cout << std::string(71, '-') << "\n";

        std::cout << std::boolalpha;
        for (const auto& code : codes)
        {
            auto nodeOpt = gmod.node(code);
            if (nodeOpt.has_value())
            {
                const auto& node = **nodeOpt;
                std::cout << std::setw(12) << code;
                std::cout << std::setw(20) << node.metadata().category();
                std::cout << std::setw(15) << node.metadata().type();
                std::cout << std::setw(12) << node.isMappable();
                std::cout << std::setw(12) << node.isLeafNode();
                std::cout << "\n";
            }
        }

        std::cout << "\n";
    }

    {
        std::cout << "11. Gmod: Working with different versions\n";
        std::cout << "--------------------------------------------\n";

        const auto& vis = VIS::instance();

        // Compare node counts across versions
        std::cout << "Node counts across VIS versions:\n";
        for (const auto& version : vis.versions())
        {
            const auto& gmod = vis.gmod(version);
            size_t count = 0;
            for ([[maybe_unused]] const auto& [code, node] : gmod)
            {
                count++;
            }

            std::cout << "  " << std::setw(5) << std::left << VisVersions::toString(version);
            std::cout << ": " << std::setw(4) << std::right << count << " nodes\n";
        }

        std::cout << "\n";
    }

    return 0;
}
