#include <doctest/doctest.h>

#include <dnv/VistaSDK.h>

namespace dnv::vista::sdk::tests
{
    TEST_SUITE("Codebooks")
    {
        TEST_CASE("loads all versions")
        {
            const auto& vis = VIS::instance();

            for (auto version : vis.versions())
            {
                const Codebooks* codebooks = nullptr;
                REQUIRE_NOTHROW(codebooks = &vis.codebooks(version));
                REQUIRE_NE(codebooks, nullptr);

                const Codebook* position = nullptr;
                REQUIRE_NOTHROW(position = &(*codebooks)[CodebookName::Position]);
                REQUIRE_NE(position, nullptr);
            }
        }

        TEST_CASE("hasStandardValue")
        {
            const auto& codebooks = VIS::instance().codebooks(VisVersion::v3_4a);

            CHECK(codebooks[CodebookName::Position].hasStandardValue("centre"));
        }
    }
} // namespace dnv::vista::sdk::tests
