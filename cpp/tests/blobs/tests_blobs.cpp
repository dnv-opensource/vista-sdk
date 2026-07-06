#include <doctest/doctest.h>

#include <ResourcesRegistry.h>

#include <algorithm>
#include <string_view>

namespace dnv::vista::sdk::tests
{
    using namespace dnv::vista::sdk;
    using namespace dnv::vista::sdk::resources;

    bool has_blob(std::string_view name)
    {
        return find(name) != nullptr;
    }

    TEST_SUITE("Blobs")
    {
        TEST_CASE("each blob has non-empty name, data, and size")
        {
            for (const auto& blob : all())
            {
                CHECK_FALSE(blob.name.empty());
                CHECK_NE(blob.data, nullptr);
                CHECK_GT(blob.size, 0);
            }
        }

        TEST_CASE("find returns nullptr for unknown blob")
        {
            CHECK_EQ(find("nonexistent.json.gz"), nullptr);
        }

        TEST_CASE("find returns valid blob for known names")
        {
            auto* blob = find("gmod-vis-3-11a.json.gz");
            REQUIRE_NE(blob, nullptr);
            CHECK_EQ(blob->name, "gmod-vis-3-11a.json.gz");
            CHECK_NE(blob->data, nullptr);
            CHECK_GT(blob->size, 0);
        }

        TEST_CASE("embedded data starts with gzip magic bytes")
        {
            for (const auto& blob : all())
            {
                if (blob.size >= 2)
                {
                    CHECK_EQ(blob.data[0], 0x1F);
                    CHECK_EQ(blob.data[1], 0x8B);
                }
            }
        }

        TEST_CASE("blob names are unique")
        {
            const auto& blobs = all();
            for (size_t i = 0; i < blobs.size(); ++i)
            {
                for (size_t j = i + 1; j < blobs.size(); ++j)
                {
                    CHECK_NE(blobs[i].name, blobs[j].name);
                }
            }
        }
    }
} // namespace dnv::vista::sdk::tests
