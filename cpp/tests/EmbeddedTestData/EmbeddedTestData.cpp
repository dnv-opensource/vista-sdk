#include "EmbeddedTestData.h"

#include <TestdataRegistry.h>

namespace dnv::vista::sdk
{
    std::string EmbeddedTestData::text(std::string_view filename)
    {
        const auto* resource = testdata::find(filename);

        if (!resource || resource->size == 0)
        {
            return {};
        }

        return std::string{ resource->str() };
    }

    std::vector<std::string> EmbeddedTestData::listFiles()
    {
        const auto& resources = testdata::all();
        std::vector<std::string> filenames;
        filenames.reserve(resources.size());

        for (const auto& r : resources)
        {
            filenames.emplace_back(r.name);
        }

        return filenames;
    }
} // namespace dnv::vista::sdk
