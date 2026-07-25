/**
 * @file gmod-cli.cpp
 * @brief CLI tool for exploring VIS Gmod tree
 * @details Command-line interface to navigate and explore the Generic Product Model
 *
 * Usage:
 *   gmod-cli                   # Show tree overview for latest VIS version
 *   gmod-cli --version 3-4a    # Use specific VIS version
 *   gmod-cli --list-versions   # List available VIS versions
 *   gmod-cli --code 411.1      # Show node details
 *   gmod-cli --search "engine" # Search nodes by name
 *   gmod-cli --tree 411        # Show subtree from node
 *   gmod-cli --path "411.1-1P" # Parse and validate path
 *   gmod-cli --dump-map <file> # Dump node map (hash, code, name, category, type etc...)
 *   gmod-cli --help            # Show help
 */

#include <dnv/VistaSDK.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <set>

void printHelp();
void printVersions(const dnv::vista::sdk::VIS& vis);
void printOverview(const dnv::vista::sdk::Gmod& gmod);
void printNodeDetails(const dnv::vista::sdk::Gmod& gmod, std::string_view code);
void searchNodes(const dnv::vista::sdk::Gmod& gmod, std::string_view searchTerm);
void printTree(const dnv::vista::sdk::Gmod& gmod, std::string_view rootCode, int maxDepth);
void validatePath(
    const dnv::vista::sdk::Gmod& gmod, const dnv::vista::sdk::Locations& locations, std::string_view path);
void dumpMap(const dnv::vista::sdk::Gmod& gmod, std::string_view filename);

namespace
{
    inline constexpr int naturalCompare(std::string_view lhs, std::string_view rhs) noexcept
    {
        size_t i = 0;
        size_t j = 0;

        while (i < lhs.size() && j < rhs.size())
        {
            const char lc = lhs[i];
            const char rc = rhs[j];

            if (std::isdigit(static_cast<unsigned char>(lc)) && std::isdigit(static_cast<unsigned char>(rc)))
            {
                unsigned long long lNum = 0;
                unsigned long long rNum = 0;
                size_t lStart = i;
                size_t rStart = j;

                while (i < lhs.size() && std::isdigit(static_cast<unsigned char>(lhs[i])))
                {
                    lNum = lNum * 10 + (lhs[i] - '0');
                    ++i;
                }

                while (j < rhs.size() && std::isdigit(static_cast<unsigned char>(rhs[j])))
                {
                    rNum = rNum * 10 + (rhs[j] - '0');
                    ++j;
                }

                if (lNum < rNum)
                {
                    return -1;
                }
                if (lNum > rNum)
                {
                    return 1;
                }

                const size_t lDigits = i - lStart;
                const size_t rDigits = j - rStart;
                if (lDigits < rDigits)
                {
                    return -1;
                }
                if (lDigits > rDigits)
                {
                    return 1;
                }
            }
            else
            {
                if (lc < rc)
                {
                    return -1;
                }
                if (lc > rc)
                {
                    return 1;
                }
                ++i;
                ++j;
            }
        }

        if (i < lhs.size())
        {
            return 1;
        }
        if (j < rhs.size())
        {
            return -1;
        }
        return 0;
    }
} // namespace

