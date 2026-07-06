#include <doctest/doctest.h>

#include <dnv/vista/sdk/core/VisVersions.h>

#include <SDK/core/VisVersionsExtensions.h>

#include <algorithm>
#include <string_view>

namespace dnv::vista::sdk::tests
{
    TEST_SUITE("VisVersions")
    {
        TEST_CASE("toString")
        {
            auto visVersionString = VisVersions::toString(VisVersion::v3_9a);
            CHECK_EQ("3-9a", visVersionString);
        }

        TEST_CASE("fromString - optional overload")
        {
            auto v34a = VisVersions::fromString("3-4a");
            CHECK(v34a.has_value());
            CHECK_EQ(VisVersion::v3_4a, *v34a);

            auto v39a = VisVersions::fromString("3-9a");
            CHECK(v39a.has_value());
            CHECK_EQ(VisVersion::v3_9a, *v39a);

            auto invalid = VisVersions::fromString("invalid");
            CHECK_FALSE(invalid.has_value());
        }

        TEST_CASE("fromString - out-parameter overload")
        {
            VisVersion result;

            CHECK(VisVersions::fromString("3-4a", result));
            CHECK_EQ(VisVersion::v3_4a, result);

            CHECK(VisVersions::fromString("3-9a", result));
            CHECK_EQ(VisVersion::v3_9a, result);

            CHECK_FALSE(VisVersions::fromString("invalid", result));
        }

        TEST_CASE("toString - invalid enum returns empty string")
        {
            auto invalidVersion = static_cast<VisVersion>(255);
            CHECK_EQ("", VisVersions::toString(invalidVersion));
        }

        TEST_CASE("fromString - empty string")
        {
            CHECK_FALSE(VisVersions::fromString("").has_value());

            VisVersion version;
            CHECK_FALSE(VisVersions::fromString("", version));
        }

        TEST_CASE("fromString - whitespace only")
        {
            CHECK_FALSE(VisVersions::fromString("   ").has_value());
            CHECK_FALSE(VisVersions::fromString("\t").has_value());
            CHECK_FALSE(VisVersions::fromString("\n").has_value());

            VisVersion version;
            CHECK_FALSE(VisVersions::fromString("  ", version));
        }

        TEST_CASE("fromString - case sensitivity")
        {
            CHECK_FALSE(VisVersions::fromString("3-9A").has_value());
            CHECK(VisVersions::fromString("3-9a").has_value());
        }

        TEST_CASE("fromString - all versions roundtrip")
        {
            for (auto v : VisVersions::all())
            {
                auto str = VisVersions::toString(v);
                auto parsed = VisVersions::fromString(str);
                CHECK(parsed.has_value());
                CHECK_EQ(v, *parsed);

                VisVersion result;
                CHECK(VisVersions::fromString(str, result));
                CHECK_EQ(v, result);
            }
        }
    }

    TEST_SUITE("VisVersionsExtensions")
    {
        TEST_CASE("all returns non-empty ordered list")
        {
            auto versions = VisVersions::all();
            CHECK_FALSE(versions.empty());
        }

        TEST_CASE("all contains every version roundtrips via toString")
        {
            for (auto v : VisVersions::all())
            {
                auto str = VisVersions::toString(v);
                CHECK_FALSE(str.empty());
                auto parsed = VisVersions::fromString(str);
                CHECK(parsed.has_value());
                CHECK_EQ(v, *parsed);
            }
        }

        TEST_CASE("latest is last in all")
        {
            auto versions = VisVersions::all();
            REQUIRE_FALSE(versions.empty());
            CHECK_EQ(VisVersions::latest(), versions.back());
        }

        TEST_CASE("isValid returns true for all known versions")
        {
            for (auto v : VisVersions::all())
            {
                CHECK(VisVersions::isValid(v));
            }
        }

        TEST_CASE("isValid returns false for invalid version")
        {
            CHECK_FALSE(VisVersions::isValid(static_cast<VisVersion>(9999)));
        }
    }
} // namespace dnv::vista::sdk::tests
