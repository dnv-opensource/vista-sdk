#include <doctest/doctest.h>

#include <dnv/vista/sdk/core/VIS.h>
#include <dnv/vista/sdk/query/GmodPathQuery.h>

#include <string>
#include <vector>

namespace dnv::vista::sdk::tests
{
    TEST_SUITE("GmodPathQuery")
    {
        TEST_SUITE("Path builder")
        {
            struct PathTestCase
            {
                const char* pathStr;
                VisVersion visVersion;
                // {nodeCode, locations} - empty locations = matchAllLocations
                std::vector<std::pair<std::string, std::vector<std::string>>> parameters;
                bool expectedMatch;
            };

            static const PathTestCase pathTestCases[] = {
                { "411.1-1/C101", VisVersion::v3_4a, {}, true },
                { "411.1-1/C101", VisVersion::v3_4a, { { "411.1", { "1" } } }, true },
                { "411.1-1/C101", VisVersion::v3_4a, { { "411.1", { "A" } } }, false },
                { "433.1-P/C322.31/C173", VisVersion::v3_4a, { { "C322.31", {} } }, true },
                { "433.1-P/C322.31-2/C173", VisVersion::v3_4a, { { "433.1", { "P" } }, { "C322.31", {} } }, true },
                { "433.1-P/C322.31-2/C173", VisVersion::v3_4a, { { "433.1", { "A" } }, { "C322.31", {} } }, false },
                { "433.1-P/C322.31-2/C173",
                  VisVersion::v3_4a,
                  { { "433.1", { "P" } }, { "C322.31", { "1" } } },
                  false },
                { "433.1-A/C322.31-2/C173",
                  VisVersion::v3_4a,
                  { { "433.1", { "P" } }, { "C322.31", { "1" } } },
                  false },
                { "433.1-A/C322.31-2/C173", VisVersion::v3_4a, { { "433.1", {} }, { "C322.31", {} } }, true },
                { "433.1/C322.31-2/C173", VisVersion::v3_4a, { { "433.1", { "A" } } }, false },
                { "433.1/C322.31-2/C173", VisVersion::v3_4a, { { "433.1", {} } }, true }
            };

            TEST_CASE("path builder always matches itself")
            {
                const auto& vis = VIS::instance();
                const auto& gmod = vis.gmod(VisVersion::v3_4a);
                const auto& locations = vis.locations(VisVersion::v3_4a);

                auto path = GmodPath::fromShortPath("411.1-1/C101", gmod, locations);
                REQUIRE(path.has_value());

                auto query = GmodPathQueryBuilder::from(*path).build();
                CHECK(query.match(*path));
            }

            TEST_CASE("path builder parametrized")
            {
                for (const auto& tc : pathTestCases)
                {
                    CAPTURE(tc.pathStr);
                    CAPTURE(tc.expectedMatch);

                    const auto& vis = VIS::instance();
                    const auto& gmod = vis.gmod(tc.visVersion);
                    const auto& locs = vis.locations(tc.visVersion);

                    auto path = GmodPath::fromShortPath(tc.pathStr, gmod, locs);
                    REQUIRE(path.has_value());

                    auto builder = GmodPathQueryBuilder::from(*path);

                    // Verify unmodified builder matches itself
                    CHECK(builder.build().match(*path));

                    for (const auto& [nodeCode, locStrs] : tc.parameters)
                    {
                        if (locStrs.empty())
                        {
                            builder = std::move(builder).withNode(
                                [&nodeCode](
                                    const std::unordered_map<std::string, const GmodNode*>& nodes) -> const GmodNode* {
                                    auto it = nodes.find(nodeCode);
                                    return it != nodes.end() ? it->second : nullptr;
                                },
                                true);
                        }
                        else
                        {
                            std::vector<Location> parsedLocs;
                            for (const auto& locStr : locStrs)
                            {
                                auto loc = locs.fromString(locStr);
                                REQUIRE(loc.has_value());
                                parsedLocs.push_back(*loc);
                            }
                            builder = std::move(builder).withNode(
                                [&nodeCode](
                                    const std::unordered_map<std::string, const GmodNode*>& nodes) -> const GmodNode* {
                                    auto it = nodes.find(nodeCode);
                                    return it != nodes.end() ? it->second : nullptr;
                                },
                                parsedLocs);
                        }
                    }

                    CHECK(builder.build().match(*path) == tc.expectedMatch);
                }
            }
        }

        TEST_SUITE("Nodes builder")
        {
            struct NodesTestCase
            {
                const char* pathStr;
                VisVersion visVersion;
                std::vector<std::pair<std::string, std::vector<std::string>>> parameters;
                bool expectedMatch;
            };

            static const NodesTestCase nodesTestCases[] = {
                { "411.1-1/C101", VisVersion::v3_4a, { { "411.1", { "1" } } }, true },
                { "411.1-1/C101.61/S203.3/S110.2/C101", VisVersion::v3_7a, { { "411.1", { "1" } } }, true },
                { "411.1/C101.61-1/S203.3/S110.2/C101", VisVersion::v3_7a, { { "C101.61", { "1" } } }, true },
                { "511.11/C101.61-1/S203.3/S110.2/C101", VisVersion::v3_7a, { { "C101.61", { "1" } } }, true },
                { "411.1/C101.61-1/S203.3/S110.2/C101", VisVersion::v3_7a, { { "C101.61", {} } }, true },
                { "511.11/C101.61-1/S203.3/S110.2/C101", VisVersion::v3_7a, { { "C101.61", {} } }, true },
                { "221.11/C1141.421/C1051.7/C101.61-2/S203", VisVersion::v3_7a, { { "C101.61", {} } }, true },
                { "411.1/C101.61-1/S203.3/S110.2/C101",
                  VisVersion::v3_7a,
                  { { "411.1", {} }, { "C101.61", {} } },
                  true },
                { "511.11/C101.61-1/S203.3/S110.2/C101",
                  VisVersion::v3_7a,
                  { { "411.1", {} }, { "C101.61", {} } },
                  false },
                { "411.1/C101.61/S203.3-1/S110.2/C101", VisVersion::v3_7a, { { "S203.3", { "1" } } }, true },
                { "411.1/C101.61/S203.3-1/S110.2/C101", VisVersion::v3_7a, { { "S203.3", { "1" } } }, true },
            };

            TEST_CASE("nodes builder parametrized")
            {
                for (const auto& tc : nodesTestCases)
                {
                    CAPTURE(tc.pathStr);
                    CAPTURE(tc.expectedMatch);

                    const auto& vis = VIS::instance();
                    const auto& gmod = vis.gmod(tc.visVersion);
                    const auto& locs = vis.locations(tc.visVersion);

                    auto path = GmodPath::fromShortPath(tc.pathStr, gmod, locs);
                    REQUIRE(path.has_value());

                    auto builder = GmodPathQueryBuilder::create();

                    for (const auto& [nodeCode, locStrs] : tc.parameters)
                    {
                        const auto* nodePtr = gmod.node(nodeCode).value_or(nullptr);
                        REQUIRE(nodePtr != nullptr);

                        if (locStrs.empty())
                        {
                            builder = std::move(builder).withNode(*nodePtr, true);
                        }
                        else
                        {
                            std::vector<Location> parsedLocs;
                            for (const auto& locStr : locStrs)
                            {
                                auto loc = locs.fromString(locStr);
                                REQUIRE(loc.has_value());
                                parsedLocs.push_back(*loc);
                            }
                            builder = std::move(builder).withNode(*nodePtr, parsedLocs);
                        }
                    }

                    CHECK(builder.build().match(*path) == tc.expectedMatch);
                }
            }
        }

        TEST_SUITE("withAnyNodeBefore")
        {
            TEST_CASE("ignores parent nodes before selected node")
            {
                const auto& vis = VIS::instance();
                const auto& gmod = vis.gmod(VisVersion::v3_9a);
                const auto& locs = vis.locations(VisVersion::v3_9a);

                auto basePath = GmodPath::fromShortPath("411.1/C101.31", gmod, locs);
                REQUIRE(basePath.has_value());

                auto query = GmodPathQueryBuilder::from(*basePath)
                                 .withAnyNodeBefore([](const auto& nodes) -> const GmodNode* {
                                     auto it = nodes.find("C101");
                                     return it != nodes.end() ? it->second : nullptr;
                                 })
                                 .withoutLocations()
                                 .build();

                CHECK(query.match(*basePath));

                // different parent - should match (parent ignored)
                auto pathDiffParent = GmodPath::fromShortPath("511.11/C101.31", gmod, locs);
                REQUIRE(pathDiffParent.has_value());
                CHECK(query.match(*pathDiffParent));

                // location on C101.31 - should match (withoutLocations)
                auto pathWithLoc = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locs);
                REQUIRE(pathWithLoc.has_value());
                CHECK(query.match(*pathWithLoc));

                // different C-node - should not match
                auto pathDiffCNode = GmodPath::fromShortPath("411.1/C102.31", gmod, locs);
                REQUIRE(pathDiffCNode.has_value());
                CHECK_FALSE(query.match(*pathDiffCNode));
            }

            TEST_CASE("S206 sensor - matches any parent")
            {
                const auto& vis = VIS::instance();
                const auto& gmod = vis.gmod(VisVersion::v3_4a);
                const auto& locs = vis.locations(VisVersion::v3_4a);

                auto sensorPath = GmodPath::fromShortPath("411.1/C101.63/S206", gmod, locs);
                REQUIRE(sensorPath.has_value());

                auto query = GmodPathQueryBuilder::from(*sensorPath)
                                 .withoutLocations()
                                 .withAnyNodeBefore([](const auto& nodes) -> const GmodNode* {
                                     auto it = nodes.find("S206");
                                     return it != nodes.end() ? it->second : nullptr;
                                 })
                                 .build();

                auto pathLoc1 = GmodPath::fromShortPath("411.1-1/C101.63/S206", gmod, locs);
                auto pathLoc2 = GmodPath::fromShortPath("411.1-2/C101.63/S206", gmod, locs);
                REQUIRE(pathLoc1.has_value());
                REQUIRE(pathLoc2.has_value());
                CHECK(query.match(*pathLoc1));
                CHECK(query.match(*pathLoc2));

                auto pathC10131 = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locs);
                auto pathS203 = GmodPath::fromShortPath("411.1/C101.61/S203", gmod, locs);
                REQUIRE(pathC10131.has_value());
                REQUIRE(pathS203.has_value());
                CHECK_FALSE(query.match(*pathC10131));
                CHECK_FALSE(query.match(*pathS203));
            }

