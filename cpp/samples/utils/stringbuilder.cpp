/**
 * @file stringbuilder.cpp
 * @brief Demonstrates StringBuilder usage for efficient string construction
 * @details Shows how to use StringBuilder to build strings without repeated
 *          heap allocations: accumulation, chaining, stack/heap transition,
 *          and reuse across multiple toString() calls.
 */

#include <dnv/vista/sdk/core/LocalId.h>
#include <dnv/vista/sdk/core/VIS.h>
#include <dnv/vista/sdk/utils/StringBuilder.h>

#include <iostream>
#include <string>
#include <vector>

int main()
{
    using namespace dnv::vista::sdk;

    std::cout << "=== vista-sdk StringBuilder sample ===\n\n";

    {
        std::cout << "1. Basic accumulation\n";
        std::cout << "---------------------\n";

        StringBuilder sb;

        sb += "VE/400a/410/411/411i/411.1";
        sb += '/';
        sb += "C101/C101.3/C101.31";

        std::cout << "Path  : " << sb.view() << "\n";
        std::cout << "Length: " << sb.view().size() << "\n";
        std::cout << "Empty : " << std::boolalpha << sb.isEmpty() << "\n";

        std::cout << "\n";
    }

    {
        std::cout << "2. Chained append\n";
        std::cout << "------------------\n";

        StringBuilder sb;
        sb.append("/dnv-v2/vis-3-4a/")
            .append("411.1/C101.31-2")
            .append("/meta/")
            .append("qty-temperature")
            .append("/cnt-exhaust.gas");

        std::cout << "LocalId: " << sb.view() << "\n";

        std::cout << "\n";
    }

    {
        std::cout << "3. Numeric append (no intermediate allocation)\n";
        std::cout << "-----------------------------------------------\n";

        StringBuilder sb;
        sb.append("temperature=")
            .append(87.3)
            .append(" degC, samples=")
            .append(static_cast<std::int64_t>(1024))
            .append(", ratio=")
            .append(0.9375);

        std::cout << sb.view() << "\n";

        std::cout << "\n";
    }

    {
        std::cout << "4. clear() and reuse across a loop\n";
        std::cout << "------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto version = vis.latest();
        const auto& gmod = vis.gmod(version);
        const auto& locations = vis.locations(version);

        std::vector<std::string> shortPaths = { "411.1/C101.31", "411.1/C101.63", "621.21/S90" };

        StringBuilder sb;

        for (const auto& shortPath : shortPaths)
        {
            auto path = GmodPath::fromShortPath(shortPath, gmod, locations);
            if (!path.has_value())
            {
                continue;
            }

            sb.clear();
            path->toFullPathString(sb);
            std::cout << shortPath << " -> " << sb.view() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "5. Stack vs heap transition\n";
        std::cout << "---------------------------\n";

        StringBuilder sb;
        std::cout << "Empty builder: on stack\n";

        for (int i = 0; i < 10; ++i)
        {
            sb.append("segment-").append(static_cast<std::int64_t>(i)).append('/');
        }

        std::cout << "After 10 segments (" << sb.view().size() << " bytes):\n";
        std::cout << sb.view() << "\n";

        sb.clear(true);
        std::cout << "After clear(true): back on stack\n";

        std::cout << "\n";
    }

    {
        std::cout << "6. toString() and view()\n";
        std::cout << "------------------------\n";

        StringBuilder sb;
        sb.append("IMO").append(static_cast<std::int64_t>(8027781));

        std::string_view v = sb.view();
        std::string s = sb.toString();

        std::cout << "view()    : " << v << " (zero-copy, tied to builder lifetime)\n";
        std::cout << "toString(): " << s << " (owned copy)\n";
        std::cout << "operator<<: " << sb << " (streams the builder directly, no view()/toString() needed)\n";

        std::cout << "\n";
    }

    {
        std::cout << "7. SDK types write directly into StringBuilder\n";
        std::cout << "-----------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto version = vis.latest();
        const auto& gmod = vis.gmod(version);
        const auto& codebooks = vis.codebooks(version);
        const auto& locations = vis.locations(version);

        auto path = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations);
        auto quantityTag = codebooks[CodebookName::Quantity].createTag("temperature");
        auto contentTag = codebooks[CodebookName::Content].createTag("exhaust.gas");

        if (path.has_value())
        {
            auto localId = LocalIdBuilder::create(version)
                               .withPrimaryItem(*path)
                               .withMetadataTag(quantityTag)
                               .withMetadataTag(contentTag)
                               .build();

            StringBuilder sb;
            localId.toString(sb);
            std::cout << "LocalId (via StringBuilder) : " << sb.view() << "\n";

            sb.clear();
            path->toString(sb);
            std::cout << "GmodPath (via StringBuilder): " << sb.view() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "8. reserve() and pop_back()\n";
        std::cout << "----------------------------\n";

        StringBuilder sb{ 128 };
        std::cout << "Capacity after construction with initial size: " << sb.capacity() << " bytes\n";

        sb.append("qty-temperature").append('/').append("cnt-exhaust.gas").append('/');
        std::cout << "Before pop_back(): " << sb.view() << "\n";

        if (!sb.isEmpty() && sb.back() == '/')
        {
            sb.pop_back();
        }
        std::cout << "After pop_back() : " << sb.view() << "\n";

        std::cout << "\n";
    }

    return 0;
}
