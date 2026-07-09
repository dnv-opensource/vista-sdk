#include "dnv/vista/sdk/core/LocationBuilder.h"

#include "dnv/vista/sdk/utils/StringUtils.h"

namespace dnv::vista::sdk
{
    LocationBuilder LocationBuilder::withLocation(const Location& location) const&
    {
        LocationBuilder builder{ *this };

        std::string locationStr = location.value();
        std::string_view span = locationStr;
        std::optional<int> number;

        for (size_t i = 0; i < span.length(); ++i)
        {
            char ch = span[i];

            if (string::isDigit(ch))
            {
                if (!number.has_value())
                {
                    number = ch - '0';
                }
                else
                {
                    std::string_view numberSubstr = span.substr(0, i + 1);
                    int parsedNumber;
                    if (!string::fromString<int>(numberSubstr, parsedNumber))
                    {
                        throw std::invalid_argument{ "Should include a valid number" };
                    }
                    number = parsedNumber;
                }
                continue;
            }

            builder = builder.withCode(ch);
        }

        if (number.has_value())
        {
            builder = builder.withNumber(number.value());
        }

        return builder;
    }

    LocationBuilder LocationBuilder::withLocation(const Location& location) &&
    {
        return static_cast<const LocationBuilder&>(*this).withLocation(location);
    }
} // namespace dnv::vista::sdk
