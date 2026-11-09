#include "dnv/vista/sdk/core/Gmod.h"

#include "dto/GmodDto.h"

#include <stdexcept>

namespace dnv::vista::sdk
{
    Gmod::Gmod(VisVersion version, const GmodDto& dto)
        : m_visVersion{ version },
          m_rootNode{ nullptr }
    {
        const auto& items = dto.items;

        std::vector<std::pair<std::string, GmodNode>> perfectItems;
        perfectItems.reserve(items.size());

        for (const auto& nodeDto : items)
        {
            perfectItems.emplace_back(std::string{ nodeDto.code }, GmodNode{ version, nodeDto });
        }

        m_nodeMap = PerfectHashMap<GmodNode>{ std::move(perfectItems) };

        for (const auto& relation : dto.relations)
        {
            const GmodNode* parent = m_nodeMap.find(relation[0]);
            if (!parent)
            {
                throw std::runtime_error{ "Parent node not found in Gmod" };
            }
            const GmodNode* child = m_nodeMap.find(relation[1]);
            if (!child)
            {
                throw std::runtime_error{ "Child node not found in Gmod" };
            }
            const_cast<GmodNode*>(parent)->addChild(const_cast<GmodNode*>(child));
            const_cast<GmodNode*>(child)->addParent(const_cast<GmodNode*>(parent));
        }

        for (const auto& [key, node] : m_nodeMap)
        {
            const_cast<GmodNode&>(node).trim();
        }

        const GmodNode* root = m_nodeMap.find("VE");
        if (!root)
        {
            throw std::runtime_error{ "Root node 'VE' not found in Gmod" };
        }
        m_rootNode = const_cast<GmodNode*>(root);
    }

    bool Gmod::traverse(TraverseHandler handler, TraversalOptions options) const
    {
        return traverse(*m_rootNode, handler, options);
    }

    bool Gmod::traverse(const GmodNode& startNode, TraverseHandler handler, TraversalOptions options) const
    {
        struct DummyState
        {
        };

        DummyState state;

        TraverseHandlerWithState<DummyState> wrappedHandler =
            [&handler](DummyState&, const TraversalPath& parents, const GmodNode& node) -> TraversalHandlerResult {
            return handler(parents, node);
        };

        return traverse(state, startNode, wrappedHandler, options);
    }
} // namespace dnv::vista::sdk
