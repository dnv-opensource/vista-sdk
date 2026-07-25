/**
 * @file codebooks-cli.cpp
 * @brief CLI tool for exploring VIS codebooks
 * @details Command-line interface to view and search VIS codebook values
 *
 * Usage:
 *   codebooks-cli                      # Print all codebooks for latest VIS version
 *   codebooks-cli --version 3-4a       # Print codebooks for specific version
 *   codebooks-cli --list-versions      # List available VIS versions
 *   codebooks-cli --codebook qty       # Print only Quantity codebook
 *   codebooks-cli --search temperature # Search for value in all codebooks
 *   codebooks-cli --help               # Show help
 */

#include <dnv/VistaSDK.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace
{
    struct CodebookInfo
    {
        dnv::vista::sdk::CodebookName name;
        std::string displayName;
    };

    const std::vector<CodebookInfo> g_allCodebooks = {
        { dnv::vista::sdk::CodebookName::Quantity, "Quantity" },
        { dnv::vista::sdk::CodebookName::Content, "Content" },
        { dnv::vista::sdk::CodebookName::Calculation, "Calculation" },
        { dnv::vista::sdk::CodebookName::State, "State" },
        { dnv::vista::sdk::CodebookName::Command, "Command" },
        { dnv::vista::sdk::CodebookName::Type, "Type" },
        { dnv::vista::sdk::CodebookName::FunctionalServices, "Functional Services" },
        { dnv::vista::sdk::CodebookName::MaintenanceCategory, "Maintenance Category" },
        { dnv::vista::sdk::CodebookName::ActivityType, "Activity Type" },
        { dnv::vista::sdk::CodebookName::Position, "Position" },
        { dnv::vista::sdk::CodebookName::Detail, "Detail" }
    };
} // namespace

void printHelp();
void printVersions(const dnv::vista::sdk::VIS& vis);
void printCodebooks(const dnv::vista::sdk::Codebooks& codebooks);
void printSingleCodebook(const dnv::vista::sdk::Codebooks& codebooks, std::string_view codebookName);
void searchInCodebooks(const dnv::vista::sdk::Codebooks& codebooks, std::string_view searchTerm);

int main(int argc, char* argv[])
{
    using namespace dnv::vista::sdk;

    const auto& vis = VIS::instance();

    if (argc == 1)
    {
        const auto& codebooks = vis.codebooks(vis.latest());
        printCodebooks(codebooks);
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

        const auto& codebooks = vis.codebooks(versionOpt.value());
        printCodebooks(codebooks);
        return 0;
    }

    if (arg1 == "--codebook" || arg1 == "-c")
    {
        if (argc < 3)
        {
            std::cerr << "Error: --codebook requires a codebook name argument\n";
            return 1;
        }

        std::string_view codebookName = argv[2];
        const auto& codebooks = vis.codebooks(vis.latest());
        printSingleCodebook(codebooks, codebookName);
        return 0;
    }

    if (arg1 == "--search" || arg1 == "-s")
    {
        if (argc < 3)
        {
            std::cerr << "Error: --search requires a search term\n";
            return 1;
        }

        std::string_view searchTerm = argv[2];
        const auto& codebooks = vis.codebooks(vis.latest());
        searchInCodebooks(codebooks, searchTerm);
        return 0;
    }

    std::cerr << "Error: Unknown option '" << arg1 << "'\n";
    std::cerr << "Use --help to see available options\n";
    return 1;
}

void printHelp()
{
    std::cout << "VIS Codebooks CLI - Explore VIS codebook values\n";
    std::cout << "Vista SDK version: " << dnv::vista::sdk::version() << "\n\n";
    std::cout << "Usage:\n";
    std::cout << "  codebooks-cli                       Print all codebooks (latest version)\n";
    std::cout << "  codebooks-cli --version <ver>       Print codebooks for specific version\n";
    std::cout << "  codebooks-cli --list-versions       List available VIS versions\n";
    std::cout << "  codebooks-cli --codebook <name>     Print specific codebook only\n";
    std::cout << "  codebooks-cli --search <term>       Search for value in all codebooks\n";
    std::cout << "  codebooks-cli --help                Show this help\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help                Show this help message\n";
    std::cout << "  -l, --list-versions       List all available VIS versions\n";
    std::cout << "  -v, --version <version>   Use specific VIS version (e.g., 3-4a)\n";
    std::cout << "  -c, --codebook <name>     Show specific codebook (qty, cnt, state, etc.)\n";
    std::cout << "  -s, --search <term>       Search for term in all codebooks\n\n";
    std::cout << "Examples:\n";
    std::cout << "  codebooks-cli\n";
    std::cout << "  codebooks-cli --version 3-4a\n";
    std::cout << "  codebooks-cli --codebook qty\n";
    std::cout << "  codebooks-cli --search temperature\n";
}

void printVersions(const dnv::vista::sdk::VIS& vis)
{
    using namespace dnv::vista::sdk;

    std::cout << "Available VIS versions:\n\n";

    const auto& versions = vis.versions();
    auto latest = vis.latest();

    for (const auto& version : versions)
    {
        std::cout << "  " << VisVersions::toString(version);
        if (version == latest)
        {
            std::cout << " (latest)";
        }
        std::cout << "\n";
    }
}

