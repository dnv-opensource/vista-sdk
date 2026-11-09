#include "dnv/vista/sdk/transport/ISO19848.h"

#include "EmbeddedResources/EmbeddedResources.h"

#include "dnv/vista/sdk/utils/StringUtils.h"
#include "SDK/core/internal/VersionedCache.h"
#include "SDK/core/dto/ISO19848Dtos.h"
#include "ISO19848VersionsExtensions.h"

#include <stdexcept>

namespace dnv::vista::sdk::transport
{
    namespace
    {
        using sdk::internal::getOrLoad;
        using sdk::internal::VersionedCache;

        [[nodiscard]] constexpr bool isValidIso19848DateTime(std::string_view str) noexcept
        {
            constexpr std::size_t expectedLength{ 20 }; // "yyyy-MM-ddTHH:mm:ssZ"

            if (str.size() != expectedLength)
            {
                return false;
            }

            using string::isAllDigits;

            return isAllDigits(str.substr(0, 4)) && str[4] == '-' && isAllDigits(str.substr(5, 2)) && str[7] == '-' &&
                   isAllDigits(str.substr(8, 2)) && str[10] == 'T' && isAllDigits(str.substr(11, 2)) &&
                   str[13] == ':' && isAllDigits(str.substr(14, 2)) && str[16] == ':' &&
                   isAllDigits(str.substr(17, 2)) && str[19] == 'Z';
        }

        VersionedCache<ISO19848Version, DataChannelTypeNames>& dataChannelTypeNamesCache()
        {
            static VersionedCache<ISO19848Version, DataChannelTypeNames> cache;
            return cache;
        }

        VersionedCache<ISO19848Version, FormatDataTypes>& formatDataTypesCache()
        {
            static VersionedCache<ISO19848Version, FormatDataTypes> cache;
            return cache;
        }
    } // namespace

    ParseResult<DataChannelTypeName> DataChannelTypeNames::fromString(std::string_view dataChannelTypeName) const
    {
        if (string::isNullOrWhiteSpace(dataChannelTypeName))
        {
            return ParseResult<DataChannelTypeName>::invalid();
        }

        const auto trimmedType = string::trim(dataChannelTypeName);

        for (const auto& value : m_values)
        {
            if (value.type == trimmedType)
            {
                return ParseResult<DataChannelTypeName>::ok(value);
            }
        }

        return ParseResult<DataChannelTypeName>::invalid();
    }

    ValidateResult<Value> FormatDataType::validate(std::string_view value) const
    {
        if (type == "String")
        {
            return ValidateResult<Value>::ok(Value::String{ std::string{ value } });
        }
        else if (type == "Boolean")
        {
            const auto trimmed = string::trim(value);
            bool b;
            if (string::iequals(trimmed, "True") || trimmed == "1")
            {
                b = true;
            }
            else if (string::iequals(trimmed, "False") || trimmed == "0")
            {
                b = false;
            }
            else
            {
                return ValidateResult<Value>::invalid("Invalid boolean value - Value='" + std::string{ value } + "'");
            }
            return ValidateResult<Value>::ok(Value::Boolean{ b });
        }
        else if (type == "Integer")
        {
            std::int64_t i;
            if (!string::fromString(value, i))
            {
                return ValidateResult<Value>::invalid("Invalid integer value - Value='" + std::string{ value } + "'");
            }
            return ValidateResult<Value>::ok(Value::Integer{ i });
        }
        else if (type == "Decimal")
        {
            try
            {
                return ValidateResult<Value>::ok(Value::Decimal{ value });
            }
            catch (const std::exception& ex)
            {
                return ValidateResult<Value>::invalid(
                    "Invalid decimal value - Value='" + std::string{ value } + "'. " + ex.what());
            }
        }
        else if (type == "DateTime")
        {
            if (!isValidIso19848DateTime(value))
            {
                return ValidateResult<Value>::invalid(
                    "Invalid datetime value - Value='" + std::string{ value } +
                    "'. Expected format: yyyy-MM-ddTHH:mm:ssZ");
            }

            auto opt = sdk::DateTimeOffset::fromString(value);
            if (!opt.has_value())
            {
                return ValidateResult<Value>::invalid(
                    "Invalid datetime value - Value='" + std::string{ value } +
                    "'. Expected format: yyyy-MM-ddTHH:mm:ssZ");
            }
            return ValidateResult<Value>::ok(Value::DateTime{ opt.value() });
        }
        else
        {
            throw std::invalid_argument{ "Invalid format type: " + type };
        }
    }

    ParseResult<FormatDataType> FormatDataTypes::fromString(std::string_view formatDataType) const
    {
        if (string::isNullOrWhiteSpace(formatDataType))
        {
            return ParseResult<FormatDataType>::invalid();
        }

        const auto trimmedType = string::trim(formatDataType);

        for (const auto& value : m_values)
        {
            if (value.type == trimmedType)
            {
                return ParseResult<FormatDataType>::ok(value);
            }
        }

        return ParseResult<FormatDataType>::invalid();
    }

    ISO19848& ISO19848::instance()
    {
        static ISO19848 instance;
        return instance;
    }

    const std::vector<ISO19848Version>& ISO19848::versions() const noexcept
    {
        static const std::vector<ISO19848Version> versions = []() {
            const auto arr = ISO19848Versions::all();
            return std::vector<ISO19848Version>(arr.begin(), arr.end());
        }();

        return versions;
    }

    ISO19848Version ISO19848::latest() const noexcept
    {
        return ISO19848Versions::latest();
    }

    const DataChannelTypeNames& ISO19848::dataChannelTypeNames(ISO19848Version version)
    {
        return getOrLoad(dataChannelTypeNamesCache(), version, [](ISO19848Version v) {
            auto versionStr = ISO19848Versions::toString(v);
            auto dto = sdk::EmbeddedResources::dataChannelTypeNames(versionStr);

            if (!dto.has_value())
            {
                throw std::out_of_range{ "DataChannelTypeNames not available for version: " +
                                         std::string{ versionStr } };
            }

            std::vector<DataChannelTypeName> values;
            values.reserve(dto->values.size());

            for (const auto& entry : dto->values)
            {
                values.emplace_back(entry.type, entry.description);
            }

            return DataChannelTypeNames{ std::move(values) };
        });
    }

    const FormatDataTypes& ISO19848::formatDataTypes(ISO19848Version version)
    {
        return getOrLoad(formatDataTypesCache(), version, [](ISO19848Version v) {
            auto versionStr = ISO19848Versions::toString(v);
            auto dto = sdk::EmbeddedResources::formatDataTypes(versionStr);

            if (!dto.has_value())
            {
                throw std::out_of_range{ "FormatDataTypes not available for version: " + std::string{ versionStr } };
            }

            std::vector<FormatDataType> values;
            values.reserve(dto->values.size());

            for (const auto& entry : dto->values)
            {
                values.emplace_back(entry.type, entry.description);
            }

            return FormatDataTypes{ std::move(values) };
        });
    }
} // namespace dnv::vista::sdk::transport
