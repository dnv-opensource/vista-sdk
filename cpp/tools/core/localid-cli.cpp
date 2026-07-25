/**
 * @file localid-cli.cpp
 * @brief CLI tool for working with VIS LocalIds
 * @details Command-line interface to parse, validate, query, and analyze LocalIds
 *
 * Usage:
 *   localid-cli --parse <localid>               # Parse and display structure
 *   localid-cli --validate <file>               # Validate LocalIds from file
 *   localid-cli --query <file> --primary <path> # Filter LocalIds with query
 *   localid-cli --stats <file>                  # Show statistics
 *   localid-cli --help                          # Show help
 */

#include <dnv/VistaSDK.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <vector>

void printHelp();
void parseLocalId(std::string_view localIdStr);
void validateFile(std::string_view filename, bool showStats);
void queryFile(std::string_view filename, int argc, char* argv[], int startArg);
void showStats(std::string_view filename);

struct ValidationResult
{
    size_t lineNumber;
    std::string localIdStr;
    bool isValid;
    std::string errorMessage;
};

int main(int argc, char* argv[])
{
    using namespace dnv::vista::sdk;

    if (argc == 1)
    {
        printHelp();
        return 0;
    }

    std::string_view arg1 = argv[1];

    if (arg1 == "--help" || arg1 == "-h")
    {
        printHelp();
        return 0;
    }

    if (arg1 == "--parse" || arg1 == "-p")
    {
        if (argc < 3)
        {
            std::cerr << "Error: --parse requires a LocalId argument\n";
            return 1;
        }

        std::string_view localIdStr = argv[2];
        parseLocalId(localIdStr);
        return 0;
    }

    if (arg1 == "--validate" || arg1 == "-v")
    {
        if (argc < 3)
        {
            std::cerr << "Error: --validate requires a filename argument\n";
            return 1;
        }

        std::string_view filename = argv[2];
        bool showStats = false;

        if (argc > 3)
        {
            std::string_view arg3 = argv[3];
            if (arg3 == "--stats" || arg3 == "-s")
            {
                showStats = true;
            }
        }

        validateFile(filename, showStats);
        return 0;
    }

    if (arg1 == "--query" || arg1 == "-q")
    {
        if (argc < 3)
        {
            std::cerr << "Error: --query requires a filename argument\n";
            return 1;
        }

        std::string_view filename = argv[2];
        queryFile(filename, argc, argv, 3);
        return 0;
    }

    if (arg1 == "--stats" || arg1 == "-s")
    {
        if (argc < 3)
        {
            std::cerr << "Error: --stats requires a filename argument\n";
            return 1;
        }

        std::string_view filename = argv[2];
        showStats(filename);
        return 0;
    }

    std::cerr << "Error: Unknown option '" << arg1 << "'\n";
    std::cerr << "Use --help to see available options\n";
    return 1;
}

void printHelp()
{
    std::cout << "VIS LocalId CLI - Parse, validate and query LocalIds\n";
    std::cout << "Vista SDK version: " << dnv::vista::sdk::version() << "\n\n";
    std::cout << "Usage:\n";
    std::cout << "  localid-cli --parse <localid>              Parse and display LocalId structure\n";
    std::cout << "  localid-cli --validate <file>              Validate LocalIds from file (one per line)\n";
    std::cout << "  localid-cli --validate <file> --stats      Validate and show statistics\n";
    std::cout << "  localid-cli --query <file> [options]       Filter LocalIds with query\n";
    std::cout << "  localid-cli --stats <file>                 Show statistics about LocalIds\n";
    std::cout << "  localid-cli --help                         Show this help\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help                      Show this help message\n";
    std::cout << "  -p, --parse <localid>           Parse single LocalId\n";
    std::cout << "  -v, --validate <file>           Validate LocalIds from file\n";
    std::cout << "  -q, --query <file>              Query LocalIds from file\n";
    std::cout << "  -s, --stats <file>              Show statistics\n\n";
    std::cout << "Query options:\n";
    std::cout << "  --primary <path>                Filter by primary item path (e.g., \"411.1/C101\")\n";
    std::cout << "  --tag <name> <value>            Filter by metadata tag (e.g., qty temperature)\n";
    std::cout << "  --no-secondary                  Match only LocalIds without secondary item\n\n";
    std::cout << "Examples:\n";
    std::cout << "  localid-cli --parse \"/dnv-v2/vis-3-4a/411.1/C101.31/meta/qty-temperature\"\n";
    std::cout << "  localid-cli --validate localids.txt\n";
    std::cout << "  localid-cli --validate localids.txt --stats\n";
    std::cout << "  localid-cli --query localids.txt --primary \"411.1/C101\"\n";
    std::cout << "  localid-cli --query localids.txt --tag qty temperature\n";
    std::cout << "  localid-cli --stats localids.txt\n";
}

