#include "LocalIdParsingErrorBuilder.h"

#include "dnv/vista/sdk/core/ParsingErrors.h"

namespace dnv::vista::sdk::internal
{
    bool LocalIdParsingErrorBuilder::hasError() const noexcept
    {
        return !m_errors.empty();
    }

    ParsingErrors LocalIdParsingErrorBuilder::build() const
    {
        if (m_errors.empty())
        {
            return ParsingErrors{};
        }

        std::vector<ParsingErrors::ErrorEntry> errors;
        errors.reserve(m_errors.size());

        for (const auto& [state, message] : m_errors)
        {
            std::string stateStr;
            switch (state)
            {
                case LocalIdParsingState::NamingRule:
                {
                    stateStr = "NamingRule";
                    break;
                }
                case LocalIdParsingState::VisVersion:
                {
                    stateStr = "VisVersion";
                    break;
                }
                case LocalIdParsingState::PrimaryItem:
                {
                    stateStr = "PrimaryItem";
                    break;
                }
                case LocalIdParsingState::SecondaryItem:
                {
                    stateStr = "SecondaryItem";
                    break;
                }
                case LocalIdParsingState::ItemDescription:
                {
                    stateStr = "ItemDescription";
                    break;
                }
                case LocalIdParsingState::MetaQuantity:
                {
                    stateStr = "MetaQuantity";
                    break;
                }
                case LocalIdParsingState::MetaContent:
                {
                    stateStr = "MetaContent";
                    break;
                }
                case LocalIdParsingState::MetaCalculation:
                {
                    stateStr = "MetaCalculation";
                    break;
                }
                case LocalIdParsingState::MetaState:
                {
                    stateStr = "MetaState";
                    break;
                }
                case LocalIdParsingState::MetaCommand:
                {
                    stateStr = "MetaCommand";
                    break;
                }
                case LocalIdParsingState::MetaType:
                {
                    stateStr = "MetaType";
                    break;
                }
                case LocalIdParsingState::MetaPosition:
                {
                    stateStr = "MetaPosition";
                    break;
                }
                case LocalIdParsingState::MetaDetail:
                {
                    stateStr = "MetaDetail";
                    break;
                }
                case LocalIdParsingState::EmptyState:
                {
                    stateStr = "EmptyState";
                    break;
                }
                case LocalIdParsingState::Formatting:
                {
                    stateStr = "Formatting";
                    break;
                }
                case LocalIdParsingState::Completeness:
                {
                    stateStr = "Completeness";
                    break;
                }
                case LocalIdParsingState::NamingEntity:
                {
                    stateStr = "NamingEntity";
                    break;
                }
                case LocalIdParsingState::IMONumber:
                {
                    stateStr = "IMONumber";
                    break;
                }
            }

            errors.emplace_back(std::move(stateStr), message);
        }

        return ParsingErrors{ std::move(errors) };
    }

    LocalIdParsingErrorBuilder& LocalIdParsingErrorBuilder::addError(
        LocalIdParsingState state, std::string_view message)
    {
        m_errors.emplace_back(state, std::string{ message });
        return *this;
    }
} // namespace dnv::vista::sdk::internal