int main(int argc, char* argv[])
{
    using namespace dnv::vista::sdk;

    const auto& vis = VIS::instance();

    if (argc == 1)
    {
        const auto& gmod = vis.gmod(vis.latest());
        printOverview(gmod);
        return 0;
    }

    std::string_view arg1 = argv[1];

    if (arg1 == "--help" || arg1 == "-h")
    {
        printHelp();
        return 0;
    }

    if (arg1 == "--list-versions" || arg1 == "-l")
    {
        printVersions(vis);
        return 0;
    }

    VisVersion version = vis.latest();
    int argOffset = 1;

    if (arg1 == "--version" || arg1 == "-v")
    {
        if (argc < 3)
        {
            std::cerr << "Error: --version requires a version argument\n";
            std::cerr << "Use --list-versions to see available versions\n";
            return 1;
        }

        std::string_view versionStr = argv[2];
        auto versionOpt = VisVersions::fromString(versionStr);
        if (!versionOpt.has_value())
        {
            std::cerr << "Error: Unknown VIS version '" << versionStr << "'\n";
            std::cerr << "Use --list-versions to see available versions\n";
            return 1;
        }

        version = versionOpt.value();
        argOffset = 3;

        if (argc == 3)
        {
            const auto& gmod = vis.gmod(version);
            printOverview(gmod);
            return 0;
        }

        arg1 = argv[argOffset];
    }

    const auto& gmod = vis.gmod(version);
    const auto& locations = vis.locations(version);

    if (arg1 == "--code" || arg1 == "-c")
    {
        if (argc < argOffset + 2)
        {
            std::cerr << "Error: --code requires a node code argument\n";
            return 1;
        }

        std::string_view code = argv[argOffset + 1];
        printNodeDetails(gmod, code);
        return 0;
    }

    if (arg1 == "--search" || arg1 == "-s")
    {
        if (argc < argOffset + 2)
        {
            std::cerr << "Error: --search requires a search term\n";
            return 1;
        }

        std::string_view searchTerm = argv[argOffset + 1];
        searchNodes(gmod, searchTerm);
        return 0;
    }

    if (arg1 == "--tree" || arg1 == "-t")
    {
        if (argc < argOffset + 2)
        {
            std::cerr << "Error: --tree requires a root node code\n";
            return 1;
        }

        std::string_view rootCode = argv[argOffset + 1];
        int maxDepth = 3;

        if (argc > argOffset + 2)
        {
            try
            {
                maxDepth = std::stoi(argv[argOffset + 2]);
            }
            catch (...)
            {
                std::cerr << "Warning: Invalid depth value, using default: 3\n";
            }
        }

        printTree(gmod, rootCode, maxDepth);
        return 0;
    }

    if (arg1 == "--path" || arg1 == "-p")
    {
        if (argc < argOffset + 2)
        {
            std::cerr << "Error: --path requires a path string\n";
            return 1;
        }

        std::string_view path = argv[argOffset + 1];
        validatePath(gmod, locations, path);
        return 0;
    }

    if (arg1 == "--dump-map")
    {
        if (argc < argOffset + 2)
        {
            std::cerr << "Error: --dump-map requires a filename\n";
            return 1;
        }

        std::string_view filename = argv[argOffset + 1];
        dumpMap(gmod, filename);
        return 0;
    }

    std::cerr << "Error: Unknown option '" << arg1 << "'\n";
    std::cerr << "Use --help to see available options\n";
    return 1;
}

void printHelp()
{
    std::cout << "VIS Gmod CLI - Navigate the Generic Product Model\n";
    std::cout << "Vista SDK version: " << dnv::vista::sdk::version() << "\n\n";
    std::cout << "Usage:\n";
    std::cout << "  gmod-cli                       Show tree overview (latest version)\n";
    std::cout << "  gmod-cli --version <ver>       Use specific VIS version\n";
    std::cout << "  gmod-cli --list-versions       List available VIS versions\n";
    std::cout << "  gmod-cli --code <code>         Show node details\n";
    std::cout << "  gmod-cli --search <term>       Search nodes by name/definition\n";
    std::cout << "  gmod-cli --tree <code> [d]     Show subtree (depth d, default 3)\n";
    std::cout << "  gmod-cli --path <path>         Parse and validate Gmod path\n";
    std::cout << "  gmod-cli --dump-map <file>     Export all nodes to CSV file\n";
    std::cout << "  gmod-cli --help                Show this help\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help              Show this help message\n";
    std::cout << "  -l, --list-versions     List all available VIS versions\n";
    std::cout << "  -v, --version <ver>     Use specific VIS version (e.g., 3-4a)\n";
    std::cout << "  -c, --code <code>       Show detailed info for node code\n";
    std::cout << "  -s, --search <term>     Search for nodes matching term\n";
    std::cout << "  -t, --tree <code>       Display tree structure from node\n";
    std::cout << "  -p, --path <path>       Validate a Gmod path string\n";
    std::cout << "  --dump-map <file>       Export all nodes to CSV (hash,code,category,type,name,common "
                 "name,definition,install substructure,normal assignment names)\n\n";
    std::cout << "Examples:\n";
    std::cout << "  gmod-cli --code 411.1\n";
    std::cout << "  gmod-cli --search \"main engine\"\n";
    std::cout << "  gmod-cli --tree 400a 2\n";
    std::cout << "  gmod-cli --path \"411.1-1P\"\n";
    std::cout << "  gmod-cli --version 3-7a --code C101.31\n";
    std::cout << "  gmod-cli --dump-map gmod_nodes.txt\n";
    std::cout << "  gmod-cli --version 3-7a --dump-map gmod_nodes.csv\n";
}

