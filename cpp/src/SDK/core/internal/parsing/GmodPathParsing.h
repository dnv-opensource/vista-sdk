/**
 * @file GmodPathParsing.h
 * @brief Internal parsing functions for Gmod path string conversion
 */

#pragma once

#include "dnv/vista/sdk/core/Gmod.h"
#include "dnv/vista/sdk/core/GmodPath.h"
#include "dnv/vista/sdk/core/Locations.h"
#include "dnv/vista/sdk/utils/StringUtils.h"
#include "GmodParsePathResult.h"
#include "LocationSetsVisitor.h"

#include <vector>

namespace dnv::vista::sdk::internal
{
    constexpr size_t MAX_LOCATION_SETS = 16;

    constexpr const char* ERROR_EMPTY_ITEM = "Item is empty";
    constexpr const char* ERROR_PREFIX_FAILED_NODE = "Failed to get GmodNode for ";

    /** @brief Node code and optional location for a single segment of a short path being parsed */
    struct PathNode
    {
        std::string_view code;
        std::optional<Location> location;

        explicit PathNode(std::string_view c)
            : code{ c },
              location{ std::nullopt }
        {}

        PathNode(std::string_view c, const Location& loc)
            : code{ c },
              location{ loc }
        {}
    };

    /** @brief Mutable state threaded through gmod.traverse() during short path parsing */
    struct ParseContext
    {
        std::vector<PathNode> parts;
        size_t currentPartIndex;
        PathNode toFind;
        std::optional<std::unordered_map<std::string, Location>> locations;
        std::optional<GmodPath> path;
        const Gmod* gmod;

        ParseContext(
            std::vector<PathNode>&& pathNodeVector,
            PathNode&& t,
            std::optional<std::unordered_map<std::string, Location>>&& l,
            std::optional<GmodPath>&& gmodPath,
            const Gmod& g)
            : parts{ std::move(pathNodeVector) },
              currentPartIndex{ 0 },
              toFind{ std::move(t) },
              locations{ std::move(l) },
              path{ std::move(gmodPath) },
              gmod{ &g }
        {}
    };

