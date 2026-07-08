#include "EmbeddedResources.h"

#include "Compression/Gzip.h"

#include "SDK/core/dto/CodebooksDto.h"
#include "SDK/core/dto/GmodDto.h"
#include "SDK/core/dto/GmodVersioningDto.h"
#include "SDK/core/dto/ISO19848Dtos.h"
#include "SDK/core/dto/LocationsDto.h"

#include <ResourcesRegistry.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace dnv::vista::sdk
{
    namespace
    {
        std::vector<uint8_t> decompressedStream(std::string_view resourceName)
        {
            auto resource = resources::find(resourceName);
            if (!resource)
            {
                throw std::runtime_error{ "Resource not found: " + std::string{ resourceName } };
            }

            return compression::gunzip({ resource->data, resource->size });
        }

        json::Document parseResource(std::string_view resourceName)
        {
            auto decompressed = decompressedStream(resourceName);
            std::string_view jsonStr{ reinterpret_cast<const char*>(decompressed.data()), decompressed.size() };

            auto doc = json::Document::fromString(jsonStr);
            if (!doc)
            {
                throw std::runtime_error{ "Failed to parse JSON for resource: " + std::string{ resourceName } };
            }

            return std::move(*doc);
        }

        template <typename Predicate, typename FromJsonFn>
        auto loadResourceIf(Predicate predicate, FromJsonFn fromJson)
            -> std::optional<decltype(fromJson(std::declval<const json::Document&>()))>
        {
            const auto& res = resources::all();

            auto it = std::ranges::find_if(res, predicate);
            if (it == res.end())
            {
                return std::nullopt;
            }

            return fromJson(parseResource(it->name));
        }

        template <typename FromJsonFn>
        auto loadResource(std::string_view resourceKey, std::string_view version, FromJsonFn fromJson)
        {
            return loadResourceIf(
                [resourceKey, version](const auto& r) {
                    const bool matchesKey = r.name.find(resourceKey) != std::string_view::npos;
                    const bool isGzipped = r.name.ends_with(".gz");
                    const bool matchesVersion = version.empty() || r.name.find(version) != std::string_view::npos;
                    return matchesKey && isGzipped && matchesVersion;
                },
                fromJson);
        }
    } // namespace

    std::vector<std::string> EmbeddedResources::visVersions()
    {
        const auto& res = resources::all();
        std::vector<std::string> versions;

        for (const auto& r : res)
        {
            if (r.name.ends_with(".gz") && r.name.find("gmod") != std::string_view::npos &&
                r.name.find("versioning") == std::string_view::npos)
            {
                constexpr std::string_view prefix = "gmod-vis-";
                constexpr std::string_view suffix = ".json.gz";

                auto prefixPos = r.name.find(prefix);
                if (prefixPos != std::string_view::npos)
                {
                    auto versionStart = prefixPos + prefix.size();
                    auto suffixPos = r.name.find(suffix, versionStart);
                    if (suffixPos != std::string_view::npos)
                    {
                        versions.emplace_back(r.name.substr(versionStart, suffixPos - versionStart));
                    }
                }
            }
        }

        if (versions.empty())
        {
            throw std::runtime_error{ "Did not find required resources (expected gmod-vis-*.json.gz resources)" };
        }

        return versions;
    }

    std::vector<std::string> EmbeddedResources::iso19848Versions()
    {
        const auto& res = resources::all();
        std::vector<std::string> versions;

        for (const auto& r : res)
        {
            if (r.name.ends_with(".gz") && r.name.find("iso19848") != std::string_view::npos &&
                r.name.find("data-channel-type-names") != std::string_view::npos)
            {
                constexpr std::string_view prefix = "iso19848-";
                constexpr std::string_view suffix = "-data-channel-type-names.json.gz";

                auto prefixPos = r.name.find(prefix);
                if (prefixPos != std::string_view::npos)
                {
                    auto versionStart = prefixPos + prefix.size();
                    auto suffixPos = r.name.find(suffix, versionStart);
                    if (suffixPos != std::string_view::npos)
                    {
                        versions.emplace_back(r.name.substr(versionStart, suffixPos - versionStart));
                    }
                }
            }
        }

        if (versions.empty())
        {
            throw std::runtime_error{
                "Did not find required resources (expected iso19848-*-data-channel-type-names.json.gz resources)"
            };
        }

        return versions;
    }

    std::optional<GmodDto> EmbeddedResources::gmod(std::string_view visVersion)
    {
        return loadResourceIf(
            [visVersion](const auto& r) {
                return r.name.ends_with(".gz") && r.name.find("gmod") != std::string_view::npos &&
                       r.name.find("versioning") == std::string_view::npos &&
                       r.name.find(visVersion) != std::string_view::npos;
            },
            [](const json::Document& doc) { return gmodDtoFromJson(doc); });
    }

    std::optional<CodebooksDto> EmbeddedResources::codebooks(std::string_view visVersion)
    {
        return loadResource(
            "codebooks", visVersion, [](const json::Document& doc) { return codebooksDtoFromJson(doc); });
    }

    std::optional<LocationsDto> EmbeddedResources::locations(std::string_view visVersion)
    {
        return loadResource(
            "locations", visVersion, [](const json::Document& doc) { return locationsDtoFromJson(doc); });
    }

    std::optional<std::unordered_map<std::string, GmodVersioningDto>> EmbeddedResources::gmodVersioning()
    {
        const auto& res = resources::all();
        std::unordered_map<std::string, GmodVersioningDto> dtos;

        for (const auto& r : res)
        {
            if (r.name.find("gmod-vis-versioning") != std::string_view::npos && r.name.ends_with(".gz"))
            {
                auto dto = gmodVersioningDtoFromJson(parseResource(r.name));
                dtos[dto.visVersion] = std::move(dto);
            }
        }

        if (dtos.empty())
        {
            return std::nullopt;
        }

        return dtos;
    }

    std::optional<DataChannelTypeNamesDto> EmbeddedResources::dataChannelTypeNames(std::string_view version)
    {
        return loadResource("data-channel-type-names", version, [](const json::Document& doc) {
            return dataChannelTypeNamesDtoFromJson(doc);
        });
    }

    std::optional<FormatDataTypesDto> EmbeddedResources::formatDataTypes(std::string_view version)
    {
        return loadResource(
            "format-data-types", version, [](const json::Document& doc) { return formatDataTypesDtoFromJson(doc); });
    }
} // namespace dnv::vista::sdk
