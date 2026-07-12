#include <doctest/doctest.h>

#include <dnv/vista/sdk/core/LocalId.h>
#include <dnv/vista/sdk/core/ParsingErrors.h>
#include <dnv/vista/sdk/core/VIS.h>

#include <JSON/Parser.h>

#include <EmbeddedTestData.h>

#include <string>
#include <vector>

namespace dnv::vista::sdk::tests
{
    TEST_SUITE("GmodVersioning")
    {
        TEST_CASE("convert path")
        {
            struct TestData
            {
                std::string_view inputPath;
                std::string_view expectedPath;
                VisVersion sourceVersion;
                VisVersion targetVersion;
            };

            static constexpr TestData cases[] = {
                { "411.1/C101.72/I101", "411.1/C101.72/I101", VisVersion::v3_4a, VisVersion::v3_6a },
                { "323.51/H362.1", "323.61/H362.1", VisVersion::v3_4a, VisVersion::v3_6a },
                { "321.38/C906", "321.39/C906", VisVersion::v3_4a, VisVersion::v3_6a },
                { "511.331/C221", "511.31/C121.31/C221", VisVersion::v3_4a, VisVersion::v3_6a },
                { "511.11/C101.663i/C663.5/CS6d",
                  "511.11/C101.663i/C663.6/CS6d",
                  VisVersion::v3_4a,
                  VisVersion::v3_6a },
                { "511.11-1/C101.663i/C663.5/CS6d",
                  "511.11-1/C101.663i/C663.6/CS6d",
                  VisVersion::v3_4a,
                  VisVersion::v3_6a },
                { "1012.21/C1147.221/C1051.7/C101.22",
                  "1012.21/C1147.221/C1051.7/C101.93",
                  VisVersion::v3_4a,
                  VisVersion::v3_6a },
                { "1012.21/C1147.221/C1051.7/C101.61/S203.6",
                  "1012.21/C1147.221/C1051.7/C101.311/C467.5",
                  VisVersion::v3_4a,
                  VisVersion::v3_6a },
                { "001", "001", VisVersion::v3_4a, VisVersion::v3_6a },
                { "038.7/F101.2/F71", "038.7/F101.2/F71", VisVersion::v3_4a, VisVersion::v3_6a },
                { "1012.21/C1147.221/C1051.7/C101.61/S203.6/S61",
                  "1012.21/C1147.221/C1051.7/C101.311/C467.5/S61",
                  VisVersion::v3_4a,
                  VisVersion::v3_6a },
                { "000a", "000a", VisVersion::v3_4a, VisVersion::v3_6a },
                { "1012.21/C1147.221/C1051.7/C101.61/S203.2/S101",
                  "1012.21/C1147.221/C1051.7/C101.61/S203.3/S110.1/S101",
                  VisVersion::v3_4a,
                  VisVersion::v3_6a },
                { "1012.21/C1147.221/C1051.7/C101.661i/C624",
                  "1012.21/C1147.221/C1051.7/C101.661i/C621",
                  VisVersion::v3_4a,
                  VisVersion::v3_6a },
                // "1012.22/S201.1/C151.2/S110.2/C101.61/S203.2/S101" -> cycle, skipped
                { "1012.22/S201.1/C151.2/S110.2/C101.64i",
                  "1012.22/S201.1/C151.2/S110.2/C101.64",
                  VisVersion::v3_4a,
                  VisVersion::v3_6a },
                { "632.32i/S110.2/C111.42/G203.31/S90.5/C401",
                  "632.32i/S110.2/C111.42/G203.31/S90.5/C401",
                  VisVersion::v3_4a,
                  VisVersion::v3_6a },
                { "864.11/G71.21/C101.64i/S201.1/C151.31/S110.2/C111.42/G204.41/S90.2/S51",
                  "864.11/G71.21/C101.64/S201.1/C151.31/S110.2/C111.42/G204.41/S90.2/S51",
                  VisVersion::v3_4a,
                  VisVersion::v3_6a },
                { "864.11/G71.21/C101.64i/S201.1/C151.31/S110.2/C111.41/G240.1/G242.2/S90.5/C401",
                  "864.11/G71.21/C101.64/S201.1/C151.31/S110.2/C111.41/G240.1/G242.2/S90.5/C401",
                  VisVersion::v3_4a,
                  VisVersion::v3_6a },
                { "221.31/C1141.41/C664.2/C471", "221.31/C1141.41/C664.2/C471", VisVersion::v3_4a, VisVersion::v3_6a },
                // "354.2/C1096" -> was deleted, skipped
                { "514/E15", "514", VisVersion::v3_4a, VisVersion::v3_6a },
                { "1346/S201.1/C151.31/S110.2/C111.1/C109.16/C509",
                  "1346/S201.1/C151.31/S110.2/C111.1/C109.126/C509",
                  VisVersion::v3_7a,
                  VisVersion::v3_8a },
                // "851/H231" -> effect of SD, skipped
            };

            const auto& vis = VIS::instance();

            for (const auto& tc : cases)
            {
                CAPTURE(tc.inputPath);

                const auto& sourceGmod = vis.gmod(tc.sourceVersion);
                const auto& targetGmod = vis.gmod(tc.targetVersion);
                const auto& sourceLocations = vis.locations(tc.sourceVersion);
                const auto& targetLocations = vis.locations(tc.targetVersion);

                ParsingErrors sourceErrors;
                ParsingErrors targetErrors;

                auto sourcePath = GmodPath::fromShortPath(tc.inputPath, sourceGmod, sourceLocations, sourceErrors);
                REQUIRE(sourcePath.has_value());

                auto parsedTargetPath =
                    GmodPath::fromShortPath(tc.expectedPath, targetGmod, targetLocations, targetErrors);
                REQUIRE(parsedTargetPath.has_value());

                auto targetPath = vis.convertPath(tc.sourceVersion, *sourcePath, tc.targetVersion);

                REQUIRE(targetPath.has_value());
                CHECK(sourcePath->toString() == std::string{ tc.inputPath });
                CHECK(parsedTargetPath->toString() == std::string{ tc.expectedPath });
                CHECK(targetPath->toString() == std::string{ tc.expectedPath });
            }
        }

        TEST_CASE("convert full path")
        {
            const auto& vis = VIS::instance();

            const auto& sourceGmod = vis.gmod(VisVersion::v3_4a);
            const auto& targetGmod = vis.gmod(VisVersion::v3_6a);
            const auto& sourceLocations = vis.locations(VisVersion::v3_4a);
            const auto& targetLocations = vis.locations(VisVersion::v3_6a);

            constexpr std::string_view inputPath = "VE/600a/630/632/632.3/632.32/632.32i-2/S110";
            constexpr std::string_view expectedPath = "VE/600a/630/632/632.3/632.32/632.32i-2/SS5/S110";

            ParsingErrors sourceErrors;
            ParsingErrors targetErrors;

            auto sourcePath = GmodPath::fromFullPath(inputPath, sourceGmod, sourceLocations, sourceErrors);
            REQUIRE(sourcePath.has_value());

            auto parsedTargetPath = GmodPath::fromFullPath(expectedPath, targetGmod, targetLocations, targetErrors);
            REQUIRE(parsedTargetPath.has_value());

            auto targetPath = vis.convertPath(VisVersion::v3_4a, *sourcePath, VisVersion::v3_6a);

            REQUIRE(targetPath.has_value());
            CHECK(sourcePath->toFullPathString() == std::string{ inputPath });
            CHECK(parsedTargetPath->toFullPathString() == std::string{ expectedPath });
            CHECK(targetPath->toFullPathString() == std::string{ expectedPath });
        }

        TEST_CASE("conversion exceptions")
        {
            const auto& vis = VIS::instance();
            const auto& sourceGmod = vis.gmod(VisVersion::v3_7a);
            const auto& sourceLocations = vis.locations(VisVersion::v3_7a);

            ParsingErrors errors;
            auto sourcePath = GmodPath::fromShortPath("244.1i/H101.111/H401", sourceGmod, sourceLocations, errors);
            REQUIRE(sourcePath.has_value());

            CHECK_THROWS_AS(
                (void)vis.convertPath(VisVersion::v3_7a, *sourcePath, VisVersion::v3_8a), std::runtime_error);
        }

        TEST_CASE("finds path")
        {
            const auto& gmod = VIS::instance().gmod(VisVersion::v3_4a);

            bool completed =
                gmod.traverse([](const TraversalPath& parents, const GmodNode& node) -> TraversalHandlerResult {
                    if (parents.empty())
                    {
                        return TraversalHandlerResult::Continue;
                    }

                    std::vector<GmodNode> parentNodes;
                    parentNodes.reserve(parents.size());
                    for (const GmodNode* p : parents)
                    {
                        parentNodes.push_back(*p);
                    }

                    GmodPath path{ std::move(parentNodes), node };
                    if (path.toString() == "1012.22/S201.1/C151.2/S110.2/C101.61/S203.2/S101")
                    {
                        return TraversalHandlerResult::Stop;
                    }

                    return TraversalHandlerResult::Continue;
                });

            CHECK_FALSE(completed);
        }

        TEST_CASE("one path to root for asset functions")
        {
            const auto& vis = VIS::instance();

            auto onePathToRoot = [](const GmodNode& node, auto& self) -> bool {
                if (node.isRoot())
                {
                    return true;
                }
                const auto& parents = node.parents();
                return parents.size() == 1 && self(*parents[0], self);
            };

            for (const auto version : vis.versions())
            {
                const auto& gmod = vis.gmod(version);
                for (const auto& [code, node] : gmod)
                {
                    if (!node.isAssetFunctionNode())
                    {
                        continue;
                    }
                    CAPTURE(code);
                    CHECK(onePathToRoot(node, onePathToRoot));
                }
            }
        }

        TEST_CASE("convert node")
        {
            struct TestData
            {
                std::string_view inputCode;
                std::optional<std::string_view> location;
                std::string_view expectedCode;
            };

            static constexpr TestData cases[] = {
                { "1014.211", std::nullopt, "1014.211" }, { "323.5", std::nullopt, "323.6" },
                { "412.72", std::nullopt, "412.7i" },     { "323.4", std::nullopt, "323.5" },
                { "323.51", std::nullopt, "323.61" },     { "323.6", std::nullopt, "323.7" },
                { "C101.212", std::nullopt, "C101.22" },  { "C101.22", std::nullopt, "C101.93" },
                { "511.31", std::nullopt, "C121.1" },     { "C101.31", "5", "C101.31" }
            };

            const auto& vis = VIS::instance();
            const auto& sourceGmod = vis.gmod(VisVersion::v3_4a);
            const auto& targetGmod = vis.gmod(VisVersion::v3_6a);

            for (const auto& tc : cases)
            {
                CAPTURE(tc.inputCode);

                std::optional<GmodNode> sourceNode;
                std::optional<GmodNode> expectedNode;

                if (tc.location.has_value())
                {
                    // withLocation is private in C++ - location preservation is covered
                    // by "convert node preserves location" test case
                    continue;
                }
                else
                {
                    sourceNode = sourceGmod[tc.inputCode];
                    expectedNode = targetGmod[tc.expectedCode];
                }

                REQUIRE(sourceNode.has_value());
                REQUIRE(expectedNode.has_value());

                auto targetNode = vis.convertNode(VisVersion::v3_4a, *sourceNode, VisVersion::v3_6a);

                REQUIRE(targetNode.has_value());
                CHECK(targetNode->code() == expectedNode->code());
                CHECK(targetNode->location() == expectedNode->location());
            }
        }

        TEST_CASE("convert node preserves location")
        {
            const auto& vis = VIS::instance();
            const auto& sourceGmod = vis.gmod(VisVersion::v3_7a);
            const auto& sourceLocations = vis.locations(VisVersion::v3_7a);

            ParsingErrors errors;
            auto sourcePath = GmodPath::fromShortPath("691.811i-A", sourceGmod, sourceLocations, errors);
            REQUIRE(sourcePath.has_value());
            REQUIRE(sourcePath->node().location().has_value());

            auto convertedPath = vis.convertPath(VisVersion::v3_7a, *sourcePath, VisVersion::v3_9a);
            REQUIRE(convertedPath.has_value());
            REQUIRE(convertedPath->node().location().has_value());
            CHECK(convertedPath->node().location()->value() == sourcePath->node().location()->value());
        }

        TEST_CASE("convert local id")
        {
            struct TestData
            {
                std::string_view sourceLocalIdStr;
                std::string_view targetLocalIdStr;
            };

            static constexpr TestData cases[] = {
                {
                    "/dnv-v2/vis-3-4a/411.1/C101/sec/411.1/C101.64i/S201/meta/cnt-condensate",
                    "/dnv-v2/vis-3-5a/411.1/C101/sec/411.1/C101.64/S201/meta/cnt-condensate",
                },
                {
                    "/dnv-v2/vis-3-4a/411.1/C101.64i-1/S201.1/C151.2/S110/meta/cnt-hydraulic.oil/state-running",
                    "/dnv-v2/vis-3-9a/411.1/C101.64-1/S201.1/C151.2/S110/meta/cnt-hydraulic.oil/state-running",
                }
            };

            const auto& vis = VIS::instance();

            for (const auto& tc : cases)
            {
                CAPTURE(tc.sourceLocalIdStr);

                auto sourceLocalId = LocalId::fromString(tc.sourceLocalIdStr);
                REQUIRE(sourceLocalId.has_value());

                auto targetLocalId = LocalId::fromString(tc.targetLocalIdStr);
                REQUIRE(targetLocalId.has_value());

                auto convertedLocalId = vis.convertLocalId(*sourceLocalId, targetLocalId->builder().version().value());

                REQUIRE(convertedLocalId.has_value());
                CHECK(convertedLocalId->toString() == std::string{ tc.targetLocalIdStr });
            }
        }

        TEST_CASE("convert path with location")
        {
            const auto& vis = VIS::instance();
            const auto& sourceGmod = vis.gmod(VisVersion::v3_7a);
            const auto& targetGmod = vis.gmod(VisVersion::v3_9a);
            const auto& sourceLocations = vis.locations(VisVersion::v3_7a);
            const auto& targetLocations = vis.locations(VisVersion::v3_9a);

            ParsingErrors sourceErrors;
            ParsingErrors targetErrors;

            auto sourcePath =
                GmodPath::fromShortPath("691.811i-A/H101.11-1", sourceGmod, sourceLocations, sourceErrors);
            REQUIRE(sourcePath.has_value());

            auto expectedPath =
                GmodPath::fromShortPath("691.83111i-A/H101.11-1", targetGmod, targetLocations, targetErrors);
            REQUIRE(expectedPath.has_value());

            auto convertedPath = vis.convertPath(VisVersion::v3_7a, *sourcePath, VisVersion::v3_9a);
            REQUIRE(convertedPath.has_value());
            CHECK(convertedPath->toString() == expectedPath->toString());
        }

        TEST_CASE("convert all valid paths to latest")
        {
            const auto& vis = VIS::instance();

            struct GmodPathTestItem
            {
                std::string path;
                std::string visVersion;
            };

            auto raw = EmbeddedTestData::text("GmodPaths.json");
            REQUIRE_FALSE(raw.empty());

            auto docOpt = json::Document::fromString(raw);
            REQUIRE(docOpt.has_value());

            auto arrayRef = (*docOpt)["Valid"].rootRef<json::Array>();
            REQUIRE(arrayRef.has_value());
            const json::Array& array = arrayRef->get();

            std::vector<GmodPathTestItem> items;
            items.reserve(array.size());
            for (const auto& elem : array)
            {
                GmodPathTestItem item;
                item.path = *elem["path"].root<std::string>();
                item.visVersion = *elem["visVersion"].root<std::string>();
                items.push_back(std::move(item));
            }

            for (const auto& item : items)
            {
                CAPTURE(item.path);

                auto sourceVersionOpt = VisVersions::fromString(item.visVersion);
                REQUIRE(sourceVersionOpt.has_value());

                const auto& sourceGmod = vis.gmod(*sourceVersionOpt);
                const auto& sourceLocations = vis.locations(*sourceVersionOpt);

                ParsingErrors errors;
                auto sourcePath = GmodPath::fromShortPath(item.path, sourceGmod, sourceLocations, errors);
                REQUIRE(sourcePath.has_value());

                auto targetPath = vis.convertPath(*sourceVersionOpt, *sourcePath, vis.latest());
                CHECK(targetPath.has_value());
            }
        }
    }
} // namespace dnv::vista::sdk::tests
