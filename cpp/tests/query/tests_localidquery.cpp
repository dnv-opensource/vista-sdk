#include <doctest/doctest.h>

#include <dnv/VistaSDK.h>

namespace dnv::vista::sdk::tests
{
    TEST_SUITE("LocalIdQuery")
    {
        TEST_CASE("empty query matches all LocalIds")
        {
            auto query = LocalIdQueryBuilder::create().build();

            auto localId = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31/meta/qty-power");
            REQUIRE(localId.has_value());
            CHECK(query.match(*localId));

            auto localId2 =
                LocalId::fromString("/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened");
            REQUIRE(localId2.has_value());
            CHECK(query.match(*localId2));
        }

        TEST_CASE("from LocalId builds exact-match query")
        {
            auto localId = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31/meta/qty-power");
            REQUIRE(localId.has_value());

            auto query = LocalIdQueryBuilder::from(*localId).build();
            CHECK(query.match(*localId));
        }

        TEST_CASE("from LocalId string overload with withoutLocations")
        {
            auto builderOpt =
                LocalIdQueryBuilder::from("/dnv-v2/vis-3-7a/511.11/C101/meta/qty-pressure/cnt-lubricating.oil");
            REQUIRE(builderOpt.has_value());

            const auto& vis = VIS::instance();
            const auto& gmod = vis.gmod(VisVersion::v3_7a);
            const auto& locations = vis.locations(VisVersion::v3_7a);
            auto primaryPath = GmodPath::fromShortPath("511.11/C101", gmod, locations);
            REQUIRE(primaryPath.has_value());

            auto query = std::move(*builderOpt)
                             .withPrimaryItem(
                                 *primaryPath,
                                 [](GmodPathQueryBuilder::Path builder) { return builder.withoutLocations().build(); })
                             .build();

            auto other = LocalId::fromString("/dnv-v2/vis-3-7a/511.11-1/C101/meta/qty-pressure/cnt-lubricating.oil");
            REQUIRE(other.has_value());
            CHECK(query.match(*other));
        }

        TEST_SUITE("withPrimaryItem")
        {
            TEST_CASE("match primary item without locations")
            {
                const auto& vis = VIS::instance();
                const auto& gmod = vis.gmod(VisVersion::v3_4a);
                const auto& locations = vis.locations(VisVersion::v3_4a);

                auto path = GmodPath::fromShortPath("411.1/C101.31", gmod, locations);
                REQUIRE(path.has_value());

                auto query =
                    LocalIdQueryBuilder::create()
                        .withPrimaryItem(
                            *path,
                            [](GmodPathQueryBuilder::Path builder) { return builder.withoutLocations().build(); })
                        .build();

                auto localId1 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31/meta/qty-power");
                REQUIRE(localId1.has_value());
                CHECK(query.match(*localId1));

                // Different location on 411.1 - should match (locations ignored)
                auto localId2 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1-1/C101.31/meta/qty-power");
                REQUIRE(localId2.has_value());
                CHECK(query.match(*localId2));

                // Different node - should not match
                auto localId3 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C102.31/meta/qty-power");
                REQUIRE(localId3.has_value());
                CHECK_FALSE(query.match(*localId3));
            }

            TEST_CASE("no match with different location when locations required")
            {
                const auto& vis = VIS::instance();
                const auto& gmod = vis.gmod(VisVersion::v3_4a);
                const auto& locations = vis.locations(VisVersion::v3_4a);

                auto path = GmodPath::fromShortPath("411.1/C101.31-1", gmod, locations);
                REQUIRE(path.has_value());

                auto query = LocalIdQueryBuilder::create().withPrimaryItem(*path).build();

                auto localId = LocalId::fromString(
                    "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet");
                REQUIRE(localId.has_value());
                CHECK_FALSE(query.match(*localId));
            }

            TEST_CASE("match primary with different location using withoutLocations on GmodPathQuery")
            {
                const auto& vis = VIS::instance();
                const auto& gmod = vis.gmod(VisVersion::v3_4a);
                const auto& locations = vis.locations(VisVersion::v3_4a);

                auto path = GmodPath::fromShortPath("411.1-2/C101.63/S206", gmod, locations);
                REQUIRE(path.has_value());

                auto query = LocalIdQueryBuilder::create()
                                 .withPrimaryItem(GmodPathQueryBuilder::from(*path).withoutLocations().build())
                                 .build();

                auto localId = LocalId::fromString(
                    "/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/~propulsion.engine/~cooling.system/meta/qty-temperature/"
                    "cnt-exhaust.gas/pos-inlet");
                REQUIRE(localId.has_value());
                CHECK(query.match(*localId));
            }

            TEST_CASE("no match with different first node")
            {
                const auto& vis = VIS::instance();
                const auto& gmod = vis.gmod(VisVersion::v3_4a);
                const auto& locations = vis.locations(VisVersion::v3_4a);

                auto path = GmodPath::fromShortPath("411.1", gmod, locations);
                REQUIRE(path.has_value());

                auto query =
                    LocalIdQueryBuilder::create()
                        .withPrimaryItem(
                            *path,
                            [](GmodPathQueryBuilder::Path builder) { return builder.withoutLocations().build(); })
                        .build();

                auto localId =
                    LocalId::fromString("/dnv-v2/vis-3-4a/511.11-21O/C101.67/S208/meta/qty-pressure/cnt-air/state-low");
                REQUIRE(localId.has_value());
                CHECK_FALSE(query.match(*localId));
            }

            TEST_CASE("Nodes builder - match by node code")
            {
                const auto& vis = VIS::instance();
                const auto& gmod = vis.gmod(VisVersion::v3_4a);

                auto nodeC101 = gmod.node("C101");
                REQUIRE(nodeC101.has_value());

                auto query = LocalIdQueryBuilder::create()
                                 .withPrimaryItem([&nodeC101](GmodPathQueryBuilder::Nodes nodes) {
                                     return nodes.withNode(**nodeC101, true).build();
                                 })
                                 .build();

                auto localId1 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31/meta/qty-power");
                REQUIRE(localId1.has_value());
                CHECK(query.match(*localId1));

                auto localId2 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C102.31/meta/qty-power");
                REQUIRE(localId2.has_value());
                CHECK_FALSE(query.match(*localId2));
            }

            TEST_CASE("withNode matchAllLocations via Path configure")
            {
                const auto& vis = VIS::instance();
                const auto& gmod = vis.gmod(VisVersion::v3_7a);

                auto node433 = gmod.node("433.1");
                REQUIRE(node433.has_value());
                const auto* pNode433 = *node433;

                auto localId =
                    LocalId::fromString("/dnv-v2/vis-3-7a/433.1-S/C322.91/S205/meta/qty-conductivity/detail-relative");
                REQUIRE(localId.has_value());

                auto query = LocalIdQueryBuilder::from(*localId)
                                 .withPrimaryItem(
                                     localId->primaryItem(),
                                     [pNode433](GmodPathQueryBuilder::Path builder) {
                                         return builder
                                             .withNode(
                                                 [pNode433](const std::unordered_map<std::string, const GmodNode*>&) {
                                                     return pNode433;
                                                 },
                                                 true)
                                             .build();
                                     })
                                 .build();

                CHECK(query.match(*localId));
            }
        }

        TEST_SUITE("withSecondaryItem")
        {
            TEST_CASE("match secondary item without locations")
            {
                const auto& vis = VIS::instance();
                const auto& gmod = vis.gmod(VisVersion::v3_4a);
                const auto& locations = vis.locations(VisVersion::v3_4a);

                auto path = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations);
                REQUIRE(path.has_value());

                auto query = LocalIdQueryBuilder::create()
                                 .withSecondaryItem(GmodPathQueryBuilder::from(*path).withoutLocations().build())
                                 .build();

                auto localId = LocalId::fromString(
                    "/dnv-v2/vis-3-4a/411.1/C101.63/S206/sec/411.1/C101.31-5/~propulsion.engine/~cooling.system"
                    "/~for.propulsion.engine/~cylinder.5/meta/qty-temperature/cnt-exhaust.gas/pos-inlet");
                REQUIRE(localId.has_value());
                CHECK(query.match(*localId));
            }
        }

