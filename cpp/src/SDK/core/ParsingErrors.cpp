#include "dnv/vista/sdk/core/ParsingErrors.h"

namespace dnv::vista::sdk
{
    std::string ParsingErrors::toString() const
    {
        if (m_errors.empty())
        {
            return "Success";
        }

        std::string result;
        result.reserve(16 + m_errors.size() * 32);
        result += "Parsing errors:\n";

        for (const auto& [type, message] : m_errors)
        {
            result += '\t';
            result += type;
            result += " - ";
            result += message;
            result += '\n';
        }

        return result;
    }
} // namespace dnv::vista::sdk