void printVersions(const dnv::vista::sdk::VIS& vis)
{
    using namespace dnv::vista::sdk;

    std::cout << "Available VIS versions:\n";
    for (auto version : vis.versions())
    {
        std::cout << "  " << VisVersions::toString(version);
        if (version == vis.latest())
        {
            std::cout << " (latest)";
        }
        std::cout << "\n";
    }
}

void printOverview(const dnv::vista::sdk::Gmod& gmod)
{
    using namespace dnv::vista::sdk;

    std::cout << "Gmod Overview - VIS version: " << VisVersions::toString(gmod.version()) << "\n";
    std::cout << std::string(60, '=') << "\n\n";

    size_t totalNodes = 0;
    for ([[maybe_unused]] const auto& [code, node] : gmod)
    {
        totalNodes++;
    }

    std::cout << "Total nodes: " << totalNodes << "\n";
    std::cout << "Root node: " << gmod.rootNode().code() << " (" << gmod.rootNode().metadata().name() << ")\n\n";

    std::cout << "Top-level categories:\n";
    const auto& root = gmod.rootNode();

    std::vector<const GmodNode*> sortedChildren(root.children().begin(), root.children().end());
    std::sort(sortedChildren.begin(), sortedChildren.end(), [](const GmodNode* a, const GmodNode* b) {
        return naturalCompare(a->code(), b->code()) < 0;
    });

    for (const auto* child : sortedChildren)
    {
        std::cout << "  " << std::setw(10) << std::left << child->code() << " - " << child->metadata().name() << "\n";
    }

    std::cout << "\nUse --help to see more options\n";
}