        TEST_SUITE("withTags")
        {
            TEST_CASE("match by content tag")
            {
                auto query = LocalIdQueryBuilder::create()
                                 .withTags([](MetadataTagsQueryBuilder& tags) {
                                     return tags.withTag(CodebookName::Content, "sea.water").build();
                                 })
                                 .build();

                auto localId = LocalId::fromString(
                    "/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened");
                REQUIRE(localId.has_value());
                CHECK(query.match(*localId));
            }

            TEST_CASE("match by quantity tag")
            {
                const auto& vis = VIS::instance();
                const auto& codebooks = vis.codebooks(VisVersion::v3_4a);

                auto qtyTag = codebooks[CodebookName::Quantity].createTag("power");
                REQUIRE(qtyTag.has_value());

                auto query =
                    LocalIdQueryBuilder::create()
                        .withTags([&qtyTag](MetadataTagsQueryBuilder& tags) { return tags.withTag(*qtyTag).build(); })
                        .build();

                auto localId1 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31/meta/qty-power");
                REQUIRE(localId1.has_value());
                CHECK(query.match(*localId1));

                auto localId2 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31/meta/qty-temperature");
                REQUIRE(localId2.has_value());
                CHECK_FALSE(query.match(*localId2));
            }

            TEST_CASE("MetadataTagsQuery overload")
            {
                auto tagsQuery = MetadataTagsQueryBuilder::create().withTag(CodebookName::Content, "sea.water").build();

                auto query = LocalIdQueryBuilder::create().withTags(tagsQuery).build();

                auto localId = LocalId::fromString(
                    "/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened");
                REQUIRE(localId.has_value());
                CHECK(query.match(*localId));
            }

            TEST_CASE("chained withTags calls accumulate tags instead of replacing them")
            {
                auto query = LocalIdQueryBuilder::create()
                                 .withTags([](MetadataTagsQueryBuilder& tags) {
                                     return tags.withTag(CodebookName::Quantity, "temperature").build();
                                 })
                                 .withTags([](MetadataTagsQueryBuilder& tags) {
                                     return tags.withTag(CodebookName::Content, "exhaust.gas").build();
                                 })
                                 .build();

                auto localIdWithBoth =
                    LocalId::fromString("/dnv-v2/vis-3-4a/411.1/meta/qty-temperature/cnt-exhaust.gas");
                REQUIRE(localIdWithBoth.has_value());
                CHECK(query.match(*localIdWithBoth));

                auto localIdWithOnlySecond = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/meta/cnt-exhaust.gas");
                REQUIRE(localIdWithOnlySecond.has_value());
                CHECK_FALSE(query.match(*localIdWithOnlySecond));
            }
        }

