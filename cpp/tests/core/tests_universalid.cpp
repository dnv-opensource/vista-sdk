#include <doctest/doctest.h>

#include <dnv/VistaSDK.h>

#include <EmbeddedTestData.h>

namespace dnv::vista::sdk::tests
{
    namespace
    {
        static constexpr std::string_view testCase1 =
            "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/621.21/S90/sec/411.1/C101/meta/qty-mass/cnt-fuel.oil/pos-inlet";
        static constexpr std::string_view testCase2 =
            "data.dnv.com/IMO1234567/dnv-v2/vis-3-7a/612.21/C701.23/C633/meta/calc~accumulate";
    } // namespace

    TEST_SUITE("UniversalId")
    {
        TEST_CASE("try parsing")
        {
            for (const auto& testCase : { testCase1, testCase2 })
            {
                CAPTURE(testCase);

                ParsingErrors errors;
                auto universalId = UniversalIdBuilder::fromString(testCase, errors);

                CHECK(universalId.has_value());
                CHECK_FALSE(errors.hasErrors());
            }
        }

        TEST_CASE("parsing")
        {
            for (const auto& testCase : { testCase1, testCase2 })
            {
                CAPTURE(testCase);

                auto universalId = UniversalId::fromString(testCase);

                REQUIRE(universalId.has_value());
                CHECK(universalId->imoNumber() == ImoNumber{ 1234567 });
            }
        }

        TEST_CASE("toString")
        {
            for (const auto& testCase : { testCase1, testCase2 })
            {
                CAPTURE(testCase);

                auto universalId = UniversalIdBuilder::fromString(testCase);

                REQUIRE(universalId.has_value());
                CHECK(universalId->toString() == std::string{ testCase });
            }
        }

        TEST_CASE("builder - add and remove all")
        {
            for (const auto& testCase : { testCase1, testCase2 })
            {
                CAPTURE(testCase);

                auto universalId = UniversalIdBuilder::fromString(testCase);

                REQUIRE(universalId.has_value());

                const auto& builder = universalId->builder();
                CHECK(builder.localId().has_value());
                CHECK(builder.imoNumber().has_value());

                auto emptyBuilder = builder.withoutImoNumber().withoutLocalId();

                CHECK_FALSE(emptyBuilder.localId().has_value());
                CHECK_FALSE(emptyBuilder.imoNumber().has_value());
            }
        }

        TEST_CASE("builder - withOptional nullopt throws")
        {
            auto builder = UniversalIdBuilder::create(VisVersion::v3_4a);

            CHECK_THROWS_AS(
                (void)builder.withLocalId(std::optional<LocalIdBuilder>{ std::nullopt }), std::invalid_argument);
            CHECK_THROWS_AS(
                (void)builder.withImoNumber(std::optional<ImoNumber>{ std::nullopt }), std::invalid_argument);
        }

        TEST_CASE("build - programmatic")
        {
            const auto& vis = VIS::instance();
            const auto& codebooks = vis.codebooks(VisVersion::v3_4a);
            const auto& gmod = vis.gmod(VisVersion::v3_4a);
            const auto& locations = vis.locations(VisVersion::v3_4a);

            auto qtyTag = codebooks[CodebookName::Quantity].createTag("mass");
            auto cntTag = codebooks[CodebookName::Content].createTag("fuel.oil");
            auto posTag = codebooks[CodebookName::Position].createTag("inlet");

            REQUIRE(qtyTag.has_value());
            REQUIRE(cntTag.has_value());
            REQUIRE(posTag.has_value());

            auto primaryItem = GmodPath::fromShortPath("621.21/S90", gmod, locations);
            auto secondaryItem = GmodPath::fromShortPath("411.1/C101", gmod, locations);

            REQUIRE(primaryItem.has_value());
            REQUIRE(secondaryItem.has_value());

            auto localIdBuilder = LocalIdBuilder::create(VisVersion::v3_4a)
                                      .withPrimaryItem(*primaryItem)
                                      .withSecondaryItem(*secondaryItem)
                                      .withMetadataTag(*qtyTag)
                                      .withMetadataTag(*cntTag)
                                      .withMetadataTag(*posTag);

            auto universalIdBuilder = UniversalIdBuilder::create(VisVersion::v3_4a)
                                          .withImoNumber(ImoNumber{ 1234567 })
                                          .withLocalId(localIdBuilder);

            CHECK(universalIdBuilder.isValid());

            auto universalId = universalIdBuilder.build();

            CHECK(universalId.imoNumber() == ImoNumber{ 1234567 });
            CHECK(universalId.localId().builder().isValid());
            CHECK(universalId.toString() == std::string{ testCase1 });
        }

        TEST_CASE("equality")
        {
            auto uid1 = UniversalId::fromString(testCase1);
            auto uid2 = UniversalId::fromString(testCase1);
            auto uid3 = UniversalId::fromString(testCase2);

            REQUIRE(uid1.has_value());
            REQUIRE(uid2.has_value());
            REQUIRE(uid3.has_value());

            CHECK(*uid1 == *uid2);
            CHECK_FALSE(*uid1 != *uid2);
            CHECK(*uid1 != *uid3);
            CHECK_FALSE(*uid1 == *uid3);
        }

        TEST_CASE("build - missing IMO number throws")
        {
            auto builder = UniversalIdBuilder::create(VisVersion::v3_4a)
                               .withLocalId(LocalIdBuilder::create(VisVersion::v3_4a)
                                                .withPrimaryItem(GmodPath::fromShortPath(
                                                                     "411.1",
                                                                     VIS::instance().gmod(VisVersion::v3_4a),
                                                                     VIS::instance().locations(VisVersion::v3_4a))
                                                                     .value()));

            CHECK_FALSE(builder.isValid());
            CHECK_THROWS_AS((void)builder.build(), std::invalid_argument);
        }

        TEST_CASE("build - missing LocalId throws")
        {
            auto builder =
                UniversalIdBuilder::create(VisVersion::v3_4a).withImoNumber(ImoNumber{ 1234567 }).withoutLocalId();

            CHECK_FALSE(builder.isValid());
            CHECK_THROWS_AS((void)builder.build(), std::invalid_argument);
        }

        TEST_CASE("parsing errors - invalid format")
        {
            ParsingErrors errors;
            auto universalId = UniversalIdBuilder::fromString("invalid-format", errors);

            CHECK_FALSE(universalId.has_value());
            CHECK(errors.hasErrors());
        }

        TEST_CASE("parsing errors - wrong naming entity")
        {
            ParsingErrors errors;
            auto universalId =
                UniversalIdBuilder::fromString("wrong.entity.com/IMO1234567/dnv-v2/vis-3-4a/411.1", errors);

            CHECK_FALSE(universalId.has_value());
            CHECK(errors.hasErrors());
        }

        TEST_CASE("parsing errors - invalid IMO number")
        {
            ParsingErrors errors;
            auto universalId = UniversalIdBuilder::fromString("data.dnv.com/INVALID/dnv-v2/vis-3-4a/411.1", errors);

            CHECK_FALSE(universalId.has_value());
            CHECK(errors.hasErrors());
        }

        TEST_CASE("parsing errors - empty string")
        {
            ParsingErrors errors;
            auto universalId = UniversalIdBuilder::fromString("", errors);

            CHECK_FALSE(universalId.has_value());
            CHECK(errors.hasErrors());
        }

        TEST_CASE("parsing errors - missing LocalId")
        {
            ParsingErrors errors;
            auto universalId = UniversalIdBuilder::fromString("data.dnv.com/IMO1234567", errors);

            CHECK_FALSE(universalId.has_value());
            CHECK(errors.hasErrors());
        }

        TEST_CASE("accessors")
        {
            auto universalId = UniversalId::fromString(testCase1);

            REQUIRE(universalId.has_value());

            CHECK(universalId->imoNumber() == ImoNumber{ 1234567 });
            CHECK(universalId->imoNumber().toString() == "IMO1234567");

            const auto& localId = universalId->localId();
            CHECK(localId.builder().isValid());
            CHECK_FALSE(localId.builder().isEmpty());
            CHECK(localId.toString().find("621.21") != std::string::npos);
            CHECK(localId.toString().find("qty-mass") != std::string::npos);

            const auto& builder = universalId->builder();
            CHECK(builder.isValid());
            CHECK(builder.imoNumber().has_value());
            CHECK(builder.localId().has_value());
        }

        TEST_CASE("copy constructor")
        {
            auto uid1 = UniversalId::fromString(testCase1);
            REQUIRE(uid1.has_value());

            UniversalId uid2 = *uid1;

            CHECK(uid1->imoNumber() == uid2.imoNumber());
            CHECK(uid1->toString() == uid2.toString());
        }

        TEST_CASE("copy assignment")
        {
            auto uid1 = UniversalId::fromString(testCase1);
            auto uid2 = UniversalId::fromString(testCase2);

            REQUIRE(uid1.has_value());
            REQUIRE(uid2.has_value());

            *uid2 = *uid1;

            CHECK(uid1->imoNumber() == uid2->imoNumber());
            CHECK(uid1->toString() == uid2->toString());
        }

        TEST_CASE("move constructor")
        {
            auto uid1 = UniversalId::fromString(testCase1);
            REQUIRE(uid1.has_value());

            const auto expectedStr = uid1->toString();
            const auto expectedImo = uid1->imoNumber();

            UniversalId uid2 = std::move(*uid1);

            CHECK(expectedImo == uid2.imoNumber());
            CHECK(expectedStr == uid2.toString());
        }

        TEST_CASE("move assignment")
        {
            auto uid1 = UniversalId::fromString(testCase1);
            auto uid2 = UniversalId::fromString(testCase2);

            REQUIRE(uid1.has_value());
            REQUIRE(uid2.has_value());

            const auto expectedStr = uid1->toString();
            const auto expectedImo = uid1->imoNumber();

            *uid2 = std::move(*uid1);

            CHECK(expectedImo == uid2->imoNumber());
            CHECK(expectedStr == uid2->toString());
        }
    }
} // namespace dnv::vista::sdk::tests
