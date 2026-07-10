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
        m_nodeMap.reserve(items.size());

        for (const auto& nodeDto : items)
        {
            m_nodeMap.emplace(std::string{ nodeDto.code }, GmodNode{ version, nodeDto });
        }

        for (const auto& relation : dto.relations)
        {
            auto parentIt = m_nodeMap.find(relation[0]);
            if (parentIt == m_nodeMap.end())
            {
                throw std::runtime_error{ "Parent node not found in Gmod" };
            }
            auto childIt = m_nodeMap.find(relation[1]);
            if (childIt == m_nodeMap.end())
            {
                throw std::runtime_error{ "Child node not found in Gmod" };
            }
            parentIt->second.addChild(&childIt->second);
            childIt->second.addParent(&parentIt->second);
        }

        for (auto& [key, node] : m_nodeMap)
        {
            node.trim();
        }

        auto rootIt = m_nodeMap.find("VE");
        if (rootIt == m_nodeMap.end())
        {
            throw std::runtime_error{ "Root node 'VE' not found in Gmod" };
        }
        m_rootNode = &rootIt->second;
    }
} // namespace dnv::vista::sdk
