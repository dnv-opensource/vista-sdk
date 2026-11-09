#include <doctest/doctest.h>

#include <dnv/VistaSDK.h>

#include <JSON/Json.h>

#include <EmbeddedTestData.h>

#include <string>
#include <string_view>
#include <vector>

namespace dnv::vista::sdk::tests
{
    namespace
    {
        struct CodebookTestData
        {
            std::vector<std::pair<std::string, std::string>> validPosition;
            std::vector<std::pair<std::string, std::string>> positions;
            std::vector<std::array<std::string, 4>> states;
            std::vector<std::array<std::string, 8>> tag;
            std::vector<std::array<std::string, 3>> detailTag;
        };

        CodebookTestData loadCodebookTestData()
        {
            auto raw = EmbeddedTestData::text("Codebook.json");
            REQUIRE_FALSE(raw.empty());

            auto docOpt = json::Document::fromString(raw);
            REQUIRE(docOpt.has_value());

            CodebookTestData data;

            {
                auto arrRef = (*docOpt)["ValidPosition"].rootRef<json::Array>();
                REQUIRE(arrRef.has_value());
                for (const auto& elem : arrRef->get())
                {
                    const auto& row = elem.rootRef<json::Array>()->get();
                    data.validPosition.emplace_back(*row[0].root<std::string>(), *row[1].root<std::string>());
                }
            }

            {
                auto arrRef = (*docOpt)["Positions"].rootRef<json::Array>();
                REQUIRE(arrRef.has_value());
                for (const auto& elem : arrRef->get())
                {
                    const auto& row = elem.rootRef<json::Array>()->get();
                    data.positions.emplace_back(*row[0].root<std::string>(), *row[1].root<std::string>());
                }
            }

            {
                auto arrRef = (*docOpt)["States"].rootRef<json::Array>();
                REQUIRE(arrRef.has_value());
                for (const auto& elem : arrRef->get())
                {
                    const auto& row = elem.rootRef<json::Array>()->get();
                    data.states.push_back({
                        *row[0].root<std::string>(),
                        *row[1].root<std::string>(),
                        *row[2].root<std::string>(),
                        *row[3].root<std::string>(),
                    });
                }
            }

            {
                auto arrRef = (*docOpt)["Tag"].rootRef<json::Array>();
                REQUIRE(arrRef.has_value());
                for (const auto& elem : arrRef->get())
                {
                    const auto& row = elem.rootRef<json::Array>()->get();
                    data.tag.push_back({
                        *row[0].root<std::string>(),
                        *row[1].root<std::string>(),
                        *row[2].root<std::string>(),
                        *row[3].root<std::string>(),
                        *row[4].root<std::string>(),
                        *row[5].root<std::string>(),
                        *row[6].root<std::string>(),
                        *row[7].root<std::string>(),
                    });
                }
            }

            {
                auto arrRef = (*docOpt)["DetailTag"].rootRef<json::Array>();
                REQUIRE(arrRef.has_value());
                for (const auto& elem : arrRef->get())
                {
                    const auto& row = elem.rootRef<json::Array>()->get();
                    data.detailTag.push_back({
                        *row[0].root<std::string>(),
                        *row[1].root<std::string>(),
                        *row[2].root<std::string>(),
                    });
                }
            }

            return data;
        }

        PositionValidationResult parsePositionValidationResult(std::string_view str)
        {
            if (str == "Valid")
            {
                return PositionValidationResult::Valid;
            }
            if (str == "Invalid")
            {
                return PositionValidationResult::Invalid;
            }
            if (str == "InvalidOrder")
            {
                return PositionValidationResult::InvalidOrder;
            }
            if (str == "InvalidGrouping")
            {
                return PositionValidationResult::InvalidGrouping;
            }
            if (str == "Custom")
            {
                return PositionValidationResult::Custom;
            }
            FAIL("Unknown PositionValidationResult: " << str);
            return PositionValidationResult::Invalid;
        }
    } // namespace

