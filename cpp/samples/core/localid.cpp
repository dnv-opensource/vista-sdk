/**
 * @file localid.cpp
 * @brief Demonstrates usage of vista-sdk LocalId and LocalIdBuilder APIs
 * @details This sample shows how to build LocalIds, parse LocalId strings,
 *          work with metadata tags, use verbose mode, and handle MQTT formatting
 */

#include <dnv/vista/sdk/core/LocalId.h>
#include <dnv/vista/sdk/core/ParsingErrors.h>
#include <dnv/vista/sdk/core/VIS.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

int main()
{
    using namespace dnv::vista::sdk;

    const auto& vis = VIS::instance();
    const auto& gmod = vis.gmod(vis.latest());
    const auto& locations = vis.locations(vis.latest());
    const auto& codebooks = vis.codebooks(vis.latest());

    std::cout << "=== vista-sdk LocalId Sample ===\n\n";

    {
        std::cout << "1. LocalIdBuilder: Building a simple LocalId\n";
        std::cout << "-----------------------------------------------\n";

        // Parse a Gmod path for the primary item
        auto primaryItem = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations);

        // Create a metadata tag
        auto quantityTag = codebooks[CodebookName::Quantity].createTag("temperature");

        if (primaryItem.has_value() && quantityTag.has_value())
        {
            // Build the LocalId
            auto localId = LocalIdBuilder::create(vis.latest())
                               .withPrimaryItem(*primaryItem)
                               .withMetadataTag(*quantityTag)
                               .build();

            std::cout << "Built LocalId: " << localId.toString() << "\n";
            std::cout << "  VIS Version : " << VisVersions::toString(localId.version()) << "\n";
            std::cout << "  Primary Item: " << localId.primaryItem().toString() << "\n";
            std::cout << "  Quantity    : " << localId.quantity()->value() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "2. LocalIdBuilder: Building with multiple metadata tags\n";
        std::cout << "----------------------------------------------------------\n";

        auto primaryItem = GmodPath::fromShortPath("621.21/S90", gmod, locations);
        auto qtyTag = codebooks[CodebookName::Quantity].createTag("mass");
        auto cntTag = codebooks[CodebookName::Content].createTag("fuel.oil");
        auto posTag = codebooks[CodebookName::Position].createTag("inlet");

        if (primaryItem.has_value() && qtyTag.has_value() && cntTag.has_value() && posTag.has_value())
        {
            auto localId = LocalIdBuilder::create(vis.latest())
                               .withPrimaryItem(*primaryItem)
                               .withMetadataTag(*qtyTag)
                               .withMetadataTag(*cntTag)
                               .withMetadataTag(*posTag)
                               .build();

            std::cout << "Built LocalId: " << localId.toString() << "\n";
            std::cout << "  Metadata tags:\n";
            std::cout << "    Quantity: " << localId.quantity()->value() << "\n";
            std::cout << "    Content : " << localId.content()->value() << "\n";
            std::cout << "    Position: " << localId.position()->value() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "3. LocalIdBuilder: Building with secondary item\n";
        std::cout << "--------------------------------------------------\n";

        auto primaryItem = GmodPath::fromShortPath("621.21/S90", gmod, locations);
        auto secondaryItem = GmodPath::fromShortPath("411.1/C101", gmod, locations);
        auto qtyTag = codebooks[CodebookName::Quantity].createTag("mass");
        auto cntTag = codebooks[CodebookName::Content].createTag("fuel.oil");
        auto posTag = codebooks[CodebookName::Position].createTag("inlet");

        if (primaryItem.has_value() && secondaryItem.has_value() && qtyTag.has_value() && cntTag.has_value() &&
            posTag.has_value())
        {
            auto localId = LocalIdBuilder::create(vis.latest())
                               .withPrimaryItem(*primaryItem)
                               .withSecondaryItem(*secondaryItem)
                               .withMetadataTag(*qtyTag)
                               .withMetadataTag(*cntTag)
                               .withMetadataTag(*posTag)
                               .build();

            std::cout << "Built LocalId: " << localId.toString() << "\n";
            std::cout << "  Primary item  : " << localId.primaryItem().toString() << "\n";
            std::cout << "  Secondary item: " << localId.secondaryItem()->toString() << "\n";
            std::cout << "  Has secondary : " << std::boolalpha << localId.secondaryItem().has_value() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "4. LocalId: Parsing from string\n";
        std::cout << "---------------------------------\n";

        std::string localIdStr = "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature";

        // Parse without error details
        auto localId = LocalId::fromString(localIdStr);

        if (localId.has_value())
        {
            std::cout << "Parsed successfully: " << localId->toString() << "\n";
            std::cout << "  VIS Version : " << VisVersions::toString(localId->version()) << "\n";
            std::cout << "  Primary item: " << localId->primaryItem().toString() << "\n";
            std::cout << "  Quantity    : " << localId->quantity()->value() << "\n";
        }
        else
        {
            std::cout << "Parse failed\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "5. LocalId: Parsing with error handling\n";
        std::cout << "------------------------------------------\n";

        std::vector<std::string> testStrings = {
            "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature",
            "/dnv-v2/vis-3-4a/621.21/S90/sec/411.1/C101/meta/qty-mass/cnt-fuel.oil/pos-inlet",
            "/dnv-v2/vis-3-7a/612.21/C701.23/C633/meta/calc~accumulate",
            "",
            "/dnv-v2/INVALID/411.1/meta/qty-temperature",
            "/dnv-v2/vis-3-4a/INVALID/meta/qty-temperature",
            "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-invalid_quantity",
            "//vis-3-4a/not-a-valid-path/meta/qty-temperature"
        };

        for (const auto& str : testStrings)
        {
            ParsingErrors errors;
            auto localId = LocalId::fromString(str, errors);

            std::cout << "Parsing: \"" << (str.empty() ? "(empty)" : str) << "\"\n";

            if (localId.has_value())
            {
                std::cout << "  Success: " << localId->toString() << "\n";
            }
            else
            {
                std::cout << "  Failed:\n";
                if (errors.hasErrors())
                {
                    for (const auto& [type, message] : errors)
                    {
                        std::cout << "  [" << type << "] " << message << "\n";
                    }
                }
            }
            std::cout << "\n";
        }
    }

    {
        std::cout << "6. LocalIdBuilder: Modifying existing LocalIds\n";
        std::cout << "------------------------------------------------\n";

        auto original = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature");

        if (original.has_value())
        {
            std::cout << "Original: " << original->toString() << "\n";

            // Add content tag
            auto cntTag = codebooks[CodebookName::Content].createTag("water");

            // Remove quantity, add position
            auto posTag = codebooks[CodebookName::Position].createTag("outlet");

            if (cntTag.has_value())
            {
                auto modified = original->builder().withMetadataTag(*cntTag).build();
                std::cout << "Modified (added content): " << modified.toString() << "\n";
            }

            if (posTag.has_value())
            {
                auto modified2 =
                    original->builder().withoutMetadataTag(CodebookName::Quantity).withMetadataTag(*posTag).build();
                std::cout << "Modified (removed qty, added pos): " << modified2.toString() << "\n";
            }
        }

        std::cout << "\n";
    }

    {
        std::cout << "7. LocalIdBuilder: Verbose mode\n";
        std::cout << "----------------------------------\n";

        auto primaryItem = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations);
        auto qtyTag = codebooks[CodebookName::Quantity].createTag("temperature");

        // Build without verbose mode
        if (primaryItem.has_value() && qtyTag.has_value())
        {
            auto normalLocalId =
                LocalIdBuilder::create(vis.latest()).withPrimaryItem(*primaryItem).withMetadataTag(*qtyTag).build();

            std::cout << "Normal mode : " << normalLocalId.toString() << "\n";

            // Build with verbose mode
            auto verboseLocalId = LocalIdBuilder::create(vis.latest())
                                      .withPrimaryItem(*primaryItem)
                                      .withMetadataTag(*qtyTag)
                                      .withVerboseMode(true)
                                      .build();

            std::cout << "Verbose mode: " << verboseLocalId.toString() << "\n";
            std::cout << "  Includes human-readable common names for better understanding\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "8. LocalId: Accessing components\n";
        std::cout << "----------------------------------\n";

        auto localId =
            LocalId::fromString("/dnv-v2/vis-3-4a/621.21/S90/sec/411.1/C101/meta/qty-mass/cnt-fuel.oil/pos-inlet");

        if (localId.has_value())
        {
            std::cout << "LocalId: " << localId->toString() << "\n\n";

            std::cout << "Components:\n";
            std::cout << "  VIS Version   : " << VisVersions::toString(localId->version()) << "\n";
            std::cout << "  Primary item  : " << localId->primaryItem().toString() << "\n";

            if (localId->secondaryItem().has_value())
            {
                std::cout << "  Secondary item: " << localId->secondaryItem()->toString() << "\n";
            }

            std::cout << "\nMetadata tags:\n";
            if (localId->quantity().has_value())
            {
                std::cout << "  Quantity   : " << localId->quantity()->value() << "\n";
            }
            if (localId->content().has_value())
            {
                std::cout << "  Content    : " << localId->content()->value() << "\n";
            }
            if (localId->calculation().has_value())
            {
                std::cout << "  Calculation: " << localId->calculation()->value() << "\n";
            }
            if (localId->state().has_value())
            {
                std::cout << "  State      : " << localId->state()->value() << "\n";
            }
            if (localId->command().has_value())
            {
                std::cout << "  Command    : " << localId->command()->value() << "\n";
            }
            if (localId->type().has_value())
            {
                std::cout << "  Type       : " << localId->type()->value() << "\n";
            }
            if (localId->position().has_value())
            {
                std::cout << "  Position   : " << localId->position()->value() << "\n";
            }
            if (localId->detail().has_value())
            {
                std::cout << "  Detail     : " << localId->detail()->value() << "\n";
            }

            std::cout << "\nAll metadata tags (" << localId->metadataTags().size() << "):\n";
            for (const auto& tag : localId->metadataTags())
            {
                std::cout << "  " << CodebookNames::toPrefix(tag.name()) << "-" << tag.value()
                          << (tag.isCustom() ? " (custom)" : "") << "\n";
            }
        }

        std::cout << "\n";
    }

    {
        std::cout << "9. LocalId: Equality and comparison\n";
        std::cout << "-------------------------------------\n";

        auto localId1 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature");
        auto localId2 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature");
        auto localId3 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-pressure");

        std::cout << std::boolalpha;
        if (localId1.has_value() && localId2.has_value() && localId3.has_value())
        {
            std::cout << "LocalId1: " << localId1->toString() << "\n";
            std::cout << "LocalId2: " << localId2->toString() << "\n";
            std::cout << "LocalId3: " << localId3->toString() << "\n\n";

            std::cout << "LocalId1 == LocalId2? " << (*localId1 == *localId2) << "\n";
            std::cout << "LocalId1 == LocalId3? " << (*localId1 == *localId3) << "\n";
            std::cout << "LocalId1 != LocalId3? " << (*localId1 != *localId3) << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "10. LocalIdBuilder: Custom metadata tags\n";
        std::cout << "-------------------------------------------\n";

        auto primaryItem = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations);

        // Create custom quantity tag
        auto customQtyTag = codebooks[CodebookName::Quantity].createTag("my_custom_measurement");

        if (primaryItem.has_value() && customQtyTag.has_value())
        {
            auto localId = LocalIdBuilder::create(vis.latest())
                               .withPrimaryItem(*primaryItem)
                               .withMetadataTag(*customQtyTag)
                               .build();

            std::cout << "LocalId with custom tag: " << localId.toString() << "\n";
            std::cout << "  Has custom tag : " << std::boolalpha << localId.hasCustomTag() << "\n";
            std::cout << "  Custom quantity: " << localId.quantity()->value() << "\n";
            std::cout << "  Is custom      : " << localId.quantity()->isCustom() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "11. LocalIdBuilder: Validation\n";
        std::cout << "--------------------------------\n";

        // Valid builder
        auto primaryItem = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations);
        auto qtyTag = codebooks[CodebookName::Quantity].createTag("temperature");

        std::cout << std::boolalpha;

        if (primaryItem.has_value() && qtyTag.has_value())
        {
            auto validBuilder =
                LocalIdBuilder::create(vis.latest()).withPrimaryItem(*primaryItem).withMetadataTag(*qtyTag);

            std::cout << "Valid builder:\n";
            std::cout << "  Is valid: " << validBuilder.isValid() << "\n";
            std::cout << "  Is empty: " << validBuilder.isEmpty() << "\n";

            // Empty builder
            auto emptyBuilder = LocalIdBuilder::create(vis.latest());
            std::cout << "\nEmpty builder:\n";
            std::cout << "  Is valid: " << emptyBuilder.isValid() << "\n";
            std::cout << "  Is empty: " << emptyBuilder.isEmpty() << "\n";

            // Builder missing metadata
            auto incompleteBuilder = LocalIdBuilder::create(vis.latest()).withPrimaryItem(*primaryItem);
            std::cout << "\nBuilder with primary item but no metadata:\n";
            std::cout << "  Is valid: " << incompleteBuilder.isValid() << "\n";
            std::cout << "  Is empty: " << incompleteBuilder.isEmpty() << "\n";

            // Try to build invalid
            std::cout << "\nAttempting to build invalid LocalId:\n";
            try
            {
                auto invalid = incompleteBuilder.build();
                std::cout << "  ERROR: Invalid build succeeded (unexpected)\n";
            }
            catch (const std::invalid_argument& e)
            {
                std::cout << "  Correctly rejected: " << e.what() << "\n";
            }
        }

        std::cout << "\n";
    }

    return 0;
}
