#include <doctest/doctest.h>

#include <dnv/VistaSDK.h>

namespace dnv::vista::sdk::tests
{
    namespace ts = transport::timeseries;

    TEST_SUITE("DataChannelId")
    {
        TEST_CASE("Construction - from LocalId string")
        {
            auto id = ts::DataChannelId::fromString(
                "/dnv-v2/vis-3-4a/411.1/C101.62/S205/meta/qty-temperature/cnt-lubricating.oil/pos-inlet");
            REQUIRE(id.has_value());
            CHECK(id->isLocalId());
            CHECK_FALSE(id->isShortId());
            CHECK(id->localId().has_value());
            CHECK_FALSE(id->shortId().has_value());
        }

        TEST_CASE("Construction - from short id string")
        {
            auto id = ts::DataChannelId::fromString("CH001");
            REQUIRE(id.has_value());
            CHECK_FALSE(id->isLocalId());
            CHECK(id->isShortId());
            REQUIRE(id->shortId().has_value());
            CHECK_EQ(*id->shortId(), "CH001");
            CHECK_FALSE(id->localId().has_value());
        }

        TEST_CASE("Equality - same LocalIds")
        {
            std::string s = "/dnv-v2/vis-3-4a/411.1/C101.62/S205/meta/qty-temperature/cnt-lubricating.oil/pos-inlet";
            auto id1 = ts::DataChannelId::fromString(s);
            auto id2 = ts::DataChannelId::fromString(s);
            REQUIRE(id1.has_value());
            REQUIRE(id2.has_value());
            CHECK_EQ(*id1, *id2);
        }

        TEST_CASE("Equality - same ShortIds")
        {
            auto id1 = ts::DataChannelId::fromString("CH001");
            auto id2 = ts::DataChannelId::fromString("CH001");
            REQUIRE(id1.has_value());
            REQUIRE(id2.has_value());
            CHECK_EQ(*id1, *id2);
        }

        TEST_CASE("Inequality - different types")
        {
            auto localId =
                ts::DataChannelId::fromString("/dnv-v2/vis-3-4a/411.1/C101.44i-5A/C261/meta/qty-temperature");
            auto shortId = ts::DataChannelId::fromString("CH001");
            REQUIRE(localId.has_value());
            REQUIRE(shortId.has_value());
            CHECK_NE(*localId, *shortId);
        }

        TEST_CASE("Inequality - different values")
        {
            auto id1 = ts::DataChannelId::fromString("CH001");
            auto id2 = ts::DataChannelId::fromString("CH002");
            REQUIRE(id1.has_value());
            REQUIRE(id2.has_value());
            CHECK_NE(*id1, *id2);
        }

        TEST_CASE("toString - LocalId")
        {
            std::string s = "/dnv-v2/vis-3-4a/411.1/C101.44i-6A/C261/meta/qty-temperature";
            auto id = ts::DataChannelId::fromString(s);
            REQUIRE(id.has_value());
            CHECK_EQ(id->toString(), s);
        }

        TEST_CASE("toString - ShortId")
        {
            auto id = ts::DataChannelId::fromString("CH001");
            REQUIRE(id.has_value());
            CHECK_EQ(id->toString(), "CH001");
        }

        TEST_CASE("fromString - invalid LocalId becomes ShortId")
        {
            auto id = ts::DataChannelId::fromString("/invalid/local/id");
            REQUIRE(id.has_value());
            CHECK_FALSE(id->isLocalId());
            CHECK(id->isShortId());
            REQUIRE(id->shortId().has_value());
            CHECK_EQ(*id->shortId(), "/invalid/local/id");
        }

        TEST_CASE("fromString - empty string returns nullopt")
        {
            CHECK_FALSE(ts::DataChannelId::fromString("").has_value());
            CHECK_FALSE(ts::DataChannelId::fromString(std::string_view{}).has_value());
        }

        TEST_CASE("match - on LocalId")
        {
            std::string s = "/dnv-v2/vis-3-4a/511.15-1/E32/meta/qty-power";
            auto id = ts::DataChannelId::fromString(s);
            REQUIRE(id.has_value());

            auto result = id->match(
                [](const LocalId& l) { return std::string{ "local:" } + l.toString(); },
                [](std::string_view sv) { return std::string{ "short:" } + std::string{ sv }; });

            CHECK_EQ(result, "local:" + s);
        }

        TEST_CASE("match - on ShortId")
        {
            auto id = ts::DataChannelId::fromString("CH001");
            REQUIRE(id.has_value());

            auto result = id->match(
                [](const LocalId& l) { return std::string{ "local:" } + l.toString(); },
                [](std::string_view sv) { return std::string{ "short:" } + std::string{ sv }; });

            CHECK_EQ(result, "short:CH001");
        }

        TEST_CASE("copy and move - ShortId")
        {
            auto original = ts::DataChannelId::fromString("CH001");
            REQUIRE(original.has_value());

            ts::DataChannelId copy{ *original };
            CHECK_EQ(*original, copy);
            CHECK(copy.isShortId());

            ts::DataChannelId moved{ std::move(*original) };
            CHECK(moved.isShortId());
            REQUIRE(moved.shortId().has_value());
            CHECK_EQ(*moved.shortId(), "CH001");
        }

        TEST_CASE("copy and move - LocalId")
        {
            std::string s = "/dnv-v2/vis-3-4a/511.15-1/E32/meta/qty-power";
            auto original = ts::DataChannelId::fromString(s);
            REQUIRE(original.has_value());

            ts::DataChannelId copy{ *original };
            CHECK_EQ(*original, copy);
            CHECK(copy.isLocalId());
            REQUIRE(copy.localId().has_value());
            CHECK_EQ(copy.localId()->get().toString(), s);

            ts::DataChannelId moved{ std::move(*original) };
            CHECK(moved.isLocalId());
            REQUIRE(moved.localId().has_value());
            CHECK_EQ(moved.localId()->get().toString(), s);
        }

        TEST_CASE("copy and move assignment")
        {
            auto src = ts::DataChannelId::fromString("CH001");
            auto dst = ts::DataChannelId::fromString("/dnv-v2/vis-3-4a/511.11-2/C101/meta/qty-rotational.frequency");
            REQUIRE(src.has_value());
            REQUIRE(dst.has_value());

            // copy assignment
            *dst = *src;
            CHECK_EQ(*dst, *src);
            CHECK(dst->isShortId());

            // move assignment
            auto src2 = ts::DataChannelId::fromString("CH002");
            REQUIRE(src2.has_value());
            *dst = std::move(*src2);
            CHECK(dst->isShortId());
            REQUIRE(dst->shortId().has_value());
            CHECK_EQ(*dst->shortId(), "CH002");
        }

        TEST_CASE("copy and move assignment - target becomes LocalId")
        {
            std::string s = "/dnv-v2/vis-3-4a/511.15-1/E32/meta/qty-power";
            auto src = ts::DataChannelId::fromString(s);
            auto dst = ts::DataChannelId::fromString("CH001");
            REQUIRE(src.has_value());
            REQUIRE(dst.has_value());

            // copy assignment
            *dst = *src;
            CHECK_EQ(*dst, *src);
            CHECK(dst->isLocalId());
            REQUIRE(dst->localId().has_value());
            CHECK_EQ(dst->localId()->get().toString(), s);

            // move assignment
            std::string s2 = "/dnv-v2/vis-3-4a/511.11-2/C101/meta/qty-rotational.frequency";
            auto src2 = ts::DataChannelId::fromString(s2);
            REQUIRE(src2.has_value());
            *dst = std::move(*src2);
            CHECK(dst->isLocalId());
            REQUIRE(dst->localId().has_value());
            CHECK_EQ(dst->localId()->get().toString(), s2);
        }
    }
} // namespace dnv::vista::sdk::tests
