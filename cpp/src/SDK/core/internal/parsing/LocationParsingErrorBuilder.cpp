#include "LocationParsingErrorBuilder.h"

#include "dnv/vista/sdk/core/ParsingErrors.h"

namespace dnv::vista::sdk::internal
{
    bool LocationParsingErrorBuilder::hasError() const noexcept
    {
        return !m_errors.empty();
    }

    ParsingErrors LocationParsingErrorBuilder::build() const
    {
        if (m_errors.empty())
        {
            return ParsingErrors{};
        }

        std::vector<ParsingErrors::ErrorEntry> errors;
        errors.reserve(m_errors.size());

        for (const auto& [result, message] : m_errors)
        {
            std::string resultStr;
            switch (result)
            {
                case LocationValidationResult::Invalid:
                {
                    resultStr = "Invalid";
                    break;
                }
                case LocationValidationResult::InvalidCode:
                {
                    resultStr = "InvalidCode";
                    break;
                }
                case LocationValidationResult::InvalidOrder:
                {
                    resultStr = "InvalidOrder";
                    break;
                }
                case LocationValidationResult::NullOrWhiteSpace:
                {
                    resultStr = "NullOrWhiteSpace";
                    break;
                }
                case LocationValidationResult::Valid:
                {
                    resultStr = "Valid";
                    break;
                }
            }

            errors.emplace_back(std::move(resultStr), message);
        }

        return ParsingErrors{ std::move(errors) };
    }

    LocationParsingErrorBuilder& LocationParsingErrorBuilder::addError(
        LocationValidationResult validationResult, std::string_view message)
    {
        m_errors.emplace_back(validationResult, std::string{ message });
        return *this;
    }
} // namespace dnv::vista::sdk::internal
