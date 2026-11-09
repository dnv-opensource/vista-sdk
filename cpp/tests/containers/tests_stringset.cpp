#include <doctest/doctest.h>

#include <dnv/VistaSDK.h>

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace dnv::vista::sdk::tests
{
    TEST_SUITE("StringSet")
    {
        TEST_CASE("empty construction")
        {
            StringSet s;
            CHECK(s.isEmpty());
            CHECK_EQ(s.size(), 0u);
        }

        TEST_CASE("initializer_list construction")
        {
            StringSet s{ "alpha", "beta", "gamma" };
            CHECK_FALSE(s.isEmpty());
            CHECK_EQ(s.size(), 3u);
        }

        TEST_CASE("insert - returns true on new key")
        {
            StringSet s;
            CHECK(s.insert("hello"));
            CHECK_EQ(s.size(), 1u);
        }

        TEST_CASE("insert - returns false on duplicate key")
        {
            StringSet s;
            CHECK(s.insert("hello"));
            CHECK_FALSE(s.insert("hello"));
            CHECK_EQ(s.size(), 1u);
        }

        TEST_CASE("insert - string_view")
        {
            StringSet s;
            std::string_view sv = "world";
            CHECK(s.insert(sv));
            CHECK_EQ(s.size(), 1u);
        }

        TEST_CASE("insert - const char*")
        {
            StringSet s;
            CHECK(s.insert("raw"));
            CHECK_EQ(s.size(), 1u);
        }

        TEST_CASE("insert - rvalue std::string avoids copy")
        {
            StringSet s;
            std::string key = "moved";
            CHECK(s.insert(std::move(key)));
            CHECK_EQ(s.size(), 1u);
            CHECK(s.contains("moved"));
        }

        TEST_CASE("insert - rvalue std::string duplicate returns false")
        {
            StringSet s;
            s.insert(std::string{ "hello" });
            CHECK_FALSE(s.insert(std::string{ "hello" }));
            CHECK_EQ(s.size(), 1u);
        }

        TEST_CASE("contains - existing key")
        {
            StringSet s{ "VE", "400", "411.1" };
            CHECK(s.contains("VE"));
            CHECK(s.contains("400"));
            CHECK(s.contains("411.1"));
        }

        TEST_CASE("contains - missing key")
        {
            StringSet s{ "VE", "400" };
            CHECK_FALSE(s.contains("missing"));
            CHECK_FALSE(s.contains(""));
            CHECK_FALSE(s.contains("VEE"));
        }

        TEST_CASE("contains - empty set")
        {
            StringSet s;
            CHECK_FALSE(s.contains("anything"));
        }

        TEST_CASE("contains - heterogeneous string_view")
        {
            StringSet s{ "VE", "411.1" };
            std::string_view sv = "VE";
            CHECK(s.contains(sv));
            std::string_view miss = "missing";
            CHECK_FALSE(s.contains(miss));
        }

        TEST_CASE("contains - heterogeneous std::string")
        {
            StringSet s{ "411.1", "400" };
            std::string key = "411.1";
            CHECK(s.contains(key));
            std::string miss = "nope";
            CHECK_FALSE(s.contains(miss));
        }

        TEST_CASE("find - existing key returns pointer")
        {
            StringSet s{ "VE", "400", "411.1" };

            const std::string* p = s.find("VE");
            REQUIRE(p != nullptr);
            CHECK_EQ(*p, "VE");

            p = s.find("411.1");
            REQUIRE(p != nullptr);
            CHECK_EQ(*p, "411.1");
        }

        TEST_CASE("find - missing key returns nullptr")
        {
            StringSet s{ "VE", "400" };
            CHECK_EQ(s.find("missing"), nullptr);
            CHECK_EQ(s.find(""), nullptr);
            CHECK_EQ(s.find("VEX"), nullptr);
        }

        TEST_CASE("find - empty set returns nullptr")
        {
            StringSet s;
            CHECK_EQ(s.find("anything"), nullptr);
        }

        TEST_CASE("find - heterogeneous string_view")
        {
            StringSet s{ "VE", "411.1" };
            std::string_view sv = "VE";
            const std::string* p = s.find(sv);
            REQUIRE(p != nullptr);
            CHECK_EQ(*p, "VE");
        }

        TEST_CASE("find - heterogeneous std::string")
        {
            StringSet s{ "411.1", "400" };
            std::string key = "400";
            const std::string* p = s.find(key);
            REQUIRE(p != nullptr);
            CHECK_EQ(*p, "400");
        }

        TEST_CASE("no false positives - similar-looking keys not inserted")
        {
            StringSet s{ "VE", "400", "411", "412" };
            CHECK_EQ(s.find("V"), nullptr);
            CHECK_EQ(s.find("VEE"), nullptr);
            CHECK_EQ(s.find("4000"), nullptr);
            CHECK_EQ(s.find("4110"), nullptr);
            CHECK_EQ(s.find("4120"), nullptr);
            CHECK_EQ(s.find("401"), nullptr);
            CHECK_EQ(s.find("410"), nullptr);
        }

        TEST_CASE("erase - removes an existing key and reports true")
        {
            StringSet s{ "alpha", "beta", "gamma" };
            CHECK(s.erase("beta"));
            CHECK_EQ(s.size(), 2u);
            CHECK_FALSE(s.contains("beta"));
            CHECK(s.contains("alpha"));
            CHECK(s.contains("gamma"));
        }

        TEST_CASE("erase - missing key reports false and leaves set unchanged")
        {
            StringSet s{ "alpha", "beta" };
            CHECK_FALSE(s.erase("not_there"));
            CHECK_EQ(s.size(), 2u);
            CHECK(s.contains("alpha"));
            CHECK(s.contains("beta"));
        }

        TEST_CASE("erase - empty set returns false")
        {
            StringSet s;
            CHECK_FALSE(s.erase("anything"));
            CHECK_EQ(s.size(), 0u);
        }

        TEST_CASE("erase - heterogeneous key types")
        {
            StringSet s{ "alpha", "beta", "gamma", "delta" };

            std::string_view sv = "alpha";
            CHECK(s.erase(sv));
            CHECK_FALSE(s.contains("alpha"));

            std::string owned = "beta";
            CHECK(s.erase(owned));
            CHECK_FALSE(s.contains("beta"));

            CHECK(s.erase("gamma"));
            CHECK_FALSE(s.contains("gamma"));

            std::string rval = "delta";
            CHECK(s.erase(std::move(rval)));
            CHECK(s.isEmpty());
        }

        TEST_CASE("erase - then re-insert same key succeeds")
        {
            StringSet s{ "alpha", "beta" };
            CHECK(s.erase("alpha"));
            CHECK(s.insert("alpha"));
            CHECK_EQ(s.size(), 2u);
            CHECK(s.contains("alpha"));
        }

        TEST_CASE("erase - backward-shift preserves lookup of entries displaced past the removed slot")
        {
            StringSet s;
            s.reserve(64);

            std::vector<std::string> keys;
            for (int i = 0; i < 20; ++i)
            {
                keys.push_back("collide_key_" + std::to_string(i));
            }
            for (const auto& k : keys)
            {
                CHECK(s.insert(k));
            }

            CHECK(s.erase(keys.front()));

            for (size_t i = 1; i < keys.size(); ++i)
            {
                CAPTURE(keys[i]);
                CHECK(s.contains(keys[i]));
            }
            CHECK_FALSE(s.contains(keys.front()));
            CHECK_EQ(s.size(), keys.size() - 1);
        }

        TEST_CASE("erase - all entries one by one leaves an empty, iterable set")
        {
            StringSet s{ "alpha", "beta", "gamma", "delta" };

            CHECK(s.erase("alpha"));
            CHECK(s.erase("beta"));
            CHECK(s.erase("gamma"));
            CHECK(s.erase("delta"));

            CHECK(s.isEmpty());
            CHECK_EQ(s.size(), 0u);
            CHECK(s.begin() == s.end());
        }

        TEST_CASE("erase - iteration after erase visits only remaining entries")
        {
            StringSet s{ "one", "two", "three", "four", "five" };
            CHECK(s.erase("three"));

            std::vector<std::string> seen;
            for (const auto& key : s)
            {
                seen.push_back(key);
            }

            CHECK_EQ(seen.size(), 4u);
            CHECK(std::find(seen.begin(), seen.end(), "three") == seen.end());
            for (const auto& expected : { "one", "two", "four", "five" })
            {
                CHECK(std::find(seen.begin(), seen.end(), expected) != seen.end());
            }
        }

        TEST_CASE("reserve does not lose entries")
        {
            StringSet s;
            s.insert("alpha");
            s.insert("beta");
            s.reserve(256);
            CHECK_EQ(s.size(), 2u);
            CHECK(s.contains("alpha"));
            CHECK(s.contains("beta"));
        }

        TEST_CASE("resize on growth - all entries retrievable")
        {
            // Force multiple resize cycles by inserting past 75% load
            std::vector<std::string> keys;
            for (int i = 0; i < 64; ++i)
            {
                keys.push_back("key_" + std::to_string(i));
            }

            StringSet s;
            for (const auto& k : keys)
            {
                s.insert(k);
            }

            CHECK_EQ(s.size(), 64u);

            for (const auto& k : keys)
            {
                REQUIRE_MESSAGE(s.contains(k), "key not found: ", k);
            }
        }

        TEST_CASE("iteration visits all entries exactly once")
        {
            std::vector<std::string> keys{ "a", "b", "c", "d", "e" };

            StringSet s;
            for (const auto& k : keys)
            {
                s.insert(k);
            }

            std::vector<std::string> visited;
            for (const auto& k : s)
            {
                visited.push_back(k);
            }

            CHECK_EQ(visited.size(), keys.size());

            for (const auto& k : keys)
            {
                int count = 0;
                for (const auto& v : visited)
                {
                    if (v == k)
                    {
                        ++count;
                    }
                }
                CHECK_EQ(count, 1);
            }
        }

        TEST_CASE("iteration on empty set is a no-op")
        {
            StringSet s;
            int count = 0;
            for ([[maybe_unused]] const auto& _ : s)
            {
                ++count;
            }
            CHECK_EQ(count, 0);
        }

        TEST_CASE("copy construction")
        {
            StringSet s1{ "x", "y", "z" };
            StringSet s2 = s1;

            CHECK_EQ(s1.size(), s2.size());
            CHECK(s2.contains("x"));
            CHECK(s2.contains("y"));
            CHECK(s2.contains("z"));
            // original still intact
            CHECK(s1.contains("x"));
        }

        TEST_CASE("move construction")
        {
            StringSet s1{ "x", "y" };
            StringSet s2 = std::move(s1);

            CHECK(s2.contains("x"));
            CHECK(s2.contains("y"));
        }

        TEST_CASE("copy assignment")
        {
            StringSet s1{ "x", "y", "z" };
            StringSet s2{ "old" };
            s2 = s1;

            CHECK_EQ(s2.size(), 3u);
            CHECK(s2.contains("x"));
            CHECK(s2.contains("y"));
            CHECK(s2.contains("z"));
            CHECK_FALSE(s2.contains("old"));
            // original unchanged
            CHECK(s1.contains("x"));
        }

        TEST_CASE("move assignment")
        {
            StringSet s1{ "x", "y", "z" };
            StringSet s2{ "old" };
            s2 = std::move(s1);

            CHECK_EQ(s2.size(), 3u);
            CHECK(s2.contains("x"));
            CHECK_FALSE(s2.contains("old"));
        }

        TEST_CASE("many entries - power-of-two boundary")
        {
            // 16 entries, exactly at initial capacity boundary
            StringSet s;
            for (int i = 1; i <= 16; ++i)
            {
                s.insert("entry_" + std::to_string(i));
            }
            CHECK_EQ(s.size(), 16u);
            for (int i = 1; i <= 16; ++i)
            {
                CHECK(s.contains("entry_" + std::to_string(i)));
            }
        }
    }
} // namespace dnv::vista::sdk::tests
