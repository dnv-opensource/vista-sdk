/**
 * @file codebooks.cpp
 * @brief Demonstrates usage of vista-sdk Codebooks API
 * @details This sample shows how to use VIS singleton, access codebooks,
 *          validate values, create metadata tags, and work with VIS versions
 */

#include <dnv/vista/sdk/core/VIS.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

int main()
{
    using namespace dnv::vista::sdk;

    std::cout << "=== vista-sdk Codebooks Sample ===\n\n";

    {
        std::cout << "1. VIS Singleton: Version management\n";
        std::cout << "-------------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& allVersions = vis.versions();

        std::cout << "Latest VIS version: " << VisVersions::toString(vis.latest()) << "\n";
        std::cout << "Available versions (" << allVersions.size() << "):\n";
        for (const auto& version : allVersions)
        {
            std::cout << "  - " << VisVersions::toString(version) << "\n";
        }
        std::cout << "\n";
    }

    {
        std::cout << "2. Codebooks: Accessing codebooks for a version\n";
        std::cout << "-------------------------------------------------------\n";

        const auto& codebooks = VIS::instance().codebooks(VIS::instance().latest());

        std::cout << "Codebooks for version: " << VisVersions::toString(codebooks.version()) << "\n";

        const auto& quantity = codebooks[CodebookName::Quantity];
        const auto& position = codebooks[CodebookName::Position];
        const auto& state = codebooks[CodebookName::State];

        std::cout << "Quantity codebook has " << std::setw(3) << quantity.standardValues().size()
                  << " standard values\n";
        std::cout << "Position codebook has " << std::setw(3) << position.standardValues().size()
                  << " standard values\n";
        std::cout << "State    codebook has " << std::setw(3) << state.standardValues().size() << " standard values\n";
        std::cout << "\n";
    }

    {
        std::cout << "3. Codebook: Checking standard values and groups\n";
        std::cout << "-------------------------------------------------------\n";

        const auto& quantity = VIS::instance().codebooks(VIS::instance().latest())[CodebookName::Quantity];

        std::cout << std::boolalpha;
        std::cout << "Has 'temperature'? " << quantity.hasStandardValue("temperature") << "\n";
        std::cout << "Has 'pressure'   ? " << quantity.hasStandardValue("pressure") << "\n";
        std::cout << "Has 'invalid_qty'? " << quantity.hasStandardValue("invalid_qty") << "\n";

        std::cout << "\nGroups in Quantity codebook: " << quantity.groups().size() << "\n";
        for (const auto& group : quantity.groups())
        {
            std::cout << "  - " << group << "\n";
        }
        std::cout << "\n";
    }

    {
        std::cout << "4. Position Codebook: Special numeric value handling\n";
        std::cout << "-------------------------------------------------------\n";

        const auto& position = VIS::instance().codebooks(VIS::instance().latest())[CodebookName::Position];

        std::cout << std::boolalpha;
        std::cout << "Has '1'        ? " << position.hasStandardValue("1") << "\n";
        std::cout << "Has '03'       ? " << position.hasStandardValue("03") << "\n";
        std::cout << "Has '42'       ? " << position.hasStandardValue("42") << "\n";
        std::cout << "Has '1234'     ? " << position.hasStandardValue("1234") << "\n";
        std::cout << "Has 'centre'   ? " << position.hasStandardValue("centre") << "\n";
        std::cout << "Has 'port'     ? " << position.hasStandardValue("port") << "\n";
        std::cout << "Has 'starboard'? " << position.hasStandardValue("starboard") << "\n";
        std::cout << "Has 'invalid'  ? " << position.hasStandardValue("invalid") << "\n";
        std::cout << "\n";
    }

    {
        std::cout << "5. MetadataTag: Creating tags from standard values\n";
        std::cout << "-------------------------------------------------------\n";

        const auto& quantity = VIS::instance().codebooks(VIS::instance().latest())[CodebookName::Quantity];

        auto tempTag = quantity.createTag("temperature");

        if (tempTag.has_value())
        {
            std::cout << "Temperature tag created:\n";
            std::cout << "  Name:        " << CodebookNames::toPrefix(tempTag->name()) << "\n";
            std::cout << "  Value:       " << tempTag->value() << "\n";
            std::cout << "  Prefix:      " << tempTag->prefix() << "\n";
            std::cout << "  Is Custom:   " << std::boolalpha << tempTag->isCustom() << "\n";

            std::string tag;
            tempTag->toString(tag);
            std::cout << "  String repr: " << tag << "\n";
        }
        std::cout << "\n";
    }

    {
        std::cout << "6. MetadataTag: Creating custom tags\n";
        std::cout << "-------------------------------------------------------\n";

        const auto& quantity = VIS::instance().codebooks(VIS::instance().latest())[CodebookName::Quantity];

        auto customTag = quantity.createTag("custom_measurement");

        if (customTag.has_value())
        {
            std::cout << "Custom tag created:\n";
            std::cout << "  Name:        " << CodebookNames::toPrefix(customTag->name()) << "\n";
            std::cout << "  Value:       " << customTag->value() << "\n";
            std::cout << "  Prefix:      " << customTag->prefix() << "\n";
            std::cout << "  Is Custom:   " << std::boolalpha << customTag->isCustom() << "\n";

            std::string tag;
            customTag->toString(tag);
            std::cout << "  String repr: " << tag << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "7. Position Validation\n";
        std::cout << "-------------------------------------------------------\n";

        const auto& position = VIS::instance().codebooks(VIS::instance().latest())[CodebookName::Position];

        const std::vector<std::string> testPositions = {
            "centre",
            "port",
            "1",
            "42",
            "centre-starboard",
            "port-1",
            "centre-starboard-2",
            "invalid position!",
            "starboard-centre",
        };

        std::cout << "Position validation results:\n";
        for (const auto& pos : testPositions)
        {
            auto result = position.validatePosition(pos);
            constexpr size_t columnWidth = 18;
            size_t padding = pos.length() < columnWidth ? columnWidth - pos.length() : 0;
            std::cout << "  '" << pos << "'" << std::string(padding, ' ') << ": ";

            switch (result)
            {
                case PositionValidationResult::Valid:
                    std::cout << "Valid";
                    break;
                case PositionValidationResult::Custom:
                    std::cout << "Custom";
                    break;
                case PositionValidationResult::Invalid:
                    std::cout << "Invalid";
                    break;
                case PositionValidationResult::InvalidOrder:
                    std::cout << "Invalid (wrong order)";
                    break;
                case PositionValidationResult::InvalidGrouping:
                    std::cout << "Invalid (duplicate groups)";
                    break;
            }
            std::cout << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "8. CodebookName: String conversions\n";
        std::cout << "-------------------------------------------------------\n";

        std::cout << "Enum to prefix conversions:\n";
        std::cout << "  Quantity -> '" << CodebookNames::toPrefix(CodebookName::Quantity) << "'\n";
        std::cout << "  Position -> '" << CodebookNames::toPrefix(CodebookName::Position) << "'\n";
        std::cout << "  State    -> '" << CodebookNames::toPrefix(CodebookName::State) << "'\n";
        std::cout << "  Command  -> '" << CodebookNames::toPrefix(CodebookName::Command) << "'\n";
        std::cout << "  Detail   -> '" << CodebookNames::toPrefix(CodebookName::Detail) << "'\n";

        std::cout << "Prefix to enum conversions:\n";
        std::cout << "  'qty' -> CodebookName::"
                  << (CodebookNames::fromPrefix("qty") == CodebookName::Quantity ? "Quantity" : "?") << "\n";
        std::cout << "  'pos' -> CodebookName::"
                  << (CodebookNames::fromPrefix("pos") == CodebookName::Position ? "Position" : "?") << "\n";
        std::cout << "\n";
    }

    {
        std::cout << "9. Invalid operations\n";
        std::cout << "-------------------------------------------------------\n";

        const auto& quantity = VIS::instance().codebooks(VIS::instance().latest())[CodebookName::Quantity];

        std::cout << std::boolalpha;
        std::cout << "Empty string tag created ? " << quantity.createTag("").has_value() << "\n";
        std::cout << "Whitespace tag created   ? " << quantity.createTag("   ").has_value() << "\n";

        try
        {
            [[maybe_unused]] auto invalid = CodebookNames::fromPrefix("invalid_prefix");
            std::cout << "Invalid prefix accepted  : (unexpected)\n";
        }
        catch (const std::invalid_argument& e)
        {
            std::cout << "Invalid prefix rejected  : " << e.what() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "10. Building a complete metadata path\n";
        std::cout << "-------------------------------------------------------\n";

        const auto& codebooks = VIS::instance().codebooks(VIS::instance().latest());

        auto qtyTag = codebooks[CodebookName::Quantity].createTag("temperature");
        auto cntTag = codebooks[CodebookName::Content].createTag("oil");
        auto posTag = codebooks[CodebookName::Position].createTag("1");
        auto stateTag = codebooks[CodebookName::State].createTag("running");

        if (qtyTag && cntTag && posTag && stateTag)
        {
            std::string path;

            std::cout << "Building metadata path:\n";

            qtyTag->toString(path);
            std::cout << "  After Quantity: " << path << "\n";

            cntTag->toString(path);
            std::cout << "  After Content : " << path << "\n";

            posTag->toString(path);
            std::cout << "  After Position: " << path << "\n";

            stateTag->toString(path);
            std::cout << "  After State   : " << path << "\n";
        }

        std::cout << "\n";
    }

    return 0;
}
