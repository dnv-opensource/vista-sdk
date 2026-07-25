/**
 * @file locations-cli.cpp
 * @brief CLI tool for exploring VIS locations
 * @details Command-line interface to view and search VIS location codes
 *
 * Usage:
 *   locations-cli                 # Print all locations for latest VIS version
 *   locations-cli --version 3-4a  # Print locations for specific version
 *   locations-cli --list-versions # List available VIS versions
 *   locations-cli --code P        # Show details for location code 'P'
 *   locations-cli --search port   # Search for location by name
 *   locations-cli --help          # Show help
 */

#include <dnv/VistaSDK.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

void printHelp();
void printVersions(const dnv::vista::sdk::VIS& vis);
void printLocations(const dnv::vista::sdk::Locations& locations);
void printLocationByCode(const dnv::vista::sdk::Locations& locations, std::string_view code);
void searchInLocations(const dnv::vista::sdk::Locations& locations, std::string_view searchTerm);

int main(int argc, char* argv[])
{
    using namespace dnv::vista::sdk;

    const auto& vis = VIS::instance();

    if (argc == 1)
    {
        const auto& locations = vis.locations(vis.latest());
        printLocations(locations);
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

        const auto& locations = vis.locations(versionOpt.value());
        printLocations(locations);
        return 0;
    }

    if (arg1 == "--code" || arg1 == "-c")
    {
        if (argc < 3)
        {
            std::cerr << "Error: --code requires a location code argument\n";
            return 1;
        }

        std::string_view code = argv[2];
        const auto& locations = vis.locations(vis.latest());
        printLocationByCode(locations, code);
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
        const auto& locations = vis.locations(vis.latest());
        searchInLocations(locations, searchTerm);
        return 0;
    }

    std::cerr << "Error: Unknown option '" << arg1 << "'\n";
    std::cerr << "Use --help to see available options\n";
    return 1;
}