void parseLocalId(std::string_view localIdStr)
{
    using namespace dnv::vista::sdk;

    std::cout << "Parsing LocalId: " << localIdStr << "\n";
    std::cout << std::string(80, '=') << "\n\n";

    auto localIdOpt = LocalId::fromString(localIdStr);
    if (!localIdOpt.has_value())
    {
        std::cout << "INVALID: Could not parse LocalId\n";
        return;
    }

    const auto& localId = *localIdOpt;

    std::cout << "VIS Version:    " << VisVersions::toString(localId.version()) << "\n";
    std::cout << "Naming Rule:    " << LocalId::namingRule() << "\n";

    auto verboseLocalId = localId.builder().withVerboseMode(true).build();
    std::cout << "Verbose:        " << verboseLocalId.toString() << "\n\n";

    std::cout << "Primary Item:\n";
    std::cout << "  Short path:   " << localId.primaryItem().toString() << "\n";
    std::cout << "  Full path:    " << localId.primaryItem().toFullPathString() << "\n";

    if (localId.primaryItem().node().location().has_value())
    {
        std::cout << "  Location:     " << localId.primaryItem().node().location()->value() << "\n";
    }

    std::cout << "\n  Path breakdown:\n";
    const auto& primaryPath = localId.primaryItem();

    for (const auto& parentNode : primaryPath.parents())
    {
        std::cout << "    " << std::left << std::setw(15) << parentNode.code() << " - " << parentNode.metadata().name();

        if (parentNode.location().has_value())
        {
            std::cout << " [" << parentNode.location()->value() << "]";
        }

        std::cout << "\n";
    }

    const auto& targetNode = primaryPath.node();
    std::cout << "    " << std::left << std::setw(15) << targetNode.code() << " - " << targetNode.metadata().name();

    if (targetNode.location().has_value())
    {
        std::cout << " [" << targetNode.location()->value() << "]";
    }

    std::cout << "\n";

    if (localId.secondaryItem().has_value())
    {
        const auto& secondary = localId.secondaryItem().value();
        std::cout << "\nSecondary Item:\n";
        std::cout << "  Short path:   " << secondary.toString() << "\n";
        std::cout << "  Full path:    " << secondary.toFullPathString() << "\n";

        std::cout << "\n  Path breakdown:\n";

        for (const auto& parentNode : secondary.parents())
        {
            std::cout << "    " << std::left << std::setw(15) << parentNode.code() << " - "
                      << parentNode.metadata().name();

            if (parentNode.location().has_value())
            {
                std::cout << " [" << parentNode.location()->value() << "]";
            }

            std::cout << "\n";
        }

        const auto& secondaryTargetNode = secondary.node();
        std::cout << "    " << std::left << std::setw(15) << secondaryTargetNode.code() << " - "
                  << secondaryTargetNode.metadata().name();

        if (secondaryTargetNode.location().has_value())
        {
            std::cout << " [" << secondaryTargetNode.location()->value() << "]";
        }

        std::cout << "\n";
    }
    else
    {
        std::cout << "\nSecondary Item: (none)\n";
    }

    const auto& tags = localId.metadataTags();
    std::cout << "\nMetadata Tags:  " << tags.size() << " tag(s)\n";
    if (!tags.empty())
    {
        for (const auto& tag : tags)
        {
            std::cout << "  - " << std::left << std::setw(15) << CodebookNames::toPrefix(tag.name()) << " : "
                      << tag.value() << "\n";
        }
    }

    std::cout << "\nFull String:    " << localId.toString() << "\n";
}