void printNodeDetails(const dnv::vista::sdk::Gmod& gmod, std::string_view code)
{
    using namespace dnv::vista::sdk;

    auto nodeOpt = gmod.node(code);
    if (!nodeOpt.has_value())
    {
        std::cerr << "Error: Node '" << code << "' not found in Gmod\n";
        return;
    }

    const auto* node = *nodeOpt;
    const auto& meta = node->metadata();

    std::cout << "Node Details: " << node->code() << "\n";
    std::cout << std::string(60, '=') << "\n\n";

    std::cout << "Code:       " << node->code() << "\n";
    std::cout << "Category:   " << meta.category() << "\n";
    std::cout << "Type:       " << meta.type() << "\n";
    std::cout << "Name:       " << meta.name() << "\n";

    if (meta.commonName().has_value())
    {
        const auto& commonName = meta.commonName().value();
        if (!commonName.empty())
        {
            std::cout << "Common:     " << commonName << "\n";
        }
    }

    if (meta.definition().has_value())
    {
        std::cout << "Definition: " << meta.definition().value() << "\n";
    }

    if (meta.installSubstructure().has_value())
    {
        std::cout << "Install:    " << meta.installSubstructure().value() << "\n";
    }

    std::cout << "\nHierarchy:\n";
    std::cout << "Is root:    " << std::boolalpha << node->isRoot() << "\n";

    if (!node->parents().isEmpty())
    {
        std::cout << "Parents (" << node->parents().size() << "):\n";
        std::vector<const GmodNode*> sortedParents(node->parents().begin(), node->parents().end());
        std::sort(sortedParents.begin(), sortedParents.end(), [](const GmodNode* a, const GmodNode* b) {
            return naturalCompare(a->code(), b->code()) < 0;
        });
        for (const auto* parent : sortedParents)
        {
            std::cout << "  - " << parent->code() << " (" << parent->metadata().name() << ")\n";
        }
    }

    if (!node->children().isEmpty())
    {
        std::cout << "Children (" << node->children().size() << "):\n";
        std::vector<const GmodNode*> sortedChildren(node->children().begin(), node->children().end());
        std::sort(sortedChildren.begin(), sortedChildren.end(), [](const GmodNode* a, const GmodNode* b) {
            return naturalCompare(a->code(), b->code()) < 0;
        });
        size_t maxShow = 14;
        size_t count = 0;
        for (const auto* child : sortedChildren)
        {
            if (count++ >= maxShow)
            {
                std::cout << "  ... and " << (node->children().size() - maxShow) << " more\n";
                break;
            }
            std::cout << "  - " << child->code() << " (" << child->metadata().name() << ")\n";
        }
    }

    if (auto prodTypeOpt = node->productType())
    {
        const auto* prodType = *prodTypeOpt;
        std::cout << "\nProduct Type: " << prodType->code() << " (" << prodType->metadata().name() << ")\n";
    }

    if (auto prodSelOpt = node->productSelection())
    {
        const auto* prodSel = *prodSelOpt;
        std::cout << "Product Selection: " << prodSel->code() << " (" << prodSel->metadata().name() << ")\n";
    }
}