void printHelp()
{
    std::cout << "VIS Locations CLI - Explore VIS location codes\n";
    std::cout << "Vista SDK version: " << dnv::vista::sdk::version() << "\n\n";
    std::cout << "Usage:\n";
    std::cout << "  locations-cli                       Print all locations (latest version)\n";
    std::cout << "  locations-cli --version <ver>       Print locations for specific version\n";
    std::cout << "  locations-cli --list-versions       List available VIS versions\n";
    std::cout << "  locations-cli --code <code>         Show details for specific location code\n";
    std::cout << "  locations-cli --search <term>       Search for location by name\n";
    std::cout << "  locations-cli --help                Show this help\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help                Show this help message\n";
    std::cout << "  -l, --list-versions       List all available VIS versions\n";
    std::cout << "  -v, --version <version>   Use specific VIS version (e.g., 3-4a)\n";
    std::cout << "  -c, --code <code>         Show specific location code (e.g., P, S, F)\n";
    std::cout << "  -s, --search <term>       Search for term in location names\n\n";
    std::cout << "Examples:\n";
    std::cout << "  locations-cli\n";
    std::cout << "  locations-cli --version 3-4a\n";
    std::cout << "  locations-cli --code P\n";
    std::cout << "  locations-cli --search port\n";
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

void printLocations(const dnv::vista::sdk::Locations& locations)
{
    using namespace dnv::vista::sdk;

    std::cout << "=== VIS Location Codes ===\n";
    std::cout << "Version: " << VisVersions::toString(locations.version()) << "\n\n";

    const auto& relativeLocations = locations.relativeLocations();

    std::cout << "==========================================\n";
    std::cout << "Total Locations: " << relativeLocations.size() << "\n";
    std::cout << "==========================================\n\n";

    std::cout << std::left << std::setw(8) << "Code";
    std::cout << std::setw(25) << "Name";
    std::cout << "Definition\n";
    std::cout << std::string(120, '-') << "\n";

    for (const auto& location : relativeLocations)
    {
        std::cout << std::left << std::setw(8) << location.code();
        std::cout << std::setw(25) << location.name();

        const auto& defOpt = location.definition();
        if (!defOpt.has_value())
        {
            std::cout << "(no definition)\n";
            continue;
        }

        const auto& definition = defOpt.value();
        const size_t defWidth = 80;
        const size_t indent = 33; // 8 + 25

        if (definition.size() <= defWidth)
        {
            std::cout << definition << "\n";
        }
        else
        {
            std::string_view remaining = definition;
            bool first = true;
            while (!remaining.empty())
            {
                size_t lineLen = std::min(defWidth, remaining.size());

                if (lineLen < remaining.size())
                {
                    size_t lastSpace = remaining.substr(0, lineLen).rfind(' ');
                    if (lastSpace != std::string_view::npos)
                    {
                        lineLen = lastSpace;
                    }
                }

                if (!first)
                {
                    std::cout << std::string(indent, ' ');
                }
                std::cout << remaining.substr(0, lineLen) << "\n";

                remaining.remove_prefix(lineLen);
                if (!remaining.empty() && remaining[0] == ' ')
                {
                    remaining.remove_prefix(1);
                }
                first = false;
            }
        }
    }
}

void printLocationByCode(const dnv::vista::sdk::Locations& locations, std::string_view codeStr)
{
    using namespace dnv::vista::sdk;

    std::cout << "=== VIS Location Code ===\n";
    std::cout << "Version: " << VisVersions::toString(locations.version()) << "\n\n";

    const auto& relativeLocations = locations.relativeLocations();
    bool found = false;

    for (const auto& location : relativeLocations)
    {
        if (codeStr.size() == 1 && location.code() == codeStr[0])
        {
            std::cout << "Code:       " << location.code() << "\n";
            std::cout << "Name:       " << location.name() << "\n";
            std::cout << "Definition: ";

            if (location.definition().has_value())
            {
                std::cout << location.definition().value() << "\n";
            }
            else
            {
                std::cout << "(no definition)\n";
            }

            found = true;
            break;
        }
    }

    if (!found)
    {
        std::cerr << "Error: Location code '" << codeStr << "' not found\n";
        std::cerr << "Use locations-cli to see all available codes\n";
    }
}

void searchInLocations(const dnv::vista::sdk::Locations& locations, std::string_view searchTerm)
{
    using namespace dnv::vista::sdk;

    std::cout << "=== VIS Location Search ===\n";
    std::cout << "Version: " << VisVersions::toString(locations.version()) << "\n";
    std::cout << "Search term: \"" << searchTerm << "\"\n\n";

    std::vector<std::reference_wrapper<const RelativeLocation>> matches;

    std::string searchLower{ searchTerm };
    std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    const auto& relativeLocations = locations.relativeLocations();

    for (const auto& location : relativeLocations)
    {
        std::string codeLower{ location.code() };
        std::string nameLower = location.name();

        std::transform(codeLower.begin(), codeLower.end(), codeLower.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        bool match = false;
        if (codeLower.find(searchLower) != std::string::npos || nameLower.find(searchLower) != std::string::npos)
        {
            match = true;
        }
        else if (location.definition().has_value())
        {
            std::string defLower = location.definition().value();
            std::transform(defLower.begin(), defLower.end(), defLower.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });

            if (defLower.find(searchLower) != std::string::npos)
            {
                match = true;
            }
        }

        if (match)
        {
            matches.push_back(std::cref(location));
        }
    }

    if (matches.empty())
    {
        std::cout << "No matches found for '" << searchTerm << "'\n";
        return;
    }

    std::cout << "Found " << matches.size() << " match(es):\n\n";

    std::cout << std::left << std::setw(8) << "Code";
    std::cout << std::setw(25) << "Name";
    std::cout << "Definition\n";
    std::cout << std::string(120, '-') << "\n";

    for (const auto& locRef : matches)
    {
        const auto& location = locRef.get();
        std::cout << std::left << std::setw(8) << location.code();
        std::cout << std::setw(25) << location.name();

        if (location.definition().has_value())
        {
            std::cout << location.definition().value() << "\n";
        }
        else
        {
            std::cout << "(no definition)\n";
        }
    }
}