        TEST_SUITE("secondary item presence")
        {
            TEST_CASE("from exact match requires same secondary")
            {
                auto baseLocalId = LocalId::fromString("/dnv-v2/vis-3-9a/411.1/C101.31/meta/qty-power");
                REQUIRE(baseLocalId.has_value());

                auto query = LocalIdQueryBuilder::from(*baseLocalId).build();
                CHECK(query.match(*baseLocalId));

                auto otherLocalId = LocalId::fromString("/dnv-v2/vis-3-9a/411.1/C101.31/sec/412.3/meta/qty-power");
                REQUIRE(otherLocalId.has_value());
                CHECK_FALSE(query.match(*otherLocalId));
            }

            TEST_CASE("withoutSecondaryItem rejects LocalIds that have secondary")
            {
                auto baseLocalId = LocalId::fromString("/dnv-v2/vis-3-9a/411.1/C101.31/meta/qty-power");
                REQUIRE(baseLocalId.has_value());

                auto otherLocalId = LocalId::fromString("/dnv-v2/vis-3-9a/411.1/C101.31/sec/412.3/meta/qty-power");
                REQUIRE(otherLocalId.has_value());

                auto query = LocalIdQueryBuilder::from(*baseLocalId).withoutSecondaryItem().build();
                CHECK_FALSE(query.match(*otherLocalId));
            }

            TEST_CASE("withAnySecondaryItem accepts LocalIds with or without secondary")
            {
                auto baseLocalId = LocalId::fromString("/dnv-v2/vis-3-9a/411.1/C101.31/meta/qty-power");
                REQUIRE(baseLocalId.has_value());

                auto otherLocalId = LocalId::fromString("/dnv-v2/vis-3-9a/411.1/C101.31/sec/412.3/meta/qty-power");
                REQUIRE(otherLocalId.has_value());

                auto query = LocalIdQueryBuilder::from(*baseLocalId).withAnySecondaryItem().build();
                CHECK(query.match(*baseLocalId));
                CHECK(query.match(*otherLocalId));
            }
        }

