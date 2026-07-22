#include <doctest/doctest.h>

#include <dnv/VistaSDK.h>

#include <string>
#include <string_view>
#include <vector>

namespace dnv::vista::sdk::tests
{
    using Map = PerfectHashMap<GmodNode>;

    namespace
    {
        const Gmod& testGmod()
        {
            return VIS::instance().gmod(VIS::instance().latest());
        }

        std::vector<std::pair<std::string, GmodNode>> makeItems(std::initializer_list<const char*> codes)
        {
            const Gmod& gmod = testGmod();
            std::vector<std::pair<std::string, GmodNode>> items;
            items.reserve(codes.size());
            for (const char* code : codes)
            {
                items.emplace_back(code, gmod[code]);
            }
            return items;
        }

        std::vector<std::pair<std::string, GmodNode>> makeItemsFromGmod(size_t count)
        {
            const Gmod& gmod = testGmod();
            std::vector<std::pair<std::string, GmodNode>> items;
            items.reserve(count);
            size_t i = 0;
            for (const auto& [code, node] : gmod)
            {
                if (i >= count)
                {
                    break;
                }
                items.emplace_back(code, node);
                ++i;
            }
            return items;
        }
    } // namespace

    TEST_SUITE("PerfectHashMap")
    {
        TEST_CASE("empty construction")
        {
            Map m;
            CHECK(m.isEmpty());
            CHECK_EQ(m.count(), 0u);
            CHECK_EQ(m.size(), 0u);
        }

        TEST_CASE("single entry")
        {
            auto m = Map{ makeItems({ "VE" }) };
            CHECK_FALSE(m.isEmpty());
            CHECK_EQ(m.count(), 1u);
        }

        TEST_CASE("duplicate keys throw")
        {
            auto items = makeItems({ "VE" });
            items.emplace_back("VE", testGmod()["400a"]);
            CHECK_THROWS_AS(Map(std::move(items)), std::invalid_argument);
        }

        TEST_CASE("many entries - power-of-two boundary")
        {
            // 16 entries, exactly at a tableSize boundary
            auto items = makeItemsFromGmod(16);
            Map m{ std::move(items) };
            CHECK_EQ(m.count(), 16u);
        }

        TEST_CASE("find - existing key returns pointer")
        {
            auto m = Map{ makeItems({ "VE", "400a", "411.1" }) };

            const GmodNode* v = m.find("VE");
            REQUIRE(v != nullptr);
            CHECK_EQ(v->code(), "VE");

            v = m.find("400a");
            REQUIRE(v != nullptr);
            CHECK_EQ(v->code(), "400a");

            v = m.find("411.1");
            REQUIRE(v != nullptr);
            CHECK_EQ(v->code(), "411.1");
        }

        TEST_CASE("find - missing key returns nullptr")
        {
            auto m = Map{ makeItems({ "VE", "400a" }) };

            CHECK_EQ(m.find("missing"), nullptr);
            CHECK_EQ(m.find(""), nullptr);
            CHECK_EQ(m.find("VE2"), nullptr);
        }

        TEST_CASE("find - empty map returns nullptr")
        {
            Map m;
            CHECK_EQ(m.find("anything"), nullptr);
        }

        TEST_CASE("find - heterogeneous lookup string_view")
        {
            auto m = Map{ makeItems({ "VE", "411.1" }) };

            std::string_view sv = "VE";
            const GmodNode* v = m.find(sv);
            REQUIRE(v != nullptr);
            CHECK_EQ(v->code(), "VE");
        }

        TEST_CASE("find - heterogeneous lookup std::string")
        {
            auto m = Map{ makeItems({ "VE", "411.1" }) };

            std::string key = "411.1";
            const GmodNode* v = m.find(key);
            REQUIRE(v != nullptr);
            CHECK_EQ(v->code(), "411.1");
        }

        TEST_CASE("contains - existing key")
        {
            auto m = Map{ makeItems({ "VE", "400a", "411.1" }) };

            CHECK(m.contains("VE"));
            CHECK(m.contains("400a"));
            CHECK(m.contains("411.1"));
        }

        TEST_CASE("contains - missing key")
        {
            auto m = Map{ makeItems({ "VE", "400a" }) };

            CHECK_FALSE(m.contains("missing"));
            CHECK_FALSE(m.contains(""));
        }

        TEST_CASE("contains - empty map")
        {
            Map m;
            CHECK_FALSE(m.contains("anything"));
        }

        TEST_CASE("at - existing key")
        {
            auto m = Map{ makeItems({ "VE", "400a" }) };

            CHECK_EQ(m.at("VE").code(), "VE");
            CHECK_EQ(m.at("400a").code(), "400a");
        }

        TEST_CASE("at - missing key throws")
        {
            auto m = Map{ makeItems({ "VE" }) };

            CHECK_THROWS_AS((void)m.at("missing"), std::out_of_range);
        }

        TEST_CASE("all entries retrievable after construction")
        {
            auto items = makeItemsFromGmod(32);

            // keep a copy for verification before move
            std::vector<std::pair<std::string, GmodNode>> copy = items;

            Map m{ std::move(items) };
            CHECK_EQ(m.count(), copy.size());

            for (const auto& [k, v] : copy)
            {
                const GmodNode* found = m.find(k);
                REQUIRE_MESSAGE(found != nullptr, "key not found: ", k);
                CHECK_EQ(found->code(), v.code());
            }
        }

        TEST_CASE("no false positives - keys not inserted are not found")
        {
            auto m = Map{ makeItems({ "VE", "400a", "400", "410" }) };

            // similar-looking keys that were NOT inserted
            CHECK_EQ(m.find("V"), nullptr);
            CHECK_EQ(m.find("VEE"), nullptr);
            CHECK_EQ(m.find("4000"), nullptr);
            CHECK_EQ(m.find("4110"), nullptr);
            CHECK_EQ(m.find("4120"), nullptr);
            CHECK_EQ(m.find("401"), nullptr);
        }

        TEST_CASE("iteration visits all entries exactly once")
        {
            auto items = makeItems({ "VE", "400a", "400", "410", "411" });
            std::vector<std::pair<std::string, GmodNode>> copy = items;

            Map m{ std::move(items) };

            std::vector<std::string> visited;
            for (const auto& [k, v] : m)
            {
                visited.push_back(k);
            }

            CHECK_EQ(visited.size(), copy.size());

            for (const auto& [k, v] : copy)
            {
                int count = 0;
                for (const auto& vk : visited)
                {
                    if (vk == k)
                    {
                        ++count;
                    }
                }
                CHECK_EQ(count, 1);
            }
        }

        TEST_CASE("iteration on empty map is a no-op")
        {
            Map m;
            int count = 0;
            for ([[maybe_unused]] const auto& _ : m)
            {
                ++count;
            }
            CHECK_EQ(count, 0);
        }

        TEST_CASE("operator== - equal maps")
        {
            auto m1 = Map{ makeItems({ "VE", "400a" }) };
            auto m2 = Map{ makeItems({ "VE", "400a" }) };
            CHECK(m1 == m2);
        }

        TEST_CASE("operator== - different values")
        {
            auto m1 = Map{ makeItems({ "VE", "400a" }) };
            auto m2 = Map{ makeItems({ "VE", "410" }) };
            CHECK_FALSE(m1 == m2);
        }

        TEST_CASE("operator== - different keys")
        {
            auto m1 = Map{ makeItems({ "VE", "400a" }) };
            auto m2 = Map{ makeItems({ "VE", "410" }) };
            CHECK_FALSE(m1 == m2);
        }

        TEST_CASE("operator== - two empty maps")
        {
            Map m1, m2;
            CHECK(m1 == m2);
        }

        TEST_CASE("copy construction")
        {
            auto m1 = Map{ makeItems({ "VE", "400a" }) };
            Map m2 = m1;

            CHECK(m1 == m2);
            // original still works
            CHECK_EQ(m1.at("VE").code(), "VE");
        }

        TEST_CASE("move construction")
        {
            auto m1 = Map{ makeItems({ "VE", "400a" }) };
            Map m2 = std::move(m1);

            REQUIRE(m2.find("VE") != nullptr);
            CHECK_EQ(m2.at("VE").code(), "VE");
        }

        TEST_CASE("copy assignment")
        {
            auto m1 = Map{ makeItems({ "VE", "400a" }) };
            Map m2 = Map{ makeItems({ "410" }) };
            m2 = m1;

            CHECK_EQ(m2.count(), 2u);
            REQUIRE(m2.find("VE") != nullptr);
            CHECK_EQ(m2.at("VE").code(), "VE");
            CHECK_EQ(m2.find("410"), nullptr);
            // original unchanged
            CHECK_EQ(m1.at("VE").code(), "VE");
        }

        TEST_CASE("move assignment")
        {
            auto m1 = Map{ makeItems({ "VE", "400a" }) };
            Map m2 = Map{ makeItems({ "410" }) };
            m2 = std::move(m1);

            CHECK_EQ(m2.count(), 2u);
            REQUIRE(m2.find("VE") != nullptr);
            CHECK_EQ(m2.at("VE").code(), "VE");
            CHECK_EQ(m2.find("410"), nullptr);
        }
    }
} // namespace dnv::vista::sdk::tests
