/**
 * @file gmodsubset.cpp
 * @brief Demonstrates building Asset Models (Digital Twin structures) from GmodPaths
 * @details This sample shows how to construct hierarchical asset models from GmodPaths,
 *          perform code-based lookups, extract paths from LocalIds, and export to JSON.
 *          Asset models form the foundation for defining vessel equipment, creating
 *          visualizations, and generating LocalIds for sensor data.
 */

#include <dnv/VistaSDK.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

using namespace dnv::vista::sdk;

class AssetNode
{
private:
    GmodNode m_node;
    std::optional<GmodPath> m_path;
    size_t m_depth;
    std::string m_displayName;
    std::vector<AssetNode*> m_children;
    AssetNode* m_parent = nullptr;

public:
    AssetNode(const GmodNode& n, std::optional<GmodPath> p, size_t d)
        : m_node{ n },
          m_path{ std::move(p) },
          m_depth{ d }
    {
        if (m_path.has_value())
        {
            auto names = m_path->commonNames();
            if (!names.empty())
            {
                m_displayName = names.back().second;
            }
        }
        if (m_displayName.empty())
        {
            m_displayName = m_node.metadata().name();
        }
    }

    const GmodNode& node() const { return m_node; }
    const std::optional<GmodPath>& path() const { return m_path; }
    size_t depth() const { return m_depth; }
    const std::string& displayName() const { return m_displayName; }
    AssetNode* parent() const { return m_parent; }

    const std::vector<AssetNode*>& children() const { return m_children; }

    void addChild(AssetNode* child)
    {
        child->m_parent = this;
        m_children.push_back(child);
    }

    std::string toJson(int indent = 0) const
    {
        auto escape = [](std::string_view s) {
            std::string out;
            out.reserve(s.size() + 2);
            out += '"';
            for (char c : s)
            {
                if (c == '"' || c == '\\')
                {
                    out += '\\';
                }
                out += c;
            }
            out += '"';
            return out;
        };

        std::string pad(indent * 2, ' ');
        std::string pad2((indent + 1) * 2, ' ');

        std::ostringstream o;
        o << "{\n";
        o << pad2 << R"("path": )" << escape(m_path.has_value() ? m_path->toString() : "") << ",\n";
        o << pad2 << R"("code": )" << escape(m_node.code()) << ",\n";
        o << pad2 << R"("name": )" << escape(m_node.metadata().name()) << ",\n";
        if (m_node.metadata().commonName().has_value())
        {
            o << pad2 << R"("commonName": )" << escape(*m_node.metadata().commonName()) << ",\n";
        }
        else
        {
            o << pad2 << R"("commonName": null,)" << "\n";
        }
        o << pad2 << R"("displayName": )" << escape(m_displayName) << ",\n";
        o << pad2 << R"("category": )" << escape(m_node.metadata().category()) << ",\n";
        o << pad2 << R"("type": )" << escape(m_node.metadata().type()) << ",\n";
        if (m_node.location().has_value())
        {
            o << pad2 << R"("location": )" << escape(m_node.location()->value()) << ",\n";
        }
        else
        {
            o << pad2 << R"("location": null,)" << "\n";
        }

        std::vector<const AssetNode*> withPath;
        for (const auto& child : m_children)
        {
            if (child->m_path.has_value())
            {
                withPath.push_back(child);
            }
        }
        std::sort(withPath.begin(), withPath.end(), [](const auto* a, const auto* b) {
            return a->m_path->toString() < b->m_path->toString();
        });

        o << pad2 << "\"children\": [";
        if (withPath.empty())
        {
            o << "]\n";
        }
        else
        {
            o << "\n";
            for (size_t i = 0; i < withPath.size(); ++i)
            {
                o << pad2 << "  " << withPath[i]->toJson(indent + 2);
                if (i + 1 < withPath.size())
                {
                    o << ',';
                }
                o << "\n";
            }
            o << pad2 << "]\n";
        }
        o << pad << '}';
        return o.str();
    }

    void printTree(const std::string& prefix = "", bool isLast = true, bool isRoot = true) const
    {
        if (!isRoot)
        {
            std::cout << prefix << (isLast ? "└─ " : "├─ ");
        }
        std::cout << m_node.code();

        if (m_node.location().has_value())
        {
            std::cout << " [" << m_node.location()->value() << "]";
        }

        std::cout << ": " << m_displayName << "\n";

        std::vector<const AssetNode*> sortedChildren;
        for (const auto& child : m_children)
        {
            if (child && child->m_path.has_value())
            {
                sortedChildren.push_back(child);
            }
        }

        std::sort(sortedChildren.begin(), sortedChildren.end(), [](const auto* a, const auto* b) {
            return a->m_path->toString() < b->m_path->toString();
        });

        for (size_t i = 0; i < sortedChildren.size(); ++i)
        {
            bool childIsLast = (i == sortedChildren.size() - 1);
            std::string newPrefix = prefix;
            if (!isRoot)
            {
                newPrefix += (isLast ? "   " : "│  ");
            }
            sortedChildren[i]->printTree(newPrefix, childIsLast, false);
        }
    }
};