    /**
     * @brief Traversal handler called by gmod.traverse() during short path parsing
     * @return TraversalHandlerResult indicating whether to continue, skip subtree, or stop
     */
    inline TraversalHandlerResult parseHandler(
        ParseContext& context, const TraversalPath& parents, const GmodNode& current)
    {
        PathNode& toFind = context.toFind;
        bool found = (current.code() == toFind.code);

        if (!found && current.isLeafNode())
        {
            return TraversalHandlerResult::SkipSubtree;
        }

        if (!found)
        {
            return TraversalHandlerResult::Continue;
        }

        if (toFind.location.has_value())
        {
            if (!context.locations.has_value())
            {
                context.locations = std::unordered_map<std::string, Location>{};
                context.locations->reserve(8);
            }
            context.locations->insert_or_assign(std::string{ toFind.code }, toFind.location.value());
        }
        if (context.currentPartIndex < context.parts.size())
        {
            toFind = context.parts[context.currentPartIndex++];
            return TraversalHandlerResult::Continue;
        }

        // Determine starting node for prefix building
        const GmodNode* firstParent = parents.empty() ? nullptr : parents[0];
        const GmodNode* startNode = nullptr;

        if (firstParent && firstParent->parents().size() == 1)
        {
            startNode = firstParent->parents()[0];
        }
        else if (current.parents().size() == 1)
        {
            startNode = current.parents()[0];
        }

        if (startNode == nullptr || startNode->parents().size() > 1)
        {
            return TraversalHandlerResult::Stop;
        }

        // Build list of prefix node pointers (just pointers, no copy yet)
        TraversalPath prefixNodePtrs;
        std::vector<std::string_view> seenCodes;

        // Mark existing parents as seen
        for (const GmodNode* parent : parents)
        {
            seenCodes.push_back(parent->code());
        }

        // Walk up to find prefix nodes
        while (startNode->parents().size() == 1)
        {
            bool alreadySeen = false;
            for (const auto& code : seenCodes)
            {
                if (code == startNode->code())
                {
                    alreadySeen = true;
                    break;
                }
            }
            if (!alreadySeen)
            {
                prefixNodePtrs.push_back(startNode);
                seenCodes.push_back(startNode->code());
            }

            startNode = startNode->parents()[0];
            if (startNode->parents().size() > 1)
            {
                return TraversalHandlerResult::Stop;
            }
        }

        // Check if root needed
        bool rootFound = false;
        for (const auto& code : seenCodes)
        {
            if (code == context.gmod->rootNode().code())
            {
                rootFound = true;
                break;
            }
        }
        if (!rootFound)
        {
            prefixNodePtrs.push_back(&context.gmod->rootNode());
        }

        // Now build final pathParents in ONE pass with exact size
        std::vector<GmodNode> pathParents;
        pathParents.reserve(prefixNodePtrs.size() + parents.size());

        // Add prefix nodes in reverse order (they were built backwards)
        for (size_t i = prefixNodePtrs.size(); i > 0; --i)
        {
            pathParents.emplace_back(*prefixNodePtrs[i - 1]);
        }

        // Add traversal parents with locations if needed
        for (const GmodNode* parent : parents)
        {
            pathParents.emplace_back(*parent);
            if (context.locations.has_value())
            {
                auto locIt = context.locations->find(std::string{ parent->code() });
                if (locIt != context.locations->end())
                {
                    pathParents.back().setLocation(locIt->second);
                }
            }
        }

        // Create endNode with location if needed
        GmodNode endNode = current;
        if (toFind.location.has_value())
        {
            endNode.setLocation(toFind.location.value());
        }

        internal::LocationSetsVisitor visitor;

        for (size_t i = 0; i < pathParents.size() + 1; ++i)
        {
            GmodNode& n = (i < pathParents.size()) ? pathParents[i] : endNode;

            auto set = visitor.visit(n, i, pathParents, endNode);
            if (!set.has_value())
            {
                if (n.location().has_value())
                {
                    return TraversalHandlerResult::Stop;
                }
                continue;
            }

            const auto& [setStart, setEnd, location] = set.value();
            if (setStart == setEnd)
            {
                continue;
            }

            if (!location.has_value())
            {
                continue;
            }

            for (size_t j = setStart; j <= setEnd; ++j)
            {
                if (j < pathParents.size())
                {
                    pathParents[j].setLocation(*location);
                }
                else
                {
                    endNode.setLocation(*location);
                }
            }
        }

        context.path = GmodPath{ std::move(pathParents), std::move(endNode), true /* skipVerify */ };

        return TraversalHandlerResult::Stop;
    }

    /**
     * @brief Parse a short Gmod path string with detailed error reporting
     * @param item Short path string (e.g., "411.1/513-14C")
     * @param gmod Gmod instance to resolve nodes from
     * @param locations Locations instance to parse location suffixes
     * @return GmodParsePathResult containing either the parsed GmodPath or detailed error message
     * @details Uses tree traversal with gmod.traverse() to find complete paths from root.
     *          More complex than fromFullPathString as it must search the tree.
     */
    inline GmodParsePathResult fromShortPath(
        std::string_view item, const Gmod& gmod, const Locations& locations) noexcept
    {
        try
        {
            if (gmod.version() != locations.version())
            {
                return GmodParsePathResult::error("Got different VIS versions for Gmod and Locations arguments");
            }

            if (item.empty())
            {
                return GmodParsePathResult::error(ERROR_EMPTY_ITEM);
            }

            item = string::trim(item);
            if (item.empty())
            {
                return GmodParsePathResult::error(ERROR_EMPTY_ITEM);
            }

            if (!item.empty() && item[0] == '/')
            {
                item = item.substr(1);
            }

            std::vector<PathNode> parts;

            for (std::string_view remaining = item; !remaining.empty();)
            {
                const size_t sep = remaining.find('/');
                const std::string_view partStr = remaining.substr(0, sep);
                remaining = (sep == std::string_view::npos) ? std::string_view{} : remaining.substr(sep + 1);

                if (partStr.empty())
                {
                    continue;
                }

                const size_t dashPos = partStr.find('-');
                std::string_view codePart = dashPos != std::string_view::npos ? partStr.substr(0, dashPos) : partStr;

                const auto nodeOpt = gmod.node(codePart);
                if (!nodeOpt.has_value())
                {
                    return GmodParsePathResult::error(std::string{ ERROR_PREFIX_FAILED_NODE } + std::string{ partStr });
                }

                if (dashPos != std::string_view::npos)
                {
                    const std::string_view locationPart = partStr.substr(dashPos + 1);
                    auto parsedLocationOpt = locations.fromString(locationPart);
                    if (!parsedLocationOpt.has_value())
                    {
                        return GmodParsePathResult::error(
                            std::string{ "Failed to parse location - " } + std::string{ locationPart });
                    }
                    parts.emplace_back(codePart, parsedLocationOpt.value());
                }
                else
                {
                    parts.emplace_back(codePart);
                }

                if (!remaining.empty() && !(*nodeOpt)->isLeafNode())
                {
                    return GmodParsePathResult::error(
                        std::string{ (*nodeOpt)->code() } +
                        " is not a valid start GmodNode for a short GmodPath. Only leaf "
                        "nodes are allowed as parents in a short path");
                }
            }

            if (parts.empty())
            {
                return GmodParsePathResult::error("Failed to find any parts");
            }

            PathNode toFind = std::move(parts[0]);
            std::vector<PathNode> remainingParts(
                std::make_move_iterator(parts.begin() + 1), std::make_move_iterator(parts.end()));

            auto baseNode = gmod.node(toFind.code);
            if (!baseNode.has_value())
            {
                return GmodParsePathResult::error("Failed to find base node");
            }

            ParseContext context(std::move(remainingParts), std::move(toFind), std::nullopt, std::nullopt, gmod);

            Gmod::TraverseHandlerWithState<ParseContext> handler = parseHandler;
            gmod.traverse(context, *baseNode.value(), handler, TraversalOptions{});
            if (!context.path.has_value())
            {
                return GmodParsePathResult::error("Failed to find path after traversal");
            }

            return GmodParsePathResult::ok(std::move(context.path.value()));
        }
        catch (const std::bad_alloc&)
        {
            return GmodParsePathResult::error("out of memory");
        }
    }