        TEST_SUITE("withAnyNodeBefore")
        {
            TEST_CASE("match any path before C101")
            {
                const auto& vis = VIS::instance();
                const auto& gmod = vis.gmod(VisVersion::v3_9a);
                const auto& locations = vis.locations(VisVersion::v3_9a);
                const auto& codebooks = vis.codebooks(VisVersion::v3_9a);

                auto basePath = GmodPath::fromShortPath("411.1/C101.31", gmod, locations);
                REQUIRE(basePath.has_value());

                auto baseLocalId = LocalId::fromString("/dnv-v2/vis-3-9a/411.1/C101.31/sec/412.3/meta/qty-power");
                REQUIRE(baseLocalId.has_value());

                auto nodeC101 = gmod.node("C101");
                REQUIRE(nodeC101.has_value());
                const auto* pNodeC101 = *nodeC101;

                auto specificQuery =
                    LocalIdQueryBuilder::from(*baseLocalId)
                        .withPrimaryItem(
                            baseLocalId->primaryItem(),
                            [pNodeC101](GmodPathQueryBuilder::Path path) {
                                return path
                                    .withAnyNodeBefore(
                                        [pNodeC101](const std::unordered_map<std::string, const GmodNode*>&) {
                                            return pNodeC101;
                                        })
                                    .build();
                            })
                        .build();

                auto qtyPowerTag = codebooks[CodebookName::Quantity].createTag("power");
                REQUIRE(qtyPowerTag.has_value());

                auto generalQuery =
                    LocalIdQueryBuilder::create()
                        .withPrimaryItem(
                            *basePath,
                            [pNodeC101](GmodPathQueryBuilder::Path path) {
                                return path
                                    .withAnyNodeBefore(
                                        [pNodeC101](const std::unordered_map<std::string, const GmodNode*>&) {
                                            return pNodeC101;
                                        })
                                    .build();
                            })
                        .withTags([&qtyPowerTag](MetadataTagsQueryBuilder& tags) {
                            return tags.withTag(*qtyPowerTag).build();
                        })
                        .build();

                CHECK(generalQuery.match(*baseLocalId));

                auto l2 = LocalId::fromString("/dnv-v2/vis-3-9a/411.1/C101.31/sec/412.3/meta/qty-power");
                REQUIRE(l2.has_value());
                CHECK(specificQuery.match(*l2));
                CHECK(generalQuery.match(*l2));

                // No secondary - specificQuery fails (from() locked in exact secondary),
                // generalQuery passes (doesn't care about secondary)
                auto l3 = LocalId::fromString("/dnv-v2/vis-3-9a/411.1/C101.31/meta/qty-power");
                REQUIRE(l3.has_value());
                CHECK_FALSE(specificQuery.match(*l3));
                CHECK(generalQuery.match(*l3));

                // Different C-node - both fail
                auto l4 = LocalId::fromString("/dnv-v2/vis-3-9a/411.1/C102.31/meta/qty-power");
                REQUIRE(l4.has_value());
                CHECK_FALSE(specificQuery.match(*l4));
                CHECK_FALSE(generalQuery.match(*l4));

                // Different secondary - specificQuery fails, generalQuery passes
                auto l5 = LocalId::fromString("/dnv-v2/vis-3-9a/411.1/C101.31/sec/412.2/meta/qty-power");
                REQUIRE(l5.has_value());
                CHECK_FALSE(specificQuery.match(*l5));
                CHECK(generalQuery.match(*l5));

                // Different quantity - both fail
                auto l6 = LocalId::fromString("/dnv-v2/vis-3-9a/411.1/C101.31/sec/412.2/meta/qty-pressure");
                REQUIRE(l6.has_value());
                CHECK_FALSE(specificQuery.match(*l6));
                CHECK_FALSE(generalQuery.match(*l6));
            }
        }