void printCodebooks(const dnv::vista::sdk::Codebooks& codebooks)
{
    using namespace dnv::vista::sdk;

    std::cout << "=== VIS Codebooks - Standard Values ===\n";
    std::cout << "Version: " << VisVersions::toString(codebooks.version()) << "\n\n";

    std::cout << "==========================================\n";
    std::cout << "Available Codebooks\n";
    std::cout << "==========================================\n\n";

    for (const auto& info : g_allCodebooks)
    {
        try
        {
            const auto& codebook = codebooks[info.name];
            const auto& standardValues = codebook.standardValues();
            const auto& groups = codebook.groups();

            std::cout << std::left << std::setw(30) << info.displayName;
            std::cout << " ('" << std::left << std::setw(10) << CodebookNames::toPrefix(info.name) << "')";
            std::cout << " : " << std::right << std::setw(3) << standardValues.size() << " values";
            if (!groups.isEmpty())
            {
                std::cout << ", " << std::setw(2) << groups.size() << " groups";
            }
            std::cout << "\n";
        }
        catch (const std::exception&)
        {
            // Codebook doesn't exist in this version, skip it
        }
    }

    std::cout << "\n";

    for (const auto& info : g_allCodebooks)
    {
        try
        {
            const auto& codebook = codebooks[info.name];
            const auto& standardValues = codebook.standardValues();

            std::cout << "==========================================\n";
            std::cout << info.displayName << " (" << CodebookNames::toPrefix(info.name) << ")\n";
            std::cout << "==========================================\n";
            std::cout << "Total standard values: " << standardValues.size() << "\n\n";

            const size_t columnsCount = 3;
            const size_t columnWidth = 35;
            size_t count = 0;

            for (const auto& value : standardValues)
            {
                std::cout << std::left << std::setw(columnWidth) << value;
                count++;

                if (count % columnsCount == 0)
                {
                    std::cout << "\n";
                }
            }

            if (count % columnsCount != 0)
            {
                std::cout << "\n";
            }

            std::cout << "\n";
        }
        catch (const std::exception&)
        {
            // Codebook doesn't exist in this version, skip it
        }
    }

    std::cout << "==========================================\n";
    std::cout << "Codebook Groups\n";
    std::cout << "==========================================\n\n";

    for (const auto& info : g_allCodebooks)
    {
        try
        {
            const auto& codebook = codebooks[info.name];
            const auto& groups = codebook.groups();

            if (!groups.isEmpty())
            {
                std::cout << std::left << std::setw(30) << info.displayName << " : " << std::setw(2) << groups.size()
                          << " groups\n";
                for (const auto& group : groups)
                {
                    std::cout << "  - " << group << "\n";
                }
                std::cout << "\n";
            }
        }
        catch (const std::exception&)
        {
            // Codebook doesn't exist in this version, skip it
        }
    }
}

void printSingleCodebook(const dnv::vista::sdk::Codebooks& codebooks, std::string_view codebookName)
{
    using namespace dnv::vista::sdk;

    try
    {
        auto codebookNameEnum = CodebookNames::fromPrefix(codebookName);
        const auto& codebook = codebooks[codebookNameEnum];
        const auto& standardValues = codebook.standardValues();

        std::cout << "Codebook     : '" << CodebookNames::toPrefix(codebookNameEnum) << "'\n";
        std::cout << "VIS Version  : " << VisVersions::toString(codebooks.version()) << "\n";
        std::cout << "Total values : " << standardValues.size() << "\n\n";

        if (standardValues.isEmpty())
        {
            std::cout << "No standard values (accepts all custom values)\n";
            return;
        }

        const size_t columnsCount = 3;
        const size_t columnWidth = 35;
        size_t count = 0;

        for (const auto& value : standardValues)
        {
            std::cout << std::left << std::setw(columnWidth) << value;
            count++;

            if (count % columnsCount == 0)
            {
                std::cout << "\n";
            }
        }

        if (count % columnsCount != 0)
        {
            std::cout << "\n";
        }

        const auto& groups = codebook.groups();
        if (!groups.isEmpty())
        {
            std::cout << "\nGroups (" << groups.size() << "):\n";
            for (const auto& group : groups)
            {
                std::cout << "  - " << group << "\n";
            }
        }
    }
    catch (const std::exception&)
    {
        std::cerr << "Error: Unknown codebook '" << codebookName << "'\n";
        std::cerr
            << "Available codebooks: qty, cnt, calc, state, cmd, type, funct.svc, maint.cat, act.type, pos, detail\n";
    }
}

void searchInCodebooks(const dnv::vista::sdk::Codebooks& codebooks, std::string_view searchTerm)
{
    using namespace dnv::vista::sdk;

    std::cout << "Searching for '" << searchTerm << "' in VIS " << VisVersions::toString(codebooks.version())
              << " codebooks...\n\n";

    bool foundAny = false;

    for (const auto& info : g_allCodebooks)
    {
        try
        {
            const auto& codebook = codebooks[info.name];
            const auto& standardValues = codebook.standardValues();

            std::vector<std::string> matches;
            for (const auto& value : standardValues)
            {
                std::string valueLower = value;
                std::string searchLower{ searchTerm };
                std::transform(valueLower.begin(), valueLower.end(), valueLower.begin(), [](unsigned char c) {
                    return static_cast<char>(std::tolower(c));
                });
                std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), [](unsigned char c) {
                    return static_cast<char>(std::tolower(c));
                });

                if (valueLower.find(searchLower) != std::string::npos)
                {
                    matches.push_back(value);
                }
            }

            if (!matches.empty())
            {
                foundAny = true;
                std::cout << std::left << std::setw(30) << info.displayName;
                std::cout << " ('" << std::left << std::setw(10) << CodebookNames::toPrefix(info.name) << "')";
                std::cout << " : " << std::right << std::setw(2) << matches.size() << " match(es)\n";
                for (const auto& match : matches)
                {
                    std::cout << "  - " << match << "\n";
                }
                std::cout << "\n";
            }
        }
        catch (const std::exception&)
        {
            // Codebook doesn't exist in this version, skip it
        }
    }

    if (!foundAny)
    {
        std::cout << "No matches found for '" << searchTerm << "'\n";
    }
}
