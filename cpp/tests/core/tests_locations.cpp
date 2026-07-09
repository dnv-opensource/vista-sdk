#include <doctest/doctest.h>

#include <dnv/vista/sdk/core/ParsingErrors.h>
#include <dnv/vista/sdk/core/VIS.h>

#include <JSON/Parser.h>

#include <EmbeddedTestData.h>

#include <string>
#include <string_view>
#include <vector>

namespace dnv::vista::sdk::tests
{
    namespace
    {
        struct LocationTestCase
        {
            std::string value;
            bool success;
            std::string output;
            std::vector<std::string> expectedErrors;
        };

        std::vector<LocationTestCase> loadLocationTestCases()
        {
            auto raw = EmbeddedTestData::text("Locations.json");
            REQUIRE_FALSE(raw.empty());

            auto docOpt = json::Document::fromString(raw);
            REQUIRE(docOpt.has_value());

            auto arrayRef = (*docOpt)["locations"].rootRef<json::Array>();
            REQUIRE(arrayRef.has_value());
            const json::Array& array = arrayRef->get();

            std::vector<LocationTestCase> cases;
            cases.reserve(array.size());

            for (const auto& elem : array)
            {
                LocationTestCase tc;

                if (elem["value"].type() == json::Type::Null)
                {
                    continue;
                }

                auto valueOpt = elem["value"].root<std::string>();
                if (!valueOpt)
                {
                    continue;
                }
                tc.value = std::move(*valueOpt);

                auto successOpt = elem["success"].root<bool>();
                if (!successOpt)
                {
                    continue;
                }
                tc.success = *successOpt;

                if (auto out = elem["output"].root<std::string>())
                {
                    tc.output = std::move(*out);
                }

                if (elem["expectedErrorMessages"].type() == json::Type::Array)
                {
                    auto errRef = elem["expectedErrorMessages"].rootRef<json::Array>();
                    if (errRef.has_value())
                    {
                        for (const auto& e : errRef->get())
                        {
                            if (auto msg = e.root<std::string>())
                            {
                                tc.expectedErrors.push_back(std::move(*msg));
                            }
                        }
                    }
                }

                cases.push_back(std::move(tc));
            }

            return cases;
        }
    } // namespace

    TEST_SUITE("Locations")
    {
        TEST_CASE("loads for all versions")
        {
            const auto& vis = VIS::instance();

            for (const auto& version : vis.versions())
            {
                INFO("version: ", VisVersions::toString(version));
                const auto& locations = vis.locations(version);
                CHECK_FALSE(locations.relativeLocations().empty());
                CHECK_FALSE(locations.groups().empty());
            }
        }

        TEST_CASE("LocationGroup enum values are unique and start at 0")
        {
            const int values[] = { static_cast<int>(LocationGroup::Number),
                                   static_cast<int>(LocationGroup::Side),
                                   static_cast<int>(LocationGroup::Vertical),
                                   static_cast<int>(LocationGroup::Transverse),
                                   static_cast<int>(LocationGroup::Longitudinal) };

            CHECK_EQ(values[0], 0);
            CHECK_EQ(std::size(values), 5u);

            for (std::size_t i = 0; i < std::size(values) - 1; ++i)
            {
                CHECK_EQ(values[i + 1], values[i] + 1);
            }
        }

        TEST_CASE("fromString - empty or whitespace returns nullopt with error")
        {
            const auto& locations = VIS::instance().locations(VisVersion::v3_4a);

            for (std::string_view input : { "", "  " })
            {
                INFO("input: '", input, "'");
                ParsingErrors errors;
                auto result = locations.fromString(input, errors);
                CHECK_FALSE(result.has_value());
                CHECK(errors.hasErrors());
            }
        }

        TEST_CASE("fromString - test data")
        {
            const auto& locations = VIS::instance().locations(VisVersion::v3_4a);
            const auto cases = loadLocationTestCases();

            CHECK_FALSE(cases.empty());

            for (const auto& tc : cases)
            {
                INFO("value: '", tc.value, "'");

                auto result = locations.fromString(tc.value);

                ParsingErrors errors;
                auto resultWithErrors = locations.fromString(tc.value, errors);

                if (tc.success)
                {
                    REQUIRE(result.has_value());
                    REQUIRE(resultWithErrors.has_value());
                    CHECK_FALSE(errors.hasErrors());
                    CHECK_EQ(result->value(), tc.output);
                    CHECK_EQ(resultWithErrors->value(), tc.output);
                }
                else
                {
                    CHECK_FALSE(result.has_value());
                    CHECK_FALSE(resultWithErrors.has_value());

                    if (!tc.expectedErrors.empty())
                    {
                        REQUIRE(errors.hasErrors());

                        std::size_t i = 0;
                        for (const auto& [type, message] : errors)
                        {
                            INFO("error[", i, "]: '", message, "'");
                            if (i < tc.expectedErrors.size())
                            {
                                CHECK_EQ(message, tc.expectedErrors[i]);
                            }
                            ++i;
                        }
                        CHECK_EQ(i, tc.expectedErrors.size());
                    }
                }
            }
        }
    }
} // namespace dnv::vista::sdk::tests
