/**
 * @file ISO19848Dtos.h
 * @brief Internal DTOs for deserializing ISO 19848 JSON resources
 */

#pragma once

#include "JSON/Json.h"

#include <stdexcept>
#include <string>
#include <vector>

namespace dnv::vista::sdk
{
    struct DataChannelTypeNameDto
    {
        std::string type;        ///< Data channel type identifier (e.g. "Inst")
        std::string description; ///< Human-readable description
    };

    struct DataChannelTypeNamesDto
    {
        std::vector<DataChannelTypeNameDto> values; ///< All data channel type names
    };

    struct FormatDataTypeDto
    {
        std::string type;        ///< Format data type identifier (e.g. "Boolean")
        std::string description; ///< Human-readable description
    };

    struct FormatDataTypesDto
    {
        std::vector<FormatDataTypeDto> values; ///< All format data types
    };

    inline DataChannelTypeNameDto dataChannelTypeNameDtoFromJson(const json::Document& doc)
    {
        DataChannelTypeNameDto dto;

        auto typeOpt = doc["type"].root<std::string>();
        if (!typeOpt)
        {
            throw std::runtime_error{ "DataChannelTypeNameDto: missing required field 'type'" };
        }
        dto.type = std::move(*typeOpt);

        auto descOpt = doc["description"].root<std::string>();
        if (!descOpt)
        {
            throw std::runtime_error{ "DataChannelTypeNameDto: missing required field 'description'" };
        }
        dto.description = std::move(*descOpt);

        return dto;
    }

    inline DataChannelTypeNamesDto dataChannelTypeNamesDtoFromJson(const json::Document& doc)
    {
        DataChannelTypeNamesDto dto;

        if (!doc.contains("values"))
        {
            throw std::runtime_error{ "DataChannelTypeNamesDto: missing required field 'values'" };
        }
        const auto& valuesDoc = doc["values"];
        if (valuesDoc.type() != json::Type::Array)
        {
            throw std::runtime_error{ "DataChannelTypeNamesDto: 'values' must be an array" };
        }

        dto.values.reserve(valuesDoc.size());
        for (size_t i = 0; i < valuesDoc.size(); ++i)
        {
            dto.values.push_back(dataChannelTypeNameDtoFromJson(valuesDoc.at(i)));
        }

        return dto;
    }

    inline FormatDataTypeDto formatDataTypeDtoFromJson(const json::Document& doc)
    {
        FormatDataTypeDto dto;

        auto typeOpt = doc["type"].root<std::string>();
        if (!typeOpt)
        {
            throw std::runtime_error{ "FormatDataTypeDto: missing required field 'type'" };
        }
        dto.type = std::move(*typeOpt);

        auto descOpt = doc["description"].root<std::string>();
        if (!descOpt)
        {
            throw std::runtime_error{ "FormatDataTypeDto: missing required field 'description'" };
        }
        dto.description = std::move(*descOpt);

        return dto;
    }

    inline FormatDataTypesDto formatDataTypesDtoFromJson(const json::Document& doc)
    {
        FormatDataTypesDto dto;

        if (!doc.contains("values"))
        {
            throw std::runtime_error{ "FormatDataTypesDto: missing required field 'values'" };
        }
        const auto& valuesDoc = doc["values"];
        if (valuesDoc.type() != json::Type::Array)
        {
            throw std::runtime_error{ "FormatDataTypesDto: 'values' must be an array" };
        }

        dto.values.reserve(valuesDoc.size());
        for (size_t i = 0; i < valuesDoc.size(); ++i)
        {
            dto.values.push_back(formatDataTypeDtoFromJson(valuesDoc.at(i)));
        }

        return dto;
    }
} // namespace dnv::vista::sdk
