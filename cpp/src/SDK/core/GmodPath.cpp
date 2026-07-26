#include "dnv/vista/sdk/core/GmodPath.h"

#include "internal/parsing/GmodPathParsing.h"
#include "dnv/vista/sdk/core/ParsingErrors.h"
#include "dnv/vista/sdk/core/VIS.h"

#include <stdexcept>

namespace dnv::vista::sdk
{
    GmodPath::GmodPath(std::vector<GmodNode>&& parents, GmodNode node, bool skipVerify)
        : m_visVersion{ node.version() },
          m_parents{ std::move(parents) },
          m_node{ std::move(node) }
    {
        if (!skipVerify)
        {
            if (m_parents.empty())
            {
                throw std::invalid_argument{ "Invalid gmod path - no parents, and " + std::string{ m_node.code() } +
                                             " is not the root of gmod" };
            }

            if (!m_parents.empty() && !m_parents[0].isRoot())
            {
                throw std::invalid_argument{ "Invalid gmod path - first parent should be root of gmod (VE), but was " +
                                             std::string{ m_parents[0].code() } };
            }

            for (size_t i = 0; i < m_parents.size(); ++i)
            {
                const auto& parent = m_parents[i];
                const auto nextIndex = i + 1;
                const auto& child = nextIndex < m_parents.size() ? m_parents[nextIndex] : m_node;

                if (!parent.isChild(child))
                {
                    throw std::invalid_argument{ "Invalid gmod path - " + std::string{ child.code() } +
                                                 " not child of " + std::string{ parent.code() } };
                }
            }

            internal::LocationSetsVisitor visitor;
            for (size_t i = 0; i < m_parents.size() + 1; ++i)
            {
                const auto& n = i < m_parents.size() ? m_parents[i] : m_node;
                auto _ = visitor.visit(n, i, m_parents, m_node);
            }
        }
    }

    GmodPath::GmodPath(std::vector<GmodNode>&& parents, GmodNode node)
        : GmodPath{ std::move(parents), std::move(node), false }
    {}

    bool GmodPath::isIndividualizable() const
    {
        internal::LocationSetsVisitor locationSetsVisitor;

        for (size_t i = 0; i < length(); ++i)
        {
            const GmodNode& node = (i < m_parents.size()) ? m_parents[i] : m_node;

            auto set = locationSetsVisitor.visit(node, i, m_parents, m_node);
            if (set.has_value())
            {
                return true;
            }
        }

        return false;
    }

    GmodPath GmodPath::withoutLocations() const
    {
        std::vector<GmodNode> newParents;
        newParents.reserve(m_parents.size());

        for (const auto& parent : m_parents)
        {
            newParents.push_back(parent.withoutLocation());
        }

        GmodNode newNode = m_node.withoutLocation();

        return GmodPath{ std::move(newParents), std::move(newNode), true };
    }

    std::string GmodPath::toFullPathString() const
    {
        StringBuilder sb;
        toFullPathString(sb);
        return sb.toString();
    }

    std::vector<GmodIndividualizableSet> GmodPath::individualizableSets() const
    {
        std::vector<GmodIndividualizableSet> result;

        internal::LocationSetsVisitor locationSetsVisitor;

        for (size_t i = 0; i < length(); ++i)
        {
            const GmodNode& node = (i < m_parents.size()) ? m_parents[i] : m_node;

            auto set = locationSetsVisitor.visit(node, i, m_parents, m_node);
            if (!set.has_value())
            {
                continue;
            }

            const auto& [startIdx, endIdx, location] = set.value();

            if (startIdx == endIdx)
            {
                result.emplace_back(std::vector<int>{ static_cast<int>(startIdx) }, *this);

                continue;
            }

            std::vector<int> nodes;
            nodes.reserve(endIdx - startIdx + 1);
            for (size_t j = startIdx; j <= endIdx; ++j)
            {
                nodes.push_back(static_cast<int>(j));
            }

            result.emplace_back(std::move(nodes), *this);
        }

        return result;
    }