        TEST_SUITE("withoutLocations")
        {
            TEST_CASE("withoutLocations preserves withAnyNodeBefore configuration")
            {
                const auto& vis = VIS::instance();
                const auto& gmod = vis.gmod(VisVersion::v3_9a);
                const auto& locations = vis.locations(VisVersion::v3_9a);

                // base path: 411.1/C101.31 -> configure withAnyNodeBefore(C101) then withoutLocations
                auto basePath = GmodPath::fromShortPath("411.1/C101.31", gmod, locations);
                REQUIRE(basePath.has_value());

                auto nodeC101 = gmod.node("C101");
                REQUIRE(nodeC101.has_value());
                const auto* pNodeC101 = *nodeC101;

                auto query = LocalIdQueryBuilder::create()
                                 .withPrimaryItem(
                                     *basePath,
                                     [pNodeC101](GmodPathQueryBuilder::Path builder) {
                                         return builder
                                             .withAnyNodeBefore(
                                                 [pNodeC101](const std::unordered_map<std::string, const GmodNode*>&) {
                                                     return pNodeC101;
                                                 })
                                             .build();
                                     })
                                 .withoutLocations()
                                 .build();

                // 511.11/C101.31 has a different parent but same C101 family -> should match
                auto localIdDiffParent = LocalId::fromString("/dnv-v2/vis-3-9a/511.11/C101.31/meta/qty-power");
                REQUIRE(localIdDiffParent.has_value());
                CHECK(query.match(*localIdDiffParent));

                // 411.1/C102.31 has a different C-node -> should not match
                auto localIdDiffNode = LocalId::fromString("/dnv-v2/vis-3-9a/411.1/C102.31/meta/qty-power");
                REQUIRE(localIdDiffNode.has_value());
                CHECK_FALSE(query.match(*localIdDiffNode));
            }

            TEST_CASE("strips locations from primary item path")
            {
                const auto& vis = VIS::instance();
                const auto& gmod = vis.gmod(VisVersion::v3_4a);
                const auto& locations = vis.locations(VisVersion::v3_4a);

                auto primaryPath = GmodPath::fromShortPath("1036.13i-2/C662", gmod, locations);
                REQUIRE(primaryPath.has_value());

                auto localId = LocalId::fromString(
                    "/dnv-v2/vis-3-4a/1036.13i-1/C662/sec/411.1-2/C101/meta/qty-pressure/cnt-cargo"
                    "/state-high.high/pos-stage-3/detail-discharge");
                REQUIRE(localId.has_value());

                // With location: no match (different instance number)
                auto builder = LocalIdQueryBuilder::create().withPrimaryItem(*primaryPath);
                CHECK_FALSE(builder.build().match(*localId));

                // Without location via configure: match
                auto builderWithoutLoc = builder.withPrimaryItem(
                    *primaryPath, [](GmodPathQueryBuilder::Path p) { return p.withoutLocations().build(); });
                CHECK(builderWithoutLoc.build().match(*localId));

                // Path stripped of locations still needs withoutLocations configure to match
                auto primaryNoLoc = primaryPath->withoutLocations();
                auto builder2 = LocalIdQueryBuilder::create().withPrimaryItem(primaryNoLoc);
                CHECK_FALSE(builder2.build().match(*localId));

                auto builder2WithoutLoc = builder2.withPrimaryItem(
                    primaryNoLoc, [](GmodPathQueryBuilder::Path p) { return p.withoutLocations().build(); });
                CHECK(builder2WithoutLoc.build().match(*localId));
            }
        }

