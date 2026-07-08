/**
 * @file GmodDto.h
 * @brief Internal DTOs for deserializing Gmod (Generic product model) JSON resources
 */

#pragma once

#include "JSON/Json.h"

#include <array>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace dnv::vista::sdk
{
    struct GmodNodeDto
    {
        using NormalAssignmentNames = std::unordered_map<std::string, std::string>;

        std::string category;                                       ///< Node category (e.g. "PRODUCT")
        std::string type;                                           ///< Node type (e.g. "SELECTION")
        std::string code;                                           ///< Node code identifier
        std::optional<std::string> name;                            ///< Specific name, absent on most nodes
        std::optional<std::string> commonName;                      ///< Common/generic name
        std::optional<std::string> definition;                      ///< Definition text
        std::optional<std::string> commonDefinition;                ///< Common definition text
        std::optional<bool> installSubstructure;                    ///< Whether to install substructure
        std::optional<NormalAssignmentNames> normalAssignmentNames; ///< Map of position tag to assignment name
    };

    struct GmodDto
    {
        using Relations = std::vector<std::array<std::string, 2>>;

        std::string visVersion;         ///< VIS version (JSON field "visRelease")
        std::vector<GmodNodeDto> items; ///< All Gmod nodes
        Relations relations;            ///< DAG edges as [parent, child] pairs
    };

    inline GmodNodeDto gmodNodeDtoFromJson(const json::Document& doc)
    {
        GmodNodeDto dto;

        auto categoryOpt = doc["category"].root<std::string>();
        if (!categoryOpt)
        {
            throw std::runtime_error{ "GmodNodeDto: missing required field 'category'" };
        }
        dto.category = std::move(*categoryOpt);

        auto typeOpt = doc["type"].root<std::string>();
        if (!typeOpt)
        {
            throw std::runtime_error{ "GmodNodeDto: missing required field 'type'" };
        }
        dto.type = std::move(*typeOpt);

        auto codeOpt = doc["code"].root<std::string>();
        if (!codeOpt)
        {
            throw std::runtime_error{ "GmodNodeDto: missing required field 'code'" };
        }
        dto.code = std::move(*codeOpt);

        if (auto val = doc["name"].root<std::string>())
        {
            dto.name = std::move(*val);
        }
        if (auto val = doc["commonName"].root<std::string>())
        {
            dto.commonName = std::move(*val);
        }
        if (auto val = doc["definition"].root<std::string>())
        {
            dto.definition = std::move(*val);
        }
        if (auto val = doc["commonDefinition"].root<std::string>())
        {
            dto.commonDefinition = std::move(*val);
        }
        if (auto val = doc["installSubstructure"].root<bool>())
        {
            dto.installSubstructure = *val;
        }

        if (doc.contains("normalAssignmentNames"))
        {
            const auto& nanDoc = doc["normalAssignmentNames"];
            GmodNodeDto::NormalAssignmentNames nan;
            for (auto it = nanDoc.objectBegin(); it != nanDoc.objectEnd(); ++it)
            {
                auto val = it.value().root<std::string>();
                if (!val)
                {
                    throw std::runtime_error{ "GmodNodeDto: normalAssignmentNames value must be a string" };
                }
                nan.emplace(it.key(), std::move(*val));
            }
            dto.normalAssignmentNames = std::move(nan);
        }

        return dto;
    }

    inline GmodDto gmodDtoFromJson(const json::Document& doc)
    {
        GmodDto dto;

        auto visReleaseOpt = doc["visRelease"].root<std::string>();
        if (!visReleaseOpt || visReleaseOpt->empty())
        {
            throw std::runtime_error{ "GmodDto: missing required field 'visRelease'" };
        }
        dto.visVersion = std::move(*visReleaseOpt);

        if (!doc.contains("items"))
        {
            throw std::runtime_error{ "GmodDto: missing required field 'items'" };
        }
        const auto& itemsDoc = doc["items"];
        if (itemsDoc.type() != json::Type::Array)
        {
            throw std::runtime_error{ "GmodDto: 'items' must be an array" };
        }

        dto.items.reserve(itemsDoc.size());
        for (size_t i = 0; i < itemsDoc.size(); ++i)
        {
            dto.items.push_back(gmodNodeDtoFromJson(itemsDoc.at(i)));
        }

        if (!doc.contains("relations"))
        {
            throw std::runtime_error{ "GmodDto: missing required field 'relations'" };
        }
        const auto& relationsDoc = doc["relations"];
        if (relationsDoc.type() != json::Type::Array)
        {
            throw std::runtime_error{ "GmodDto: 'relations' must be an array" };
        }

        dto.relations.reserve(relationsDoc.size());
        for (size_t i = 0; i < relationsDoc.size(); ++i)
        {
            const auto& pairDoc = relationsDoc.at(i);
            if (pairDoc.type() != json::Type::Array || pairDoc.size() != 2)
            {
                throw std::runtime_error{ "GmodDto: each relation must be a 2-element array" };
            }
            auto from = pairDoc.at(0).root<std::string>();
            auto to = pairDoc.at(1).root<std::string>();
            if (!from || !to)
            {
                throw std::runtime_error{ "GmodDto: relation elements must be strings" };
            }
            dto.relations.push_back({ std::move(*from), std::move(*to) });
        }

        return dto;
    }
} // namespace dnv::vista::sdk