    std::optional<std::string> GmodPath::normalAssignmentName(size_t nodeDepth) const
    {
        if (nodeDepth >= length())
        {
            return std::nullopt;
        }

        const GmodNode& nodeInPath = (*this)[nodeDepth];

        const auto& normalAssignmentNames = nodeInPath.metadata().normalAssignmentNames();
        if (normalAssignmentNames.empty())
        {
            return std::nullopt;
        }

        for (size_t i = length() - 1;; --i)
        {
            const GmodNode& child = (*this)[i];
            auto it = normalAssignmentNames.find(child.code());
            if (it != normalAssignmentNames.end())
            {
                return it->second;
            }

            if (i == 0)
            {
                break;
            }
        }

        return std::nullopt;
    }

    std::vector<std::pair<size_t, std::string>> GmodPath::commonNames() const
    {
        std::vector<std::pair<size_t, std::string>> result;

        for (const auto& [depth, node] : fullPath())
        {
            bool isTarget = (depth == m_parents.size());

            if (!(node.isLeafNode() || isTarget) || !node.isFunctionNode())
            {
                continue;
            }

            // Some nodes have an empty name but a populated commonName (e.g., installSubstructure nodes)
            std::string name;
            const auto& commonName = node.metadata().commonName();
            if (commonName.has_value() && !commonName->empty())
            {
                name = *commonName;
            }
            else
            {
                name = node.metadata().name();
            }

            const auto& normalAssignmentNames = node.metadata().normalAssignmentNames();

            if (!normalAssignmentNames.empty())
            {
                auto it = normalAssignmentNames.find(m_node.code());
                if (it != normalAssignmentNames.end())
                {
                    name = it->second;
                }

                if (!m_parents.empty() && depth < m_parents.size())
                {
                    for (size_t i = m_parents.size() - 1; i >= depth; --i)
                    {
                        const GmodNode& parent = m_parents[i];
                        auto pit = normalAssignmentNames.find(parent.code());
                        if (pit != normalAssignmentNames.end())
                        {
                            name = pit->second;
                            break;
                        }

                        if (i == depth)
                        {
                            break;
                        }
                    }
                }
            }

            result.emplace_back(depth, name);
        }

        return result;
    }

