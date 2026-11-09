#include <stdexcept>
#include <string>

namespace dnv::vista::sdk::CodebookNames
{
    inline CodebookName fromPrefix(std::string_view prefix)
    {
        if (prefix == "qty")
        {
            return CodebookName::Quantity;
        }
        if (prefix == "cnt")
        {
            return CodebookName::Content;
        }
        if (prefix == "calc")
        {
            return CodebookName::Calculation;
        }
        if (prefix == "state")
        {
            return CodebookName::State;
        }
        if (prefix == "cmd")
        {
            return CodebookName::Command;
        }
        if (prefix == "type")
        {
            return CodebookName::Type;
        }
        if (prefix == "funct.svc")
        {
            return CodebookName::FunctionalServices;
        }
        if (prefix == "maint.cat")
        {
            return CodebookName::MaintenanceCategory;
        }
        if (prefix == "act.type")
        {
            return CodebookName::ActivityType;
        }
        if (prefix == "pos")
        {
            return CodebookName::Position;
        }
        if (prefix == "detail")
        {
            return CodebookName::Detail;
        }

        throw std::invalid_argument{ "Unknown prefix: " + std::string{ prefix } };
    }

    inline std::string_view toPrefix(CodebookName name)
    {
        switch (name)
        {
            case CodebookName::Quantity:
            {
                return "qty";
            }
            case CodebookName::Content:
            {
                return "cnt";
            }
            case CodebookName::Calculation:
            {
                return "calc";
            }
            case CodebookName::State:
            {
                return "state";
            }
            case CodebookName::Command:
            {
                return "cmd";
            }
            case CodebookName::Type:
            {
                return "type";
            }
            case CodebookName::FunctionalServices:
            {
                return "funct.svc";
            }
            case CodebookName::MaintenanceCategory:
            {
                return "maint.cat";
            }
            case CodebookName::ActivityType:
            {
                return "act.type";
            }
            case CodebookName::Position:
            {
                return "pos";
            }
            case CodebookName::Detail:
            {
                return "detail";
            }
            default:
            {
                throw std::invalid_argument{ "Unknown codebook: " + std::to_string(static_cast<std::uint8_t>(name)) };
            }
        }
    }

    inline std::string_view toString(CodebookName name)
    {
        switch (name)
        {
            case CodebookName::Quantity:
            {
                return "Quantity";
            }
            case CodebookName::Content:
            {
                return "Content";
            }
            case CodebookName::Calculation:
            {
                return "Calculation";
            }
            case CodebookName::State:
            {
                return "State";
            }
            case CodebookName::Command:
            {
                return "Command";
            }
            case CodebookName::Type:
            {
                return "Type";
            }
            case CodebookName::FunctionalServices:
            {
                return "FunctionalServices";
            }
            case CodebookName::MaintenanceCategory:
            {
                return "MaintenanceCategory";
            }
            case CodebookName::ActivityType:
            {
                return "ActivityType";
            }
            case CodebookName::Position:
            {
                return "Position";
            }
            case CodebookName::Detail:
            {
                return "Detail";
            }
            default:
            {
                throw std::invalid_argument{ "Unknown codebook: " + std::to_string(static_cast<std::uint8_t>(name)) };
            }
        }
    }
} // namespace dnv::vista::sdk::CodebookNames