void validateFile(std::string_view filename, bool showStats)
{
    using namespace dnv::vista::sdk;

    std::ifstream file(filename.data());
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file '" << filename << "'\n";
        return;
    }

    std::cout << "Validating LocalIds from: " << filename << "\n";
    std::cout << std::string(80, '=') << "\n\n";

    std::vector<ValidationResult> results;
    std::string line;
    size_t lineNumber = 0;
    size_t validCount = 0;
    size_t invalidCount = 0;

    while (std::getline(file, line))
    {
        lineNumber++;

        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos)
        {
            line = line.substr(0, commentPos);
        }

        auto start = line.find_first_not_of(" \t\r\n");
        auto end = line.find_last_not_of(" \t\r\n");
        if (start == std::string::npos)
        {
            continue;
        }
        std::string trimmed = line.substr(start, end - start + 1);

        ValidationResult result;
        result.lineNumber = lineNumber;
        result.localIdStr = trimmed;

        auto localIdOpt = LocalId::fromString(trimmed);
        if (localIdOpt.has_value())
        {
            result.isValid = true;
            validCount++;
        }
        else
        {
            result.isValid = false;
            result.errorMessage = "Failed to parse LocalId";
            invalidCount++;
        }

        results.push_back(result);
    }

    size_t totalCount = validCount + invalidCount;
    std::cout << "Total LocalIds: " << totalCount << "\n";
    std::cout << "Valid:          " << validCount << " (" << std::fixed << std::setprecision(1)
              << (totalCount > 0 ? (100.0 * validCount / totalCount) : 0.0) << "%)\n";
    std::cout << "Invalid:        " << invalidCount << " (" << std::fixed << std::setprecision(1)
              << (totalCount > 0 ? (100.0 * invalidCount / totalCount) : 0.0) << "%)\n";

    if (invalidCount > 0)
    {
        std::cout << "\nInvalid LocalIds:\n";
        std::cout << std::string(80, '-') << "\n";

        for (const auto& result : results)
        {
            if (!result.isValid)
            {
                std::cout << "Line " << result.lineNumber << ": " << result.localIdStr << "\n";
                std::cout << "  Error: " << result.errorMessage << "\n\n";
            }
        }
    }

    if (showStats && validCount > 0)
    {
        std::cout << "\nStatistics:\n";
        std::cout << std::string(80, '-') << "\n";

        std::map<std::string, size_t> versionCounts;
        std::map<std::string, size_t> primaryCounts;
        std::map<std::string, size_t> tagCounts;

        for (const auto& result : results)
        {
            if (result.isValid)
            {
                auto localIdOpt = LocalId::fromString(result.localIdStr);
                if (localIdOpt.has_value())
                {
                    const auto& localId = *localIdOpt;

                    std::string version{ VisVersions::toString(localId.version()) };
                    versionCounts[version]++;

                    std::string primaryPath = localId.primaryItem().toString();
                    size_t slashPos = primaryPath.find('/');
                    std::string firstNode = (slashPos != std::string::npos)
                                                ? std::string(primaryPath.substr(0, slashPos))
                                                : std::string(primaryPath);
                    primaryCounts[firstNode]++;

                    for (const auto& tag : localId.metadataTags())
                    {
                        std::string tagKey{ CodebookNames::toPrefix(tag.name()) };
                        tagCounts[tagKey]++;
                    }
                }
            }
        }

        std::cout << "\nVIS Versions:\n";
        for (const auto& [version, count] : versionCounts)
        {
            std::cout << "  " << std::left << std::setw(10) << version << ": " << count << "\n";
        }

        std::cout << "\nTop Primary Item Root Nodes:\n";
        std::vector<std::pair<std::string, size_t>> sortedPrimary(primaryCounts.begin(), primaryCounts.end());
        std::sort(sortedPrimary.begin(), sortedPrimary.end(), [](const auto& a, const auto& b) {
            return a.second > b.second;
        });

        size_t maxShow = std::min<size_t>(10, sortedPrimary.size());
        for (size_t i = 0; i < maxShow; ++i)
        {
            std::cout << "  " << std::left << std::setw(15) << sortedPrimary[i].first << ": " << sortedPrimary[i].second
                      << "\n";
        }

        std::cout << "\nTop Metadata Tags:\n";
        std::vector<std::pair<std::string, size_t>> sortedTags(tagCounts.begin(), tagCounts.end());
        std::sort(
            sortedTags.begin(), sortedTags.end(), [](const auto& a, const auto& b) { return a.second > b.second; });

        maxShow = std::min<size_t>(10, sortedTags.size());
        for (size_t i = 0; i < maxShow; ++i)
        {
            std::cout << "  " << std::left << std::setw(15) << sortedTags[i].first << ": " << sortedTags[i].second
                      << "\n";
        }
    }
}

