/**
 * @file CodebooksDto.h
 * @brief Internal DTOs for deserializing Codebooks JSON resources
 */

#pragma once

#include "JSON/Json.h"

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace dnv::vista::sdk
{
    struct CodebookDto
    {
        using Values = std::unordered_map<std::string, std::vector<std::string>>;

        std::string name; ///< Codebook name (e.g. "calculations", "commands")
        Values values;    ///< Map of group name to array of values
    };

    struct CodebooksDto
    {
        std::string visVersion;         ///< VIS version (JSON field "visRelease")
        std::vector<CodebookDto> items; ///< All codebooks
    };

    inline CodebookDto codebookDtoFromJson(const json::Document& doc)
    {
        CodebookDto dto;

        auto nameOpt = doc["name"].root<std::string>();
        if (!nameOpt)
        {
            throw std::runtime_error{ "CodebookDto: missing required field 'name'" };
        }
        dto.name = std::move(*nameOpt);

        if (!doc.contains("values"))
        {
            throw std::runtime_error{ "CodebookDto: missing required field 'values'" };
        }
        const auto& valuesDoc = doc["values"];
        if (valuesDoc.type() != json::Type::Object)
        {
            throw std::runtime_error{ "CodebookDto: 'values' must be an object" };
        }

        for (auto it = valuesDoc.objectBegin(); it != valuesDoc.objectEnd(); ++it)
        {
            const auto& groupDoc = it.value();
            if (groupDoc.type() != json::Type::Array)
            {
                throw std::runtime_error{ "CodebookDto: values group must be an array" };
            }

            std::vector<std::string> group;
            group.reserve(groupDoc.size());
            for (size_t i = 0; i < groupDoc.size(); ++i)
            {
                auto val = groupDoc.at(i).root<std::string>();
                if (!val)
                {
                    throw std::runtime_error{ "CodebookDto: values group element must be a string" };
                }
                group.push_back(std::move(*val));
            }

            dto.values.emplace(it.key(), std::move(group));
        }

        return dto;
    }

    inline CodebooksDto codebooksDtoFromJson(const json::Document& doc)
    {
        CodebooksDto dto;

        auto visReleaseOpt = doc["visRelease"].root<std::string>();
        if (!visReleaseOpt || visReleaseOpt->empty())
        {
            throw std::runtime_error{ "CodebooksDto: missing required field 'visRelease'" };
        }
        dto.visVersion = std::move(*visReleaseOpt);

        const auto& itemsDoc = doc["items"];
        if (itemsDoc.type() != json::Type::Array)
        {
            throw std::runtime_error{ "CodebooksDto: 'items' must be an array" };
        }

        dto.items.reserve(itemsDoc.size());
        for (size_t i = 0; i < itemsDoc.size(); ++i)
        {
            dto.items.push_back(codebookDtoFromJson(itemsDoc.at(i)));
        }

        return dto;
    }
} // namespace dnv::vista::sdk