        TEST_SUITE("use cases")
        {
            TEST_CASE("use case 1 - match P and S locations")
            {
                const auto& vis = VIS::instance();
                const auto& locations = vis.locations(vis.latest());

                auto localId = LocalId::fromString(
                    "/dnv-v2/vis-3-7a/433.1-P/C322/meta/qty-linear.vibration.amplitude/pos-driving.end/"
                    "detail-iso.10816");
                REQUIRE(localId.has_value());

                auto locationP = locations.fromString("P");
                auto locationS = locations.fromString("S");
                REQUIRE(locationP.has_value());
                REQUIRE(locationS.has_value());

                const auto& gmod = vis.gmod(localId->version());
                auto node433 = gmod.node("433.1");
                REQUIRE(node433.has_value());
                const auto* pNode433 = *node433;

                std::vector<Location> locs = { *locationP, *locationS };

                auto query = LocalIdQueryBuilder::from(*localId)
                                 .withPrimaryItem(
                                     localId->primaryItem(),
                                     [pNode433, &locs](GmodPathQueryBuilder::Path builder) {
                                         return builder
                                             .withNode(
                                                 [pNode433](const std::unordered_map<std::string, const GmodNode*>&) {
                                                     return pNode433;
                                                 },
                                                 locs)
                                             .build();
                                     })
                                 .build();

                CHECK(query.match(*localId));

                auto localIdS = LocalId::fromString(
                    "/dnv-v2/vis-3-7a/433.1-S/C322/meta/qty-linear.vibration.amplitude/pos-driving.end/"
                    "detail-iso.10816");
                REQUIRE(localIdS.has_value());
                CHECK(query.match(*localIdS));
            }

            TEST_CASE("use case 2 - match wind turbine arrangements by node family")
            {
                const auto& vis = VIS::instance();

                auto localId = LocalId::fromString(
                    "/dnv-v2/vis-3-7a/511.31/C121/meta/qty-linear.vibration.amplitude/pos-driving.end/"
                    "detail-iso.10816");
                REQUIRE(localId.has_value());

                const auto& gmod = vis.gmod(localId->version());

                auto node5113 = gmod.node("511.3");
                REQUIRE(node5113.has_value());

                auto query = LocalIdQueryBuilder::from(*localId)
                                 .withPrimaryItem([&node5113](GmodPathQueryBuilder::Nodes builder) {
                                     return builder.withNode(**node5113, true).build();
                                 })
                                 .build();

                CHECK(query.match(*localId));

                auto node5114 = gmod.node("511.4");
                REQUIRE(node5114.has_value());
                const auto* pNode5114 = *node5114;

                auto queryOther = LocalIdQueryBuilder::create()
                                      .withPrimaryItem([pNode5114](GmodPathQueryBuilder::Nodes builder) {
                                          return builder.withNode(*pNode5114, true).build();
                                      })
                                      .build();

                CHECK_FALSE(queryOther.match(*localId));
            }

            TEST_CASE("use case 3 - exact tag match only")
            {
                const auto& vis = VIS::instance();

                auto localId =
                    LocalId::fromString("/dnv-v2/vis-3-7a/433.1-S/C322.91/S205/meta/qty-conductivity/detail-relative");
                REQUIRE(localId.has_value());

                auto builder = localId->builder();
                auto visVersion = localId->version();

                auto query = LocalIdQueryBuilder::from(*localId)
                                 .withTags([&localId](MetadataTagsQueryBuilder& tags) {
                                     auto b = tags;
                                     for (const auto& tag : localId->metadataTags())
                                         b = b.withTag(tag);
                                     return b.withAllowOtherTags(false).build();
                                 })
                                 .build();

                CHECK(query.match(*localId));

                const auto& gmod = vis.gmod(visVersion);
                const auto& codebooks = vis.codebooks(visVersion);
                const auto& locations = vis.locations(visVersion);

                auto randomTag = codebooks[CodebookName::Content].createTag("random");
                REQUIRE(randomTag.has_value());
                auto l1 = builder.withMetadataTag(*randomTag).build();

                auto pathOpt = GmodPath::fromShortPath("433.1-1S", gmod, locations);
                REQUIRE(pathOpt.has_value());
                auto l2 = builder.withPrimaryItem(*pathOpt).build();

                CHECK_FALSE(query.match(l1));
                CHECK_FALSE(query.match(l2));
            }

            TEST_CASE("use case 4 - generic withoutLocations on primary")
            {
                auto localIdOpt =
                    LocalId::fromString("/dnv-v2/vis-3-7a/511.11/C101/meta/qty-pressure/cnt-lubricating.oil");
                REQUIRE(localIdOpt.has_value());

                auto query =
                    LocalIdQueryBuilder::from(*localIdOpt)
                        .withPrimaryItem(
                            localIdOpt->primaryItem(),
                            [](GmodPathQueryBuilder::Path builder) { return builder.withoutLocations().build(); })
                        .build();

                auto other =
                    LocalId::fromString("/dnv-v2/vis-3-7a/511.11-1/C101/meta/qty-pressure/cnt-lubricating.oil");
                REQUIRE(other.has_value());
                CHECK(query.match(*other));
            }

            TEST_CASE("withoutLocations matches when other is at latest version")
            {
                const auto latest = VIS::instance().latest();
                const auto latestStr = VisVersions::toString(latest);

                auto localIdStr =
                    "/dnv-v2/vis-" + std::string{ latestStr } + "/511.11/C101/meta/qty-pressure/cnt-lubricating.oil";
                auto otherStr =
                    "/dnv-v2/vis-" + std::string{ latestStr } + "/511.11-1/C101/meta/qty-pressure/cnt-lubricating.oil";

                auto localIdOpt = LocalId::fromString(localIdStr);
                REQUIRE(localIdOpt.has_value());

                auto query =
                    LocalIdQueryBuilder::from(*localIdOpt)
                        .withPrimaryItem(
                            localIdOpt->primaryItem(),
                            [](GmodPathQueryBuilder::Path builder) { return builder.withoutLocations().build(); })
                        .build();

                auto other = LocalId::fromString(otherStr);
                REQUIRE(other.has_value());
                CHECK(query.match(*other));
            }
        }

