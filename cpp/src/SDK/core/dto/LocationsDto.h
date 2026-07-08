/**
 * @file LocationsDto.h
 * @brief Internal DTOs for deserializing Locations JSON resources
 */

#pragma once

#include "JSON/Json.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace dnv::vista::sdk
{
    struct RelativeLocationDto
    {
        char code;                             ///< Single-character location code (e.g. 'N', 'C')
        std::string name;                      ///< Location name (e.g. "Number", "Centre")
        std::optional<std::string> definition; ///< Optional human-readable definition
    };

    struct LocationsDto
    {
        std::string visVersion;                 ///< VIS version (JSON field "visRelease")
        std::vector<RelativeLocationDto> items; ///< All relative locations
    };

    inline RelativeLocationDto relativeLocationDtoFromJson(const json::Document& doc)
    {
        RelativeLocationDto dto;

        auto codeOpt = doc["code"].root<std::string>();
        if (!codeOpt || codeOpt->size() != 1)
        {
            throw std::runtime_error{ "RelativeLocationDto: 'code' must be a single ASCII character" };
        }
        dto.code = (*codeOpt)[0];

        auto nameOpt = doc["name"].root<std::string>();
        if (!nameOpt)
        {
            throw std::runtime_error{ "RelativeLocationDto: missing required field 'name'" };
        }
        dto.name = std::move(*nameOpt);

        if (auto val = doc["definition"].root<std::string>())
        {
            dto.definition = std::move(*val);
        }

        return dto;
    }

    inline LocationsDto locationsDtoFromJson(const json::Document& doc)
    {
        LocationsDto dto;

        auto visReleaseOpt = doc["visRelease"].root<std::string>();
        if (!visReleaseOpt || visReleaseOpt->empty())
        {
            throw std::runtime_error{ "LocationsDto: missing required field 'visRelease'" };
        }
        dto.visVersion = std::move(*visReleaseOpt);

        if (!doc.contains("items"))
        {
            throw std::runtime_error{ "LocationsDto: missing required field 'items'" };
        }
        const auto& itemsDoc = doc["items"];
        if (itemsDoc.type() != json::Type::Array)
        {
            throw std::runtime_error{ "LocationsDto: missing required field 'items'" };
        }

        dto.items.reserve(itemsDoc.size());
        for (size_t i = 0; i < itemsDoc.size(); ++i)
        {
            dto.items.push_back(relativeLocationDtoFromJson(itemsDoc.at(i)));
        }

        return dto;
    }
} // namespace dnv::vista::sdk
