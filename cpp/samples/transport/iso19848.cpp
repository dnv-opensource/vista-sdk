/**
 * @file iso19848.cpp
 * @brief Demonstrates usage of vista-sdk ISO 19848 transport layer APIs
 * @details This sample shows how to work with ISO 19848 time series data specifications,
 *          including data channel type names, format data types, value validation, and pattern matching
 */

#include <dnv/VistaSDK.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    std::string pad(const std::string& s, size_t width)
    {
        return s.length() < width ? std::string(width - s.length(), ' ') : std::string{};
    }

    std::string wrapText(
        const std::string& text, size_t width = 100, const std::string& indent = "                         ")
    {
        std::ostringstream result;
        size_t pos = 0;
        bool firstLine = true;

        while (pos < text.length())
        {
            if (!firstLine)
            {
                result << "\n" << indent;
            }

            size_t remainingLength = text.length() - pos;
            size_t lineLength = std::min(width, remainingLength);

            if (pos + lineLength < text.length())
            {
                size_t lastSpace = text.rfind(' ', pos + lineLength);
                if (lastSpace != std::string::npos && lastSpace > pos)
                {
                    lineLength = lastSpace - pos;
                }
            }

            result << text.substr(pos, lineLength);
            pos += lineLength;

            if (pos < text.length() && text[pos] == ' ')
            {
                pos++;
            }

            firstLine = false;
        }

        return result.str();
    }
} // namespace