            TEST_CASE("preserves location requirement on target node")
            {
                const auto& vis = VIS::instance();
                const auto& gmod = vis.gmod(VisVersion::v3_4a);
                const auto& locs = vis.locations(VisVersion::v3_4a);

                // base path has location on 433.1, no location on C322.31
                auto basePath = GmodPath::fromShortPath("433.1-P/C322.31", gmod, locs);
                REQUIRE(basePath.has_value());

                auto query = GmodPathQueryBuilder::from(*basePath)
                                 .withAnyNodeBefore([](const auto& nodes) -> const GmodNode* {
                                     auto it = nodes.find("C322");
                                     return it != nodes.end() ? it->second : nullptr;
                                 })
                                 .build();

                CHECK(query.match(*basePath));

                // different parent location - should match (parent ignored)
                auto pathDiffParentLoc = GmodPath::fromShortPath("433.1-S/C322.31", gmod, locs);
                REQUIRE(pathDiffParentLoc.has_value());
                CHECK(query.match(*pathDiffParentLoc));

                // no location on parent - should match (parent ignored)
                auto pathNoParentLoc = GmodPath::fromShortPath("433.1/C322.31", gmod, locs);
                REQUIRE(pathNoParentLoc.has_value());
                CHECK(query.match(*pathNoParentLoc));
            }
        }

        TEST_SUITE("withAnyNodeAfter")
        {
            TEST_CASE("prefix matching - ignores children after selected node")
            {
                const auto& vis = VIS::instance();
                const auto& gmod = vis.gmod(VisVersion::v3_4a);
                const auto& locs = vis.locations(VisVersion::v3_4a);

                auto basePath = GmodPath::fromShortPath("411.1/C101.31", gmod, locs);
                REQUIRE(basePath.has_value());

                auto query = GmodPathQueryBuilder::from(*basePath)
                                 .withoutLocations()
                                 .withAnyNodeAfter([](const auto& nodes) -> const GmodNode* {
                                     auto it = nodes.find("411.1");
                                     return it != nodes.end() ? it->second : nullptr;
                                 })
                                 .build();

                // paths starting with 411.1 - should match
                auto pathC10131 = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locs);
                auto pathS206 = GmodPath::fromShortPath("411.1/C101.63/S206", gmod, locs);
                auto pathLoc1 = GmodPath::fromShortPath("411.1-1/C101.61/S203", gmod, locs);
                REQUIRE(pathC10131.has_value());
                REQUIRE(pathS206.has_value());
                REQUIRE(pathLoc1.has_value());
                CHECK(query.match(*pathC10131));
                CHECK(query.match(*pathS206));
                CHECK(query.match(*pathLoc1));

                // paths not starting with 411.1 - should not match
                auto path511 = GmodPath::fromShortPath("511.11/C101.63/S206", gmod, locs);
                auto path652 = GmodPath::fromShortPath("652.31/S90.3/S61", gmod, locs);
                REQUIRE(path511.has_value());
                REQUIRE(path652.has_value());
                CHECK_FALSE(query.match(*path511));
                CHECK_FALSE(query.match(*path652));
            }

            TEST_CASE("mid-path - ignores suffix after selected node")
            {
                const auto& vis = VIS::instance();
                const auto& gmod = vis.gmod(VisVersion::v3_4a);
                const auto& locs = vis.locations(VisVersion::v3_4a);

                auto basePath = GmodPath::fromShortPath("411.1/C101.63/S206", gmod, locs);
                REQUIRE(basePath.has_value());

                auto query = GmodPathQueryBuilder::from(*basePath)
                                 .withoutLocations()
                                 .withAnyNodeAfter([](const auto& nodes) -> const GmodNode* {
                                     auto it = nodes.find("C101.6");
                                     return it != nodes.end() ? it->second : nullptr;
                                 })
                                 .build();

                auto pathS206 = GmodPath::fromShortPath("411.1/C101.63/S206", gmod, locs);
                auto pathLoc = GmodPath::fromShortPath("411.1-1/C101.61/S203", gmod, locs);
                REQUIRE(pathS206.has_value());
                REQUIRE(pathLoc.has_value());
                CHECK(query.match(*pathS206));
                CHECK(query.match(*pathLoc));

                auto pathC10131 = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locs);
                REQUIRE(pathC10131.has_value());
                CHECK_FALSE(query.match(*pathC10131));
            }

            TEST_CASE("throws and does not mutate filter when node not in path")
            {
                const auto& vis = VIS::instance();
                const auto& gmod = vis.gmod(VisVersion::v3_4a);
                const auto& locs = vis.locations(VisVersion::v3_4a);

                // path: 411.1/C101.63/S206. "C101.31" is NOT in this path
                auto basePath = GmodPath::fromShortPath("411.1/C101.63/S206", gmod, locs);
                REQUIRE(basePath.has_value());

                auto builder = GmodPathQueryBuilder::from(*basePath).withoutLocations();

                // must throw before mutating any filter entry
                CHECK_THROWS_AS(
                    (void)std::move(builder).withAnyNodeAfter([](const auto& nodes) -> const GmodNode* {
                        auto it = nodes.find("C101.31");
                        return it != nodes.end() ? it->second : nullptr;
                    }),
                    std::invalid_argument);

                // the original unmodified builder still matches the base path
                auto safeQuery = GmodPathQueryBuilder::from(*basePath).withoutLocations().build();
                CHECK(safeQuery.match(*basePath));
            }
        }

        TEST_CASE("optional path handling")
        {
            const auto& vis = VIS::instance();
            const auto& gmod = vis.gmod(VisVersion::v3_4a);
            const auto& locs = vis.locations(VisVersion::v3_4a);

            auto path = GmodPath::fromShortPath("411.1/C101", gmod, locs);
            REQUIRE(path.has_value());

            auto query = GmodPathQueryBuilder::from(*path).build();

            std::optional<GmodPath> emptyPath;
            bool emptyResult = emptyPath.has_value() ? query.match(*emptyPath) : false;
            CHECK_FALSE(emptyResult);
            bool validResult = path.has_value() ? query.match(*path) : false;
            CHECK(validResult);
        }
    }
} // namespace dnv::vista::sdk::tests