    /**
     * @brief Parse a full Gmod path string with detailed error reporting
     * @param fullPathStr Full path string starting with root (e.g., "VE/400a/410/411/411i/411.1")
     * @param gmod Gmod instance to resolve nodes from
     * @param locations Locations instance to parse location suffixes
     * @return GmodParsePathResult containing either the parsed GmodPath or detailed error message
     * @details Direct parsing without tree traversal. Validates parent-child relationships
     *          and location individualization rules. Provides specific error messages for
     *          each failure point (empty string, missing root, invalid nodes, bad locations, etc.).
     */
    inline GmodParsePathResult fromFullPath(
        std::string_view fullPathStr, const Gmod& gmod, const Locations& locations) noexcept
    {
        if (fullPathStr.empty())
        {
            return GmodParsePathResult::error(ERROR_EMPTY_ITEM);
        }

        const std::string_view rootCode = gmod.rootNode().code();
        if (fullPathStr.size() < rootCode.size() || fullPathStr.substr(0, rootCode.size()) != rootCode)
        {
            return GmodParsePathResult::error(std::string{ "Path must start with " } + std::string{ rootCode });
        }

        std::vector<GmodNode> nodes;
        nodes.reserve(fullPathStr.size());

        for (std::string_view remaining = fullPathStr; !remaining.empty();)
        {
            const size_t sep = remaining.find('/');
            const std::string_view segment = remaining.substr(0, sep);
            remaining = (sep == std::string_view::npos) ? std::string_view{} : remaining.substr(sep + 1);

            if (segment.empty())
            {
                continue;
            }

            const size_t dashPos = segment.find('-');

            if (dashPos != std::string_view::npos)
            {
                const std::string_view codePart = segment.substr(0, dashPos);
                const std::string_view locationPart = segment.substr(dashPos + 1);

                auto nodePtr = gmod.node(codePart);
                if (!nodePtr.has_value())
                {
                    return GmodParsePathResult::error(
                        std::string{ ERROR_PREFIX_FAILED_NODE } + std::string{ codePart });
                }

                auto parsedLocationOpt = locations.fromString(locationPart);
                if (!parsedLocationOpt.has_value())
                {
                    return GmodParsePathResult::error(
                        std::string{ "Failed to parse location - " } + std::string{ locationPart });
                }

                nodes.emplace_back(**nodePtr);
                nodes.back().setLocation(std::move(parsedLocationOpt.value()));
            }
            else
            {
                auto nodePtr = gmod.node(segment);
                if (!nodePtr.has_value())
                {
                    return GmodParsePathResult::error(std::string{ ERROR_PREFIX_FAILED_NODE } + std::string{ segment });
                }

                nodes.emplace_back(**nodePtr);
            }
        }
        if (nodes.empty())
        {
            return GmodParsePathResult::error("Failed to find any nodes");
        }

        GmodNode endNode = std::move(nodes.back());
        nodes.pop_back();

        if (nodes.empty())
        {
            return GmodParsePathResult::ok(GmodPath{ {}, std::move(endNode) });
        }

        if (!nodes[0].isRoot())
        {
            return GmodParsePathResult::error("Node sequence is invalid");
        }

        bool hasLocations = endNode.location().has_value();
        for (const auto& node : nodes)
        {
            if (node.location().has_value())
            {
                hasLocations = true;

                break;
            }
        }

        if (!hasLocations)
        {
            return GmodParsePathResult::ok(GmodPath{ std::move(nodes), std::move(endNode), true /* skipVerify */ });
        }

        LocationSetsVisitor locationSetsVisitor;
        std::optional<size_t> prevNonNullLocation;

        std::pair<size_t, size_t> sets[MAX_LOCATION_SETS];
        size_t setCounter = 0;

        for (size_t i = 0; i < nodes.size() + 1; ++i)
        {
            const GmodNode& n = (i < nodes.size()) ? nodes[i] : endNode;

            auto set = locationSetsVisitor.visit(n, i, nodes, endNode);
            if (!set.has_value())
            {
                if (!prevNonNullLocation.has_value() && n.location().has_value())
                {
                    prevNonNullLocation = i;
                }

                continue;
            }

            const auto& [setStart, setEnd, location] = set.value();

            if (prevNonNullLocation.has_value())
            {
                for (size_t j = prevNonNullLocation.value(); j < setStart; ++j)
                {
                    const GmodNode& pn = (j < nodes.size()) ? nodes[j] : endNode;

                    if (pn.location().has_value())
                    {
                        return GmodParsePathResult::error(
                            "Expected all nodes in the set to be without individualization. Found " +
                            std::string{ pn.code() });
                    }
                }
            }
            prevNonNullLocation = std::nullopt;

            if (setCounter >= MAX_LOCATION_SETS)
            {
                return GmodParsePathResult::error(
                    "Path has more than " + std::to_string(MAX_LOCATION_SETS) +
                    " individualizable location sets, which is not supported");
            }
            sets[setCounter++] = { setStart, setEnd };

            if (setStart == setEnd)
            {
                continue;
            }

            if (!location.has_value())
            {
                continue;
            }

            for (size_t j = setStart; j <= setEnd; ++j)
            {
                if (j < nodes.size())
                {
                    nodes[j].setLocation(*location);
                }
                else
                {
                    endNode.setLocation(*location);
                }
            }
        }

        std::pair<size_t, size_t> currentSet = { std::numeric_limits<size_t>::max(),
                                                 std::numeric_limits<size_t>::max() };
        size_t currentSetIndex = 0;

        for (size_t i = 0; i < nodes.size() + 1; ++i)
        {
            while (currentSetIndex < setCounter &&
                   (currentSet.second == std::numeric_limits<size_t>::max() || currentSet.second < i))
            {
                currentSet = sets[currentSetIndex++];
            }

            bool insideSet =
                (currentSet.first != std::numeric_limits<size_t>::max() && i >= currentSet.first &&
                 i <= currentSet.second);
            const GmodNode& n = (i < nodes.size()) ? nodes[i] : endNode;

            if (insideSet)
            {
                const GmodNode& expectedLocationNode =
                    (currentSet.second < nodes.size()) ? nodes[currentSet.second] : endNode;

                if (n.location() != expectedLocationNode.location())
                {
                    return GmodParsePathResult::error(
                        "Expected all nodes in the set to be individualized the same. Found " +
                        std::string{ n.code() } + " with location " +
                        (n.location().has_value() ? n.location()->value() : "null"));
                }
            }
            else
            {
                if (n.location().has_value())
                {
                    return GmodParsePathResult::error(
                        "Expected all nodes in the set to be without individualization. Found " +
                        std::string{ n.code() });
                }
            }
        }

        return GmodParsePathResult::ok(GmodPath{ std::move(nodes), std::move(endNode), true /* skipVerify */ });
    }
} // namespace dnv::vista::sdk::internal