class AssetModel
{
private:
    VisVersion m_visVersion;
    AssetNode* m_root = nullptr;
    std::map<std::string, std::unique_ptr<AssetNode>> m_nodeMap;
    std::map<std::string, std::vector<AssetNode*>> m_nodesByCode;

    explicit AssetModel(VisVersion version)
        : m_visVersion{ version }
    {}

    void addPath(const GmodPath& path)
    {
        std::vector<std::pair<size_t, GmodNode>> fullPathVec;
        for (const auto& [depth, node] : path.fullPath())
        {
            fullPathVec.push_back({ depth, node });
        }

        std::string nodeId;
        std::string parentId;
        for (size_t i = 0; i < fullPathVec.size(); ++i)
        {
            const auto& [depth, node] = fullPathVec[i];

            parentId = nodeId;
            if (i > 0)
            {
                nodeId += '/';
            }
            nodeId += node.toString();

            if (m_nodeMap.find(nodeId) != m_nodeMap.end())
            {
                continue;
            }

            std::optional<GmodPath> nodePath;
            if (depth > 0)
            {
                std::vector<GmodNode> parents;
                parents.reserve(i);
                for (size_t j = 0; j < i; ++j)
                {
                    parents.push_back(fullPathVec[j].second);
                }
                nodePath = GmodPath(std::move(parents), node);
            }

            auto assetNode = std::make_unique<AssetNode>(node, std::move(nodePath), depth);
            AssetNode* raw = assetNode.get();

            m_nodesByCode[std::string{ node.code() }].push_back(raw);

            if (depth == 0)
            {
                if (!m_root)
                {
                    m_root = raw;
                }
                m_nodeMap[nodeId] = std::move(assetNode);
                continue;
            }

            auto parentIt = m_nodeMap.find(parentId);
            if (parentIt != m_nodeMap.end() && parentIt->second)
            {
                parentIt->second->addChild(raw);
            }

            m_nodeMap[nodeId] = std::move(assetNode);
        }
    }

public:
    static AssetModel fromPaths(VisVersion version, const std::vector<GmodPath>& paths)
    {
        AssetModel model(version);
        for (const auto& path : paths)
        {
            model.addPath(path);
        }
        return model;
    }

    static AssetModel fromPathStrings(VisVersion version, const std::vector<std::string_view>& pathStrings)
    {
        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(version);
        const auto& locations = vis.locations(version);

        std::vector<GmodPath> paths;
        for (const auto& pathStr : pathStrings)
        {
            auto path = GmodPath::fromShortPath(pathStr, gmod, locations);
            if (path.has_value())
            {
                paths.push_back(std::move(*path));
            }
            else
            {
                std::cout << "  Warning: Could not parse path '" << pathStr << "'\n";
            }
        }
        return fromPaths(version, paths);
    }

    AssetNode* node(const std::string& nodeId) const
    {
        auto it = m_nodeMap.find(nodeId);
        return it != m_nodeMap.end() ? it->second.get() : nullptr;
    }

    const std::map<std::string, std::vector<AssetNode*>>& allCodes() const { return m_nodesByCode; }

    const std::vector<AssetNode*>& nodesByCode(const std::string& code) const
    {
        static const std::vector<AssetNode*> empty;
        auto it = m_nodesByCode.find(code);
        return it != m_nodesByCode.end() ? it->second : empty;
    }

    size_t nodeCount() const { return m_nodeMap.size(); }

    size_t maxDepth() const
    {
        size_t max = 0;
        for (const auto& [_, node] : m_nodeMap)
        {
            if (node)
            {
                max = std::max(max, node->depth());
            }
        }
        return max;
    }

    VisVersion visVersion() const { return m_visVersion; }

    std::string toJson() const { return m_root ? m_root->toJson() : "{}"; }

    void printTree() const
    {
        if (m_root)
        {
            m_root->printTree();
        }
    }
};

