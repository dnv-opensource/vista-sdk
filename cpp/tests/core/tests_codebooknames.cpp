#include <doctest/doctest.h>

#include <dnv/vista/sdk/core/CodebookName.h>
#include <dnv/vista/sdk/core/VIS.h>

#include <stdexcept>

namespace dnv::vista::sdk::tests
{
    TEST_SUITE("CodebookNames")
    {
        TEST_CASE("enum values are sequential")
        {
            CHECK_EQ(1, static_cast<std::uint8_t>(CodebookName::Quantity));
            CHECK_EQ(2, static_cast<std::uint8_t>(CodebookName::Content));
            CHECK_EQ(3, static_cast<std::uint8_t>(CodebookName::Calculation));
            CHECK_EQ(4, static_cast<std::uint8_t>(CodebookName::State));
            CHECK_EQ(5, static_cast<std::uint8_t>(CodebookName::Command));
            CHECK_EQ(6, static_cast<std::uint8_t>(CodebookName::Type));
            CHECK_EQ(7, static_cast<std::uint8_t>(CodebookName::FunctionalServices));
            CHECK_EQ(8, static_cast<std::uint8_t>(CodebookName::MaintenanceCategory));
            CHECK_EQ(9, static_cast<std::uint8_t>(CodebookName::ActivityType));
            CHECK_EQ(10, static_cast<std::uint8_t>(CodebookName::Position));
            CHECK_EQ(11, static_cast<std::uint8_t>(CodebookName::Detail));
        }

        TEST_CASE("fromPrefix - valid prefixes")
        {
            CHECK_EQ(CodebookNames::fromPrefix("qty"), CodebookName::Quantity);
            CHECK_EQ(CodebookNames::fromPrefix("cnt"), CodebookName::Content);
            CHECK_EQ(CodebookNames::fromPrefix("calc"), CodebookName::Calculation);
            CHECK_EQ(CodebookNames::fromPrefix("state"), CodebookName::State);
            CHECK_EQ(CodebookNames::fromPrefix("cmd"), CodebookName::Command);
            CHECK_EQ(CodebookNames::fromPrefix("type"), CodebookName::Type);
            CHECK_EQ(CodebookNames::fromPrefix("funct.svc"), CodebookName::FunctionalServices);
            CHECK_EQ(CodebookNames::fromPrefix("maint.cat"), CodebookName::MaintenanceCategory);
            CHECK_EQ(CodebookNames::fromPrefix("act.type"), CodebookName::ActivityType);
            CHECK_EQ(CodebookNames::fromPrefix("pos"), CodebookName::Position);
            CHECK_EQ(CodebookNames::fromPrefix("detail"), CodebookName::Detail);
        }

        TEST_CASE("fromPrefix - invalid prefix throws")
        {
            CHECK_THROWS_AS(CodebookNames::fromPrefix("invalid"), std::invalid_argument);
        }

        TEST_CASE("fromPrefix - empty string throws")
        {
            CHECK_THROWS_AS(CodebookNames::fromPrefix(""), std::invalid_argument);
        }

        TEST_CASE("fromPrefix - case sensitive")
        {
            CHECK_THROWS_AS(CodebookNames::fromPrefix("QTY"), std::invalid_argument);
            CHECK_THROWS_AS(CodebookNames::fromPrefix("Qty"), std::invalid_argument);
            CHECK_THROWS_AS(CodebookNames::fromPrefix("CNT"), std::invalid_argument);
        }

        TEST_CASE("fromPrefix - whitespace not trimmed")
        {
            CHECK_THROWS_AS(CodebookNames::fromPrefix(" qty"), std::invalid_argument);
            CHECK_THROWS_AS(CodebookNames::fromPrefix("qty "), std::invalid_argument);
            CHECK_THROWS_AS(CodebookNames::fromPrefix(" qty "), std::invalid_argument);
        }

        TEST_CASE("fromPrefix - similar but invalid prefixes throw")
        {
            CHECK_THROWS_AS(CodebookNames::fromPrefix("quantity"), std::invalid_argument);
            CHECK_THROWS_AS(CodebookNames::fromPrefix("content"), std::invalid_argument);
        }

        TEST_CASE("toPrefix - valid codebook names")
        {
            CHECK_EQ(CodebookNames::toPrefix(CodebookName::Quantity), "qty");
            CHECK_EQ(CodebookNames::toPrefix(CodebookName::Content), "cnt");
            CHECK_EQ(CodebookNames::toPrefix(CodebookName::Calculation), "calc");
            CHECK_EQ(CodebookNames::toPrefix(CodebookName::State), "state");
            CHECK_EQ(CodebookNames::toPrefix(CodebookName::Command), "cmd");
            CHECK_EQ(CodebookNames::toPrefix(CodebookName::Type), "type");
            CHECK_EQ(CodebookNames::toPrefix(CodebookName::FunctionalServices), "funct.svc");
            CHECK_EQ(CodebookNames::toPrefix(CodebookName::MaintenanceCategory), "maint.cat");
            CHECK_EQ(CodebookNames::toPrefix(CodebookName::ActivityType), "act.type");
            CHECK_EQ(CodebookNames::toPrefix(CodebookName::Position), "pos");
            CHECK_EQ(CodebookNames::toPrefix(CodebookName::Detail), "detail");
        }

        TEST_CASE("fromPrefix/toPrefix roundtrip")
        {
            const std::string_view prefixes[] = { "qty",       "cnt",       "calc",     "state", "cmd",   "type",
                                                  "funct.svc", "maint.cat", "act.type", "pos",   "detail" };

            for (auto prefix : prefixes)
            {
                CHECK_EQ(CodebookNames::toPrefix(CodebookNames::fromPrefix(prefix)), prefix);
            }
        }
    }
} // namespace dnv::vista::sdk::tests
