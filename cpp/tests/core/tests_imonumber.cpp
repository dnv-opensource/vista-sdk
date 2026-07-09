#include <doctest/doctest.h>

#include <dnv/vista/sdk/core/ImoNumber.h>

#include <JSON/Parser.h>

#include <EmbeddedTestData.h>

#include <optional>
#include <string>
#include <vector>

namespace dnv::vista::sdk::tests
{
    namespace
    {
        struct ImoNumberTestData
        {
            std::string value;
            bool success;
            std::optional<std::string> output;
        };

        std::vector<ImoNumberTestData> loadImoNumberData()
        {
            auto raw = EmbeddedTestData::text("ImoNumbers.json");
            REQUIRE_FALSE(raw.empty());

            auto docOpt = json::Document::fromString(raw);
            REQUIRE(docOpt.has_value());

            auto arrayRef = (*docOpt)["imoNumbers"].rootRef<json::Array>();
            REQUIRE(arrayRef.has_value());
            const json::Array& array = arrayRef->get();

            std::vector<ImoNumberTestData> result;
            result.reserve(array.size());

            for (const auto& elem : array)
            {
                ImoNumberTestData data;

                auto valueOpt = elem["value"].root<std::string>();
                if (!valueOpt)
                {
                    continue;
                }
                data.value = std::move(*valueOpt);

                auto successOpt = elem["success"].root<bool>();
                if (!successOpt)
                {
                    continue;
                }
                data.success = *successOpt;

                if (auto out = elem["output"].root<std::string>())
                {
                    data.output = std::move(*out);
                }

                result.push_back(std::move(data));
            }

            return result;
        }
    } // namespace

    TEST_SUITE("ImoNumber")
    {
        TEST_CASE("fromString - test data")
        {
            const auto cases = loadImoNumberData();
            CHECK_FALSE(cases.empty());

            for (const auto& tc : cases)
            {
                INFO("value: '", tc.value, "'");

                auto result = ImoNumber::fromString(tc.value);

                if (tc.success)
                {
                    REQUIRE(result.has_value());
                    if (tc.output.has_value())
                    {
                        CHECK_EQ(result->toString(), tc.output.value());
                    }
                }
                else
                {
                    CHECK_FALSE(result.has_value());
                }
            }
        }

        TEST_CASE("isValid - valid numbers")
        {
            CHECK(ImoNumber::isValid(9074729));
            CHECK(ImoNumber::isValid(9785811));
            CHECK(ImoNumber::isValid(9704611));
        }

        TEST_CASE("isValid - invalid numbers")
        {
            CHECK_FALSE(ImoNumber::isValid(-1));
            CHECK_FALSE(ImoNumber::isValid(0));
            CHECK_FALSE(ImoNumber::isValid(1));
            CHECK_FALSE(ImoNumber::isValid(123412034));
            CHECK_FALSE(ImoNumber::isValid(1234507));
        }

        TEST_CASE("constructor - valid int")
        {
            CHECK_NOTHROW(ImoNumber{ 9074729 });
            auto imo = ImoNumber{ 9074729 };
            CHECK_EQ(imo.toString(), "IMO9074729");
        }

        TEST_CASE("constructor - invalid int throws")
        {
            CHECK_THROWS_AS(ImoNumber{ 1234507 }, std::invalid_argument);
            CHECK_THROWS_AS(ImoNumber{ 0 }, std::invalid_argument);
        }

        TEST_CASE("constructor - valid string")
        {
            CHECK_NOTHROW(ImoNumber{ "9074729" });
            CHECK_NOTHROW(ImoNumber{ "IMO9074729" });

            auto imo1 = ImoNumber{ "9074729" };
            auto imo2 = ImoNumber{ "IMO9074729" };

            CHECK_EQ(imo1.toString(), "IMO9074729");
            CHECK_EQ(imo2.toString(), "IMO9074729");
            CHECK_EQ(imo1, imo2);
        }

        TEST_CASE("constructor - invalid string throws")
        {
            CHECK_THROWS_AS(ImoNumber{ "1234507" }, std::invalid_argument);
            CHECK_THROWS_AS(ImoNumber{ "IM9074729" }, std::invalid_argument);
            CHECK_THROWS_AS((void)ImoNumber{ "" }, std::invalid_argument);
            CHECK_THROWS_AS(ImoNumber{ "abc" }, std::invalid_argument);
        }

        TEST_CASE("equality - same values")
        {
            auto imo1 = ImoNumber{ 9074729 };
            auto imo2 = ImoNumber{ "IMO9074729" };
            auto imo3 = ImoNumber{ "9074729" };

            CHECK_EQ(imo1, imo2);
            CHECK_EQ(imo2, imo3);
            CHECK_EQ(imo1, imo3);
        }

        TEST_CASE("equality - different values")
        {
            auto imo1 = ImoNumber{ 9074729 };
            auto imo2 = ImoNumber{ 9785811 };

            CHECK_NE(imo1, imo2);
        }

        TEST_CASE("value - returns raw integer")
        {
            auto imo = ImoNumber{ 9074729 };
            CHECK_EQ(imo.value(), 9074729);

            auto imo2 = ImoNumber{ 9785811 };
            CHECK_EQ(imo2.value(), 9785811);
        }

        TEST_CASE("toString - format with prefix")
        {
            auto imo = ImoNumber{ 9074729 };
            CHECK_EQ(imo.toString(), "IMO9074729");
        }
    }
} // namespace dnv::vista::sdk::tests
