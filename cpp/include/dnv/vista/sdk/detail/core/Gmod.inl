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
} // namespace dnv::vista::sdk
