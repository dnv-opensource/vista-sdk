/**
 * @file universalid.cpp
 * @brief Demonstrates usage of vista-sdk UniversalId and UniversalIdBuilder APIs
 * @details This sample shows how to build UniversalIds combining IMO numbers with LocalIds,
 *          parse UniversalId strings, and work with globally unique sensor identifiers
 */

#include <dnv/VistaSDK.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

int main()
{
    using namespace dnv::vista::sdk;

    std::cout << "=== vista-sdk UniversalId Sample ===\n\n";

    {
        std::cout << "1. UniversalIdBuilder: Building a simple UniversalId\n";
        std::cout << "------------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());
        const auto& locations = vis.locations(vis.latest());
        const auto& codebooks = vis.codebooks(vis.latest());

        // Create IMO number
        auto imoNumber = ImoNumber{ 1234567 };

        // Create LocalIdBuilder
        auto primaryItem = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations);
        auto qtyTag = codebooks[CodebookName::Quantity].createTag("temperature");

        if (primaryItem.has_value() && qtyTag.has_value())
        {
            auto localIdBuilder =
                LocalIdBuilder::create(vis.latest()).withPrimaryItem(*primaryItem).withMetadataTag(*qtyTag);

            // Build UniversalId
            auto universalId =
                UniversalIdBuilder::create(vis.latest()).withImoNumber(imoNumber).withLocalId(localIdBuilder).build();

            std::cout << "Built UniversalId: " << universalId.toString() << "\n";
            std::cout << "  IMO Number: " << universalId.imoNumber().toString() << "\n";
            std::cout << "  LocalId   : " << universalId.localId().toString() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "2. UniversalIdBuilder: Building with full LocalId\n";
        std::cout << "---------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());
        const auto& locations = vis.locations(vis.latest());
        const auto& codebooks = vis.codebooks(vis.latest());

        // Build a complete LocalId with secondary item and multiple metadata tags
        auto primaryItem = GmodPath::fromShortPath("621.21/S90", gmod, locations);
        auto secondaryItem = GmodPath::fromShortPath("411.1/C101", gmod, locations);
        auto qtyTag = codebooks[CodebookName::Quantity].createTag("mass");
        auto cntTag = codebooks[CodebookName::Content].createTag("fuel.oil");
        auto posTag = codebooks[CodebookName::Position].createTag("inlet");

        if (primaryItem.has_value() && secondaryItem.has_value() && qtyTag.has_value() && cntTag.has_value() &&
            posTag.has_value())
        {
            auto localIdBuilder = LocalIdBuilder::create(vis.latest())
                                      .withPrimaryItem(*primaryItem)
                                      .withSecondaryItem(*secondaryItem)
                                      .withMetadataTag(*qtyTag)
                                      .withMetadataTag(*cntTag)
                                      .withMetadataTag(*posTag);

            // Create UniversalId
            auto imoNumber = ImoNumber{ "IMO9074729" };
            auto universalId =
                UniversalIdBuilder::create(vis.latest()).withImoNumber(imoNumber).withLocalId(localIdBuilder).build();

            std::cout << "Built UniversalId: " << universalId.toString() << "\n";
            std::cout << "  Format       : {naming-entity}/{imo-number}{local-id}\n";
            std::cout << "  Naming entity: " << UniversalId::namingEntity() << "\n";
            std::cout << "  IMO Number   : " << universalId.imoNumber().toString() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "3. UniversalId: Parsing from string\n";
        std::cout << "-------------------------------------\n";

        std::string universalIdStr = "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature";

        // Parse without error details
        auto universalId = UniversalId::fromString(universalIdStr);

        if (universalId.has_value())
        {
            std::cout << "Parsed successfully: " << universalId->toString() << "\n";
            std::cout << "  IMO Number : " << universalId->imoNumber().toString() << "\n";
            std::cout << "  LocalId    : " << universalId->localId().toString() << "\n";
            std::cout << "  VIS Version: " << VisVersions::toString(universalId->localId().version()) << "\n";
        }
        else
        {
            std::cout << "Parse failed\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "4. UniversalId: Parsing with error handling\n";
        std::cout << "----------------------------------------------\n";

        std::vector<std::string> testStrings = {
            "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature",
            "data.dnv.com/IMO9074729/dnv-v2/vis-3-4a/621.21/S90/sec/411.1/C101/meta/qty-mass/cnt-fuel.oil/pos-inlet",
            "data.dnv.com/IMO9785811/dnv-v2/vis-3-7a/612.21/C701.23/C633/meta/calc~accumulate",
            "",
            "data.dnv.com/INVALID/dnv-v2/vis-3-4a/411.1/meta/qty-temperature",
            "wrong.entity/IMO1234567/dnv-v2/vis-3-4a/411.1/meta/qty-temperature",
            "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/INVALID/meta/qty-temperature"
        };

        for (const auto& str : testStrings)
        {
            ParsingErrors errors;
            auto universalId = UniversalId::fromString(str, errors);

            std::cout << "Parsing: \"" << (str.empty() ? "(empty)" : str.substr(0, 60))
                      << (str.length() > 60 ? "..." : "") << "\"\n";

            if (universalId.has_value())
            {
                std::cout << "  Success\n";
                std::cout << "    IMO: " << universalId->imoNumber().toString() << "\n";
            }
            else
            {
                std::cout << "  Failed:\n";
                if (errors.hasErrors())
                {
                    for (const auto& [type, message] : errors)
                    {
                        std::cout << "    - " << message << "\n";
                    }
                }
            }
            std::cout << "\n";
        }
    }

    {
        std::cout << "5. UniversalIdBuilder: Modifying existing UniversalIds\n";
        std::cout << "--------------------------------------------------------\n";

        auto original =
            UniversalId::fromString("data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature");

        if (original.has_value())
        {
            std::cout << "Original                    : " << original->toString() << "\n";

            const auto& vis = VIS::instance();
            const auto& codebooks = vis.codebooks(vis.latest());

            // Change IMO number
            auto newImo = ImoNumber{ 9074729 };
            auto modified = original->builder().withImoNumber(newImo).build();

            std::cout << "Modified (new IMO)          : " << modified.toString() << "\n";

            // Modify the LocalId part
            auto cntTag = codebooks[CodebookName::Content].createTag("water");
            if (cntTag.has_value())
            {
                auto modified2 =
                    original->builder().withLocalId(original->localId().builder().withMetadataTag(*cntTag)).build();

                std::cout << "Modified (added content tag): " << modified2.toString() << "\n";
            }
        }

        std::cout << "\n";
    }

    {
        std::cout << "6. UniversalId: Accessing components\n";
        std::cout << "--------------------------------------\n";

        auto universalId = UniversalId::fromString(
            "data.dnv.com/IMO9074729/dnv-v2/vis-3-4a/621.21/S90/sec/411.1/C101/meta/qty-mass/cnt-fuel.oil/pos-inlet");

        if (universalId.has_value())
        {
            std::cout << "UniversalId: " << universalId->toString() << "\n\n";

            std::cout << "IMO Number:\n";
            std::cout << "  Value: " << universalId->imoNumber().toString() << "\n";

            std::cout << "\nLocalId components:\n";
            const auto& localId = universalId->localId();
            std::cout << "  VIS Version   : " << VisVersions::toString(localId.version()) << "\n";
            std::cout << "  Primary item  : " << localId.primaryItem().toString() << "\n";

            if (localId.secondaryItem().has_value())
            {
                std::cout << "  Secondary item: " << localId.secondaryItem()->toString() << "\n";
            }

            std::cout << "\nMetadata tags:\n";
            if (localId.quantity().has_value())
                std::cout << "  Quantity: " << localId.quantity()->value() << "\n";
            if (localId.content().has_value())
                std::cout << "  Content : " << localId.content()->value() << "\n";
            if (localId.position().has_value())
                std::cout << "  Position: " << localId.position()->value() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "7. UniversalId: Equality and comparison\n";
        std::cout << "-----------------------------------------\n";

        auto uid1 =
            UniversalId::fromString("data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature");
        auto uid2 =
            UniversalId::fromString("data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature");
        auto uid3 =
            UniversalId::fromString("data.dnv.com/IMO9074729/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature");
        auto uid4 =
            UniversalId::fromString("data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-pressure");

        std::cout << std::boolalpha;
        if (uid1.has_value() && uid2.has_value() && uid3.has_value() && uid4.has_value())
        {
            std::cout << "UniversalId1: Same IMO, same LocalId\n";
            std::cout << "UniversalId2: Same IMO, same LocalId\n";
            std::cout << "UniversalId3: Different IMO, same LocalId\n";
            std::cout << "UniversalId4: Same IMO, different LocalId\n\n";

            std::cout << "uid1 == uid2? " << (*uid1 == *uid2) << " (same IMO & LocalId)\n";
            std::cout << "uid1 == uid3? " << (*uid1 == *uid3) << " (different IMO)\n";
            std::cout << "uid1 == uid4? " << (*uid1 == *uid4) << " (different LocalId)\n";
            std::cout << "uid1 != uid3? " << (*uid1 != *uid3) << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "8. UniversalIdBuilder: Validation\n";
        std::cout << "-----------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());
        const auto& locations = vis.locations(vis.latest());
        const auto& codebooks = vis.codebooks(vis.latest());

        auto imoNumber = ImoNumber{ 1234567 };
        auto primaryItem = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations);
        auto qtyTag = codebooks[CodebookName::Quantity].createTag("temperature");

        if (primaryItem.has_value() && qtyTag.has_value())
        {
            auto localIdBuilder =
                LocalIdBuilder::create(vis.latest()).withPrimaryItem(*primaryItem).withMetadataTag(*qtyTag);

            // Valid builder
            auto validBuilder =
                UniversalIdBuilder::create(vis.latest()).withImoNumber(imoNumber).withLocalId(localIdBuilder);

            std::cout << std::boolalpha;
            std::cout << "Valid builder:\n";
            std::cout << "  Is valid: " << validBuilder.isValid() << "\n";

            // Missing IMO number
            auto missingImo = UniversalIdBuilder::create(vis.latest()).withLocalId(localIdBuilder);
            std::cout << "\nBuilder missing IMO number:\n";
            std::cout << "  Is valid: " << missingImo.isValid() << "\n";

            // Missing LocalId
            auto missingLocalId = UniversalIdBuilder::create(vis.latest()).withImoNumber(imoNumber);
            std::cout << "\nBuilder missing LocalId:\n";
            std::cout << "  Is valid: " << missingLocalId.isValid() << "\n";

            // Try to build invalid
            std::cout << "\nAttempting to build without IMO number:\n";
            try
            {
                auto invalid = missingImo.build();
                std::cout << "  ERROR: Invalid build succeeded (unexpected)\n";
            }
            catch (const std::invalid_argument& e)
            {
                std::cout << "  Correctly rejected: " << e.what() << "\n";
            }
        }

        std::cout << "\n";
    }

    {
        std::cout << "9. UniversalId: Real-world vessel examples\n";
        std::cout << "---------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());
        const auto& locations = vis.locations(vis.latest());
        const auto& codebooks = vis.codebooks(vis.latest());

        struct VesselSensor
        {
            std::string vesselName;
            int imoNumber;
            std::string sensorPath;
            std::string quantity;
        };

        std::vector<VesselSensor> sensors = { { "MV Queen Mary 2", 9241061, "411.1/C101.31-2", "temperature" },
                                              { "MS Oasis of the Seas", 9383936, "621.21/S90", "mass" },
                                              { "MS Symphony of the Seas", 9744001, "612.21/C701.23", "pressure" } };

        for (const auto& sensor : sensors)
        {
            auto imo = ImoNumber{ sensor.imoNumber };
            auto path = GmodPath::fromShortPath(sensor.sensorPath, gmod, locations);
            auto tag = codebooks[CodebookName::Quantity].createTag(sensor.quantity);

            if (path.has_value() && tag.has_value())
            {
                auto localIdBuilder = LocalIdBuilder::create(vis.latest()).withPrimaryItem(*path).withMetadataTag(*tag);

                auto universalId =
                    UniversalIdBuilder::create(vis.latest()).withImoNumber(imo).withLocalId(localIdBuilder).build();

                std::cout << sensor.vesselName << ":\n";
                std::cout << "  " << universalId.toString() << "\n\n";
            }
        }
    }

    {
        std::cout << "10. UniversalIdBuilder: Remove and add operations\n";
        std::cout << "---------------------------------------------------\n";

        auto original =
            UniversalId::fromString("data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature");

        if (original.has_value())
        {
            std::cout << "Original: " << original->toString() << "\n";

            // Remove IMO number
            auto withoutImo = original->builder().withoutImoNumber();
            std::cout << "\nAfter withoutImoNumber():\n";
            std::cout << "  Is valid: " << std::boolalpha << withoutImo.isValid() << "\n";

            // Remove LocalId
            auto withoutLocalId = original->builder().withoutLocalId();
            std::cout << "\nAfter withoutLocalId():\n";
            std::cout << "  Is valid: " << std::boolalpha << withoutLocalId.isValid() << "\n";

            // Add them back
            const auto& vis = VIS::instance();
            const auto& gmod = vis.gmod(vis.latest());
            const auto& locations = vis.locations(vis.latest());
            const auto& codebooks = vis.codebooks(vis.latest());

            auto newImo = ImoNumber{ 9074729 };
            auto path = GmodPath::fromShortPath("621.21/S90", gmod, locations);
            auto tag = codebooks[CodebookName::Quantity].createTag("mass");

            if (path.has_value() && tag.has_value())
            {
                auto newLocalId = LocalIdBuilder::create(vis.latest()).withPrimaryItem(*path).withMetadataTag(*tag);
                auto rebuilt = withoutImo.withImoNumber(newImo).withLocalId(newLocalId).build();
                std::cout << "\nRebuilt with new components: " << rebuilt.toString() << "\n";
            }
        }

        std::cout << "\n";
    }

    return 0;
}