    TEST_SUITE("Codebook")
    {
        TEST_CASE("position validation")
        {
            const auto& codebook = VIS::instance().codebooks(VisVersion::v3_4a)[CodebookName::Position];
            const auto testData = loadCodebookTestData();

            for (const auto& [input, expectedStr] : testData.validPosition)
            {
                INFO("input: ", input);
                CHECK_EQ(codebook.validatePosition(input), parsePositionValidationResult(expectedStr));
            }
        }

        TEST_CASE("positions - hasStandardValue")
        {
            const auto& positions = VIS::instance().codebooks(VisVersion::v3_4a)[CodebookName::Position];
            const auto testData = loadCodebookTestData();

            for (const auto& [invalidStandardValue, validStandardValue] : testData.positions)
            {
                INFO("invalid: ", invalidStandardValue, " valid: ", validStandardValue);
                CHECK_FALSE(positions.hasStandardValue(invalidStandardValue));
                CHECK(positions.hasStandardValue(validStandardValue));
            }
        }

        TEST_CASE("standard values - known value")
        {
            const auto& positions = VIS::instance().codebooks(VisVersion::v3_4a)[CodebookName::Position];

            CHECK(positions.hasStandardValue("upper"));
            CHECK(positions.groups().contains("Vertical"));
        }

        TEST_CASE("states - hasGroup and hasStandardValue")
        {
            const auto& states = VIS::instance().codebooks(VisVersion::v3_4a)[CodebookName::State];
            const auto testData = loadCodebookTestData();

            for (const auto& row : testData.states)
            {
                INFO(
                    "invalidGroup: ",
                    row[0],
                    " validValue: ",
                    row[1],
                    " validGroup: ",
                    row[2],
                    " secondValidValue: ",
                    row[3]);
                CHECK_FALSE(states.hasGroup(row[0]));
                CHECK(states.hasStandardValue(row[1]));
                CHECK(states.hasGroup(row[2]));
                CHECK(states.hasStandardValue(row[3]));
            }
        }

        TEST_CASE("createTag - standard and custom position tags")
        {
            const auto& codebook = VIS::instance().codebooks(VisVersion::v3_4a)[CodebookName::Position];
            const auto testData = loadCodebookTestData();

            for (const auto& row : testData.tag)
            {
                const std::string& firstTag = row[0];
                const std::string& secondTag = row[1];
                const std::string& thirdTag = row[2];
                const char thirdTagPrefix = row[3][0];
                const std::string& customTag = row[4];
                const char customTagPrefix = row[5][0];
                const std::string& firstInvalidTag = row[6];
                const std::string& secondInvalidTag = row[7];

                INFO("firstTag: ", firstTag);

                auto tag1 = codebook.createTag(firstTag);
                REQUIRE(tag1.has_value());
                CHECK_EQ(tag1->value(), firstTag);
                CHECK_FALSE(tag1->isCustom());

                auto tag2 = codebook.createTag(secondTag);
                REQUIRE(tag2.has_value());
                CHECK_EQ(tag2->value(), secondTag);
                CHECK_FALSE(tag2->isCustom());

                auto tag3 = codebook.createTag(thirdTag);
                REQUIRE(tag3.has_value());
                CHECK_EQ(tag3->value(), thirdTag);
                CHECK_FALSE(tag3->isCustom());
                CHECK_EQ(tag3->prefix(), thirdTagPrefix);

                auto tag4 = codebook.createTag(customTag);
                REQUIRE(tag4.has_value());
                CHECK_EQ(tag4->value(), customTag);
                CHECK(tag4->isCustom());
                CHECK_EQ(tag4->prefix(), customTagPrefix);

                CHECK_FALSE(codebook.createTag(firstInvalidTag).has_value());
                CHECK_FALSE(codebook.createTag(secondInvalidTag).has_value());
            }
        }

        TEST_CASE("groups - contains expected group")
        {
            const auto& positions = VIS::instance().codebooks(VisVersion::v3_4a)[CodebookName::Position];
            const auto& groups = positions.groups();

            CHECK_GT(groups.size(), 1u);
            CHECK(groups.contains("Vertical"));
        }

        TEST_CASE("groups - iterate count")
        {
            const auto& positions = VIS::instance().codebooks(VisVersion::v3_4a)[CodebookName::Position];

            size_t count = 0;
            for ([[maybe_unused]] const auto& _ : positions.groups())
            {
                ++count;
            }

            CHECK_EQ(count, 11u);
        }

        TEST_CASE("standardValues - iterate count")
        {
            const auto& positions = VIS::instance().codebooks(VisVersion::v3_4a)[CodebookName::Position];

            size_t count = 0;
            for ([[maybe_unused]] const auto& _ : positions.standardValues())
            {
                ++count;
            }

            CHECK_EQ(count, 28u);
        }

        TEST_CASE("createTag - detail codebook")
        {
            const auto& codebook = VIS::instance().codebooks(VisVersion::v3_4a)[CodebookName::Detail];
            const auto testData = loadCodebookTestData();

            for (const auto& row : testData.detailTag)
            {
                INFO("valid: ", row[0]);
                CHECK(codebook.createTag(row[0]).has_value());
                CHECK_FALSE(codebook.createTag(row[1]).has_value());
                CHECK_FALSE(codebook.createTag(row[2]).has_value());
            }
        }
    }

    TEST_CASE("toString - returns formatted tag without trailing separator")
    {
        const auto& codebooks = VIS::instance().codebooks(VisVersion::v3_4a);

        auto quantityTag = codebooks[CodebookName::Quantity].createTag("temperature");
        REQUIRE(quantityTag.has_value());
        CHECK_EQ(quantityTag->toString(), "qty-temperature");

        auto contentTag = codebooks[CodebookName::Content].createTag("exhaust.gas");
        REQUIRE(contentTag.has_value());
        CHECK_EQ(contentTag->toString(), "cnt-exhaust.gas");

        auto positionTag = codebooks[CodebookName::Position].createTag("inlet");
        REQUIRE(positionTag.has_value());
        CHECK_EQ(positionTag->toString(), "pos-inlet");
    }
} // namespace dnv::vista::sdk::tests