int main()
{
    using namespace dnv::vista::sdk;
    using namespace dnv::vista::sdk::transport;

    std::cout << "=== vista-sdk ISO 19848 Sample ===\n\n";

    {
        std::cout << "1. ISO19848: Singleton and version management\n";
        std::cout << "------------------------------------------------\n";

        auto& iso = ISO19848::instance();
        auto latest = iso.latest();
        const auto& allVersions = iso.versions();

        std::cout << "Latest ISO 19848 version: " << ISO19848Versions::toString(latest) << "\n";
        std::cout << "Available versions (" << allVersions.size() << "):\n";
        for (const auto& version : allVersions)
        {
            std::cout << "  - " << ISO19848Versions::toString(version) << "\n";
        }
        std::cout << "\n";
    }

    {
        std::cout << "2. DataChannelTypeNames: Accessing type names\n";
        std::cout << "------------------------------------------------\n";

        auto& iso = ISO19848::instance();
        auto typeNames = iso.dataChannelTypeNames(ISO19848Version::v2024);

        std::cout << "Data channel type names for v2024 (" << typeNames.size() << "):\n";
        for (const auto& typeName : typeNames)
        {
            std::cout << "  " << std::left << std::setw(20) << typeName.type << " - ";
            std::cout << wrapText(typeName.description) << "\n";
        }
        std::cout << "\n";
    }

    {
        std::cout << "3. DataChannelTypeNames: Parsing and validation\n";
        std::cout << "--------------------------------------------------\n";

        auto& iso = ISO19848::instance();
        auto typeNames = iso.dataChannelTypeNames(ISO19848Version::v2024);

        std::vector<std::string> testTypes = { "Inst", "Average", "SetPoint", "Alert", "invalid_type", "average" };

        std::cout << std::boolalpha;
        for (const auto& typeStr : testTypes)
        {
            auto result = typeNames.fromString(typeStr);
            std::cout << "  '" << typeStr << "'" << pad(typeStr, 15) << " -> ";
            if (result)
            {
                std::cout << "Valid\n";
                std::cout << "                         " << wrapText(result.value()->description) << "\n";
            }
            else
            {
                std::cout << "Invalid\n";
            }
        }
        std::cout << "\n";
    }

    {
        std::cout << "4. FormatDataTypes: Accessing format types\n";
        std::cout << "---------------------------------------------\n";

        auto& iso = ISO19848::instance();
        auto formatTypes = iso.formatDataTypes(ISO19848Version::v2024);

        std::cout << "Format data types for v2024 (" << formatTypes.size() << "):\n";
        for (const auto& formatType : formatTypes)
        {
            std::cout << "  " << std::left << std::setw(10) << formatType.type << " - ";
            std::cout << wrapText(formatType.description, 100, "               ") << "\n";
        }
        std::cout << "\n";
    }

    {
        std::cout << "5. FormatDataTypes: Parsing and validation\n";
        std::cout << "---------------------------------------------\n";

        auto& iso = ISO19848::instance();
        auto formatTypes = iso.formatDataTypes(ISO19848Version::v2024);

        std::vector<std::string> testTypes = { "Decimal",  "Integer", "Boolean", "String",
                                               "DateTime", "decimal", "INVALID" };

        std::cout << std::boolalpha;
        for (const auto& typeStr : testTypes)
        {
            auto result = formatTypes.fromString(typeStr);
            std::cout << "  '" << typeStr << "'" << pad(typeStr, 11) << "-> ";
            if (result)
            {
                std::cout << "Valid (" << result.value()->type << ")\n";
            }
            else
            {
                std::cout << "Invalid\n";
            }
        }
        std::cout << "\n";
    }

    {
        std::cout << "6. FormatDataType: Validating decimal values\n";
        std::cout << "-----------------------------------------------\n";

        auto& iso = ISO19848::instance();
        auto formatTypes = iso.formatDataTypes(ISO19848Version::v2024);
        auto decimalResult = formatTypes.fromString("Decimal");

        if (decimalResult)
        {
            auto& decimalType = *decimalResult.value();

            std::vector<std::string> testValues = { "123.456", "-456.789", "0.001", "999999.99", "not_a_number", "" };

            std::cout << std::boolalpha;
            std::cout << "Validating Decimal values:\n";
            for (const auto& value : testValues)
            {
                auto validateResult = decimalType.validate(value);
                std::cout << "  '" << value << "'" << pad(value, 16) << "-> ";
                if (validateResult)
                {
                    std::cout << "Valid\n";
                }
                else
                {
                    std::cout << "Invalid";
                    if (!validateResult.errors().empty())
                    {
                        std::cout << " (" << validateResult.errors()[0] << ")";
                    }
                    std::cout << "\n";
                }
            }
        }
        std::cout << "\n";
    }

    {
        std::cout << "7. FormatDataType: Validating integer values\n";
        std::cout << "-----------------------------------------------\n";

        auto& iso = ISO19848::instance();
        auto formatTypes = iso.formatDataTypes(ISO19848Version::v2024);
        auto integerResult = formatTypes.fromString("Integer");

        if (integerResult)
        {
            auto& integerType = *integerResult.value();

            std::vector<std::string> testValues = { "42", "-123", "0", "2147483647", "123.456", "not_an_integer" };

            std::cout << std::boolalpha;
            std::cout << "Validating Integer values:\n";
            for (const auto& value : testValues)
            {
                auto validateResult = integerType.validate(value);
                std::cout << "  '" << value << "'" << pad(value, 16) << "-> ";
                if (validateResult)
                {
                    std::cout << "Valid\n";
                }
                else
                {
                    std::cout << "Invalid\n";
                }
            }
        }
        std::cout << "\n";
    }

    {
        std::cout << "8. FormatDataType: Validating boolean and string values\n";
        std::cout << "----------------------------------------------------------\n";

        auto& iso = ISO19848::instance();
        auto formatTypes = iso.formatDataTypes(ISO19848Version::v2024);

        // Boolean validation
        auto booleanResult = formatTypes.fromString("Boolean");
        if (booleanResult)
        {
            auto& booleanType = *booleanResult.value();
            std::cout << "Boolean validation:\n";

            std::vector<std::string> boolValues = { "true", "false", "True", "FALSE", "1", "0", "maybe", "123", "" };
            for (const auto& value : boolValues)
            {
                auto result = booleanType.validate(value);
                std::cout << "  '" << value << "'" << pad(value, 9) << "-> " << (result ? "Valid" : "Invalid") << "\n";
            }
        }

        std::cout << "\n";

        // String validation (always valid)
        auto stringResult = formatTypes.fromString("String");
        if (stringResult)
        {
            auto& stringType = *stringResult.value();
            std::cout << "String validation (always valid):\n";

            std::vector<std::string> stringValues = { "hello", "123", "", "with spaces", "special@chars!" };
            for (const auto& value : stringValues)
            {
                auto result = stringType.validate(value);
                std::cout << "  '" << value << "'" << pad(value, 18) << "-> " << (result ? "Valid" : "Invalid") << "\n";
            }
        }
        std::cout << "\n";
    }

    {
        std::cout << "9. FormatDataType: Validating DateTime values\n";
        std::cout << "------------------------------------------------\n";

        auto& iso = ISO19848::instance();
        auto formatTypes = iso.formatDataTypes(ISO19848Version::v2024);
        auto dateTimeResult = formatTypes.fromString("DateTime");

        if (dateTimeResult)
        {
            auto& dateTimeType = *dateTimeResult.value();

            std::vector<std::string> testValues = { "1994-11-20T10:25:33Z", "2024-12-31T23:59:59Z",
                                                    "2000-01-01T00:00:00Z", "1994-11-20T10:25:33+02:00",
                                                    "1994-11-20T10:25",     "1994-11-20",
                                                    "invalid_date" };

            std::cout << std::boolalpha;
            std::cout << "Validating DateTime values (exact format: yyyy-MM-ddTHH:mm:ssZ):\n";
            for (const auto& value : testValues)
            {
                auto validateResult = dateTimeType.validate(value);
                std::cout << "  '" << value << "'" << pad(value, 26) << "-> ";
                if (validateResult)
                {
                    std::cout << "Valid\n";
                }
                else
                {
                    std::cout << "Invalid\n";
                }
            }
        }
        std::cout << "\n";
    }

    {
        std::cout << "10. FormatDataType: Pattern matching with validated values\n";
        std::cout << "-------------------------------------------------------------\n";

        auto& iso = ISO19848::instance();
        auto formatTypes = iso.formatDataTypes(ISO19848Version::v2024);
        auto decimalResult = formatTypes.fromString("Decimal");

        if (decimalResult)
        {
            auto& decimalType = *decimalResult.value();

            std::cout << "Converting temperature values using pattern matching:\n";

            std::vector<std::string> temperatures = { "23.5", "-15.2", "100.0", "not_a_number" };
            for (const auto& tempStr : temperatures)
            {
                try
                {
                    auto celsius = decimalType.match(
                        tempStr,
                        [](const Decimal& d) -> double { return d.toDouble(); },
                        [](std::int64_t) -> double { return 0.0; },
                        [](bool) -> double { return 0.0; },
                        [](std::string_view) -> double { return 0.0; },
                        [](const DateTimeOffset&) -> double { return 0.0; });

                    auto fahrenheit = celsius * 9.0 / 5.0 + 32.0;
                    std::cout << "  " << std::setw(8) << tempStr << "°C = " << std::fixed << std::setprecision(1)
                              << fahrenheit << "°F\n";
                }
                catch (const std::invalid_argument& e)
                {
                    std::cout << "  " << tempStr << " - Error: " << e.what() << "\n";
                }
            }
        }
        std::cout << "\n";
    }

    {
        std::cout << "11. Value: Creating and inspecting values\n";
        std::cout << "--------------------------------------------\n";

        // Create different value types
        Value stringValue = Value::String{ "sensor_data" };
        Value integerValue = Value::Integer{ 42 };
        Value booleanValue = Value::Boolean{ true };
        Value decimalValue = Value::Decimal{ 123.456 };
        Value dateTimeValue = Value::DateTime{ DateTimeOffset::now() };

        std::cout << std::boolalpha;
        std::cout << "Value type inspection:\n";

        std::cout << "  String value:\n";
        std::cout << "    isString : " << stringValue.string().has_value() << "\n";
        std::cout << "    isInteger: " << stringValue.integer().has_value() << "\n";
        std::cout << "    string() : "
                  << (stringValue.string().has_value() ? std::string{ *stringValue.string() } : "nullopt") << "\n";

        std::cout << "\n  Integer value:\n";
        std::cout << "    isInteger: " << integerValue.integer().has_value() << "\n";
        std::cout << "    isDecimal: " << integerValue.decimal().has_value() << "\n";
        std::cout << "    integer(): "
                  << (integerValue.integer().has_value() ? std::to_string(*integerValue.integer()) : "nullopt") << "\n";

        std::cout << "\n  Boolean value:\n";
        std::cout << "    isBoolean: " << booleanValue.boolean().has_value() << "\n";
        std::cout << "    boolean(): "
                  << (booleanValue.boolean().has_value() ? (*booleanValue.boolean() ? "true" : "false") : "nullopt")
                  << "\n";

        std::cout << "\n  Decimal value:\n";
        std::cout << "    isDecimal: " << decimalValue.decimal().has_value() << "\n";
        std::cout << "    decimal(): "
                  << (decimalValue.decimal().has_value() ? decimalValue.decimal()->toString() : "nullopt") << "\n";

        std::cout << "\n  DateTime value:\n";
        std::cout << "    isDateTime: " << dateTimeValue.dateTime().has_value() << "\n";
        std::cout << "    dateTime(): " << (dateTimeValue.dateTime().has_value() ? "has value" : "nullopt") << "\n";

        std::cout << "\n";
    }

    {
        std::cout << "12. Value: Pattern matching\n";
        std::cout << "-----------------------------\n";

        std::vector<Value> values = { Value{ Value::Decimal{ 23.5 } },
                                      Value{ Value::Integer{ 42 } },
                                      Value{ Value::Boolean{ true } },
                                      Value{ Value::String{ "sensor_reading" } },
                                      Value{ Value::DateTime{ DateTimeOffset::now() } } };

        std::cout << "Processing values with pattern matching:\n";

        for (const auto& value : values)
        {
            auto description = value.match(
                [](const Decimal& d) -> std::string { return "Decimal : " + d.toString(); },
                [](std::int64_t i) -> std::string { return "Integer : " + std::to_string(i); },
                [](bool b) -> std::string { return std::string{ "Boolean : " } + (b ? "true" : "false"); },
                [](std::string_view s) -> std::string { return "String  : " + std::string{ s }; },
                [](const DateTimeOffset& dt) -> std::string { return "DateTime: " + dt.toString(); });

            std::cout << "  " << description << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "13. Practical example: Sensor data validation pipeline\n";
        std::cout << "---------------------------------------------------------\n";

        auto& iso = ISO19848::instance();
        auto formatTypes = iso.formatDataTypes(ISO19848Version::v2024);

        // Simulate sensor readings
        struct SensorReading
        {
            std::string sensorId;
            std::string dataType;
            std::string value;
        };

        std::vector<SensorReading> readings = { { "TEMP_001", "Decimal", "23.5" },
                                                { "RPM_002", "Integer", "1500" },
                                                { "ALARM_003", "Boolean", "false" },
                                                { "STATUS_004", "String", "operational" },
                                                { "TIME_005", "DateTime", "2024-01-15T14:30:00Z" },
                                                { "INVALID_006", "Decimal", "not_a_number" } };

        std::cout << "Processing sensor readings:\n";

        for (const auto& reading : readings)
        {
            std::cout << "\n  Sensor: " << reading.sensorId << "\n";
            std::cout << "    Type : " << reading.dataType << "\n";
            std::cout << "    Value: " << reading.value << "\n";

            auto typeResult = formatTypes.fromString(reading.dataType);
            if (typeResult)
            {
                auto& formatType = *typeResult.value();
                auto validateResult = formatType.validate(reading.value);

                if (validateResult)
                {
                    std::cout << "    [OK] Valid - Ready for storage/processing\n";
                }
                else
                {
                    std::cout << "    [ERROR] Invalid";
                    if (!validateResult.errors().empty())
                    {
                        std::cout << " - " << validateResult.errors()[0];
                    }
                    std::cout << "\n";
                }
            }
            else
            {
                std::cout << "    [ERROR] Unknown data type\n";
            }
        }

        std::cout << "\n";
    }

    return 0;
}
