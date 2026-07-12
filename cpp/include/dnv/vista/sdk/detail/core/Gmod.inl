#include <limits>
#include <stdexcept>

namespace dnv::vista::sdk
{
    inline const GmodNode& Gmod::operator[](std::string_view key) const
    {
        auto it = m_nodeMap.find(std::string{ key });
        if (it != m_nodeMap.end())
        {
            return it->second;
        }

        throw std::out_of_range{ "Key not found in Gmod node map: " + std::string{ key } };
    }

    inline VisVersion Gmod::version() const noexcept
    {
        return m_visVersion;
    }

    inline const GmodNode& Gmod::rootNode() const noexcept
    {
        return *m_rootNode;
    }

    inline std::optional<const GmodNode*> Gmod::node(std::string_view code) const noexcept
    {
        auto it = m_nodeMap.find(std::string{ code });
        if (it != m_nodeMap.end())
        {
            return &it->second;
        }
        return std::nullopt;
    }

    inline auto Gmod::begin() const noexcept
    {
        return m_nodeMap.begin();
    }

    inline auto Gmod::end() const noexcept
    {
        return m_nodeMap.end();
    }

    inline auto Gmod::cbegin() const noexcept
    {
        return m_nodeMap.cbegin();
    }

    inline auto Gmod::cend() const noexcept
    {
        return m_nodeMap.cend();
    }

    template <typename TState>
    bool Gmod::traverse(TState& state, TraverseHandlerWithState<TState> handler, TraversalOptions options) const
    {
        return traverse(state, *m_rootNode, handler, options);
    }

    template <typename TState>
    bool Gmod::traverse(
        TState& state,
        const GmodNode& startNode,
        TraverseHandlerWithState<TState> handler,
        TraversalOptions options) const
    {
        TraversalParents parents;

        struct Dfs
        {
            TState& state;
            TraverseHandlerWithState<TState>& handler;
            TraversalParents& parents;
            const TraversalOptions& options;

            bool visit(const GmodNode& node)
            {
                const TraversalHandlerResult result = handler(state, parents.asList(), node);

                if (result == TraversalHandlerResult::Stop)
                {
                    return false;
                }

                if (result == TraversalHandlerResult::SkipSubtree)
                {
                    return true;
                }

                const bool skipOccurrenceCheck = Gmod::isProductSelectionAssignment(parents.lastOrDefault(), &node);
                if (!skipOccurrenceCheck && parents.occurrences(node) >= options.maxTraversalOccurrence)
                {
                    return true;
                }

                parents.push(&node);

                for (const GmodNode* child : node.children())
                {
                    if (!child)
                    {
                        continue;
                    }

                    if (!visit(*child))
                    {
                        parents.pop();
                        return false;
                    }
                }

                parents.pop();
                return true;
            }
        };

        return Dfs{ state, handler, parents, options }.visit(startNode);
    }

    inline bool Gmod::isProductSelectionAssignment(const GmodNode* parent, const GmodNode* child) noexcept
    {
        if (!parent || !child)
        {
            return false;
        }
        if (!parent->isFunctionNode())
        {
            return false;
        }
        return child->isProductSelection();
    }

    constexpr bool Gmod::isPotentialParent(std::string_view type) noexcept
    {
        return type == "SELECTION" || type == "GROUP" || type == "LEAF";
    }

    inline bool Gmod::pathExistsBetween(
        const TraversalPath& fromPath, const GmodNode& to, TraversalPath& remainingParents) const
    {
        remainingParents.clear();

        const GmodNode* lastAssetFunction = nullptr;
        size_t assetFunctionIndex = std::numeric_limits<size_t>::max();
        for (size_t i = fromPath.size(); i > 0; --i)
        {
            size_t idx = i - 1;
            if (fromPath[idx]->isAssetFunctionNode())
            {
                lastAssetFunction = fromPath[idx];
                assetFunctionIndex = idx;
                break;
            }
        }

        const GmodNode& startNode = lastAssetFunction ? *lastAssetFunction : rootNode();

        struct PathExistsState
        {
            const GmodNode& targetNode;
            const TraversalPath& fromPath;
            TraversalPath& remainingParents;
            size_t assetFunctionIndex;
            bool found = false;

            PathExistsState(const GmodNode& target, const TraversalPath& from, TraversalPath& remaining, size_t afIndex)
                : targetNode{ target },
                  fromPath{ from },
                  remainingParents{ remaining },
                  assetFunctionIndex{ afIndex }
            {}

            PathExistsState(const PathExistsState&) = delete;
            PathExistsState(PathExistsState&&) = delete;
            PathExistsState& operator=(const PathExistsState&) = delete;
            PathExistsState& operator=(PathExistsState&&) = delete;
        };

        PathExistsState state(to, fromPath, remainingParents, assetFunctionIndex);

        auto handler =
            [](PathExistsState& s, const TraversalPath& parents, const GmodNode& node) -> TraversalHandlerResult {
            if (node.code() != s.targetNode.code())
            {
                return TraversalHandlerResult::Continue;
            }

            TraversalPath completePath;
            completePath.reserve(parents.size());
            for (const GmodNode* parent : parents)
            {
                if (!parent->isRoot())
                {
                    completePath.push_back(parent);
                }
            }
            size_t startIndex = 0;

            if (s.assetFunctionIndex != std::numeric_limits<size_t>::max())
            {
                startIndex = s.assetFunctionIndex;
            }

            size_t requiredNodes = s.fromPath.size() - startIndex;
            if (completePath.size() < requiredNodes)
            {
                return TraversalHandlerResult::Continue;
            }

            bool match = true;
            for (size_t i = 0; i < requiredNodes; ++i)
            {
                size_t fromPathIdx = startIndex + i;
                if (fromPathIdx >= s.fromPath.size() || i >= completePath.size())
                {
                    match = false;
                    break;
                }
                if (completePath[i]->code() != s.fromPath[fromPathIdx]->code())
                {
                    match = false;
                    break;
                }
            }

            if (match)
            {
                for (size_t i = requiredNodes; i < completePath.size(); ++i)
                {
                    s.remainingParents.push_back(completePath[i]);
                }
                s.found = true;
                return TraversalHandlerResult::Stop;
            }

            return TraversalHandlerResult::Continue;
        };

        TraverseHandlerWithState<PathExistsState> wrappedHandler = handler;

        traverse(state, startNode, wrappedHandler, TraversalOptions{});

        return state.found;
    }
} // namespace dnv::vista::sdk