        TEST_SUITE("happy path")
        {
            TEST_CASE("match all combinations of primary, secondary, and tags")
            {
                auto localId = LocalId::fromString(
                    "/dnv-v2/vis-3-4a/1036.13i-1/C662/sec/411.1-2/C101/meta/qty-pressure/cnt-cargo"
                    "/state-high.high/pos-stage-3/detail-discharge");
                REQUIRE(localId.has_value());

                const auto& primaryItem = localId->primaryItem();
                const auto& secondaryItemOpt = localId->secondaryItem();
                REQUIRE(secondaryItemOpt.has_value());
                const auto& secondaryItem = *secondaryItemOpt;

                // Exact match from builder
                CHECK(LocalIdQueryBuilder::from(*localId).build().match(*localId));

                for (bool individualized : { true, false })
                {
                    CAPTURE(individualized);

                    auto primaryPathBuilder = GmodPathQueryBuilder::from(primaryItem);
                    auto secondaryPathBuilder = GmodPathQueryBuilder::from(secondaryItem);

                    if (!individualized)
                    {
                        primaryPathBuilder = std::move(primaryPathBuilder).withoutLocations();
                        secondaryPathBuilder = std::move(secondaryPathBuilder).withoutLocations();
                    }

                    auto primaryQuery = primaryPathBuilder.build();
                    auto secondaryQuery = secondaryPathBuilder.build();

                    // Primary only
                    CHECK(LocalIdQueryBuilder::create().withPrimaryItem(primaryQuery).build().match(*localId));
                    // Secondary only
                    CHECK(LocalIdQueryBuilder::create().withSecondaryItem(secondaryQuery).build().match(*localId));
                    // Tags only
                    {
                        auto b = LocalIdQueryBuilder::create();
                        for (const auto& tag : localId->metadataTags())
                            b = std::move(b).withTags(
                                [&tag](MetadataTagsQueryBuilder& tags) { return tags.withTag(tag).build(); });
                        CHECK(b.build().match(*localId));
                    }
                    // Primary + secondary
                    CHECK(LocalIdQueryBuilder::create()
                              .withPrimaryItem(primaryQuery)
                              .withSecondaryItem(secondaryQuery)
                              .build()
                              .match(*localId));
                    // Primary + tags
                    {
                        auto b = LocalIdQueryBuilder::create().withPrimaryItem(primaryQuery);
                        for (const auto& tag : localId->metadataTags())
                            b = std::move(b).withTags(
                                [&tag](MetadataTagsQueryBuilder& tags) { return tags.withTag(tag).build(); });
                        CHECK(b.build().match(*localId));
                    }
                    // Secondary + tags
                    {
                        auto b = LocalIdQueryBuilder::create().withSecondaryItem(secondaryQuery);
                        for (const auto& tag : localId->metadataTags())
                            b = std::move(b).withTags(
                                [&tag](MetadataTagsQueryBuilder& tags) { return tags.withTag(tag).build(); });
                        CHECK(b.build().match(*localId));
                    }
                    // Primary + secondary + tags
                    {
                        auto b = LocalIdQueryBuilder::create()
                                     .withPrimaryItem(primaryQuery)
                                     .withSecondaryItem(secondaryQuery);
                        for (const auto& tag : localId->metadataTags())
                            b = std::move(b).withTags(
                                [&tag](MetadataTagsQueryBuilder& tags) { return tags.withTag(tag).build(); });
                        CHECK(b.build().match(*localId));
                    }
                }
            }
        }

        TEST_CASE("Nodes builder - withNode without matchAllLocations (default false)")
        {
            // Verifies withNode(node, matchAllLocations=false) requires no location on the node
            const auto& vis = VIS::instance();
            const auto& gmod = vis.gmod(VisVersion::v3_4a);

            auto nodeC101 = gmod.node("C101");
            REQUIRE(nodeC101.has_value());

            // matchAllLocations=false (default): node must have no location in the target
            auto query = LocalIdQueryBuilder::create()
                             .withPrimaryItem([&nodeC101](GmodPathQueryBuilder::Nodes nodes) {
                                 return nodes.withNode(**nodeC101).build();
                             })
                             .build();

            // C101 in target has no location -> should match
            auto localId1 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C101.31/meta/qty-power");
            REQUIRE(localId1.has_value());
            CHECK(query.match(*localId1));

            // C101.31 resolves to C101 (parent code), still no loc on C101 -> match
            auto localId2 = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/C102.31/meta/qty-power");
            REQUIRE(localId2.has_value());
            CHECK_FALSE(query.match(*localId2));
        }

        TEST_CASE("optional handling - empty optional is false, valid optional is true")
        {
            auto query = LocalIdQueryBuilder::create().build();

            std::optional<LocalId> emptyLocalId;
            bool emptyResult = emptyLocalId.has_value() ? query.match(*emptyLocalId) : false;
            CHECK_FALSE(emptyResult);

            auto validLocalId = LocalId::fromString("/dnv-v2/vis-3-4a/411.1/meta/qty-power");
            bool validResult = validLocalId.has_value() ? query.match(*validLocalId) : false;
            CHECK(validResult);
        }