    std::optional<GmodPath> GmodPath::fromShortPath(std::string_view item, VisVersion visVersion) noexcept
    {
        try
        {
            const auto& vis = VIS::instance();
            const auto& gmod = vis.gmod(visVersion);
            const auto& locations = vis.locations(visVersion);
            return fromShortPath(item, gmod, locations);
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    std::optional<GmodPath> GmodPath::fromShortPath(
        std::string_view item, const Gmod& gmod, const Locations& locations) noexcept
    {
        auto result = internal::fromShortPath(item, gmod, locations);
        if (result)
        {
            return std::move(result.path());
        }
        return std::nullopt;
    }

    std::optional<GmodPath> GmodPath::fromShortPath(
        std::string_view item, const Gmod& gmod, const Locations& locations, ParsingErrors& errors) noexcept
    {
        try
        {
            auto result = internal::fromShortPath(item, gmod, locations);

            if (result)
            {
                return std::move(result.path());
            }

            std::vector<ParsingErrors::ErrorEntry> errorEntries;
            errorEntries.push_back({ "GmodPath.Parse", result.errorMessage() });
            errors = ParsingErrors{ std::move(errorEntries) };

            return std::nullopt;
        }
        catch (const std::bad_alloc&)
        {
            return std::nullopt;
        }
    }

    std::optional<GmodPath> GmodPath::fromFullPath(
        std::string_view fullPathStr, const Gmod& gmod, const Locations& locations) noexcept
    {
        auto result = internal::fromFullPath(fullPathStr, gmod, locations);
        if (result)
        {
            return std::move(result.path());
        }
        return std::nullopt;
    }

    std::optional<GmodPath> GmodPath::fromFullPath(
        std::string_view fullPathStr, const Gmod& gmod, const Locations& locations, ParsingErrors& errors) noexcept
    {
        try
        {
            auto result = internal::fromFullPath(fullPathStr, gmod, locations);

            if (result)
            {
                return std::move(result.path());
            }

            std::vector<ParsingErrors::ErrorEntry> errorEntries;
            errorEntries.push_back({ "GmodPath.Parse", result.errorMessage() });
            errors = ParsingErrors{ std::move(errorEntries) };

            return std::nullopt;
        }
        catch (const std::bad_alloc&)
        {
            return std::nullopt;
        }
    }

    GmodIndividualizableSet::GmodIndividualizableSet(const std::vector<int>& nodeIndices, const GmodPath& sourcePath)
        : m_nodeIndices{ nodeIndices },
          m_path{ sourcePath }
    {
        if (m_nodeIndices.empty())
        {
            throw std::runtime_error{ "GmodIndividualizableSet cannot be empty" };
        }

        for (int i : m_nodeIndices)
        {
            const auto& node = sourcePath[static_cast<size_t>(i)];
            bool isTargetNode = (static_cast<size_t>(i) == sourcePath.length() - 1);
            bool isInSet = (m_nodeIndices.size() > 1);

            if (!node.isIndividualizable(isTargetNode, isInSet))
            {
                throw std::runtime_error{ "GmodIndividualizableSet nodes must be individualizable" };
            }
        }

        if (m_nodeIndices.size() > 1)
        {
            const auto& firstLocation = sourcePath[static_cast<size_t>(m_nodeIndices[0])].location();

            for (size_t i = 1; i < m_nodeIndices.size(); ++i)
            {
                const auto& nodeLocation = sourcePath[static_cast<size_t>(m_nodeIndices[i])].location();
                if (firstLocation != nodeLocation)
                {
                    throw std::runtime_error{ "GmodIndividualizableSet nodes have different locations" };
                }
            }
        }

        bool hasShortPathNode = false;
        for (int i : m_nodeIndices)
        {
            const auto& node = sourcePath[static_cast<size_t>(i)];
            bool isTarget = (static_cast<size_t>(i) == sourcePath.length() - 1);

            if (node.isLeafNode() || isTarget)
            {
                hasShortPathNode = true;
                break;
            }
        }

        if (!hasShortPathNode)
        {
            throw std::runtime_error{ "GmodIndividualizableSet has no nodes that are part of the short path" };
        }
    }

    GmodPath GmodIndividualizableSet::build()
    {
        if (!m_path.has_value())
        {
            throw std::runtime_error{ "GmodIndividualizableSet has already been built" };
        }

        GmodPath result = std::move(*m_path);
        m_path.reset();

        return result;
    }

    std::vector<GmodNode> GmodIndividualizableSet::nodes() const
    {
        if (!m_path.has_value())
        {
            throw std::runtime_error{ "GmodIndividualizableSet has already been built" };
        }

        std::vector<GmodNode> result;
        result.reserve(m_nodeIndices.size());

        for (int nodeIdx : m_nodeIndices)
        {
            if (nodeIdx < 0 || static_cast<size_t>(nodeIdx) >= m_path->length())
            {
                throw std::out_of_range{ "Node index out of bounds" };
            }

            result.push_back((*m_path)[static_cast<size_t>(nodeIdx)]);
        }

        return result;
    }

    std::string GmodIndividualizableSet::toString() const
    {
        if (!m_path.has_value())
        {
            throw std::runtime_error{ "GmodIndividualizableSet has already been built" };
        }

        std::string out;
        bool firstNodeAppended = false;

        for (size_t j = 0; j < m_nodeIndices.size(); ++j)
        {
            int nodeIdx = m_nodeIndices[j];
            if (nodeIdx < 0 || static_cast<size_t>(nodeIdx) >= m_path->length())
            {
                continue;
            }

            const GmodNode& currentNode = (*m_path)[static_cast<size_t>(nodeIdx)];

            if (currentNode.isLeafNode() || j == m_nodeIndices.size() - 1)
            {
                if (firstNodeAppended)
                {
                    out += '/';
                }

                out += currentNode.toString();
                firstNodeAppended = true;
            }
        }

        return out;
    }
} // namespace dnv::vista::sdk