void searchNodes(const dnv::vista::sdk::Gmod& gmod, std::string_view searchTerm)
{
    using namespace dnv::vista::sdk;

    std::string searchLower(searchTerm);
    std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    std::cout << "Searching for: \"" << searchTerm << "\"\n";
    std::cout << std::string(60, '=') << "\n\n";

    std::vector<const GmodNode*> matches;

    for (const auto& [code, node] : gmod)
    {
        std::string_view name = node.metadata().name();
        std::string nameLower(name);
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        std::string commonLower;
        if (node.metadata().commonName().has_value())
        {
            const auto& commonName = node.metadata().commonName().value();
            commonLower = commonName;
            std::transform(commonLower.begin(), commonLower.end(), commonLower.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
        }

        std::string defLower;
        if (node.metadata().definition().has_value())
        {
            const auto& definition = node.metadata().definition().value();
            defLower = definition;
            std::transform(defLower.begin(), defLower.end(), defLower.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
        }

        if (nameLower.find(searchLower) != std::string::npos || commonLower.find(searchLower) != std::string::npos ||
            defLower.find(searchLower) != std::string::npos || code.find(searchTerm) != std::string::npos)
        {
            matches.push_back(&node);
        }
    }

    if (matches.empty())
    {
        std::cout << "No matches found.\n";
        return;
    }

    std::sort(matches.begin(), matches.end(), [](const GmodNode* a, const GmodNode* b) {
        return naturalCompare(a->code(), b->code()) < 0;
    });

    std::cout << "Found " << matches.size() << " match(es):\n\n";

    for (const auto* node : matches)
    {
        std::cout << std::setw(12) << std::left << node->code() << " - " << node->metadata().name();

        if (node->metadata().commonName().has_value())
        {
            const auto& commonName = node->metadata().commonName().value();
            if (!commonName.empty())
            {
                std::cout << " (" << commonName << ")";
            }
        }

        std::cout << "\n";

        if (matches.size() <= 20 && node->metadata().definition().has_value())
        {
            std::cout << std::string(15, ' ') << "\"" << node->metadata().definition().value() << "\"\n";
        }
    }

    if (matches.size() > 20)
    {
        std::cout << "\nTip: Use --code <code> to see details for a specific node\n";
    }
}

void printTreeRecursive(
    const dnv::vista::sdk::GmodNode& node,
    int currentDepth,
    int maxDepth,
    std::set<const dnv::vista::sdk::GmodNode*>& visited)
{
    using namespace dnv::vista::sdk;

    if (currentDepth > maxDepth)
    {
        return;
    }

    if (visited.count(&node) > 0)
    {
        std::cout << std::string(currentDepth * 2, ' ') << "- " << node.code() << " (already shown)\n";
        return;
    }

    visited.insert(&node);

    std::cout << std::string(currentDepth * 2, ' ') << "- " << node.code() << " (" << node.metadata().name() << ")\n";

    std::vector<const GmodNode*> sortedChildren(node.children().begin(), node.children().end());
    std::sort(sortedChildren.begin(), sortedChildren.end(), [](const GmodNode* a, const GmodNode* b) {
        return naturalCompare(a->code(), b->code()) < 0;
    });

    for (const auto* child : sortedChildren)
    {
        printTreeRecursive(*child, currentDepth + 1, maxDepth, visited);
    }
}

void printTree(const dnv::vista::sdk::Gmod& gmod, std::string_view rootCode, int maxDepth)
{
    using namespace dnv::vista::sdk;

    auto nodeOpt = gmod.node(rootCode);
    if (!nodeOpt.has_value())
    {
        std::cerr << "Error: Node '" << rootCode << "' not found in Gmod\n";
        return;
    }

    const auto* node = *nodeOpt;

    std::cout << "Tree from node: " << node->code() << " (max depth: " << maxDepth << ")\n";
    std::cout << std::string(60, '=') << "\n\n";

    std::set<const GmodNode*> visited;
    printTreeRecursive(*node, 0, maxDepth, visited);
}

void validatePath(
    const dnv::vista::sdk::Gmod& gmod, const dnv::vista::sdk::Locations& locations, std::string_view pathStr)
{
    using namespace dnv::vista::sdk;

    std::cout << "Validating path: \"" << pathStr << "\"\n";
    std::cout << std::string(60, '=') << "\n\n";

    auto pathOpt = GmodPath::fromShortPath(pathStr, gmod, locations);
    if (pathOpt.has_value())
    {
        const auto& path = *pathOpt;

        std::cout << "Valid short path\n\n";
        std::cout << "    Short path:     " << path.toString() << "\n";
        std::cout << "    Full path:      " << path.toFullPathString() << "\n";
        std::cout << "    Length:         " << path.length() << "\n";

        if (path.node().location().has_value())
        {
            std::cout << "Location:       " << path.node().location()->value() << "\n";
        }

        std::cout << "\nPath nodes:\n";
        for (size_t i = 0; i < path.length(); ++i)
        {
            const auto& parents = path.parents();
            const GmodNode* node = (i == path.length() - 1) ? &path.node() : &parents[i];
            std::cout << "  " << (i + 1) << ". " << node->code() << " - " << node->metadata().name() << "\n";
        }

        return;
    }

    pathOpt = GmodPath::fromFullPath(pathStr, gmod, locations);
    if (pathOpt.has_value())
    {
        const auto& path = *pathOpt;

        std::cout << "Valid full path\n\n";
        std::cout << "    Short path:     " << path.toString() << "\n";
        std::cout << "    Full path:      " << path.toFullPathString() << "\n";
        std::cout << "    Length:         " << path.length() << "\n";

        if (path.node().location().has_value())
        {
            std::cout << "Location:       " << path.node().location()->value() << "\n";
        }

        return;
    }

    std::cout << "Invalid path\n";
    std::cout << "    The path could not be parsed as either:\n";
    std::cout << "      - Short path (e.g., \"411.1-1P\")\n";
    std::cout << "      - Full path (e.g., \"VE/400a/410/411/411.1-1P\")\n";
}

void dumpMap(const dnv::vista::sdk::Gmod& gmod, std::string_view filename)
{
    using namespace dnv::vista::sdk;

    std::cout << "Dumping Gmod node map to " << filename << "...\n";

    auto escapeCsv = [](const std::string& str) -> std::string {
        std::string result = str;
        std::replace(result.begin(), result.end(), '\n', ' ');
        size_t pos = 0;
        while ((pos = result.find('"', pos)) != std::string::npos)
        {
            result.insert(pos, 1, '"');
            pos += 2;
        }
        return result;
    };
    std::ofstream outFile(filename.data(), std::ios::out | std::ios::trunc);
    if (!outFile.is_open())
    {
        std::cerr << "Error: Failed to open " << filename << " for writing\n";
        return;
    }

    std::vector<std::pair<std::string, const GmodNode*>> sortedEntries;
    sortedEntries.reserve(8000);
    for (const auto& [code, node] : gmod)
    {
        sortedEntries.emplace_back(std::string{ code }, &node);
    }
    std::sort(sortedEntries.begin(), sortedEntries.end(), [](const auto& a, const auto& b) {
        return naturalCompare(a.first, b.first) < 0;
    });

    // Write header
    outFile << "HASH,CODE,CATEGORY,TYPE,NAME,COMMON NAME,DEFINITION,COMMON DEFINITION,INSTALL SUBSTRUCTURE,NORMAL "
               "ASSIGNMENT NAMES\n";

    size_t count = 0;
    for (const auto& [code, nodePtr] : sortedEntries)
    {
        std::uint32_t hashValue = dnv::vista::sdk::internal::hashString(code);

        // Prepare NAME field
        std::string_view name = nodePtr->metadata().name();
        std::string displayName = escapeCsv(std::string(name));

        // Prepare COMMON NAME field
        std::string displayCommonName;
        if (nodePtr->metadata().commonName().has_value())
        {
            std::string_view commonName = nodePtr->metadata().commonName().value();
            displayCommonName = escapeCsv(std::string(commonName));
        }

        // Prepare DEFINITION field
        std::string displayDefinition;
        if (nodePtr->metadata().definition().has_value())
        {
            std::string_view definition = nodePtr->metadata().definition().value();
            displayDefinition = escapeCsv(std::string(definition));
        }

        // Prepare COMMON DEFINITION field
        std::string displayCommonDefinition;
        if (nodePtr->metadata().commonDefinition().has_value())
        {
            std::string_view commonDefinition = nodePtr->metadata().commonDefinition().value();
            displayCommonDefinition = escapeCsv(std::string(commonDefinition));
        }

        // Prepare INSTALL SUBSTRUCTURE field
        std::string installSubstructureStr;
        auto installSubstructure = nodePtr->metadata().installSubstructure();
        if (installSubstructure.has_value())
        {
            installSubstructureStr = installSubstructure.value() ? "true" : "false";
        }
        else
        {
            installSubstructureStr = "N/A";
        }

        // Prepare NORMAL ASSIGNMENT NAMES field
        std::string normalAssignmentNamesStr;
        const auto& normalAssignmentNames = nodePtr->metadata().normalAssignmentNames();
        bool first = true;
        for (const auto& [key, value] : normalAssignmentNames)
        {
            if (!first)
            {
                normalAssignmentNamesStr += ", ";
            }
            normalAssignmentNamesStr += key + "=" + value;
            first = false;
        }

        outFile << "0x" << std::hex << std::setw(8) << std::setfill('0') << hashValue << std::dec << std::setfill(' ')
                << ",\"" << code << "\""
                << ",\"" << nodePtr->metadata().category() << "\""
                << ",\"" << nodePtr->metadata().type() << "\""
                << ",\"" << displayName << "\""
                << ",\"" << displayCommonName << "\""
                << ",\"" << displayDefinition << "\""
                << ",\"" << displayCommonDefinition << "\""
                << ",\"" << installSubstructureStr << "\""
                << ",\"" << normalAssignmentNamesStr << "\"\n";
        ++count;
    }

    outFile.close();
    std::cout << "Successfully dumped " << count << " nodes to " << filename << "\n";
}