int main()
{
    std::cout << "=== vista-sdk GmodSubset Sample ===\n\n";

    {
        std::cout << "1. AssetModel: Building from equipment paths (dual-engine vessel)\n";
        std::cout << "--------------------------------------------------------------------\n";

        auto visVersion = VIS::instance().latest();

        std::vector<std::string_view> assetPaths = {
            // Port Main Engine (411.1-P) with 6 cylinder
            "411.1-P/C101.31-1",    // Port engine, cylinder 1
            "411.1-P/C101.31-2",    // Port engine, cylinder 2
            "411.1-P/C101.31-3",    // Port engine, cylinder 3
            "411.1-P/C101.31-4",    // Port engine, cylinder 4
            "411.1-P/C101.31-5",    // Port engine, cylinder 5
            "411.1-P/C101.31-6",    // Port engine, cylinder 6
            "411.1-P/C101.63/S206", // Port engine, cooling system

            // Starboard Main Engine (411.1-S) with 6 cylinders
            "411.1-S/C101.31-1",    // Starboard engine, cylinder 1
            "411.1-S/C101.31-2",    // Starboard engine, cylinder 2
            "411.1-S/C101.31-3",    // Starboard engine, cylinder 3
            "411.1-S/C101.31-4",    // Starboard engine, cylinder 4
            "411.1-S/C101.31-5",    // Starboard engine, cylinder 5
            "411.1-S/C101.31-6",    // Starboard engine, cylinder 6
            "411.1-S/C101.63/S206", // Starboard engine, cooling system

            // Generator Sets
            "511.11-1/C101", // Generator 1, diesel engine
            "511.11-2/C101", // Generator 2, diesel engine

            // Fuel System
            "621.21/S90" // Fuel oil transfer system, piping
        };

        std::cout << "Defining " << assetPaths.size() << " equipment paths\n";
        std::cout << "(Each path implicitly includes all parent nodes)\n\n";

        auto model = AssetModel::fromPathStrings(visVersion, assetPaths);

        std::cout << "Built model with " << model.nodeCount() << " total nodes\n";
        std::cout << "Maximum depth   : " << model.maxDepth() << "\n\n";

        auto& engines = model.nodesByCode("C101");
        std::cout << "Lookup: All Engines (C101)\n";
        std::cout << "Found " << engines.size() << " instances of C101 (Internal combustion engine):\n";
        for (const auto* engine : engines)
        {
            if (engine->path().has_value())
            {
                std::cout << "  - " << engine->path()->toString() << "\n";
            }
        }

        std::cout << "\nLookup: All Cylinders (C101.31)\n";
        auto& cylinders = model.nodesByCode("C101.31");
        std::cout << "Found " << cylinders.size() << " instances of C101.31 (Cylinder):\n";
        size_t count = 0;
        for (const auto* cyl : cylinders)
        {
            if (count++ >= 6)
            {
                break;
            }
            if (cyl->path().has_value())
            {
                std::cout << "  - " << cyl->path()->toString() << "\n";
            }
        }
        if (cylinders.size() > 6)
        {
            std::cout << "  ... and " << (cylinders.size() - 6) << " more\n";
        }

        std::cout << "\nAll available codes in model:\n";
        std::vector<std::string> codes;
        for (const auto& [code, _] : model.allCodes())
        {
            codes.push_back(code);
        }
        std::sort(codes.begin(), codes.end());
        std::cout << codes.size() << " unique codes: [";
        for (size_t i = 0; i < codes.size(); ++i)
        {
            if (i > 0)
            {
                std::cout << ", ";
            }
            std::cout << codes[i];
        }
        std::cout << "]\n\n";

        std::cout << "Model Metadata:\n";
        std::cout << "VIS Version   : " << VisVersions::toString(model.visVersion()) << "\n";
        std::cout << "Total nodes   : " << model.nodeCount() << "\n";
        std::cout << "Maximum depth : " << model.maxDepth() << "\n\n";

        std::cout << "Asset Model Tree:\n\n";
        model.printTree();

        std::cout << "\n";
    }

    {
        std::cout << "2. AssetModel: Building from LocalIds (runtime flow)\n";
        std::cout << "-------------------------------------------------------\n";

        auto visVersion = VIS::instance().latest();

        std::vector<std::string_view> localIdStrings = {
            "/dnv-v2/vis-3-4a/411.1-1/C101.31-1/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
            "/dnv-v2/vis-3-4a/411.1-1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
            "/dnv-v2/vis-3-4a/411.1-1/C101.31-3/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
            "/dnv-v2/vis-3-4a/411.1-2/C101.31-1/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
            "/dnv-v2/vis-3-4a/411.1-2/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
            "/dnv-v2/vis-3-4a/511.11-1/C101/meta/qty-revolution",
        };

        std::cout << "Received " << localIdStrings.size() << " LocalIds from data source\n\n";

        std::vector<GmodPath> paths;
        for (const auto& lidStr : localIdStrings)
        {
            auto localIdOpt = LocalId::fromString(lidStr);
            if (localIdOpt.has_value())
            {
                const auto& localId = *localIdOpt;
                paths.push_back(localId.primaryItem());
                if (localId.secondaryItem().has_value())
                {
                    paths.push_back(*localId.secondaryItem());
                }
            }
        }

        std::cout << "Extracted " << paths.size() << " GmodPaths from LocalIds\n";

        auto model = AssetModel::fromPaths(visVersion, paths);

        std::cout << "Built model with " << model.nodeCount() << " nodes\n";
        std::cout << "Maximum depth   : " << model.maxDepth() << "\n\n";

        std::cout << "Asset Model Tree (derived from LocalIds):\n\n";
        model.printTree();

        std::cout << "\n";
    }

    {
        std::cout << "3. AssetModel: JSON export for visualization\n";
        std::cout << "-----------------------------------------------\n";

        auto visVersion = VIS::instance().latest();

        std::vector<std::string_view> assetPaths = { "411.1/C101.31-1", "411.1/C101.31-2" };

        auto model = AssetModel::fromPathStrings(visVersion, assetPaths);

        std::cout << "JSON output (for D3.js, visualization tools, etc.):\n\n";
        std::cout << model.toJson() << "\n";

        std::cout << "\n";
    }

    return 0;
}
