#include "dnv/vista/sdk/core/Locations.h"

#include "dnv/vista/sdk/core/ParsingErrors.h"
#include "dnv/vista/sdk/utils/StringUtils.h"
#include "dto/LocationsDto.h"
#include "internal/parsing/LocationParsingErrorBuilder.h"

#include <array>
#include <cassert>
#include <cctype>
#include <stdexcept>

namespace dnv::vista::sdk
{
    Locations::Locations(VisVersion version, const LocationsDto& dto)
        : m_visVersion{ version }
    {
        m_relativeLocations.reserve(dto.items.size());
        m_groups.reserve(5);

        std::unordered_map<LocationGroup, std::vector<RelativeLocation>> tempGroups;

        for (const auto& item : dto.items)
        {
            std::string locStr(1, item.code);
            Location location(locStr);

            RelativeLocation relLoc(item.code, item.name, location, item.definition);

            m_relativeLocations.push_back(std::move(relLoc));
            m_locationCodes.insert(item.code);

            // Skip H and V for grouping
            if (item.code == 'H' || item.code == 'V')
            {
                continue;
            }

            // Map code to group
            LocationGroup group;
            if (item.code == 'N')
            {
                group = LocationGroup::Number;
            }
            else if (item.code == 'P' || item.code == 'C' || item.code == 'S')
            {
                group = LocationGroup::Side;
            }
            else if (item.code == 'U' || item.code == 'M' || item.code == 'L')
            {
                group = LocationGroup::Vertical;
            }
            else if (item.code == 'I' || item.code == 'O')
            {
                group = LocationGroup::Transverse;
            }
            else if (item.code == 'F' || item.code == 'A')
            {
                group = LocationGroup::Longitudinal;
            }
            else
            {
                throw std::runtime_error{ "Unsupported location code" };
            }

            // Skip N for reversedGroups and group values
            if (group == LocationGroup::Number)
            {
                continue;
            }
            m_reversedGroups[item.code] = group;
            tempGroups[group].push_back(m_relativeLocations.back());
        }

        m_groups = std::move(tempGroups);
    }

    std::optional<Location> Locations::fromString(std::string_view locationStr) const noexcept
    {
        return fromString(locationStr, nullptr);
    }

    std::optional<Location> Locations::fromString(std::string_view locationStr, ParsingErrors& errors) const noexcept
    {
        try
        {
            internal::LocationParsingErrorBuilder errorBuilder;
            auto result = fromString(locationStr, &errorBuilder);
            errors = errorBuilder.build();

            return result;
        }
        catch (const std::bad_alloc&)
        {
            return std::nullopt;
        }
    }

