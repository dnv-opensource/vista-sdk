#include <doctest/doctest.h>

#include <dnv/vista/sdk/core/VIS.h>

#include <limits>
#include <string>
#include <string_view>
#include <unordered_set>

namespace dnv::vista::sdk::tests
{
    namespace
    {
        struct ExpectedGmodValues
        {
            std::string_view maxCode;
            std::size_t nodeCount;
        };

        const std::unordered_map<VisVersion, ExpectedGmodValues> expectedMaxes = {
            { VisVersion::v3_4a, { "C1053.3114", 6420 } },  { VisVersion::v3_5a, { "C1053.3114", 6557 } },
            { VisVersion::v3_6a, { "C1053.3114", 6557 } },  { VisVersion::v3_7a, { "H346.11113", 6672 } },
            { VisVersion::v3_8a, { "H346.11113", 6335 } },  { VisVersion::v3_9a, { "H346.11113", 6553 } },
            { VisVersion::v3_10a, { "H346.11113", 6555 } }, { VisVersion::v3_11a, { "H346.11113", 6593 } }
        };
    } // namespace

    TEST_SUITE("Gmod")
    {
        TEST_CASE("loads for all versions")
        {
            const auto& vis = VIS::instance();

            for (const auto& version : vis.versions())
            {
                INFO("version: ", VisVersions::toString(version));
                const auto& gmod = vis.gmod(version);
                CHECK(gmod.node("400a").has_value());
            }
        }

        TEST_CASE("properties per version")
        {
            const auto& vis = VIS::instance();

            for (const auto& version : vis.versions())
            {
                INFO("version: ", VisVersions::toString(version));

                const auto& gmod = vis.gmod(version);

                const GmodNode* minNode = nullptr;
                const GmodNode* maxNode = nullptr;
                std::size_t currentMinLength = std::numeric_limits<std::size_t>::max();
                std::size_t currentMaxLength = 0;
                std::size_t nodeCount = 0;

                for (const auto& [code, node] : gmod)
                {
                    ++nodeCount;
                    const std::size_t len = code.length();

                    if (minNode == nullptr || len < currentMinLength)
                    {
                        currentMinLength = len;
                        minNode = &node;
                    }
                    else if (len == currentMinLength && code < minNode->code())
                    {
                        minNode = &node;
                    }

                    if (maxNode == nullptr || len > currentMaxLength)
                    {
                        currentMaxLength = len;
                        maxNode = &node;
                    }
                    else if (len == currentMaxLength && code > maxNode->code())
                    {
                        maxNode = &node;
                    }
                }

                REQUIRE(minNode != nullptr);
                REQUIRE(maxNode != nullptr);

                CHECK_EQ(minNode->code().length(), 2u);
                CHECK_EQ(minNode->code(), "VE");

                auto it = expectedMaxes.find(version);
                REQUIRE(it != expectedMaxes.end());

                CHECK_EQ(maxNode->code().length(), 10u);
                CHECK_EQ(maxNode->code(), it->second.maxCode);
                CHECK_EQ(nodeCount, it->second.nodeCount);
            }
        }

        TEST_CASE("lookup")
        {
            const auto& gmod = VIS::instance().gmod(VisVersion::v3_4a);

            std::unordered_set<std::string> seen;
            std::size_t counter = 0;

            for (const auto& [code, node] : gmod)
            {
                INFO("code: ", code);
                CHECK(seen.insert(code).second);

                auto foundOpt = gmod.node(code);
                REQUIRE(foundOpt.has_value());
                CHECK_EQ((*foundOpt)->code(), code);
                CHECK_EQ(*foundOpt, &node);
                ++counter;
            }

            CHECK_GT(counter, 0u);

            CHECK_FALSE(gmod.node("ABC").has_value());
            CHECK_FALSE(gmod.node("").has_value());
            CHECK_FALSE(gmod.node("SDFASDFSDAFb").has_value());
            CHECK_FALSE(gmod.node("✅").has_value());
            CHECK_FALSE(gmod.node("a✅b").has_value());
            CHECK_FALSE(gmod.node("ac✅bc").has_value());
            CHECK_FALSE(gmod.node("✅bc").has_value());
            CHECK_FALSE(gmod.node("a✅").has_value());
            CHECK_FALSE(gmod.node("ag✅").has_value());
        }

        TEST_CASE("node equality")
        {
            const auto& gmod = VIS::instance().gmod(VisVersion::v3_4a);

            const auto& node1 = gmod["400a"];
            const auto& node2 = gmod["400a"];

            CHECK_EQ(&node1, &node2);

            const auto& node3 = gmod["411"];
            CHECK_NE(&node1, &node3);
            CHECK_NE(node1.code(), node3.code());
        }

        TEST_CASE("node types are non-empty")
        {
            const auto& gmod = VIS::instance().gmod(VisVersion::v3_4a);

            std::unordered_set<std::string> typeSet;
            for (const auto& [code, node] : gmod)
            {
                typeSet.insert(
                    std::string{ node.metadata().category() } + " | " + std::string{ node.metadata().type() });
            }

            CHECK_FALSE(typeSet.empty());
        }

        TEST_CASE("root node has children")
        {
            const auto& gmod = VIS::instance().gmod(VisVersion::v3_4a);
            CHECK_FALSE(gmod.rootNode().children().empty());
        }

        TEST_CASE("product type assignment")
        {
            const auto& gmod = VIS::instance().gmod(VisVersion::v3_4a);

            const auto& node1 = gmod["411.3"];
            CHECK(node1.productType().has_value());
            CHECK_FALSE(node1.productSelection().has_value());

            const auto& node2 = gmod["H601"];
            CHECK_FALSE(node2.productType().has_value());
        }

        TEST_CASE("product selection assignment")
        {
            const auto& gmod = VIS::instance().gmod(VisVersion::v3_4a);

            auto node1Opt = gmod.node("411.2");
            REQUIRE(node1Opt.has_value());
            CHECK((*node1Opt)->productSelection().has_value());
            CHECK_FALSE((*node1Opt)->productType().has_value());

            auto node2Opt = gmod.node("H601");
            REQUIRE(node2Opt.has_value());
            CHECK_FALSE((*node2Opt)->productSelection().has_value());
        }

        TEST_CASE("isProductSelection")
        {
            const auto& gmod = VIS::instance().gmod(VisVersion::v3_4a);

            auto nodeOpt = gmod.node("CS1");
            REQUIRE(nodeOpt.has_value());
            CHECK((*nodeOpt)->isProductSelection());
        }

        TEST_CASE("mappability")
        {
            const auto& gmod = VIS::instance().gmod(VisVersion::v3_4a);

            CHECK_FALSE(gmod["VE"].isMappable());
            CHECK_FALSE(gmod["300a"].isMappable());
            CHECK(gmod["300"].isMappable());
            CHECK(gmod["411"].isMappable());
            CHECK(gmod["410"].isMappable());
            CHECK_FALSE(gmod["651.21s"].isMappable());
            CHECK(gmod["924.2"].isMappable());
            CHECK_FALSE(gmod["411.1"].isMappable());
            CHECK(gmod["C101"].isMappable());
            CHECK_FALSE(gmod["CS1"].isMappable());
            CHECK(gmod["C101.663"].isMappable());
            CHECK(gmod["C101.4"].isMappable());
            CHECK_FALSE(gmod["C101.21s"].isMappable());
            CHECK(gmod["F201.11"].isMappable());
            CHECK_FALSE(gmod["C101.211"].isMappable());
        }
    }
} // namespace dnv::vista::sdk::tests
