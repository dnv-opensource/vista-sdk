#include <doctest/doctest.h>

#include <dnv/VistaSDK.h>

#include <JSON/Parser.h>

#include <EmbeddedTestData.h>

#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace dnv::vista::sdk::tests
{
    namespace
    {
        struct GmodPathTestItem
        {
            std::string path;
            std::string visVersion;
        };

        struct IndividualizableSetTestItem
        {
            bool isFullPath;
            std::string visVersion;
            std::string path;
            std::optional<std::vector<std::vector<std::string>>> expected;
        };

        std::vector<GmodPathTestItem> loadValidGmodPaths()
        {
            auto raw = EmbeddedTestData::text("GmodPaths.json");
            REQUIRE_FALSE(raw.empty());

            auto docOpt = json::Document::fromString(raw);
            REQUIRE(docOpt.has_value());

            auto arrayRef = (*docOpt)["Valid"].rootRef<json::Array>();
            REQUIRE(arrayRef.has_value());
            const json::Array& array = arrayRef->get();

            std::vector<GmodPathTestItem> result;
            result.reserve(array.size());

            for (const auto& elem : array)
            {
                GmodPathTestItem item;
                item.path = *elem["path"].root<std::string>();
                item.visVersion = *elem["visVersion"].root<std::string>();
                result.push_back(std::move(item));
            }

            return result;
        }

        std::vector<GmodPathTestItem> loadInvalidGmodPaths()
        {
            auto raw = EmbeddedTestData::text("GmodPaths.json");
            REQUIRE_FALSE(raw.empty());

            auto docOpt = json::Document::fromString(raw);
            REQUIRE(docOpt.has_value());

            auto arrayRef = (*docOpt)["Invalid"].rootRef<json::Array>();
            REQUIRE(arrayRef.has_value());
            const json::Array& array = arrayRef->get();

            std::vector<GmodPathTestItem> result;
            result.reserve(array.size());

            for (const auto& elem : array)
            {
                GmodPathTestItem item;
                item.path = *elem["path"].root<std::string>();
                item.visVersion = *elem["visVersion"].root<std::string>();
                result.push_back(std::move(item));
            }

            return result;
        }

        std::vector<IndividualizableSetTestItem> loadIndividualizableSetsData()
        {
            auto raw = EmbeddedTestData::text("IndividualizableSets.json");
            REQUIRE_FALSE(raw.empty());

            auto docOpt = json::Document::fromString(raw);
            REQUIRE(docOpt.has_value());

            auto arrayRef = docOpt->rootRef<json::Array>();
            REQUIRE(arrayRef.has_value());
            const json::Array& array = arrayRef->get();

            std::vector<IndividualizableSetTestItem> result;
            result.reserve(array.size());

            for (const auto& elem : array)
            {
                IndividualizableSetTestItem item;
                item.isFullPath = *elem["isFullPath"].root<bool>();
                item.visVersion = *elem["visVersion"].root<std::string>();
                item.path = *elem["path"].root<std::string>();

                if (elem["expected"].type() != json::Type::Null)
                {
                    const json::Array& expectedArray = elem["expected"].rootRef<json::Array>()->get();
                    std::vector<std::vector<std::string>> expected;
                    expected.reserve(expectedArray.size());

                    for (const auto& setElem : expectedArray)
                    {
                        const json::Array& setArray = setElem.rootRef<json::Array>()->get();
                        std::vector<std::string> set;
                        set.reserve(setArray.size());

                        for (const auto& strElem : setArray)
                        {
                            set.push_back(*strElem.root<std::string>());
                        }

                        expected.push_back(std::move(set));
                    }

                    item.expected = std::move(expected);
                }

                result.push_back(std::move(item));
            }

            return result;
        }
    } // namespace

    TEST_SUITE("GmodPath")
    {
        TEST_CASE("parse - valid paths")
        {
            for (const auto& item : loadValidGmodPaths())
            {
                INFO("path: ", item.path, " version: ", item.visVersion);

                auto visVersion = VisVersions::fromString(item.visVersion);
                REQUIRE(visVersion.has_value());

                const auto& gmod = VIS::instance().gmod(*visVersion);
                const auto& locations = VIS::instance().locations(*visVersion);

                auto pathOpt = GmodPath::fromShortPath(item.path, gmod, locations);

                REQUIRE(pathOpt.has_value());
                CHECK_EQ(item.path, pathOpt->toString());
            }
        }

        TEST_CASE("parse - invalid paths")
        {
            for (const auto& item : loadInvalidGmodPaths())
            {
                INFO("path: ", item.path, " version: ", item.visVersion);

                auto visVersion = VisVersions::fromString(item.visVersion);
                REQUIRE(visVersion.has_value());

                const auto& gmod = VIS::instance().gmod(*visVersion);
                const auto& locations = VIS::instance().locations(*visVersion);

                auto pathOpt = GmodPath::fromShortPath(item.path, gmod, locations);

                CHECK_FALSE(pathOpt.has_value());
            }
        }

        TEST_CASE("fullPath iteration")
        {
            const auto& gmod = VIS::instance().gmod(VisVersion::v3_4a);
            const auto& locations = VIS::instance().locations(VisVersion::v3_4a);

            const std::vector<std::pair<size_t, std::string_view>> expectation = {
                { 0, "VE" },  { 1, "400a" }, { 2, "410" },    { 3, "411" },     { 4, "411i" }, { 5, "411.1" },
                { 6, "CS1" }, { 7, "C101" }, { 8, "C101.7" }, { 9, "C101.72" }, { 10, "I101" }
            };

            auto pathOpt = GmodPath::fromShortPath("411.1/C101.72/I101", gmod, locations);
            REQUIRE(pathOpt.has_value());

            std::unordered_set<size_t> seen;
            for (const auto& [depth, node] : pathOpt->fullPath())
            {
                INFO("depth: ", depth);
                CHECK(seen.insert(depth).second);

                if (seen.size() == 1)
                {
                    CHECK_EQ(0u, depth);
                }

                CHECK_EQ(expectation[depth].second, node.code());
            }

            CHECK_EQ(expectation.size(), seen.size());
        }

        TEST_CASE("fullPathFrom iteration")
        {
            const auto& gmod = VIS::instance().gmod(VisVersion::v3_4a);
            const auto& locations = VIS::instance().locations(VisVersion::v3_4a);

            const std::vector<std::pair<size_t, std::string_view>> expectation = { { 4, "411i" },   { 5, "411.1" },
                                                                                   { 6, "CS1" },    { 7, "C101" },
                                                                                   { 8, "C101.7" }, { 9, "C101.72" },
                                                                                   { 10, "I101" } };

            auto pathOpt = GmodPath::fromShortPath("411.1/C101.72/I101", gmod, locations);
            REQUIRE(pathOpt.has_value());

            std::unordered_set<size_t> seen;
            for (const auto& [depth, node] : pathOpt->fullPathFrom(4))
            {
                INFO("depth: ", depth);
                CHECK(seen.insert(depth).second);

                if (seen.size() == 1)
                {
                    CHECK_EQ(4u, depth);
                }

                bool found = false;
                for (const auto& [expDepth, expCode] : expectation)
                {
                    if (expDepth == depth)
                    {
                        CHECK_EQ(expCode, node.code());
                        found = true;
                        break;
                    }
                }
                CHECK(found);
            }

            CHECK_EQ(expectation.size(), seen.size());
        }

        TEST_CASE("full path parsing")
        {
            const auto& gmod = VIS::instance().gmod(VisVersion::v3_4a);
            const auto& locations = VIS::instance().locations(VisVersion::v3_4a);

            struct TestCase
            {
                std::string_view shortPath;
                std::string_view expectedFullPath;
            };

            const TestCase cases[] = {
                { "411.1/C101.72/I101", "VE/400a/410/411/411i/411.1/CS1/C101/C101.7/C101.72/I101" },
                { "612.21-1/C701.13/S93", "VE/600a/610/612/612.2/612.2i-1/612.21-1/CS10/C701/C701.1/C701.13/S93" }
            };

            for (const auto& tc : cases)
            {
                INFO("short path: ", tc.shortPath);

                auto pathOpt = GmodPath::fromShortPath(tc.shortPath, gmod, locations);
                REQUIRE(pathOpt.has_value());

                CHECK_EQ(tc.expectedFullPath, pathOpt->toFullPathString());

                auto fullPathOpt = GmodPath::fromFullPath(pathOpt->toFullPathString(), gmod, locations);
                REQUIRE(fullPathOpt.has_value());

                CHECK_EQ(pathOpt->toString(), fullPathOpt->toString());
                CHECK_EQ(pathOpt->toFullPathString(), fullPathOpt->toFullPathString());
            }
        }

        TEST_CASE("toFullPathString")
        {
            const auto& gmod = VIS::instance().gmod(VisVersion::v3_7a);
            const auto& locations = VIS::instance().locations(VisVersion::v3_7a);

            {
                auto pathOpt = GmodPath::fromShortPath("511.11-1/C101.663i-1/C663", gmod, locations);
                REQUIRE(pathOpt.has_value());
                CHECK_EQ(
                    "VE/500a/510/511/511.1/511.1i-1/511.11-1/CS1/C101/C101.6/C101.66/C101.663/C101.663i-1/C663",
                    pathOpt->toFullPathString());
            }
            {
                auto pathOpt = GmodPath::fromShortPath("846/G203.32-2/S110.2-1/E31", gmod, locations);
                REQUIRE(pathOpt.has_value());
                CHECK_EQ("VE/800a/840/846/G203/G203.3-2/G203.32-2/S110/S110.2-1/CS1/E31", pathOpt->toFullPathString());
            }
        }

        TEST_CASE("does not individualize")
        {
            const auto& gmod = VIS::instance().gmod(VisVersion::v3_7a);
            const auto& locations = VIS::instance().locations(VisVersion::v3_7a);

            auto pathOpt = GmodPath::fromShortPath("500a-1", gmod, locations);
            CHECK_FALSE(pathOpt.has_value());
        }

        TEST_CASE("individualizable sets - short paths")
        {
            for (const auto& item : loadIndividualizableSetsData())
            {
                INFO("path: ", item.path, " version: ", item.visVersion);

                auto visVersion = VisVersions::fromString(item.visVersion);
                REQUIRE(visVersion.has_value());

                const auto& gmod = VIS::instance().gmod(*visVersion);
                const auto& locations = VIS::instance().locations(*visVersion);

                if (!item.expected.has_value())
                {
                    auto pathOpt = item.isFullPath ? GmodPath::fromFullPath(item.path, gmod, locations)
                                                   : GmodPath::fromShortPath(item.path, gmod, locations);
                    CHECK_FALSE(pathOpt.has_value());
                    continue;
                }

                auto pathOpt = item.isFullPath ? GmodPath::fromFullPath(item.path, gmod, locations)
                                               : GmodPath::fromShortPath(item.path, gmod, locations);
                REQUIRE(pathOpt.has_value());

                auto sets = pathOpt->individualizableSets();
                const auto& expected = *item.expected;

                REQUIRE_EQ(expected.size(), sets.size());

                for (size_t i = 0; i < expected.size(); ++i)
                {
                    auto nodes = sets[i].nodes();
                    REQUIRE_EQ(expected[i].size(), nodes.size());

                    for (size_t j = 0; j < expected[i].size(); ++j)
                    {
                        CHECK_EQ(expected[i][j], nodes[j].code());
                    }
                }
            }
        }

        TEST_CASE("individualizable sets - full paths")
        {
            for (const auto& item : loadIndividualizableSetsData())
            {
                if (item.isFullPath)
                {
                    continue;
                }

                INFO("path: ", item.path, " version: ", item.visVersion);

                auto visVersion = VisVersions::fromString(item.visVersion);
                REQUIRE(visVersion.has_value());

                const auto& gmod = VIS::instance().gmod(*visVersion);
                const auto& locations = VIS::instance().locations(*visVersion);

                if (!item.expected.has_value())
                {
                    auto pathOpt = GmodPath::fromShortPath(item.path, gmod, locations);
                    CHECK_FALSE(pathOpt.has_value());
                    continue;
                }

                auto shortPathOpt = GmodPath::fromShortPath(item.path, gmod, locations);
                REQUIRE(shortPathOpt.has_value());

                auto pathOpt = GmodPath::fromFullPath(shortPathOpt->toFullPathString(), gmod, locations);
                REQUIRE(pathOpt.has_value());

                auto sets = pathOpt->individualizableSets();
                const auto& expected = *item.expected;

                REQUIRE_EQ(expected.size(), sets.size());

                for (size_t i = 0; i < expected.size(); ++i)
                {
                    auto nodes = sets[i].nodes();
                    REQUIRE_EQ(expected[i].size(), nodes.size());

                    for (size_t j = 0; j < expected[i].size(); ++j)
                    {
                        CHECK_EQ(expected[i][j], nodes[j].code());
                    }
                }
            }
        }

        TEST_CASE("valid paths - individualizable sets unique codes")
        {
            for (const auto& item : loadValidGmodPaths())
            {
                INFO("path: ", item.path, " version: ", item.visVersion);

                auto visVersion = VisVersions::fromString(item.visVersion);
                REQUIRE(visVersion.has_value());

                const auto& gmod = VIS::instance().gmod(*visVersion);
                const auto& locations = VIS::instance().locations(*visVersion);

                auto pathOpt = GmodPath::fromShortPath(item.path, gmod, locations);
                REQUIRE(pathOpt.has_value());

                auto sets = pathOpt->individualizableSets();

                std::unordered_set<std::string> uniqueCodes;
                for (const auto& set : sets)
                {
                    for (const auto& node : set.nodes())
                    {
                        CHECK(uniqueCodes.insert(std::string{ node.code() }).second);
                    }
                }
            }
        }

        TEST_CASE("valid paths - individualizable sets unique codes (full path)")
        {
            for (const auto& item : loadValidGmodPaths())
            {
                INFO("path: ", item.path, " version: ", item.visVersion);

                auto visVersion = VisVersions::fromString(item.visVersion);
                REQUIRE(visVersion.has_value());

                const auto& gmod = VIS::instance().gmod(*visVersion);
                const auto& locations = VIS::instance().locations(*visVersion);

                auto shortPathOpt = GmodPath::fromShortPath(item.path, gmod, locations);
                REQUIRE(shortPathOpt.has_value());

                auto pathOpt = GmodPath::fromFullPath(shortPathOpt->toFullPathString(), gmod, locations);
                REQUIRE(pathOpt.has_value());

                auto sets = pathOpt->individualizableSets();

                std::unordered_set<std::string> uniqueCodes;
                for (const auto& set : sets)
                {
                    for (const auto& node : set.nodes())
                    {
                        CHECK(uniqueCodes.insert(std::string{ node.code() }).second);
                    }
                }
            }
        }
    }
} // namespace dnv::vista::sdk::tests
