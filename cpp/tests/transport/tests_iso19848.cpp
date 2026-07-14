#include <doctest/doctest.h>

#include <dnv/vista/sdk/transport/ISO19848.h>

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

namespace dnv::vista::sdk::tests
{
    using namespace dnv::vista::sdk;

    TEST_SUITE("ISO19848")
    {
        TEST_CASE("Instance")
        {
            auto& iso = transport::ISO19848::instance();
            static_cast<void>(iso);
        }

        TEST_CASE("EmbeddedResources")
        {
            auto& iso = transport::ISO19848::instance();

            auto dataChannelTypeNames = iso.dataChannelTypeNames(transport::ISO19848Version::v2024);
            CHECK(dataChannelTypeNames.begin() != dataChannelTypeNames.end());

            auto formatDataTypes = iso.formatDataTypes(transport::ISO19848Version::v2024);
            CHECK(formatDataTypes.begin() != formatDataTypes.end());
        }

        TEST_CASE("DataChannelTypeNames_Load")
        {
            auto& iso = transport::ISO19848::instance();

            SUBCASE("v2018")
            {
                auto names = iso.dataChannelTypeNames(transport::ISO19848Version::v2018);
                CHECK(names.begin() != names.end());
            }

            SUBCASE("v2024")
            {
                auto names = iso.dataChannelTypeNames(transport::ISO19848Version::v2024);
                CHECK(names.begin() != names.end());
            }
        }

        TEST_CASE("DataChannelTypeNames_Parse_Self")
        {
            auto& iso = transport::ISO19848::instance();
            auto dataChannelTypeNames = iso.dataChannelTypeNames(transport::ISO19848Version::v2024);

            for (const auto& typeName : dataChannelTypeNames)
            {
                auto result = dataChannelTypeNames.fromString(typeName.type);
                REQUIRE(result);
                REQUIRE(result.value().has_value());
                CHECK(typeName.type == result.value()->type);
            }
        }

        TEST_CASE("DataChannelTypeNames_Parse")
        {
            auto& iso = transport::ISO19848::instance();
            auto dataChannelTypeNames = iso.dataChannelTypeNames(transport::ISO19848Version::v2024);

            struct TestCase
            {
                std::string_view value;
                bool expectedOk;
            };

            static constexpr TestCase cases[] = {
                { "Inst", true },
                { "Average", true },
                { "Max", true },
                { "Min", true },
                { "Median", true },
                { "Mode", true },
                { "StandardDeviation", true },
                { "Calculated", true },
                { "SetPoint", true },
                { "Command", true },
                { "Alert", true },
                { "Status", true },
                { "ManualInput", true },
                { "manualInput", false },
                { "asd", false },
                { "some-random", false },
                { "InputManual", false },
            };

            for (const auto& tc : cases)
            {
                CAPTURE(tc.value);
                auto result = dataChannelTypeNames.fromString(tc.value);
                if (tc.expectedOk)
                {
                    REQUIRE(result);
                    REQUIRE(result.value().has_value());
                    CHECK(std::string{ tc.value } == result.value()->type);
                }
                else
                {
                    CHECK(!result);
                }
            }
        }

        TEST_CASE("FormatDataTypes_Load")
        {
            auto& iso = transport::ISO19848::instance();

            SUBCASE("v2018")
            {
                auto types = iso.formatDataTypes(transport::ISO19848Version::v2018);
                CHECK(types.begin() != types.end());
            }

            SUBCASE("v2024")
            {
                auto types = iso.formatDataTypes(transport::ISO19848Version::v2024);
                CHECK(types.begin() != types.end());
            }
        }

        TEST_CASE("FormatDataType_Parse_Self")
        {
            auto& iso = transport::ISO19848::instance();
            auto types = iso.formatDataTypes(transport::ISO19848Version::v2024);

            for (const auto& typeName : types)
            {
                auto result = types.fromString(typeName.type);
                REQUIRE(result);
                REQUIRE(result.value().has_value());
                CHECK(typeName.type == result.value()->type);
            }
        }

        TEST_CASE("FormatDataType_Parse")
        {
            auto& iso = transport::ISO19848::instance();
            auto types = iso.formatDataTypes(transport::ISO19848Version::v2024);

            struct TestCase
            {
                std::string_view value;
                bool expectedOk;
            };

            static constexpr TestCase cases[] = {
                { "Decimal", true },      { "Integer", true },   { "Boolean", true }, { "String", true },
                { "DateTime", true },     { "decimal", false },  { "string", false }, { "asd", false },
                { "some-random", false }, { "TimeDate", false },
            };

            for (const auto& tc : cases)
            {
                CAPTURE(tc.value);
                auto result = types.fromString(tc.value);
                if (tc.expectedOk)
                {
                    REQUIRE(result);
                    REQUIRE(result.value().has_value());
                    CHECK(std::string{ tc.value } == result.value()->type);
                }
                else
                {
                    CHECK(!result);
                }
            }
        }

        TEST_CASE("FormatDataType_Validate")
        {
            auto& iso = transport::ISO19848::instance();
            auto types = iso.formatDataTypes(transport::ISO19848Version::v2024);

            struct TestCase
            {
                std::string_view type;
                std::string_view value;
                bool expectedOk;
            };

            static constexpr TestCase cases[] = {
                { "Decimal", "0.1", true },
                { "DateTime", "1994-11-20T10:25:33Z", true },
                { "DateTime", "1994-11-20T10", false },

                { "DateTime", "1994-11-20T10:25:33+02:00", false },
                { "DateTime", "1994-11-20T10:25:33.123Z", false },
                { "DateTime", "1994-11-20T10:25:33", false },

                { "Boolean", "1", true },
                { "Boolean", "0", true },
                { "Boolean", "yes", false },
                { "Boolean", "no", false },
                { "Boolean", "on", false },
                { "Boolean", "off", false },
                { "Boolean", "y", false },
                { "Boolean", "n", false },

                { "Integer", "2147483648", true },          // INT32_MAX + 1
                { "Integer", "9223372036854775807", true }, // INT64_MAX
                { "Integer", "9223372036854775808", false } // INT64_MAX + 1, out of range
            };

            for (const auto& tc : cases)
            {
                CAPTURE(tc.type);
                CAPTURE(tc.value);
                auto parseResult = types.fromString(tc.type);
                REQUIRE(parseResult);
                REQUIRE(parseResult.value().has_value());
                CHECK(std::string{ tc.type } == parseResult.value()->type);

                auto validateResult = parseResult.value()->validate(tc.value);
                if (tc.expectedOk)
                {
                    CHECK(validateResult);
                }
                else
                {
                    CHECK(!validateResult);
                }
            }
        }

        TEST_CASE("FormatDataType_Match_Method")
        {
            auto& iso = transport::ISO19848::instance();
            auto formatDataTypes = iso.formatDataTypes(transport::ISO19848Version::v2024);

            auto parseResult = formatDataTypes.fromString("Integer");
            REQUIRE(parseResult);
            REQUIRE(parseResult.value().has_value());
            auto integerType = *parseResult.value();

            SUBCASE("returns int")
            {
                int matchedValue = 0;
                CHECK_NOTHROW(
                    matchedValue = integerType.match(
                        "42",
                        [](const Decimal&) -> int {
                            FAIL("Expected integer, got decimal");
                            return 0;
                        },
                        [](std::int64_t i) -> int { return static_cast<int>(i); },
                        [](bool) -> int {
                            FAIL("Expected integer, got boolean");
                            return 0;
                        },
                        [](std::string_view) -> int {
                            FAIL("Expected integer, got string");
                            return 0;
                        },
                        [](const DateTimeOffset&) -> int {
                            FAIL("Expected integer, got datetime");
                            return 0;
                        }));
                CHECK(matchedValue == 42);
            }

            SUBCASE("returns string representation")
            {
                std::string matchResult;
                CHECK_NOTHROW(
                    matchResult = integerType.match(
                        "123",
                        [](const Decimal&) -> std::string {
                            FAIL("Expected integer, got decimal");
                            return "";
                        },
                        [](std::int64_t i) -> std::string { return "Matched integer: " + std::to_string(i); },
                        [](bool) -> std::string {
                            FAIL("Expected integer, got boolean");
                            return "";
                        },
                        [](std::string_view) -> std::string {
                            FAIL("Expected integer, got string");
                            return "";
                        },
                        [](const DateTimeOffset&) -> std::string {
                            FAIL("Expected integer, got datetime");
                            return "";
                        }));
                CHECK(matchResult == "Matched integer: 123");
            }

            SUBCASE("invalid value throws")
            {
                CHECK_THROWS_AS(
                    static_cast<void>(integerType.match(
                        "not_a_number",
                        [](const Decimal&) -> int { return 0; },
                        [](std::int64_t i) -> int { return static_cast<int>(i); },
                        [](bool) -> int { return 0; },
                        [](std::string_view) -> int { return 0; },
                        [](const DateTimeOffset&) -> int { return 0; })),
                    std::invalid_argument);
            }
        }

        TEST_CASE("Match_Decimal_Type")
        {
            auto& iso = transport::ISO19848::instance();
            auto formatDataTypes = iso.formatDataTypes(transport::ISO19848Version::v2024);

            auto parseResult = formatDataTypes.fromString("Decimal");
            REQUIRE(parseResult);
            REQUIRE(parseResult.value().has_value());
            auto decimalType = *parseResult.value();

            bool decimalCalled = false;
            Decimal decimalValue;
            CHECK_NOTHROW(decimalType.match(
                "123.456",
                [&](const Decimal& d) {
                    decimalCalled = true;
                    decimalValue = d;
                },
                [](std::int64_t) { FAIL("Expected decimal, got integer"); },
                [](bool) { FAIL("Expected decimal, got boolean"); },
                [](std::string_view) { FAIL("Expected decimal, got string"); },
                [](const DateTimeOffset&) { FAIL("Expected decimal, got datetime"); }));

            CHECK(decimalCalled);
            Decimal expected{ "123.456" };
            Decimal tolerance{ "0.000001" };
            CHECK((decimalValue - expected).abs() < tolerance);
        }

        TEST_CASE("Match_Integer_Type")
        {
            auto& iso = transport::ISO19848::instance();
            auto formatDataTypes = iso.formatDataTypes(transport::ISO19848Version::v2024);

            auto parseResult = formatDataTypes.fromString("Integer");
            REQUIRE(parseResult);
            REQUIRE(parseResult.value().has_value());
            auto integerType = *parseResult.value();

            bool integerCalled = false;
            int integerValue = 0;
            CHECK_NOTHROW(integerType.match(
                "42",
                [](const Decimal&) { FAIL("Expected integer, got decimal"); },
                [&](std::int64_t i) {
                    integerCalled = true;
                    integerValue = static_cast<int>(i);
                },
                [](bool) { FAIL("Expected integer, got boolean"); },
                [](std::string_view) { FAIL("Expected integer, got string"); },
                [](const DateTimeOffset&) { FAIL("Expected integer, got datetime"); }));

            CHECK(integerCalled);
            CHECK(integerValue == 42);
        }

        TEST_CASE("Match_Boolean_Type")
        {
            auto& iso = transport::ISO19848::instance();
            auto formatDataTypes = iso.formatDataTypes(transport::ISO19848Version::v2024);

            auto parseResult = formatDataTypes.fromString("Boolean");
            REQUIRE(parseResult);
            REQUIRE(parseResult.value().has_value());
            auto booleanType = *parseResult.value();

            bool booleanCalled = false;
            bool booleanValue = false;
            CHECK_NOTHROW(booleanType.match(
                "true",
                [](const Decimal&) { FAIL("Expected boolean, got decimal"); },
                [](std::int64_t) { FAIL("Expected boolean, got integer"); },
                [&](bool b) {
                    booleanCalled = true;
                    booleanValue = b;
                },
                [](std::string_view) { FAIL("Expected boolean, got string"); },
                [](const DateTimeOffset&) { FAIL("Expected boolean, got datetime"); }));

            CHECK(booleanCalled);
            CHECK(booleanValue);
        }

        TEST_CASE("Match_String_Type")
        {
            auto& iso = transport::ISO19848::instance();
            auto formatDataTypes = iso.formatDataTypes(transport::ISO19848Version::v2024);

            auto parseResult = formatDataTypes.fromString("String");
            REQUIRE(parseResult);
            REQUIRE(parseResult.value().has_value());
            auto stringType = *parseResult.value();

            bool stringCalled = false;
            std::string stringValue;
            CHECK_NOTHROW(stringType.match(
                "Hello World",
                [](const Decimal&) { FAIL("Expected string, got decimal"); },
                [](std::int64_t) { FAIL("Expected string, got integer"); },
                [](bool) { FAIL("Expected string, got boolean"); },
                [&](std::string_view s) {
                    stringCalled = true;
                    stringValue = std::string{ s };
                },
                [](const DateTimeOffset&) { FAIL("Expected string, got datetime"); }));

            CHECK(stringCalled);
            CHECK(stringValue == "Hello World");
        }

        TEST_CASE("Match_DateTime_Type")
        {
            auto& iso = transport::ISO19848::instance();
            auto formatDataTypes = iso.formatDataTypes(transport::ISO19848Version::v2024);

            auto parseResult = formatDataTypes.fromString("DateTime");
            REQUIRE(parseResult);
            REQUIRE(parseResult.value().has_value());
            auto dateTimeType = *parseResult.value();

            bool dateTimeCalled = false;
            DateTimeOffset dateTimeValue;
            CHECK_NOTHROW(dateTimeType.match(
                "1994-11-20T10:25:33Z",
                [](const Decimal&) { FAIL("Expected datetime, got decimal"); },
                [](std::int64_t) { FAIL("Expected datetime, got integer"); },
                [](bool) { FAIL("Expected datetime, got boolean"); },
                [](std::string_view) { FAIL("Expected datetime, got string"); },
                [&](const DateTimeOffset& dt) {
                    dateTimeCalled = true;
                    dateTimeValue = dt;
                }));

            CHECK(dateTimeCalled);
            CHECK(dateTimeValue.year() == 1994);
        }

        TEST_CASE("Match_Invalid_Values_Throw")
        {
            auto& iso = transport::ISO19848::instance();
            auto formatDataTypes = iso.formatDataTypes(transport::ISO19848Version::v2024);

            SUBCASE("Decimal invalid")
            {
                auto result = formatDataTypes.fromString("Decimal");
                REQUIRE(result);
                REQUIRE(result.value().has_value());
                CHECK_THROWS_AS(
                    result.value()->match(
                        "not_a_decimal",
                        [](const Decimal&) {},
                        [](std::int64_t) {},
                        [](bool) {},
                        [](std::string_view) {},
                        [](const DateTimeOffset&) {}),
                    std::invalid_argument);
            }

            SUBCASE("Integer invalid")
            {
                auto result = formatDataTypes.fromString("Integer");
                REQUIRE(result);
                REQUIRE(result.value().has_value());
                CHECK_THROWS_AS(
                    result.value()->match(
                        "not_an_integer",
                        [](const Decimal&) {},
                        [](std::int64_t) {},
                        [](bool) {},
                        [](std::string_view) {},
                        [](const DateTimeOffset&) {}),
                    std::invalid_argument);
            }

            SUBCASE("Boolean invalid")
            {
                auto result = formatDataTypes.fromString("Boolean");
                REQUIRE(result);
                REQUIRE(result.value().has_value());
                CHECK_THROWS_AS(
                    result.value()->match(
                        "not_a_boolean",
                        [](const Decimal&) {},
                        [](std::int64_t) {},
                        [](bool) {},
                        [](std::string_view) {},
                        [](const DateTimeOffset&) {}),
                    std::invalid_argument);
            }

            SUBCASE("DateTime invalid")
            {
                auto result = formatDataTypes.fromString("DateTime");
                REQUIRE(result);
                REQUIRE(result.value().has_value());
                CHECK_THROWS_AS(
                    result.value()->match(
                        "not_a_datetime",
                        [](const Decimal&) {},
                        [](std::int64_t) {},
                        [](bool) {},
                        [](std::string_view) {},
                        [](const DateTimeOffset&) {}),
                    std::invalid_argument);
            }
        }

        TEST_CASE("Match_Edge_Cases")
        {
            auto& iso = transport::ISO19848::instance();
            auto formatDataTypes = iso.formatDataTypes(transport::ISO19848Version::v2024);

            SUBCASE("negative decimal")
            {
                auto result = formatDataTypes.fromString("Decimal");
                REQUIRE(result);
                REQUIRE(result.value().has_value());
                auto decimalType = *result.value();

                bool called = false;
                Decimal value;
                CHECK_NOTHROW(decimalType.match(
                    "-456.789",
                    [&](const Decimal& d) {
                        called = true;
                        value = d;
                    },
                    [](std::int64_t) { FAIL("Expected decimal, got integer"); },
                    [](bool) { FAIL("Expected decimal, got boolean"); },
                    [](std::string_view) { FAIL("Expected decimal, got string"); },
                    [](const DateTimeOffset&) { FAIL("Expected decimal, got datetime"); }));

                CHECK(called);
                Decimal expected{ "-456.789" };
                CHECK((value - expected).abs() < Decimal{ "0.000001" });
            }

            SUBCASE("negative integer")
            {
                auto result = formatDataTypes.fromString("Integer");
                REQUIRE(result);
                REQUIRE(result.value().has_value());
                auto integerType = *result.value();

                bool called = false;
                int value = 0;
                CHECK_NOTHROW(integerType.match(
                    "-123",
                    [](const Decimal&) { FAIL("Expected integer, got decimal"); },
                    [&](std::int64_t i) {
                        called = true;
                        value = static_cast<int>(i);
                    },
                    [](bool) { FAIL("Expected integer, got boolean"); },
                    [](std::string_view) { FAIL("Expected integer, got string"); },
                    [](const DateTimeOffset&) { FAIL("Expected integer, got datetime"); }));

                CHECK(called);
                CHECK(value == -123);
            }

            SUBCASE("false boolean")
            {
                auto result = formatDataTypes.fromString("Boolean");
                REQUIRE(result);
                REQUIRE(result.value().has_value());
                auto booleanType = *result.value();

                bool called = false;
                bool value = true;
                CHECK_NOTHROW(booleanType.match(
                    "false",
                    [](const Decimal&) { FAIL("Expected boolean, got decimal"); },
                    [](std::int64_t) { FAIL("Expected boolean, got integer"); },
                    [&](bool b) {
                        called = true;
                        value = b;
                    },
                    [](std::string_view) { FAIL("Expected boolean, got string"); },
                    [](const DateTimeOffset&) { FAIL("Expected boolean, got datetime"); }));

                CHECK(called);
                CHECK_FALSE(value);
            }

            SUBCASE("empty string")
            {
                auto result = formatDataTypes.fromString("String");
                REQUIRE(result);
                REQUIRE(result.value().has_value());
                auto stringType = *result.value();

                bool called = false;
                std::string value = "not_empty";
                CHECK_NOTHROW(stringType.match(
                    "",
                    [](const Decimal&) { FAIL("Expected string, got decimal"); },
                    [](std::int64_t) { FAIL("Expected string, got integer"); },
                    [](bool) { FAIL("Expected string, got boolean"); },
                    [&](std::string_view s) {
                        called = true;
                        value = std::string{ s };
                    },
                    [](const DateTimeOffset&) { FAIL("Expected string, got datetime"); }));

                CHECK(called);
                CHECK(value.empty());
            }
        }

        TEST_CASE("Match_Decimal_Return_Values")
        {
            auto& iso = transport::ISO19848::instance();
            auto formatDataTypes = iso.formatDataTypes(transport::ISO19848Version::v2024);

            auto parseResult = formatDataTypes.fromString("Decimal");
            REQUIRE(parseResult);
            REQUIRE(parseResult.value().has_value());
            auto decimalType = *parseResult.value();

            SUBCASE("returns string")
            {
                std::string result;
                CHECK_NOTHROW(
                    result = decimalType.match(
                        "123.456",
                        [](const Decimal& d) -> std::string { return "decimal:" + d.toString(); },
                        [](std::int64_t) -> std::string {
                            FAIL("Expected decimal, got integer");
                            return "";
                        },
                        [](bool) -> std::string {
                            FAIL("Expected decimal, got boolean");
                            return "";
                        },
                        [](std::string_view) -> std::string {
                            FAIL("Expected decimal, got string");
                            return "";
                        },
                        [](const DateTimeOffset&) -> std::string {
                            FAIL("Expected decimal, got datetime");
                            return "";
                        }));
                CHECK(result == "decimal:" + Decimal{ "123.456" }.toString());
            }

            SUBCASE("returns double")
            {
                double result = 0.0;
                CHECK_NOTHROW(
                    result = decimalType.match(
                        "987.654",
                        [](const Decimal& d) -> double { return d.toDouble(); },
                        [](std::int64_t) -> double {
                            FAIL("Expected decimal, got integer");
                            return 0.0;
                        },
                        [](bool) -> double {
                            FAIL("Expected decimal, got boolean");
                            return 0.0;
                        },
                        [](std::string_view) -> double {
                            FAIL("Expected decimal, got string");
                            return 0.0;
                        },
                        [](const DateTimeOffset&) -> double {
                            FAIL("Expected decimal, got datetime");
                            return 0.0;
                        }));
                CHECK(std::abs(result - 987.654) < 0.000001);
            }
        }

        TEST_CASE("Match_Integer_Return_Values")
        {
            auto& iso = transport::ISO19848::instance();
            auto formatDataTypes = iso.formatDataTypes(transport::ISO19848Version::v2024);

            auto parseResult = formatDataTypes.fromString("Integer");
            REQUIRE(parseResult);
            REQUIRE(parseResult.value().has_value());
            auto integerType = *parseResult.value();

            SUBCASE("returns string")
            {
                std::string result;
                CHECK_NOTHROW(
                    result = integerType.match(
                        "42",
                        [](const Decimal&) -> std::string {
                            FAIL("Expected integer, got decimal");
                            return "";
                        },
                        [](std::int64_t i) -> std::string { return "integer:" + std::to_string(i); },
                        [](bool) -> std::string {
                            FAIL("Expected integer, got boolean");
                            return "";
                        },
                        [](std::string_view) -> std::string {
                            FAIL("Expected integer, got string");
                            return "";
                        },
                        [](const DateTimeOffset&) -> std::string {
                            FAIL("Expected integer, got datetime");
                            return "";
                        }));
                CHECK(result == "integer:42");
            }

            SUBCASE("returns squared value")
            {
                int result = 0;
                CHECK_NOTHROW(
                    result = integerType.match(
                        "7",
                        [](const Decimal&) -> int {
                            FAIL("Expected integer, got decimal");
                            return 0;
                        },
                        [](std::int64_t i) -> int { return static_cast<int>(i * i); },
                        [](bool) -> int {
                            FAIL("Expected integer, got boolean");
                            return 0;
                        },
                        [](std::string_view) -> int {
                            FAIL("Expected integer, got string");
                            return 0;
                        },
                        [](const DateTimeOffset&) -> int {
                            FAIL("Expected integer, got datetime");
                            return 0;
                        }));
                CHECK(result == 49);
            }
        }

        TEST_CASE("Match_Boolean_Return_Values")
        {
            auto& iso = transport::ISO19848::instance();
            auto formatDataTypes = iso.formatDataTypes(transport::ISO19848Version::v2024);

            auto parseResult = formatDataTypes.fromString("Boolean");
            REQUIRE(parseResult);
            REQUIRE(parseResult.value().has_value());
            auto booleanType = *parseResult.value();

            SUBCASE("returns string")
            {
                std::string result;
                CHECK_NOTHROW(
                    result = booleanType.match(
                        "true",
                        [](const Decimal&) -> std::string {
                            FAIL("Expected boolean, got decimal");
                            return "";
                        },
                        [](std::int64_t) -> std::string {
                            FAIL("Expected boolean, got integer");
                            return "";
                        },
                        [](bool b) -> std::string { return b ? "boolean:true" : "boolean:false"; },
                        [](std::string_view) -> std::string {
                            FAIL("Expected boolean, got string");
                            return "";
                        },
                        [](const DateTimeOffset&) -> std::string {
                            FAIL("Expected boolean, got datetime");
                            return "";
                        }));
                CHECK(result == "boolean:true");
            }

            SUBCASE("returns inverted bool")
            {
                bool result = true;
                CHECK_NOTHROW(
                    result = booleanType.match(
                        "false",
                        [](const Decimal&) -> bool {
                            FAIL("Expected boolean, got decimal");
                            return false;
                        },
                        [](std::int64_t) -> bool {
                            FAIL("Expected boolean, got integer");
                            return false;
                        },
                        [](bool b) -> bool { return !b; },
                        [](std::string_view) -> bool {
                            FAIL("Expected boolean, got string");
                            return false;
                        },
                        [](const DateTimeOffset&) -> bool {
                            FAIL("Expected boolean, got datetime");
                            return false;
                        }));
                CHECK(result);
            }
        }

        TEST_CASE("Match_String_Return_Values")
        {
            auto& iso = transport::ISO19848::instance();
            auto formatDataTypes = iso.formatDataTypes(transport::ISO19848Version::v2024);

            auto parseResult = formatDataTypes.fromString("String");
            REQUIRE(parseResult);
            REQUIRE(parseResult.value().has_value());
            auto stringType = *parseResult.value();

            SUBCASE("returns length")
            {
                size_t result = 0;
                CHECK_NOTHROW(
                    result = stringType.match(
                        "Hello World",
                        [](const Decimal&) -> size_t {
                            FAIL("Expected string, got decimal");
                            return 0;
                        },
                        [](std::int64_t) -> size_t {
                            FAIL("Expected string, got integer");
                            return 0;
                        },
                        [](bool) -> size_t {
                            FAIL("Expected string, got boolean");
                            return 0;
                        },
                        [](std::string_view s) -> size_t { return s.length(); },
                        [](const DateTimeOffset&) -> size_t {
                            FAIL("Expected string, got datetime");
                            return 0;
                        }));
                CHECK(result == 11);
            }

            SUBCASE("returns uppercase")
            {
                std::string result;
                CHECK_NOTHROW(
                    result = stringType.match(
                        "hello",
                        [](const Decimal&) -> std::string {
                            FAIL("Expected string, got decimal");
                            return "";
                        },
                        [](std::int64_t) -> std::string {
                            FAIL("Expected string, got integer");
                            return "";
                        },
                        [](bool) -> std::string {
                            FAIL("Expected string, got boolean");
                            return "";
                        },
                        [](std::string_view s) -> std::string {
                            std::string r(s);
                            std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c) {
                                return static_cast<char>(std::toupper(c));
                            });
                            return "string:" + r;
                        },
                        [](const DateTimeOffset&) -> std::string {
                            FAIL("Expected string, got datetime");
                            return "";
                        }));
                CHECK(result == "string:HELLO");
            }
        }

        TEST_CASE("Match_DateTime_Return_Values")
        {
            auto& iso = transport::ISO19848::instance();
            auto formatDataTypes = iso.formatDataTypes(transport::ISO19848Version::v2024);

            auto parseResult = formatDataTypes.fromString("DateTime");
            REQUIRE(parseResult);
            REQUIRE(parseResult.value().has_value());
            auto dateTimeType = *parseResult.value();

            SUBCASE("returns year string")
            {
                std::string result;
                CHECK_NOTHROW(
                    result = dateTimeType.match(
                        "1994-11-20T10:25:33Z",
                        [](const Decimal&) -> std::string {
                            FAIL("Expected datetime, got decimal");
                            return "";
                        },
                        [](std::int64_t) -> std::string {
                            FAIL("Expected datetime, got integer");
                            return "";
                        },
                        [](bool) -> std::string {
                            FAIL("Expected datetime, got boolean");
                            return "";
                        },
                        [](std::string_view) -> std::string {
                            FAIL("Expected datetime, got string");
                            return "";
                        },
                        [](const DateTimeOffset& dt) -> std::string {
                            return "datetime:" + std::to_string(dt.year());
                        }));
                CHECK(result == "datetime:1994");
            }

            SUBCASE("returns year check")
            {
                bool result = false;
                CHECK_NOTHROW(
                    result = dateTimeType.match(
                        "1994-11-20T10:25:33Z",
                        [](const Decimal&) -> bool {
                            FAIL("Expected datetime, got decimal");
                            return false;
                        },
                        [](std::int64_t) -> bool {
                            FAIL("Expected datetime, got integer");
                            return false;
                        },
                        [](bool) -> bool {
                            FAIL("Expected datetime, got boolean");
                            return false;
                        },
                        [](std::string_view) -> bool {
                            FAIL("Expected datetime, got string");
                            return false;
                        },
                        [](const DateTimeOffset& dt) -> bool { return dt.year() == 1994; }));
                CHECK(result);
            }
        }

        TEST_CASE("Match_Edge_Cases_And_Return_Types")
        {
            auto& iso = transport::ISO19848::instance();
            auto formatDataTypes = iso.formatDataTypes(transport::ISO19848Version::v2024);

            SUBCASE("negative decimal with custom struct")
            {
                struct DecimalInfo
                {
                    Decimal value;
                    bool isNegative;

                    std::string toString() const
                    {
                        return isNegative ? "negative:" + value.toString() : "positive:" + value.toString();
                    }
                };

                auto parseResult = formatDataTypes.fromString("Decimal");
                REQUIRE(parseResult);
                REQUIRE(parseResult.value().has_value());
                auto decimalType = *parseResult.value();

                DecimalInfo info{ Decimal{}, false };
                CHECK_NOTHROW(
                    info = decimalType.match(
                        "-456.789",
                        [](const Decimal& d) -> DecimalInfo { return { d, d < Decimal{ 0 } }; },
                        [](std::int64_t) -> DecimalInfo {
                            FAIL("Expected decimal, got integer");
                            return { Decimal{}, false };
                        },
                        [](bool) -> DecimalInfo {
                            FAIL("Expected decimal, got boolean");
                            return { Decimal{}, false };
                        },
                        [](std::string_view) -> DecimalInfo {
                            FAIL("Expected decimal, got string");
                            return { Decimal{}, false };
                        },
                        [](const DateTimeOffset&) -> DecimalInfo {
                            FAIL("Expected decimal, got datetime");
                            return { Decimal{}, false };
                        }));

                Decimal expected{ "-456.789" };
                CHECK((info.value - expected).abs() < Decimal{ "0.000001" });
                CHECK(info.isNegative);
                CHECK(info.toString() == "negative:" + expected.toString());
            }

            SUBCASE("integer fibonacci")
            {
                auto parseResult = formatDataTypes.fromString("Integer");
                REQUIRE(parseResult);
                REQUIRE(parseResult.value().has_value());
                auto integerType = *parseResult.value();

                std::vector<int> result;
                CHECK_NOTHROW(
                    result = integerType.match(
                        "5",
                        [](const Decimal&) -> std::vector<int> {
                            FAIL("Expected integer, got decimal");
                            return {};
                        },
                        [](std::int64_t n64) -> std::vector<int> {
                            std::vector<int> fib;
                            auto n = static_cast<int>(n64);
                            if (n <= 0)
                                return fib;
                            fib.push_back(0);
                            if (n > 1)
                                fib.push_back(1);
                            for (int i = 2; i < n; ++i)
                                fib.push_back(fib[static_cast<size_t>(i - 1)] + fib[static_cast<size_t>(i - 2)]);
                            return fib;
                        },
                        [](bool) -> std::vector<int> {
                            FAIL("Expected integer, got boolean");
                            return {};
                        },
                        [](std::string_view) -> std::vector<int> {
                            FAIL("Expected integer, got string");
                            return {};
                        },
                        [](const DateTimeOffset&) -> std::vector<int> {
                            FAIL("Expected integer, got datetime");
                            return {};
                        }));

                REQUIRE(result.size() == 5);
                CHECK(result[0] == 0);
                CHECK(result[1] == 1);
                CHECK(result[2] == 1);
                CHECK(result[3] == 2);
                CHECK(result[4] == 3);
            }

            SUBCASE("empty string optional return")
            {
                auto parseResult = formatDataTypes.fromString("String");
                REQUIRE(parseResult);
                REQUIRE(parseResult.value().has_value());
                auto stringType = *parseResult.value();

                std::optional<std::string> result;
                CHECK_NOTHROW(
                    result = stringType.match(
                        "",
                        [](const Decimal&) -> std::optional<std::string> {
                            FAIL("Expected string, got decimal");
                            return std::nullopt;
                        },
                        [](std::int64_t) -> std::optional<std::string> {
                            FAIL("Expected string, got integer");
                            return std::nullopt;
                        },
                        [](bool) -> std::optional<std::string> {
                            FAIL("Expected string, got boolean");
                            return std::nullopt;
                        },
                        [](std::string_view s) -> std::optional<std::string> {
                            return s.empty() ? std::nullopt : std::make_optional(std::string{ s });
                        },
                        [](const DateTimeOffset&) -> std::optional<std::string> {
                            FAIL("Expected string, got datetime");
                            return std::nullopt;
                        }));
                CHECK_FALSE(result.has_value());
            }
        }

        TEST_CASE("Value_Types_API")
        {
            SUBCASE("String value")
            {
                transport::Value value = transport::Value::String{ "test" };
                CHECK(value.string().has_value());
                CHECK_FALSE(value.decimal().has_value());
                CHECK_FALSE(value.integer().has_value());
                CHECK_FALSE(value.boolean().has_value());
                CHECK_FALSE(value.dateTime().has_value());
                CHECK(value.string().value() == "test");
            }

            SUBCASE("Decimal value")
            {
                transport::Value value = transport::Value::Decimal{ 3.14 };
                CHECK_FALSE(value.string().has_value());
                CHECK(value.decimal().has_value());
                CHECK_FALSE(value.integer().has_value());
                CHECK_FALSE(value.boolean().has_value());
                CHECK_FALSE(value.dateTime().has_value());
                CHECK(value.decimal().value() == Decimal(3.14));
            }

            SUBCASE("Integer value")
            {
                transport::Value value = transport::Value::Integer{ 42 };
                CHECK_FALSE(value.string().has_value());
                CHECK_FALSE(value.decimal().has_value());
                CHECK(value.integer().has_value());
                CHECK_FALSE(value.boolean().has_value());
                CHECK_FALSE(value.dateTime().has_value());
                CHECK(value.integer().value() == 42);
            }

            SUBCASE("Boolean value")
            {
                transport::Value value = transport::Value::Boolean{ true };
                CHECK_FALSE(value.string().has_value());
                CHECK_FALSE(value.decimal().has_value());
                CHECK_FALSE(value.integer().has_value());
                CHECK(value.boolean().has_value());
                CHECK_FALSE(value.dateTime().has_value());
                CHECK(value.boolean().value() == true);
            }

            SUBCASE("DateTime value")
            {
                auto dt = DateTimeOffset::now();
                transport::Value value = transport::Value::DateTime{ dt };
                CHECK_FALSE(value.string().has_value());
                CHECK_FALSE(value.decimal().has_value());
                CHECK_FALSE(value.integer().has_value());
                CHECK_FALSE(value.boolean().has_value());
                CHECK(value.dateTime().has_value());
                CHECK(value.dateTime().value() == dt);
            }
        }

        TEST_CASE("Value_Type_Accessor")
        {
            {
                transport::Value value = transport::Value::String{ "test" };
                CHECK(value.type() == transport::Value::Type::String);
            }
            {
                transport::Value value = transport::Value::Decimal{ 3.14 };
                CHECK(value.type() == transport::Value::Type::Decimal);
            }
            {
                transport::Value value = transport::Value::Integer{ 42 };
                CHECK(value.type() == transport::Value::Type::Integer);
            }
            {
                transport::Value value = transport::Value::Boolean{ true };
                CHECK(value.type() == transport::Value::Type::Boolean);
            }
            {
                transport::Value value = transport::Value::DateTime{ DateTimeOffset::now() };
                CHECK(value.type() == transport::Value::Type::DateTime);
            }
        }

        TEST_CASE("Value_Copy_Semantics")
        {
            SUBCASE("String copy")
            {
                transport::Value original = transport::Value::String{ "original" };
                transport::Value copy = original;
                CHECK(copy.string().has_value());
                CHECK(copy.string().value() == "original");
                CHECK(original.string().value() == copy.string().value());

                transport::Value assigned = transport::Value::Integer{ 0 };
                assigned = original;
                CHECK(assigned.string().has_value());
                CHECK(assigned.string().value() == "original");
            }

            SUBCASE("Integer copy")
            {
                transport::Value original = transport::Value::Integer{ 42 };
                transport::Value copy = original;
                CHECK(copy.integer().has_value());
                CHECK(copy.integer().value() == 42);
                CHECK(original.integer().value() == copy.integer().value());
            }

            SUBCASE("Decimal copy")
            {
                transport::Value original = transport::Value::Decimal{ 3.14 };
                transport::Value copy = original;
                CHECK(copy.decimal().has_value());
                CHECK(copy.decimal().value() == Decimal(3.14));
                CHECK(original.decimal().value() == copy.decimal().value());
            }

            SUBCASE("Boolean copy")
            {
                transport::Value original = transport::Value::Boolean{ true };
                transport::Value copy = original;
                CHECK(copy.boolean().has_value());
                CHECK(copy.boolean().value() == true);
                CHECK(original.boolean().value() == copy.boolean().value());
            }

            SUBCASE("DateTime copy")
            {
                auto dt = DateTimeOffset::now();
                transport::Value original = transport::Value::DateTime{ dt };
                transport::Value copy = original;
                CHECK(copy.dateTime().has_value());
                CHECK(original.dateTime().value() == copy.dateTime().value());
            }
        }

        TEST_CASE("Value_Move_Semantics")
        {
            SUBCASE("String move")
            {
                transport::Value original = transport::Value::String{ "original" };
                transport::Value moved = std::move(original);
                CHECK(moved.string().has_value());
                CHECK(moved.string().value() == "original");

                transport::Value moveAssigned = transport::Value::Integer{ 0 };
                transport::Value source = transport::Value::String{ "move_assigned" };
                moveAssigned = std::move(source);
                CHECK(moveAssigned.string().has_value());
                CHECK(moveAssigned.string().value() == "move_assigned");
            }

            SUBCASE("Integer move")
            {
                transport::Value original = transport::Value::Integer{ 42 };
                transport::Value moved = std::move(original);
                CHECK(moved.integer().has_value());
                CHECK(moved.integer().value() == 42);
            }

            SUBCASE("Decimal move")
            {
                transport::Value original = transport::Value::Decimal{ 3.14 };
                transport::Value moved = std::move(original);
                CHECK(moved.decimal().has_value());
                CHECK(moved.decimal().value() == Decimal(3.14));
            }

            SUBCASE("Boolean move")
            {
                transport::Value original = transport::Value::Boolean{ false };
                transport::Value moved = std::move(original);
                CHECK(moved.boolean().has_value());
                CHECK(moved.boolean().value() == false);
            }

            SUBCASE("DateTime move")
            {
                auto dt = DateTimeOffset::now();
                transport::Value original = transport::Value::DateTime{ dt };
                transport::Value moved = std::move(original);
                CHECK(moved.dateTime().has_value());
                CHECK(moved.dateTime().value() == dt);
            }
        }
    }
} // namespace dnv::vista::sdk::tests
