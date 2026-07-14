/**
 * @file shipid.cpp
 * @brief Demonstrates usage of vista-sdk ShipId API
 * @details This sample shows how to construct, parse, and inspect ShipId values,
 *          which identify a vessel in ISO 19848 transport packages either by IMO number
 *          or by an alternative identifier.
 */

#include <dnv/vista/sdk/core/ImoNumber.h>
#include <dnv/vista/sdk/transport/ShipId.h>

#include <iostream>
#include <string>
#include <vector>

int main()
{
    using namespace dnv::vista::sdk;
    using namespace dnv::vista::sdk::transport;

    std::cout << "=== vista-sdk ShipId Sample ===\n\n";

    {
        std::cout << "1. ShipId: Constructing from an IMO number\n";
        std::cout << "----------------------------------------------\n";

        auto imo = ImoNumber::fromString("9074729");
        if (imo.has_value())
        {
            ShipId shipId{ *imo };

            std::cout << "  isImoNumber: " << std::boolalpha << shipId.isImoNumber() << "\n";
            std::cout << "  isOtherId  : " << shipId.isOtherId() << "\n";
            std::cout << "  toString() : " << shipId.toString() << "\n";
        }
        std::cout << "\n";
    }

    {
        std::cout << "2. ShipId: Constructing from an alternative identifier\n";
        std::cout << "-----------------------------------------------------------\n";

        ShipId shipId{ std::string{ "VESSEL-XYZ-789" } };

        std::cout << "  isImoNumber: " << std::boolalpha << shipId.isImoNumber() << "\n";
        std::cout << "  isOtherId  : " << shipId.isOtherId() << "\n";
        std::cout << "  toString() : " << shipId.toString() << "\n";
        std::cout << "\n";
    }

    {
        std::cout << "3. ShipId::fromString: Parsing header ShipID values\n";
        std::cout << "--------------------------------------------------------\n";

        std::vector<std::string> testValues = {
            "IMO9074729",     // valid IMO number, with prefix
            "imo9074729",     // valid IMO number, case-insensitive prefix
            "IMO1234568",     // "IMO" prefix but failing checksum -> falls back to alternative id
            "VESSEL-ABC-123", // plain alternative identifier
            ""                // empty -> no ShipId
        };

        for (const auto& value : testValues)
        {
            auto shipId = ShipId::fromString(value);

            std::cout << "  '" << value << "'\n";
            if (!shipId.has_value())
            {
                std::cout << "    -> (invalid, empty input)\n";
                continue;
            }

            if (shipId->isImoNumber())
            {
                std::cout << "    -> IMO number: " << shipId->toString() << "\n";
            }
            else
            {
                std::cout << "    -> Alternative id: " << shipId->toString() << "\n";
            }
        }
        std::cout << "\n";
    }

    {
        std::cout << "4. ShipId::match: Pattern matching on the identifier kind\n";
        std::cout << "----------------------------------------------------------------\n";

        std::vector<ShipId> shipIds;
        if (auto imo = ImoNumber::fromString("9785811"); imo.has_value())
        {
            shipIds.emplace_back(*imo);
        }
        shipIds.emplace_back(std::string{ "BARGE-042" });

        for (const auto& shipId : shipIds)
        {
            auto description = shipId.match(
                [](const ImoNumber& imoNumber) { return "IMO number  : " + imoNumber.toString(); },
                [](std::string_view otherId) { return "Alternative : " + std::string{ otherId }; });

            std::cout << "  " << description << "\n";
        }
        std::cout << "\n";
    }

    {
        std::cout << "5. ShipId: Equality comparison\n";
        std::cout << "----------------------------------\n";

        auto imo1 = ImoNumber::fromString("9074729");
        auto imo2 = ImoNumber::fromString("9074729");
        auto imo3 = ImoNumber::fromString("9785811");

        if (!imo1.has_value() || !imo2.has_value() || !imo3.has_value())
        {
            return 1;
        }

        ShipId shipIdA{ *imo1 };
        ShipId shipIdB{ *imo2 };
        ShipId shipIdC{ *imo3 };

        std::cout << std::boolalpha;
        std::cout << "  IMO9074729 == IMO9074729 : " << (shipIdA == shipIdB) << "\n";
        std::cout << "  IMO9074729 == IMO9785811 : " << (shipIdA == shipIdC) << "\n";
        std::cout << "\n";
    }

    return 0;
}