void queryFile(std::string_view filename, int argc, char* argv[], int startArg)
{
    using namespace dnv::vista::sdk;

    std::string primaryPath;
    std::string tagName;
    std::string tagValue;
    bool noSecondary = false;

    for (int i = startArg; i < argc; ++i)
    {
        std::string_view arg = argv[i];

        if (arg == "--primary" && i + 1 < argc)
        {
            primaryPath = argv[++i];
        }
        else if (arg == "--tag" && i + 2 < argc)
        {
            tagName = argv[++i];
            tagValue = argv[++i];
        }
        else if (arg == "--no-secondary")
        {
            noSecondary = true;
        }
    }

    if (primaryPath.empty() && tagName.empty() && !noSecondary)
    {
        std::cerr << "Error: Query requires at least one filter option\n";
        std::cerr << "Use --primary, --tag, or --no-secondary\n";
        return;
    }

    const auto& vis = VIS::instance();
    auto version = vis.latest();
    const auto& gmod = vis.gmod(version);
    const auto& locations = vis.locations(version);

    std::optional<GmodPathQuery> pathQuery;
    if (!primaryPath.empty())
    {
        auto pathOpt = GmodPath::fromShortPath(primaryPath, gmod, locations);
        if (!pathOpt.has_value())
        {
            std::cerr << "Error: Invalid primary path '" << primaryPath << "'\n";
            return;
        }

        pathQuery = GmodPathQueryBuilder::Path::from(*pathOpt).withoutLocations().build();
    }

    std::optional<MetadataTagsQuery> tagsQuery;
    if (!tagName.empty())
    {
        CodebookName codebookNameEnum;
        try
        {
            codebookNameEnum = CodebookNames::fromPrefix(tagName);
        }
        catch (const std::exception&)
        {
            std::cerr << "Error: Unknown codebook name '" << tagName << "'\n";
            return;
        }

        tagsQuery = MetadataTagsQueryBuilder::create().withTag(codebookNameEnum, tagValue).build();
    }

    auto queryBuilder = LocalIdQueryBuilder::create();

    if (pathQuery.has_value())
    {
        queryBuilder = queryBuilder.withPrimaryItem(*pathQuery);
    }
    if (tagsQuery.has_value())
    {
        queryBuilder = queryBuilder.withTags(*tagsQuery);
    }
    if (noSecondary)
    {
        queryBuilder = queryBuilder.withoutSecondaryItem();
    }

    auto query = queryBuilder.build();

    std::ifstream file(filename.data());
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file '" << filename << "'\n";
        return;
    }

    std::cout << "Filtering LocalIds from: " << filename << "\n";
    std::cout << "Query:\n";
    if (!primaryPath.empty())
    {
        std::cout << "  Primary path: " << primaryPath << " (any location)\n";
    }
    if (!tagName.empty())
    {
        std::cout << "  Tag: " << tagName << " = " << tagValue << "\n";
    }
    if (noSecondary)
    {
        std::cout << "  No secondary item\n";
    }
    std::cout << "\n";
    std::cout << std::string(80, '=') << "\n\n";

    std::string line;
    size_t totalCount = 0;
    size_t matchCount = 0;

    while (std::getline(file, line))
    {
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos)
        {
            line = line.substr(0, commentPos);
        }

        auto start = line.find_first_not_of(" \t\r\n");
        auto end = line.find_last_not_of(" \t\r\n");
        if (start == std::string::npos)
        {
            continue;
        }
        std::string trimmed = line.substr(start, end - start + 1);

        auto localIdOpt = LocalId::fromString(trimmed);
        if (!localIdOpt.has_value())
        {
            continue;
        }

        totalCount++;

        if (query.match(*localIdOpt))
        {
            matchCount++;
            std::cout << trimmed << "\n";
        }
    }

    std::cout << "\n" << std::string(80, '=') << "\n";
    std::cout << "Matched " << matchCount << " of " << totalCount << " valid LocalIds\n";
}

void showStats(std::string_view filename)
{
    validateFile(filename, true);
}
