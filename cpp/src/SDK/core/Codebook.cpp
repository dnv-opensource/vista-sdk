#include "dnv/vista/sdk/core/Codebook.h"

#include "dto/CodebooksDto.h"

#include <stdexcept>

namespace dnv::vista::sdk
{
    namespace
    {
        CodebookName parseCodebookName(std::string_view name)
        {
            if (name == "positions")
            {
                return CodebookName::Position;
            }
            if (name == "calculations")
            {
                return CodebookName::Calculation;
            }
            if (name == "quantities")
            {
                return CodebookName::Quantity;
            }
            if (name == "states")
            {
                return CodebookName::State;
            }
            if (name == "contents")
            {
                return CodebookName::Content;
            }
            if (name == "commands")
            {
                return CodebookName::Command;
            }
            if (name == "types")
            {
                return CodebookName::Type;
            }
            if (name == "functional_services")
            {
                return CodebookName::FunctionalServices;
            }
            if (name == "maintenance_category")
            {
                return CodebookName::MaintenanceCategory;
            }
            if (name == "activity_type")
            {
                return CodebookName::ActivityType;
            }
            if (name == "detail")
            {
                return CodebookName::Detail;
            }

            throw std::invalid_argument{ "Unknown codebook name: " + std::string{ name } };
        }
    } // namespace

    Codebook::Codebook(const CodebookDto& dto)
        : m_name{ parseCodebookName(dto.name) }
    {
        size_t estimatedSize = 0;
        for (const auto& [group, values] : dto.values)
        {
            estimatedSize += values.size();
        }

        m_groupMap.reserve(estimatedSize);
        m_standardValues.reserve(estimatedSize);
        m_groups.reserve(dto.values.size());

        for (const auto& [group, values] : dto.values)
        {
            auto trimmedGroupView = string::trim(group);

            // Add to groups set (skip <number> and Number as they're not real groups)
            if (trimmedGroupView != "<number>" && trimmedGroupView != "Number")
            {
                m_groups.insert(trimmedGroupView);
            }

            for (const auto& value : values)
            {
                auto trimmedValueView = string::trim(value);

                // Skip "<number>" placeholder
                if (trimmedValueView == "<number>")
                {
                    continue;
                }

                m_groupMap.emplace(trimmedValueView, trimmedGroupView);

                m_standardValues.insert(trimmedValueView);
            }
        }
    }
} // namespace dnv::vista::sdk
