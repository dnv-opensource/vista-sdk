#include <doctest/doctest.h>

#include <dnv/VistaSDK.h>

#include <JSON/Parser.h>

#include <EmbeddedTestData.h>

#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace dnv::vista::sdk::tests
{
    namespace
    {
        struct LocalIdInput
        {
            std::string primaryItem;
            std::optional<std::string> secondaryItem;
            std::optional<std::string> quantity;
            std::optional<std::string> content;
            std::optional<std::string> position;
            VisVersion visVersion = VisVersion::v3_4a;
            bool verbose = false;
        };

        struct LocalIdTestCase
        {
            LocalIdInput input;
            std::string expectedOutput;
        };

        struct InvalidLocalIdTestCase
        {
            std::string input;
            std::vector<std::string> expectedErrorMessages;
        };

        std::vector<LocalIdTestCase> validTestData()
        {
            return { { LocalIdInput{ "411.1/C101.31-2", {}, {}, {}, {} }, "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta" },
                     { LocalIdInput{ "411.1/C101.31-2", std::nullopt, "temperature", "exhaust.gas", "inlet" },
                       "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet" },
                     { LocalIdInput{ "411.1/C101.63/S206",
                                     std::nullopt,
                                     "temperature",
                                     "exhaust.gas",
                                     "inlet",
                                     VisVersion::v3_4a,
                                     true },
                       "/dnv-v2/vis-3-4a/411.1/C101.63/S206/~propulsion.engine/~cooling.system/meta/qty-temperature/"
                       "cnt-exhaust.gas/pos-inlet" },
                     { LocalIdInput{ "411.1/C101.63/S206",
                                     "411.1/C101.31-5",
                                     "temperature",
                                     "exhaust.gas",
                                     "inlet",
                                     VisVersion::v3_4a,
                                     true },
                       "/dnv-v2/vis-3-4a/411.1/C101.63/S206/sec/411.1/C101.31-5/~propulsion.engine/~cooling.system/"
                       "~for.propulsion.engine/~cylinder.5/meta/qty-temperature/cnt-exhaust.gas/pos-inlet" },
                     { LocalIdInput{ "511.11/C101.67/S208",
                                     std::nullopt,
                                     "pressure",
                                     "starting.air",
                                     "inlet",
                                     VisVersion::v3_6a,
                                     true },
                       "/dnv-v2/vis-3-6a/511.11/C101.67/S208/~main.generator.engine/~starting.system.pneumatic/meta/"
                       "qty-pressure/cnt-starting.air/pos-inlet" } };
        }

        std::vector<std::string> validParsingData()
        {
            return { "/dnv-v2/vis-3-4a/1031/meta/cnt-refrigerant/state-leaking",
                     "/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo/pos~percentage",
                     "/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened",
                     "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
                     "/dnv-v2/vis-3-4a/411.1/C101.63/S206/~propulsion.engine/~cooling.system/meta/qty-temperature/"
                     "cnt-exhaust.gas/pos-inlet",
                     "/dnv-v2/vis-3-4a/411.1/C101.63/S206/sec/411.1/C101.31-5/~propulsion.engine/~cooling.system/"
                     "~for.propulsion.engine/~cylinder.5/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
                     "/dnv-v2/vis-3-4a/511.11-21O/C101.67/S208/meta/qty-pressure/cnt-air/state-low",
                     "/dnv-v2/vis-3-10a/623.4/M211.14/sec/433.1/C322.91/S205/meta/qty-particle.concentration/"
                     "cnt~non-ferrous/detail-size.over.400" };
        }

        std::vector<InvalidLocalIdTestCase> loadInvalidLocalIds()
        {
            auto raw = EmbeddedTestData::text("InvalidLocalIds.json");
            REQUIRE_FALSE(raw.empty());

            auto docOpt = json::Document::fromString(raw);
            REQUIRE(docOpt.has_value());

            auto arrayRef = (*docOpt)["InvalidLocalIds"].rootRef<json::Array>();
            REQUIRE(arrayRef.has_value());
            const json::Array& array = arrayRef->get();

            std::vector<InvalidLocalIdTestCase> result;
            result.reserve(array.size());

            for (const auto& elem : array)
            {
                InvalidLocalIdTestCase item;
                item.input = *elem["input"].root<std::string>();

                auto errorsRef = elem["expectedErrorMessages"].rootRef<json::Array>();
                REQUIRE(errorsRef.has_value());
                for (const auto& errElem : errorsRef->get())
                {
                    item.expectedErrorMessages.push_back(*errElem.root<std::string>());
                }

                result.push_back(std::move(item));
            }

            return result;
        }
    } // namespace

    TEST_SUITE("LocalId")
    {
        TEST_CASE("build - valid")
        {
            const auto& vis = VIS::instance();

            for (const auto& testCase : validTestData())
            {
                const auto& input = testCase.input;

                CAPTURE(input.primaryItem);

                const auto& gmod = vis.gmod(input.visVersion);
                const auto& codebooks = vis.codebooks(input.visVersion);

                ParsingErrors primaryErrors;
                auto primaryItem =
                    GmodPath::fromShortPath(input.primaryItem, gmod, vis.locations(input.visVersion), primaryErrors);
                REQUIRE(primaryItem.has_value());

                std::optional<GmodPath> secondaryItem;
                if (input.secondaryItem.has_value())
                {
                    ParsingErrors secondaryErrors;
                    secondaryItem = GmodPath::fromShortPath(
                        *input.secondaryItem, gmod, vis.locations(input.visVersion), secondaryErrors);
                    REQUIRE(secondaryItem.has_value());
                }

                auto builder = LocalIdBuilder::create(input.visVersion)
                                   .withPrimaryItem(*primaryItem)
                                   .withVerboseMode(input.verbose);

                if (secondaryItem.has_value())
                {
                    builder = builder.withSecondaryItem(*secondaryItem);
                }

                if (input.quantity.has_value())
                {
                    auto tag = codebooks[CodebookName::Quantity].createTag(*input.quantity);
                    REQUIRE(tag.has_value());
                    builder = builder.withMetadataTag(*tag);
                }

                if (input.content.has_value())
                {
                    auto tag = codebooks[CodebookName::Content].createTag(*input.content);
                    REQUIRE(tag.has_value());
                    builder = builder.withMetadataTag(*tag);
                }

                if (input.position.has_value())
                {
                    auto tag = codebooks[CodebookName::Position].createTag(*input.position);
                    REQUIRE(tag.has_value());
                    builder = builder.withMetadataTag(*tag);
                }

                CHECK_EQ(testCase.expectedOutput, builder.toString());
            }
        }

        TEST_CASE("build - all without")
        {
            const auto& vis = VIS::instance();
            const auto visVersion = VisVersion::v3_4a;
            const auto& gmod = vis.gmod(visVersion);
            const auto& codebooks = vis.codebooks(visVersion);

            ParsingErrors errors;
            auto primaryItem = GmodPath::fromShortPath("411.1/C101.31-2", gmod, vis.locations(visVersion), errors);
            REQUIRE(primaryItem.has_value());

            auto secondaryItem = GmodPath::fromShortPath("411.1/C101.31-5", gmod, vis.locations(visVersion), errors);
            REQUIRE(secondaryItem.has_value());

            auto quantityTag = codebooks[CodebookName::Quantity].createTag("temperature");
            REQUIRE(quantityTag.has_value());

            auto contentTag = codebooks[CodebookName::Content].createTag("exhaust.gas");
            REQUIRE(contentTag.has_value());

            auto positionTag = codebooks[CodebookName::Position].createTag("inlet");
            REQUIRE(positionTag.has_value());

            auto stateTag = codebooks[CodebookName::State].createTag("running");
            REQUIRE(stateTag.has_value());

            auto calculationTag = codebooks[CodebookName::Calculation].createTag("average");
            REQUIRE(calculationTag.has_value());

            auto localId = LocalIdBuilder::create(visVersion)
                               .withPrimaryItem(*primaryItem)
                               .withSecondaryItem(*secondaryItem)
                               .withVerboseMode(true)
                               .withMetadataTag(*quantityTag)
                               .withMetadataTag(*contentTag)
                               .withMetadataTag(*positionTag)
                               .withMetadataTag(*stateTag)
                               .withMetadataTag(*calculationTag);

            CHECK(localId.isValid());

            auto allWithout = localId.withoutPrimaryItem()
                                  .withoutSecondaryItem()
                                  .withoutVisVersion()
                                  .withoutMetadataTag(CodebookName::Quantity)
                                  .withoutMetadataTag(CodebookName::Position)
                                  .withoutMetadataTag(CodebookName::State)
                                  .withoutMetadataTag(CodebookName::Content)
                                  .withoutMetadataTag(CodebookName::Calculation);

            CHECK(allWithout.isEmpty());
        }

        TEST_CASE("equality")
        {
            const auto& vis = VIS::instance();
            const auto visVersion = VisVersion::v3_4a;
            const auto& gmod = vis.gmod(visVersion);
            const auto& codebooks = vis.codebooks(visVersion);

            for (const auto& testCase : validTestData())
            {
                const auto& input = testCase.input;

                CAPTURE(input.primaryItem);

                ParsingErrors errors;
                auto primaryItem = GmodPath::fromShortPath(input.primaryItem, gmod, vis.locations(visVersion), errors);
                REQUIRE(primaryItem.has_value());

                std::optional<GmodPath> secondaryItem;
                if (input.secondaryItem.has_value())
                {
                    secondaryItem =
                        GmodPath::fromShortPath(*input.secondaryItem, gmod, vis.locations(visVersion), errors);
                    REQUIRE(secondaryItem.has_value());
                }

                auto localId = LocalIdBuilder::create(visVersion).withPrimaryItem(*primaryItem);

                if (secondaryItem.has_value())
                {
                    localId = localId.withSecondaryItem(*secondaryItem);
                }

                if (input.quantity.has_value())
                {
                    auto tag = codebooks[CodebookName::Quantity].createTag(*input.quantity);
                    if (tag.has_value())
                    {
                        localId = localId.withMetadataTag(*tag);
                    }
                }

                if (input.content.has_value())
                {
                    auto tag = codebooks[CodebookName::Content].createTag(*input.content);
                    if (tag.has_value())
                    {
                        localId = localId.withMetadataTag(*tag);
                    }
                }

                if (input.position.has_value())
                {
                    auto tag = codebooks[CodebookName::Position].createTag(*input.position);
                    if (tag.has_value())
                    {
                        localId = localId.withMetadataTag(*tag);
                    }
                }

                // Self-equality
                auto otherLocalId = localId;
                CHECK_EQ(localId, otherLocalId);

                // Inequality with different position tag
                auto eqTestTag = codebooks[CodebookName::Position].createTag("upper");
                REQUIRE(eqTestTag.has_value());
                otherLocalId = localId.withMetadataTag(*eqTestTag);
                CHECK_NE(localId, otherLocalId);

                // Equality after reconstructing primary item
                if (localId.primaryItem().has_value())
                {
                    otherLocalId = localId.withPrimaryItem(*localId.primaryItem());
                    if (input.position.has_value())
                    {
                        auto tag = codebooks[CodebookName::Position].createTag(*input.position);
                        if (tag.has_value())
                        {
                            otherLocalId = otherLocalId.withMetadataTag(*tag);
                        }
                    }
                    CHECK_EQ(localId, otherLocalId);
                }
            }
        }

        TEST_CASE("parsing")
        {
            for (const auto& localIdStr : validParsingData())
            {
                CAPTURE(localIdStr);

                ParsingErrors errors;
                auto parsed = LocalIdBuilder::fromString(localIdStr, errors);

                REQUIRE_MESSAGE(
                    parsed.has_value(), "Failed to parse: " << localIdStr << "\nErrors: " << errors.toString());
                CHECK_EQ(localIdStr, parsed->toString());
            }
        }

        TEST_CASE("parsing - simple")
        {
            auto localIdAsString = "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet";

            auto localId = LocalIdBuilder::fromString(localIdAsString);
            CHECK(localId.has_value());
        }

        TEST_CASE("parsing - validation")
        {
            for (const auto& testCase : loadInvalidLocalIds())
            {
                CAPTURE(testCase.input);

                ParsingErrors errors;
                auto parsed = LocalIdBuilder::fromString(testCase.input, errors);

                CHECK_FALSE(parsed.has_value());

                std::vector<std::string> actualMessages;
                for (const auto& [type, message] : errors)
                {
                    actualMessages.push_back(message);
                }

                CHECK_EQ(testCase.expectedErrorMessages, actualMessages);
            }
        }

        TEST_CASE("parsing - empty metadata tag value fails immediately with a clear message")
        {
            ParsingErrors errors;
            auto parsed = LocalIdBuilder::fromString("/dnv-v2/vis-3-4a/411.1/H101/meta/qty-/cnt-fuel.oil", errors);
            CHECK_FALSE(parsed.has_value());

            REQUIRE(errors.hasErrors());

            bool foundMissingValueError = false;
            for (const auto& [type, msg] : errors)
            {
                CAPTURE(msg);
                if (msg.find("missing value") != std::string::npos)
                {
                    foundMissingValueError = true;
                }
            }
            CHECK(foundMissingValueError);
        }

        TEST_CASE("parsing - invalid multi-segment primary item does not emit a misleading '/meta' error")
        {
            ParsingErrors errors;
            auto parsed = LocalIdBuilder::fromString("/dnv-v2/vis-3-4a/411.1/511.11//meta/qty-temperature", errors);
            CHECK_FALSE(parsed.has_value());

            std::vector<std::string> messages;
            for (const auto& [type, msg] : errors)
            {
                messages.push_back(msg);
            }

            for (const auto& msg : messages)
            {
                CAPTURE(msg);
                CHECK(msg.find("missing '/meta' prefix") == std::string::npos);
            }
        }

        TEST_CASE("parsing - smoke test")
        {
            auto raw = EmbeddedTestData::text("LocalIds.txt");
            REQUIRE_FALSE(raw.empty());

            std::istringstream stream(raw);
            std::string localIdStr;

            struct ErrorEntry
            {
                std::string localIdStr;
                ParsingErrors parsingErrors;
            };

            std::vector<ErrorEntry> errored;

            while (std::getline(stream, localIdStr))
            {
                // Quick fix to skip invalid metadata tags "qty-content"
                if (localIdStr.find("qty-content") != std::string::npos)
                {
                    continue;
                }

                ParsingErrors errors;
                auto localId = LocalIdBuilder::fromString(localIdStr, errors);

                if (!localId.has_value())
                {
                    errored.push_back({ localIdStr, std::move(errors) });
                }
            }

            if (!errored.empty())
            {
                std::ostringstream msg;
                msg << "Found " << errored.size() << " parsing errors:\n";
                for (const auto& entry : errored)
                {
                    msg << "  LocalId: " << entry.localIdStr << "\n";
                    if (entry.parsingErrors.hasErrors())
                    {
                        msg << "  Errors: " << entry.parsingErrors.toString() << "\n";
                    }
                }
                FAIL(msg.str());
            }
        }
    }

    TEST_SUITE("Mqtt LocalId")
    {
        struct MqttLocalIdTestCase
        {
            LocalIdInput input;
            std::string expectedOutput;
        };

        std::vector<MqttLocalIdTestCase> validMqttTestData()
        {
            return {
                { LocalIdInput{ "411.1/C101.31-2", std::nullopt, "temperature", "exhaust.gas", "inlet" },
                  "dnv-v2/vis-3-4a/411.1_C101.31-2/_/qty-temperature/cnt-exhaust.gas/_/_/_/_/pos-inlet/_" },
                { LocalIdInput{ "411.1/C101.63/S206", std::nullopt, "temperature", "exhaust.gas", "inlet" },
                  "dnv-v2/vis-3-4a/411.1_C101.63_S206/_/qty-temperature/cnt-exhaust.gas/_/_/_/_/pos-inlet/_" },
                { LocalIdInput{ "411.1/C101.63/S206", "411.1/C101.31-5", "temperature", "exhaust.gas", "inlet" },
                  "dnv-v2/vis-3-4a/411.1_C101.63_S206/411.1_C101.31-5/qty-temperature/cnt-exhaust.gas/_/_/_/_/"
                  "pos-inlet/_" },
            };
        }

        TEST_CASE("build - mqtt")
        {
            const auto& vis = VIS::instance();

            for (const auto& testCase : validMqttTestData())
            {
                const auto& input = testCase.input;

                CAPTURE(input.primaryItem);

                const auto& gmod = vis.gmod(input.visVersion);
                const auto& codebooks = vis.codebooks(input.visVersion);

                ParsingErrors primaryErrors;
                auto primaryItem =
                    GmodPath::fromShortPath(input.primaryItem, gmod, vis.locations(input.visVersion), primaryErrors);
                REQUIRE(primaryItem.has_value());

                std::optional<GmodPath> secondaryItem;
                if (input.secondaryItem.has_value())
                {
                    ParsingErrors secondaryErrors;
                    secondaryItem = GmodPath::fromShortPath(
                        *input.secondaryItem, gmod, vis.locations(input.visVersion), secondaryErrors);
                    REQUIRE(secondaryItem.has_value());
                }

                auto builder = LocalIdBuilder::create(input.visVersion).withPrimaryItem(*primaryItem);

                if (secondaryItem.has_value())
                {
                    builder = builder.withSecondaryItem(*secondaryItem);
                }

                if (input.quantity.has_value())
                {
                    auto tag = codebooks[CodebookName::Quantity].createTag(*input.quantity);
                    REQUIRE(tag.has_value());
                    builder = builder.withMetadataTag(*tag);
                }

                if (input.content.has_value())
                {
                    auto tag = codebooks[CodebookName::Content].createTag(*input.content);
                    REQUIRE(tag.has_value());
                    builder = builder.withMetadataTag(*tag);
                }

                if (input.position.has_value())
                {
                    auto tag = codebooks[CodebookName::Position].createTag(*input.position);
                    REQUIRE(tag.has_value());
                    builder = builder.withMetadataTag(*tag);
                }

                auto mqttLocalId = mqtt::LocalId{ builder };
                CHECK_EQ(testCase.expectedOutput, mqttLocalId.toString());
            }
        }

        TEST_CASE("mqtt::LocalId is not a sdk::LocalId ")
        {
            static_assert(
                !std::is_base_of_v<sdk::LocalId, mqtt::LocalId>, "mqtt::LocalId must not inherit from sdk::LocalId");

            const auto& vis = VIS::instance();
            const auto visVersion = VisVersion::v3_4a;
            const auto& gmod = vis.gmod(visVersion);
            const auto& codebooks = vis.codebooks(visVersion);
            const auto& locations = vis.locations(visVersion);

            auto primaryItem = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations);
            REQUIRE(primaryItem.has_value());

            auto qtyTag = codebooks[CodebookName::Quantity].createTag("temperature");
            REQUIRE(qtyTag.has_value());

            auto builder = LocalIdBuilder::create(visVersion).withPrimaryItem(*primaryItem).withMetadataTag(*qtyTag);

            auto mqttLocalId = mqtt::LocalId{ builder };

            CHECK_FALSE(mqttLocalId.toString().starts_with("/dnv-v2/"));
        }

        TEST_CASE("mqtt::LocalId - getters")
        {
            const auto& vis = VIS::instance();
            const auto visVersion = VisVersion::v3_4a;
            const auto& gmod = vis.gmod(visVersion);
            const auto& codebooks = vis.codebooks(visVersion);
            const auto& locations = vis.locations(visVersion);

            auto primaryItem = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations);
            REQUIRE(primaryItem.has_value());

            auto secondaryItem = GmodPath::fromShortPath("411.1/C101.31-5", gmod, locations);
            REQUIRE(secondaryItem.has_value());

            auto qtyTag = codebooks[CodebookName::Quantity].createTag("temperature");
            REQUIRE(qtyTag.has_value());

            auto cntTag = codebooks[CodebookName::Content].createTag("exhaust.gas");
            REQUIRE(cntTag.has_value());

            auto builder = LocalIdBuilder::create(visVersion)
                               .withPrimaryItem(*primaryItem)
                               .withSecondaryItem(*secondaryItem)
                               .withMetadataTag(*qtyTag)
                               .withMetadataTag(*cntTag);

            auto mqttLocalId = mqtt::LocalId{ builder };

            CHECK_EQ(mqttLocalId.version(), visVersion);
            CHECK_EQ(mqttLocalId.primaryItem(), *primaryItem);
            CHECK_EQ(mqttLocalId.secondaryItem(), secondaryItem);
            CHECK_EQ(mqttLocalId.quantity(), qtyTag);
            CHECK_EQ(mqttLocalId.content(), cntTag);
            CHECK_FALSE(mqttLocalId.calculation().has_value());
            CHECK_FALSE(mqttLocalId.state().has_value());
            CHECK_FALSE(mqttLocalId.command().has_value());
            CHECK_FALSE(mqttLocalId.type().has_value());
            CHECK_FALSE(mqttLocalId.position().has_value());
            CHECK_FALSE(mqttLocalId.detail().has_value());
        }

        TEST_CASE("mqtt::LocalId - builder exposes members not reflected in the MQTT format")
        {
            const auto& vis = VIS::instance();
            const auto visVersion = VisVersion::v3_4a;
            const auto& gmod = vis.gmod(visVersion);
            const auto& codebooks = vis.codebooks(visVersion);
            const auto& locations = vis.locations(visVersion);

            auto primaryItem = GmodPath::fromShortPath("411.1/C101.63/S206", gmod, locations);
            REQUIRE(primaryItem.has_value());

            auto qtyTag = codebooks[CodebookName::Quantity].createTag("temperature");
            REQUIRE(qtyTag.has_value());

            auto builder = LocalIdBuilder::create(visVersion)
                               .withVerboseMode(true)
                               .withPrimaryItem(*primaryItem)
                               .withMetadataTag(*qtyTag);

            auto mqttLocalId = mqtt::LocalId{ builder };

            CHECK(mqttLocalId.builder().isVerboseMode());
            CHECK_FALSE(mqttLocalId.builder().hasCustomTag());
            CHECK_EQ(mqttLocalId.builder().metadataTags().size(), 1u);

            CHECK_EQ(mqttLocalId.toString().find('~'), std::string::npos);
        }

        TEST_CASE("mqtt::LocalId - equality")
        {
            const auto& vis = VIS::instance();
            const auto visVersion = VisVersion::v3_4a;
            const auto& gmod = vis.gmod(visVersion);
            const auto& codebooks = vis.codebooks(visVersion);
            const auto& locations = vis.locations(visVersion);

            auto primaryItem = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations);
            REQUIRE(primaryItem.has_value());

            auto qtyTag = codebooks[CodebookName::Quantity].createTag("temperature");
            REQUIRE(qtyTag.has_value());

            auto builder = LocalIdBuilder::create(visVersion).withPrimaryItem(*primaryItem).withMetadataTag(*qtyTag);

            auto a = mqtt::LocalId{ builder };
            auto b = mqtt::LocalId{ builder };

            auto pressureTag = codebooks[CodebookName::Quantity].createTag("pressure");
            REQUIRE(pressureTag.has_value());
            auto otherBuilder =
                LocalIdBuilder::create(visVersion).withPrimaryItem(*primaryItem).withMetadataTag(*pressureTag);
            auto c = mqtt::LocalId{ otherBuilder };

            CHECK_EQ(a, b);
            CHECK(a == b);
            CHECK_FALSE(a != b);

            CHECK_NE(a, c);
            CHECK_FALSE(a == c);
            CHECK(a != c);
        }
    }
} // namespace dnv::vista::sdk::tests
