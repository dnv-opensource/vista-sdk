#include <doctest/doctest.h>

#include <dnv/VistaSDK.h>

namespace dnv::vista::sdk::tests
{
    using dnv::vista::sdk::transport::ShipId;

    TEST_SUITE("ShipId")
    {
        TEST_CASE("construction - from imo number")
        {
            auto imo = ImoNumber::fromString("9074729");
            REQUIRE(imo.has_value());

            ShipId shipId{ *imo };

            CHECK(shipId.isImoNumber());
            CHECK_FALSE(shipId.isOtherId());
        }

        TEST_CASE("construction - from alternative id")
        {
            std::string altId = "VESSEL-123";
            ShipId shipId{ altId };

            CHECK_FALSE(shipId.isImoNumber());
            CHECK(shipId.isOtherId());
        }

        TEST_CASE("construction - empty alternative id throws")
        {
            CHECK_THROWS_AS(ShipId{ std::string{ "" } }, std::invalid_argument);
        }

        TEST_CASE("imoNumber accessor")
        {
            auto imo = ImoNumber::fromString("9074729");
            REQUIRE(imo.has_value());

            ShipId shipId{ *imo };

            auto retrievedImo = shipId.imoNumber();
            REQUIRE(retrievedImo.has_value());
            CHECK_EQ(retrievedImo->toString(), "IMO9074729");

            auto otherId = shipId.otherId();
            CHECK_FALSE(otherId.has_value());
        }

        TEST_CASE("otherId accessor")
        {
            std::string altId = "VESSEL-ABC-456";
            ShipId shipId{ altId };

            auto otherId = shipId.otherId();
            REQUIRE(otherId.has_value());
            CHECK_EQ(*otherId, "VESSEL-ABC-456");

            auto imo = shipId.imoNumber();
            CHECK_FALSE(imo.has_value());
        }

        TEST_CASE("toString - imo number")
        {
            auto imo = ImoNumber::fromString("9074729");
            REQUIRE(imo.has_value());

            ShipId shipId{ *imo };

            // Should include "IMO" prefix per ISO 19848
            CHECK_EQ(shipId.toString(), "IMO9074729");
        }

        TEST_CASE("toString - alternative id")
        {
            std::string altId = "CUSTOM-SHIP-ID";
            ShipId shipId{ altId };

            CHECK_EQ(shipId.toString(), "CUSTOM-SHIP-ID");
        }

        TEST_CASE("fromString - imo with prefix")
        {
            auto shipId = ShipId::fromString("IMO9074729");

            REQUIRE(shipId.has_value());
            CHECK(shipId->isImoNumber());

            auto imo = shipId->imoNumber();
            REQUIRE(imo.has_value());
            CHECK_EQ(imo->toString(), "IMO9074729");
        }

        TEST_CASE("fromString - imo with prefix, case insensitive")
        {
            auto shipId1 = ShipId::fromString("imo9074729");
            REQUIRE(shipId1.has_value());
            CHECK(shipId1->isImoNumber());

            auto shipId2 = ShipId::fromString("ImO9074729");
            REQUIRE(shipId2.has_value());
            CHECK(shipId2->isImoNumber());
        }

        TEST_CASE("fromString - imo with prefix, invalid falls back to alternative id")
        {
            auto shipId = ShipId::fromString("IMO1234568");

            REQUIRE(shipId.has_value());
            CHECK(shipId->isOtherId());
            CHECK_EQ(*shipId->otherId(), "IMO1234568");
        }

        TEST_CASE("fromString - alternative id")
        {
            auto shipId = ShipId::fromString("VESSEL-XYZ-789");

            REQUIRE(shipId.has_value());
            CHECK(shipId->isOtherId());

            auto otherId = shipId->otherId();
            REQUIRE(otherId.has_value());
            CHECK_EQ(*otherId, "VESSEL-XYZ-789");
        }

        TEST_CASE("fromString - empty string")
        {
            auto shipId = ShipId::fromString("");
            CHECK_FALSE(shipId.has_value());
        }

        TEST_CASE("fromString - whitespace only")
        {
            auto shipId = ShipId::fromString("   ");
            CHECK_FALSE(shipId.has_value());
        }

        TEST_CASE("equality - imo numbers")
        {
            auto imo1 = ImoNumber::fromString("9074729").value();
            auto imo2 = ImoNumber::fromString("9074729").value();
            auto imo3 = ImoNumber::fromString("1234567").value();

            ShipId shipId1{ imo1 };
            ShipId shipId2{ imo2 };
            ShipId shipId3{ imo3 };

            CHECK_EQ(shipId1, shipId2);
            CHECK_NE(shipId1, shipId3);
        }

        TEST_CASE("equality - alternative ids")
        {
            ShipId shipId1{ std::string{ "VESSEL-A" } };
            ShipId shipId2{ std::string{ "VESSEL-A" } };
            ShipId shipId3{ std::string{ "VESSEL-B" } };

            CHECK_EQ(shipId1, shipId2);
            CHECK_NE(shipId1, shipId3);
        }

        TEST_CASE("equality - imo vs alternative")
        {
            auto imo = ImoNumber::fromString("9074729").value();
            ShipId shipId1{ imo };
            ShipId shipId2{ std::string{ "9074729" } };

            // Different types should not be equal
            CHECK_NE(shipId1, shipId2);
        }

        TEST_CASE("match - imo number")
        {
            auto imo = ImoNumber::fromString("9074729").value();
            ShipId shipId{ imo };

            auto result = shipId.match(
                [](const ImoNumber& i) { return "IMO: " + i.toString(); },
                [](std::string_view s) { return "Other: " + std::string{ s }; });

            CHECK_EQ(result, "IMO: IMO9074729");
        }

        TEST_CASE("match - alternative id")
        {
            ShipId shipId{ std::string{ "CUSTOM-123" } };

            auto result = shipId.match(
                [](const ImoNumber& i) { return "IMO: " + i.toString(); },
                [](std::string_view s) { return "Other: " + std::string{ s }; });

            CHECK_EQ(result, "Other: CUSTOM-123");
        }

        TEST_CASE("copy construction")
        {
            auto imo = ImoNumber::fromString("9074729").value();
            ShipId original{ imo };
            ShipId copy{ original };

            CHECK_EQ(original, copy);
            CHECK(copy.isImoNumber());
        }

        TEST_CASE("move construction")
        {
            auto imo = ImoNumber::fromString("9074729").value();
            ShipId original{ imo };
            ShipId moved{ std::move(original) };

            CHECK(moved.isImoNumber());
            CHECK_EQ(moved.imoNumber()->toString(), "IMO9074729");
        }

        TEST_CASE("copy assignment")
        {
            auto imo1 = ImoNumber::fromString("9074729").value();
            auto imo2 = ImoNumber::fromString("1234567").value();

            ShipId shipId1{ imo1 };
            ShipId shipId2{ imo2 };

            shipId2 = shipId1;

            CHECK_EQ(shipId1, shipId2);
            CHECK_EQ(shipId2.imoNumber()->toString(), "IMO9074729");
        }

        TEST_CASE("move assignment")
        {
            auto imo1 = ImoNumber::fromString("9074729").value();
            ShipId shipId1{ imo1 };
            ShipId shipId2{ std::string{ "TEMP" } };

            shipId2 = std::move(shipId1);

            CHECK(shipId2.isImoNumber());
            CHECK_EQ(shipId2.imoNumber()->toString(), "IMO9074729");
        }

        TEST_CASE("round trip - imo number")
        {
            auto original = ImoNumber::fromString("9074729").value();
            ShipId shipId1{ original };

            auto str = shipId1.toString();
            auto shipId2 = ShipId::fromString(str);

            REQUIRE(shipId2.has_value());
            CHECK_EQ(shipId1, *shipId2);
            CHECK(shipId2->isImoNumber());
        }

        TEST_CASE("round trip - alternative id")
        {
            ShipId shipId1{ std::string{ "VESSEL-ROUND-TRIP" } };

            auto str = shipId1.toString();
            auto shipId2 = ShipId::fromString(str);

            REQUIRE(shipId2.has_value());
            CHECK_EQ(shipId1, *shipId2);
            CHECK(shipId2->isOtherId());
        }
    }
} // namespace dnv::vista::sdk::tests
