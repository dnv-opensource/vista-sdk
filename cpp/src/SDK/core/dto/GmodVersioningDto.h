/**
 * @file GmodVersioningDto.h
 * @brief Internal DTOs for deserializing Gmod Versioning JSON resources
 */

#pragma once

#include "JSON/Json.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace dnv::vista::sdk
{
    struct GmodNodeConversionDto
    {
        using Operations = std::unordered_set<std::string>;

        Operations operations;                    ///< Set of operations (e.g. "changeCode", "merge")
        std::string source;                       ///< Source node code
        std::optional<std::string> target;        ///< Target node code, absent when no remapping
        std::optional<std::string> oldAssignment; ///< Assignment name before this version
        std::optional<std::string> newAssignment; ///< Assignment name after this version
        bool deleteAssignment = false;            ///< Whether the assignment is deleted in this version
    };

    struct GmodVersioningAssignmentChangeDto
    {
        std::string oldAssignment;     ///< Assignment name in the previous version
        std::string currentAssignment; ///< Assignment name in the current version
    };

    struct GmodVersioningDto
    {
        std::string visVersion;                                       ///< VIS version (JSON field "visRelease")
        std::unordered_map<std::string, GmodNodeConversionDto> items; ///< Map of node code to conversion info
    };

    inline GmodNodeConversionDto gmodNodeConversionDtoFromJson(const json::Document& doc)
    {
        GmodNodeConversionDto dto;

        if (!doc.contains("operations"))
        {
            throw std::runtime_error{ "GmodNodeConversionDto: missing required field 'operations'" };
        }
        const auto& opsDoc = doc["operations"];
        if (opsDoc.type() != json::Type::Array)
        {
            throw std::runtime_error{ "GmodNodeConversionDto: 'operations' must be an array" };
        }
        for (size_t i = 0; i < opsDoc.size(); ++i)
        {
            auto op = opsDoc.at(i).root<std::string>();
            if (!op)
            {
                throw std::runtime_error{ "GmodNodeConversionDto: operations element must be a string" };
            }
            dto.operations.insert(std::move(*op));
        }

        auto sourceOpt = doc["source"].root<std::string>();
        if (!sourceOpt)
        {
            throw std::runtime_error{ "GmodNodeConversionDto: missing required field 'source'" };
        }
        dto.source = std::move(*sourceOpt);

        if (auto val = doc["target"].root<std::string>())
        {
            dto.target = std::move(*val);
        }
        if (auto val = doc["oldAssignment"].root<std::string>())
        {
            dto.oldAssignment = std::move(*val);
        }
        if (auto val = doc["newAssignment"].root<std::string>())
        {
            dto.newAssignment = std::move(*val);
        }
        if (auto val = doc["deleteAssignment"].root<bool>())
        {
            dto.deleteAssignment = *val;
        }

        return dto;
    }

    inline GmodVersioningAssignmentChangeDto gmodVersioningAssignmentChangeDtoFromJson(const json::Document& doc)
    {
        GmodVersioningAssignmentChangeDto dto;

        auto oldOpt = doc["oldAssignment"].root<std::string>();
        if (!oldOpt)
        {
            throw std::runtime_error{ "GmodVersioningAssignmentChangeDto: missing required field 'oldAssignment'" };
        }
        dto.oldAssignment = std::move(*oldOpt);

        auto currentOpt = doc["currentAssignment"].root<std::string>();
        if (!currentOpt)
        {
            throw std::runtime_error{ "GmodVersioningAssignmentChangeDto: missing required field 'currentAssignment'" };
        }
        dto.currentAssignment = std::move(*currentOpt);

        return dto;
    }

    inline GmodVersioningDto gmodVersioningDtoFromJson(const json::Document& doc)
    {
        GmodVersioningDto dto;

        auto visReleaseOpt = doc["visRelease"].root<std::string>();
        if (!visReleaseOpt || visReleaseOpt->empty())
        {
            throw std::runtime_error{ "GmodVersioningDto: missing required field 'visRelease'" };
        }
        dto.visVersion = std::move(*visReleaseOpt);

        if (!doc.contains("items"))
        {
            throw std::runtime_error{ "GmodVersioningDto: missing required field 'items'" };
        }
        const auto& itemsDoc = doc["items"];
        if (itemsDoc.type() != json::Type::Object)
        {
            throw std::runtime_error{ "GmodVersioningDto: 'items' must be an object" };
        }

        for (auto it = itemsDoc.objectBegin(); it != itemsDoc.objectEnd(); ++it)
        {
            dto.items.emplace(it.key(), gmodNodeConversionDtoFromJson(it.value()));
        }

        return dto;
    }
} // namespace dnv::vista::sdk