    std::optional<Location> Locations::fromString(
        std::string_view locationStr, internal::LocationParsingErrorBuilder* errorBuilder) const noexcept
    {
        try
        {
            if (string::isNullOrWhiteSpace(locationStr))
            {
                if (errorBuilder)
                {
                    errorBuilder->addError(
                        internal::LocationValidationResult::NullOrWhiteSpace,
                        "Invalid location: contains only whitespace");
                }

                return std::nullopt;
            }

            std::optional<std::size_t> prevDigitIndex;
            std::optional<std::size_t> digitStartIndex;
            std::optional<std::size_t> charsStartIndex;

            // Track which groups have been seen (4 groups: Side, Vertical, Transverse, Longitudinal)
            std::array<std::optional<char>, 4> seenGroups{};

            for (std::size_t i = 0; i < locationStr.size(); ++i)
            {
                const char ch = locationStr[i];

                if (std::isdigit(static_cast<unsigned char>(ch)))
                {
                    if (!digitStartIndex.has_value() && i != 0)
                    {
                        if (errorBuilder)
                        {
                            std::string error;
                            error +=
                                "Invalid location: numeric location should start before location code(s) in location: "
                                "'";
                            error += locationStr;
                            error += '\'';

                            errorBuilder->addError(internal::LocationValidationResult::Invalid, error);
                        }

                        return std::nullopt;
                    }

                    if (prevDigitIndex.has_value() && prevDigitIndex.value() != (i - 1))
                    {
                        if (errorBuilder)
                        {
                            std::string error;
                            error += "Invalid location: cannot have multiple separated digits in location: '";
                            error += locationStr;
                            error += '\'';

                            errorBuilder->addError(internal::LocationValidationResult::Invalid, error);
                        }

                        return std::nullopt;
                    }

                    if (!digitStartIndex.has_value())
                    {
                        digitStartIndex = i;
                    }

                    prevDigitIndex = i;
                }
                else
                {
                    // Check if code exists in reversedGroups
                    auto groupIt = m_reversedGroups.find(ch);
                    if (groupIt == m_reversedGroups.end())
                    {
                        if (m_locationCodes.find(ch) == m_locationCodes.end())
                        {
                            if (errorBuilder)
                            {
                                // Collect all invalid characters (single pass over locationStr)
                                std::string invalidChars;
                                bool first = true;
                                for (const char c : locationStr)
                                {
                                    const bool isInvalidCode = !std::isdigit(static_cast<unsigned char>(c)) &&
                                                               (c == 'N' || !m_locationCodes.contains(c));
                                    if (!isInvalidCode)
                                    {
                                        continue;
                                    }

                                    if (!first)
                                    {
                                        invalidChars += ',';
                                    }
                                    first = false;
                                    invalidChars += '\'';
                                    invalidChars += c;
                                    invalidChars += '\'';
                                }

                                std::string errorMsg;
                                errorMsg += "Invalid location code: '";
                                errorMsg += locationStr;
                                errorMsg += "' with invalid location code(s): ";
                                errorMsg += invalidChars;

                                errorBuilder->addError(internal::LocationValidationResult::InvalidCode, errorMsg);
                            }

                            return std::nullopt;
                        }
                        if (!charsStartIndex.has_value())
                        {
                            charsStartIndex = i;
                        }
                        continue;
                    }

                    const LocationGroup group = groupIt->second;

                    assert(group != LocationGroup::Number);
                    const std::size_t groupIndex = static_cast<std::size_t>(group) - 1;
                    if (seenGroups[groupIndex].has_value())
                    {
                        if (errorBuilder)
                        {
                            const char* groupName = "";
                            switch (group)
                            {
                                case LocationGroup::Side:
                                {
                                    groupName = "Side";
                                    break;
                                }
                                case LocationGroup::Vertical:
                                {
                                    groupName = "Vertical";
                                    break;
                                }
                                case LocationGroup::Transverse:
                                {
                                    groupName = "Transverse";
                                    break;
                                }
                                case LocationGroup::Longitudinal:
                                {
                                    groupName = "Longitudinal";
                                    break;
                                }
                                default:
                                {
                                    break;
                                }
                            }

                            std::string error;
                            error += "Invalid location: Multiple '";
                            error += groupName;
                            error += "' values. Got both '";
                            error += seenGroups[groupIndex].value();
                            error += "' and '";
                            error += ch;
                            error += "' in '";
                            error += locationStr;
                            error += '\'';

                            errorBuilder->addError(internal::LocationValidationResult::Invalid, error);
                        }

                        return std::nullopt;
                    }

                    seenGroups[groupIndex] = ch;

                    // Check alphabetical order
                    if (charsStartIndex.has_value() && i > 0)
                    {
                        const char prevCh = locationStr[i - 1];
                        if (!std::isdigit(static_cast<unsigned char>(prevCh)) && ch < prevCh)
                        {
                            if (errorBuilder)
                            {
                                std::string error;
                                error += "Invalid location: '";
                                error += locationStr;
                                error += "' not alphabetically sorted";

                                errorBuilder->addError(internal::LocationValidationResult::InvalidOrder, error);
                            }

                            return std::nullopt;
                        }
                    }

                    if (!charsStartIndex.has_value())
                    {
                        charsStartIndex = i;
                    }
                }
            }

            return Location{ locationStr };
        }
        catch (const std::bad_alloc&)
        {
            return std::nullopt;
        }
    }
} // namespace dnv::vista::sdk
