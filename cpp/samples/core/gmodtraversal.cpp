/**
 * @file gmodtraversal.cpp
 * @brief Demonstrates usage of vista-sdk Gmod depth-first traversal API
 * @details This sample shows how to traverse the Gmod tree using TraversalHandlerResult
 *          to control traversal flow, collect nodes at a given depth, and stop early
 */

#include <dnv/VistaSDK.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

int main()
{
    using namespace dnv::vista::sdk;

    std::cout << "=== vista-sdk GmodTraversal Sample ===\n\n";

    {
        std::cout << "1. Gmod::traverse: Counting all nodes depth-first\n";
        std::cout << "--------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());

        std::size_t total = 0;
        gmod.traverse(
            [&total]([[maybe_unused]] const TraversalPath& parents, [[maybe_unused]] const GmodNode& node)
                -> TraversalHandlerResult {
                ++total;
                return TraversalHandlerResult::Continue;
            });

        std::cout << "Total nodes visited (DFS): " << total << "\n\n";
    }

    {
        std::cout << "2. Gmod::traverse: Collecting nodes at depth 2\n";
        std::cout << "------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());

        std::vector<std::string> depth2;
        gmod.traverse([&depth2](const TraversalPath& parents, const GmodNode& node) -> TraversalHandlerResult {
            if (parents.size() == 2)
            {
                depth2.push_back(std::string(node.code()));
            }
            return TraversalHandlerResult::Continue;
        });

        std::cout << "Nodes at depth 2: " << depth2.size() << "\n";
        constexpr std::size_t maxDisplay = 5;
        for (std::size_t i = 0; i < std::min(depth2.size(), maxDisplay); ++i)
        {
            std::cout << "  " << depth2[i] << "\n";
        }
        if (depth2.size() > maxDisplay)
        {
            std::cout << "  ... and " << (depth2.size() - maxDisplay) << " more\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "3. Gmod::traverse: Stopping early after 10 nodes\n";
        std::cout << "-------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());

        std::vector<std::string> visited;
        const bool completed = gmod.traverse(
            [&visited]([[maybe_unused]] const TraversalPath& parents, const GmodNode& node) -> TraversalHandlerResult {
                visited.push_back(std::string(node.code()));
                return visited.size() >= 10 ? TraversalHandlerResult::Stop : TraversalHandlerResult::Continue;
            });

        std::cout << "Traversal completed: " << std::boolalpha << completed << "\n";
        std::cout << "First 10 nodes in DFS order:\n";
        for (const auto& code : visited)
        {
            std::cout << "  " << code << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "4. Gmod::traverse: Skipping a subtree\n";
        std::cout << "---------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());

        // Count nodes that would be visited without skipping root's first child's subtree
        std::size_t skipped = 0;
        std::string skippedCode;
        bool firstChild = true;

        gmod.traverse([&](const TraversalPath& parents, const GmodNode& node) -> TraversalHandlerResult {
            if (parents.size() == 1 && firstChild)
            {
                firstChild = false;
                skippedCode = std::string(node.code());
                return TraversalHandlerResult::SkipSubtree;
            }
            ++skipped;
            return TraversalHandlerResult::Continue;
        });

        std::cout << "Skipped subtree rooted at: " << skippedCode << "\n";
        std::cout << "Nodes visited after skip : " << skipped << "\n\n";
    }

    {
        std::cout << "5. Gmod::traverse: Using state overload\n";
        std::cout << "----------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());

        struct State
        {
            std::size_t leafCount = 0;
            std::size_t maxDepth = 0;
        };

        State state;
        Gmod::TraverseHandlerWithState<State> handler =
            [](State& s, const TraversalPath& parents, const GmodNode& node) -> TraversalHandlerResult {
            if (node.isLeafNode())
            {
                ++s.leafCount;
            }
            s.maxDepth = std::max(s.maxDepth, parents.size());
            return TraversalHandlerResult::Continue;
        };
        gmod.traverse(state, handler);

        std::cout << "Leaf nodes visited: " << state.leafCount << "\n";
        std::cout << "Max path depth    : " << state.maxDepth << "\n\n";
    }

    return 0;
}
