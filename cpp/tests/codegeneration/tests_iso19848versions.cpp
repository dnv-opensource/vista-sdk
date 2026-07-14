#include <doctest/doctest.h>

#include <dnv/vista/sdk/transport/ISO19848Versions.h>

#include <SDK/transport/ISO19848VersionsExtensions.h>

#include <algorithm>

using namespace dnv::vista::sdk::transport;

namespace dnv::vista::sdk::tests
{
    TEST_SUITE("ISO19848Versions")
    {
        TEST_CASE("toString")
        {
            auto versionString = ISO19848Versions::toString(ISO19848Version::v2024);
            CHECK_EQ("v2024", versionString);
        }

        TEST_CASE("fromString - optional overload")
        {
            auto v2018 = ISO19848Versions::fromString("v2018");
            CHECK(v2018.has_value());
            CHECK_EQ(ISO19848Version::v2018, *v2018);

            auto v2024 = ISO19848Versions::fromString("v2024");
            CHECK(v2024.has_value());
            CHECK_EQ(ISO19848Version::v2024, *v2024);

            auto invalid = ISO19848Versions::fromString("invalid");
            CHECK_FALSE(invalid.has_value());
        }

        TEST_CASE("toString - invalid enum returns empty string")
        {
            auto invalidVersion = static_cast<ISO19848Version>(255);
            CHECK_EQ("", ISO19848Versions::toString(invalidVersion));
        }

        TEST_CASE("fromString - empty string")
        {
            CHECK_FALSE(ISO19848Versions::fromString("").has_value());
        }

        TEST_CASE("fromString - whitespace only")
        {
            CHECK_FALSE(ISO19848Versions::fromString("   ").has_value());
            CHECK_FALSE(ISO19848Versions::fromString("\t").has_value());
            CHECK_FALSE(ISO19848Versions::fromString("\n").has_value());
        }

        TEST_CASE("fromString - case sensitivity")
        {
            CHECK_FALSE(ISO19848Versions::fromString("V2024").has_value());
            CHECK(ISO19848Versions::fromString("v2024").has_value());
            CHECK_FALSE(ISO19848Versions::fromString("2024").has_value());
        }
    }

    TEST_SUITE("ISO19848VersionsExtensions")
    {
        TEST_CASE("all is non-empty")
        {
            auto versions = ISO19848Versions::all();
            CHECK_FALSE(versions.empty());
        }

        TEST_CASE("all versions roundtrip through toString/fromString")
        {
            for (auto v : ISO19848Versions::all())
            {
                auto str = ISO19848Versions::toString(v);
                CHECK_FALSE(str.empty());
                auto parsed = ISO19848Versions::fromString(str);
                CHECK(parsed.has_value());
                CHECK_EQ(v, *parsed);
            }
        }

        TEST_CASE("latest is contained in all")
        {
            auto versions = ISO19848Versions::all();
            auto latest = ISO19848Versions::latest();
            CHECK_NE(std::find(versions.begin(), versions.end(), latest), versions.end());
        }
    }
} // namespace dnv::vista::sdk::tests