        TEST_SUITE("sample LocalIds")
        {
            TEST_CASE("from builds self-matching query and empty query matches all")
            {
                const std::string samples[] = {
                    "/dnv-v2/vis-3-4a/623.121/H201/sec/412.722-F/C542/meta/qty-level/cnt-lubricating.oil/state-low",
                    "/dnv-v2/vis-3-4a/412.723-F/C261/meta/qty-temperature/state-high",
                    "/dnv-v2/vis-3-4a/412.723-A/C261/meta/qty-temperature/state-high",
                    "/dnv-v2/vis-3-4a/412.723-A/C261/sec/411.1/C101/meta/qty-temperature/state-high/cmd-slow.down",
                    "/dnv-v2/vis-3-4a/623.1/sec/412.722-F/CS5/meta/qty-level/cnt-lubricating.oil/state-high",
                    "/dnv-v2/vis-3-4a/623.1/sec/412.722-F/CS5/meta/qty-level/cnt-lubricating.oil/state-low",
                    "/dnv-v2/vis-3-4a/623.22i-1/S110/sec/412.722-F/C542/meta/state-running",
                    "/dnv-v2/vis-3-4a/623.22i-1/S110/sec/412.722-F/C542/meta/state-failure",
                    "/dnv-v2/vis-3-4a/623.22i-1/S110/sec/412.722-F/C542/meta/cmd-start",
                    "/dnv-v2/vis-3-4a/623.22i-1/S110/sec/412.722-F/C542/meta/cmd-stop",
                    "/dnv-v2/vis-3-4a/623.22i-1/S110.2/E31/sec/412.722-F/C542/meta/qty-electric.current/"
                    "cnt-lubricating.oil",
                    "/dnv-v2/vis-3-4a/623.22i-1/S110/sec/412.722-F/C542/meta/state-remote.control",
                    "/dnv-v2/vis-3-4a/623.22i-2/S110/sec/412.722-F/C542/meta/state-running",
                    "/dnv-v2/vis-3-4a/623.22i-2/S110/sec/412.722-F/C542/meta/state-failure",
                    "/dnv-v2/vis-3-4a/623.22i-2/S110/sec/412.722-F/C542/meta/cmd-start",
                    "/dnv-v2/vis-3-4a/623.22i-2/S110/sec/412.722-F/C542/meta/cmd-stop",
                    "/dnv-v2/vis-3-4a/623.22i-2/S110.2/E31/sec/412.722-F/C542/meta/qty-electric.current/"
                    "cnt-lubricating.oil",
                    "/dnv-v2/vis-3-4a/623.22i-2/S110/sec/412.722-F/C542/meta/state-remote.control",
                    "/dnv-v2/vis-3-4a/623.22i/S110/sec/412.722-F/C542/meta/state-stand.by/cmd-start",
                    "/dnv-v2/vis-3-4a/623.1/sec/412.722-F/C542/meta/qty-level/cnt-lubricating.oil/state-low",
                    "/dnv-v2/vis-3-4a/623.22i/S110/sec/412.722-F/C542/meta/state-control.location",
                    "/dnv-v2/vis-3-4a/623.22i/S110/sec/412.722-F/C542/meta/detail-stand.by.start.or.power.failure",
                    "/dnv-v2/vis-3-4a/623.1/sec/412.722-F/C542/meta/qty-level/cnt-lubricating.oil/state-high",
                    "/dnv-v2/vis-3-4a/412.723-F/C261/meta/qty-temperature",
                    "/dnv-v2/vis-3-4a/412.723-A/C261/meta/qty-temperature",
                    "/dnv-v2/vis-3-4a/623.121/H201/sec/412.722-A/C542/meta/qty-level/cnt-lubricating.oil/state-high",
                    "/dnv-v2/vis-3-4a/623.121/H201/sec/412.722-A/C542/meta/qty-level/cnt-lubricating.oil/state-low",
                    "/dnv-v2/vis-3-4a/412.723-A/CS6d/meta/qty-temperature",
                    "/dnv-v2/vis-3-4a/411.1/C101.64i-1/S201.1/C151.2/S110/meta/cnt-hydraulic.oil/state-running"
                };

                auto emptyQuery = LocalIdQueryBuilder::create().build();

                for (const auto& localIdStr : samples)
                {
                    CAPTURE(localIdStr);
                    auto localId = LocalId::fromString(localIdStr);
                    REQUIRE(localId.has_value());
                    CHECK(LocalIdQueryBuilder::from(*localId).build().match(*localId));
                    CHECK(emptyQuery.match(*localId));
                }
            }
        }
    }
} // namespace dnv::vista::sdk::tests
