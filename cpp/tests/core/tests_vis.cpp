#include <doctest/doctest.h>

#include <dnv/vista/sdk/core/VIS.h>

#include <algorithm>
#include <thread>
#include <vector>

namespace dnv::vista::sdk::tests
{
    TEST_SUITE("VIS singleton")
    {
        TEST_CASE("instance returns same object")
        {
            const auto& instance1 = VIS::instance();
            const auto& instance2 = VIS::instance();

            CHECK_EQ(&instance1, &instance2);
        }

        TEST_CASE("versions returns all versions")
        {
            const auto& versions = VIS::instance().versions();

            CHECK_FALSE(versions.empty());
            CHECK_EQ(versions.back(), VIS::instance().latest());
        }

        TEST_CASE("versions returns span over the same underlying data")
        {
            const auto& vis = VIS::instance();

            const auto versions1 = vis.versions();
            const auto versions2 = vis.versions();

            CHECK_EQ(versions1.data(), versions2.data());
        }

        TEST_CASE("versions are ordered")
        {
            const auto& versions = VIS::instance().versions();

            auto it34 = std::find(versions.begin(), versions.end(), VisVersion::v3_4a);
            auto it311 = std::find(versions.begin(), versions.end(), VisVersion::v3_11a);

            REQUIRE_NE(it34, versions.end());
            REQUIRE_NE(it311, versions.end());

            CHECK_LT(std::distance(versions.begin(), it34), std::distance(versions.begin(), it311));
        }

        TEST_CASE("latest returns latest version")
        {
            const auto& versions = VIS::instance().versions();

            REQUIRE_FALSE(versions.empty());
            CHECK_EQ(VIS::instance().latest(), versions.back());
        }

        TEST_CASE("concurrent singleton access is thread-safe")
        {
            constexpr int numThreads = 10;
            std::vector<std::thread> threads;
            std::vector<const VIS*> instances(numThreads, nullptr);

            const auto expectedCount = VIS::instance().versions().size();
            const auto expectedLatest = VIS::instance().latest();

            for (int i = 0; i < numThreads; ++i)
            {
                threads.emplace_back([&instances, i, expectedCount, expectedLatest]() {
                    instances[i] = &VIS::instance();

                    const auto& versions = instances[i]->versions();
                    CHECK_EQ(versions.size(), expectedCount);
                    CHECK_EQ(instances[i]->latest(), expectedLatest);
                });
            }

            for (auto& thread : threads)
            {
                thread.join();
            }

            for (int i = 1; i < numThreads; ++i)
            {
                CHECK_EQ(instances[0], instances[i]);
            }
        }
    }
} // namespace dnv::vista::sdk::tests
