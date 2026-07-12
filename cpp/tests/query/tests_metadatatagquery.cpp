#include <doctest/doctest.h>

#include <dnv/vista/sdk/query/MetadataTagsQuery.h>

#include <string>

namespace dnv::vista::sdk::tests
{
    TEST_SUITE("MetadataTagsQuery")
    {
        TEST_CASE("empty query matches any LocalId")
        {
            auto query = MetadataTagsQueryBuilder::create().build();

            auto localId1 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/meta/qty-temperature");
            REQUIRE(localId1.has_value());
            CHECK(query.match(*localId1));

            auto localId2 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/meta/qty-temperature/cnt-exhaust.gas");
            REQUIRE(localId2.has_value());
            CHECK(query.match(*localId2));
        }

        TEST_CASE("empty query with exact mode matches only LocalIds with no tags")
        {
            auto query = MetadataTagsQueryBuilder::create().withAllowOtherTags(false).build();

            auto withTags = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/meta/qty-temperature");
            REQUIRE(withTags.has_value());
            CHECK_FALSE(query.match(*withTags));
        }

        TEST_CASE("single tag - subset mode")
        {
            auto query = MetadataTagsQueryBuilder::create()
                             .withTag(CodebookName::Content, "sea.water")
                             .withAllowOtherTags(true)
                             .build();

            // matches when tag is present (may have other tags)
            auto localId1 =
                LocalId::fromString("/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened");
            REQUIRE(localId1.has_value());
            CHECK(query.match(*localId1));

            // does not match different content value
            auto localId2 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/meta/qty-temperature/cnt-exhaust.gas");
            REQUIRE(localId2.has_value());
            CHECK_FALSE(query.match(*localId2));

            // does not match when tag is absent
            auto localId3 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/meta/qty-temperature");
            REQUIRE(localId3.has_value());
            CHECK_FALSE(query.match(*localId3));
        }

        TEST_CASE("single tag - exact mode")
        {
            auto query = MetadataTagsQueryBuilder::create()
                             .withTag(CodebookName::Content, "exhaust.gas")
                             .withAllowOtherTags(false)
                             .build();

            // matches LocalId with only that tag
            auto localId1 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/meta/cnt-exhaust.gas");
            REQUIRE(localId1.has_value());
            CHECK(query.match(*localId1));

            // does not match when extra tags are present
            auto localId2 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/meta/qty-temperature/cnt-exhaust.gas");
            REQUIRE(localId2.has_value());
            CHECK_FALSE(query.match(*localId2));
        }

        TEST_CASE("multiple tags - subset mode")
        {
            auto query = MetadataTagsQueryBuilder::create()
                             .withTag(CodebookName::Quantity, "temperature")
                             .withTag(CodebookName::Content, "exhaust.gas")
                             .withAllowOtherTags(true)
                             .build();

            // matches when both tags are present (extra tags allowed)
            auto localId1 =
                LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet");
            REQUIRE(localId1.has_value());
            CHECK(query.match(*localId1));

            // does not match when one tag is missing
            auto localId2 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/meta/qty-temperature");
            REQUIRE(localId2.has_value());
            CHECK_FALSE(query.match(*localId2));

            // does not match when content value differs
            auto localId3 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/meta/qty-temperature/cnt-sea.water");
            REQUIRE(localId3.has_value());
            CHECK_FALSE(query.match(*localId3));
        }

        TEST_CASE("multiple tags - exact mode")
        {
            auto query = MetadataTagsQueryBuilder::create()
                             .withTag(CodebookName::Quantity, "temperature")
                             .withTag(CodebookName::Content, "exhaust.gas")
                             .withAllowOtherTags(false)
                             .build();

            // matches LocalId with exactly those two tags
            auto localId1 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/meta/qty-temperature/cnt-exhaust.gas");
            REQUIRE(localId1.has_value());
            CHECK(query.match(*localId1));

            // does not match when an extra tag is present
            auto localId2 =
                LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet");
            REQUIRE(localId2.has_value());
            CHECK_FALSE(query.match(*localId2));
        }

        TEST_CASE("from LocalId - subset mode")
        {
            auto source = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/meta/qty-temperature/cnt-exhaust.gas");
            REQUIRE(source.has_value());

            auto query = MetadataTagsQueryBuilder::from(*source, true).build();

            CHECK(query.match(*source));

            // also matches LocalId with same tags plus more
            auto localId2 =
                LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet");
            REQUIRE(localId2.has_value());
            CHECK(query.match(*localId2));
        }

        TEST_CASE("from LocalId - exact mode")
        {
            auto source = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/meta/qty-temperature/cnt-exhaust.gas");
            REQUIRE(source.has_value());

            auto query = MetadataTagsQueryBuilder::from(*source, false).build();

            CHECK(query.match(*source));

            // does not match when LocalId has extra tags
            auto localId2 =
                LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet");
            REQUIRE(localId2.has_value());
            CHECK_FALSE(query.match(*localId2));
        }

        TEST_CASE("builder immutability - withTag returns new builder")
        {
            auto builder1 = MetadataTagsQueryBuilder::create();
            auto builder2 = builder1.withTag(CodebookName::Quantity, "temperature");

            auto query1 = builder1.build();
            auto query2 = builder2.build();

            auto localId = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/meta/qty-temperature");
            REQUIRE(localId.has_value());

            CHECK(query1.match(*localId)); // empty query matches anything
            CHECK(query2.match(*localId)); // qty-temperature present
        }

        TEST_CASE("chained withTag calls use move overloads")
        {
            auto query = MetadataTagsQueryBuilder::create()
                             .withTag(CodebookName::Quantity, "temperature")
                             .withTag(CodebookName::Content, "exhaust.gas")
                             .withTag(CodebookName::Position, "inlet")
                             .withAllowOtherTags(false)
                             .build();

            auto localId = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/meta/qty-temperature/cnt-exhaust.gas/pos-inlet");
            REQUIRE(localId.has_value());
            CHECK(query.match(*localId));
        }
    }
} // namespace dnv::vista::sdk::tests
