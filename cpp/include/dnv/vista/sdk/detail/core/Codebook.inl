#include "dnv/vista/sdk/utils/StringUtils.h"

#include "dnv/vista/sdk/containers/StackVector.h"

namespace dnv::vista::sdk
{
    inline CodebookName Codebook::name() const noexcept
    {
        return m_name;
    }

    inline const StringSet& Codebook::standardValues() const noexcept
    {
        return m_standardValues;
    }

    inline const StringSet& Codebook::groups() const noexcept
    {
        return m_groups;
    }

    inline bool Codebook::hasGroup(std::string_view group) const noexcept
    {
        return m_groups.contains(group);
    }

    inline bool Codebook::hasStandardValue(std::string_view value) const noexcept
    {
        if (m_name == CodebookName::Position && string::isAllDigits(value))
        {
            return true;
        }

        return m_standardValues.contains(value);
    }

    inline std::optional<MetadataTag> Codebook::createTag(std::string_view value) const noexcept
    {
        if (string::isNullOrWhiteSpace(value))
        {
            return std::nullopt;
        }

        bool isCustom = false;

        if (m_name == CodebookName::Position)
        {
            auto validity = validatePosition(value);
            if (validity < PositionValidationResult::Valid)
            {
                return std::nullopt;
            }

            if (validity == PositionValidationResult::Custom)
            {
                isCustom = true;
            }
        }
        else
        {
            if (!string::isUriUnreserved(value))
            {
                return std::nullopt;
            }
            if (m_name != CodebookName::Detail && !m_standardValues.contains(value))
            {
                isCustom = true;
            }
        }

        return MetadataTag{ m_name, std::string{ value }, isCustom };
    }

    inline PositionValidationResult Codebook::validatePosition(std::string_view position) const noexcept
    {
        if (string::isNullOrWhiteSpace(position) || !string::isUriUnreserved(position))
        {
            return PositionValidationResult::Invalid;
        }

        if (m_standardValues.contains(position))
        {
            return PositionValidationResult::Valid;
        }

        if (position.find('-') == std::string_view::npos)
        {
            if (string::isAllDigits(position))
            {
                return PositionValidationResult::Valid;
            }
            return PositionValidationResult::Custom;
        }

        // Split and validate each part
        StackVector<std::string_view, 8> parts;
        StackVector<PositionValidationResult, 8> validations;

        for (std::string_view remaining = position; !remaining.empty();)
        {
            auto sep = remaining.find('-');
            auto part = remaining.substr(0, sep);
            parts.push_back(part);
            validations.push_back(validatePosition(part));
            remaining = (sep == std::string_view::npos) ? std::string_view{} : remaining.substr(sep + 1);
        }
        auto maxValidation = std::max_element(
            validations.begin(), validations.end(), [](PositionValidationResult a, PositionValidationResult b) {
                return static_cast<int>(a) < static_cast<int>(b);
            });

        bool anyErrors = maxValidation != validations.end() && static_cast<int>(*maxValidation) < 100;

        // Check if any number appears before the last position
        bool numberNotAtEnd = false;
        for (size_t i = 0; i < parts.size(); ++i)
        {
            if (string::isAllDigits(parts[i]) && i < parts.size() - 1)
            {
                numberNotAtEnd = true;
                break;
            }
        }

        // Check alphabetical sorting of non-number parts
        StackVector<std::string_view, 8> nonNumbers;

        for (auto part : parts)
        {
            if (!string::isAllDigits(part))
            {
                nonNumbers.push_back(part);
            }
        }

        if (!std::is_sorted(nonNumbers.begin(), nonNumbers.end()))
        {
            return PositionValidationResult::InvalidOrder;
        }

        // Combine order validation results
        if (numberNotAtEnd)
        {
            return PositionValidationResult::InvalidOrder;
        }

        // If any part had errors, return now (after order check)
        if (anyErrors && maxValidation != validations.end())
        {
            return *maxValidation;
        }

        // Grouping validation - only if all parts are exactly Valid (100)
        if (std::all_of(validations.begin(), validations.end(), [](PositionValidationResult v) {
                return static_cast<int>(v) == static_cast<int>(PositionValidationResult::Valid);
            }))
        {
            StackVector<std::string_view, 8> groups;

            for (auto part : parts)
            {
                if (string::isAllDigits(part))
                {
                    groups.push_back("<number>");
                }
                else
                {
                    auto it = m_groupMap.find(part);
                    groups.push_back(it != m_groupMap.end() ? it->second : part);
                }
            }

            // Check for duplicates, excluding DEFAULT_GROUP entries from the check
            bool hasDefaultGroup = std::find(groups.begin(), groups.end(), "DEFAULT_GROUP") != groups.end();
            if (!hasDefaultGroup)
            {
                for (size_t i = 0; i < groups.size(); ++i)
                {
                    for (size_t j = i + 1; j < groups.size(); ++j)
                    {
                        if (groups[i] == groups[j])
                        {
                            return PositionValidationResult::InvalidGrouping;
                        }
                    }
                }
            }
        }

        return *maxValidation;
    }
} // namespace dnv::vista::sdk
