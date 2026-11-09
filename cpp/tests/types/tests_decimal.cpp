#include <doctest/doctest.h>

#include <dnv/VistaSDK.h>

#include <limits>

namespace dnv::vista::sdk::tests
{
    using namespace dnv::vista::sdk;

    TEST_SUITE("Decimal")
    {
        TEST_CASE("construction - default")
        {
            Decimal d;
            CHECK(d == 0);
            CHECK_FALSE(d < 0);
            CHECK_EQ(d.scale(), 0);
            CHECK_EQ(d.toString(), "0");
        }

        TEST_CASE("construction - integer")
        {
            Decimal d1{ 42 };
            CHECK_FALSE(d1 == 0);
            CHECK_FALSE(d1 < 0);
            CHECK_EQ(d1.toString(), "42");

            Decimal d2{ -123 };
            CHECK_FALSE(d2 == 0);
            CHECK(d2 < 0);
            CHECK_EQ(d2.toString(), "-123");

            Decimal d3{ 0 };
            CHECK(d3 == 0);
            CHECK_FALSE(d3 < 0);
            CHECK_EQ(d3.toString(), "0");

            Decimal d4{ std::numeric_limits<std::int64_t>::max() };
            CHECK_FALSE(d4 == 0);
            CHECK_FALSE(d4 < 0);
        }

        TEST_CASE("construction - float")
        {
            Decimal d1{ 123.456f };
            CHECK_FALSE(d1 == 0);
            CHECK_FALSE(d1 < 0);
            CHECK(d1 > 123.0);
            CHECK(d1 < 124.0);

            Decimal d2{ -123.456f };
            CHECK_FALSE(d2 == 0);
            CHECK(d2 < 0);
            CHECK(d2 < -123.0);
            CHECK(d2 > -124.0);

            Decimal d3{ 0.0f };
            CHECK(d3 == 0);
            CHECK_FALSE(d3 < 0);

            Decimal d4{ 0.001f };
            CHECK_FALSE(d4 == 0);
            CHECK_FALSE(d4 < 0);
            CHECK(d4 > 0.0);
            CHECK(d4 < 0.01);

            Decimal d5{ 123456789.0f };
            CHECK_FALSE(d5 == 0);
            CHECK_FALSE(d5 < 0);

            Decimal d6{ 42.0f };
            CHECK_FALSE(d6 == 0);
            CHECK_FALSE(d6 < 0);
            CHECK_EQ(d6.toString(), "42");

            Decimal d7{ 1e-10f };
            CHECK_FALSE(d7 == 0);
            CHECK_FALSE(d7 < 0);
        }

        TEST_CASE("construction - float IEEE754 artifact prevention")
        {
            Decimal d1{ 273.15f };
            CHECK_EQ(d1.toString(), "273.15");

            Decimal d2{ 380.4f };
            std::string str2 = d2.toString();
            CHECK(str2.substr(0, 5) == "380.4");

            Decimal d3{ 42.0f };
            CHECK_EQ(d3.toString(), "42");

            Decimal d4{ 100.0f };
            CHECK_EQ(d4.toString(), "100");

            Decimal d5{ 100000000.0f };
            std::string str5 = d5.toString();
            CHECK(str5.find('e') == std::string::npos);
            CHECK(str5.find('E') == std::string::npos);
            CHECK(d5 > 99999999.0);
            CHECK(d5 < 100000001.0);

            Decimal d6{ 0.001f };
            CHECK(d6 > 0.0);
            CHECK(d6 < 0.01);

            Decimal d7{ -273.15f };
            CHECK_EQ(d7.toString(), "-273.15");

            Decimal d8{ 99.99f };
            std::string str8 = d8.toString();
            CHECK(str8.substr(0, 5) == "99.99");

            Decimal d9{ 0.0f };
            CHECK_EQ(d9.toString(), "0");

            Decimal d10{ -0.0f };
            CHECK_EQ(d10.toString(), "0");
        }

        TEST_CASE("construction - double")
        {
            Decimal d1{ 123.456 };
            CHECK_FALSE(d1 == 0);
            CHECK_FALSE(d1 < 0);

            Decimal d2{ -123.456 };
            CHECK_FALSE(d2 == 0);
            CHECK(d2 < 0);

            Decimal d3{ 0.0 };
            CHECK(d3 == 0);
            CHECK_FALSE(d3 < 0);

            Decimal d4{ 0.001 };
            CHECK_FALSE(d4 == 0);
            CHECK_FALSE(d4 < 0);
        }

        TEST_CASE("construction - double IEEE754 artifact prevention")
        {
            Decimal d1{ 380.4 };
            CHECK_EQ(d1.toString(), "380.4");

            Decimal d2{ 42.0 };
            CHECK_EQ(d2.toString(), "42");

            Decimal d3{ 1000.0 };
            CHECK_EQ(d3.toString(), "1000");

            Decimal d4{ 100000000.0 };
            std::string str4 = d4.toString();
            CHECK(str4.find('e') == std::string::npos);
            CHECK(str4.find('E') == std::string::npos);
            CHECK_EQ(str4, "100000000");

            Decimal d5{ 1e15 };
            std::string str5 = d5.toString();
            CHECK(str5.find('e') == std::string::npos);
            CHECK(str5.find('E') == std::string::npos);

            Decimal d6{ 1e-10 };
            CHECK(d6 > 0.0);
            CHECK(d6 < 0.001);

            Decimal d7{ -380.4 };
            CHECK_EQ(d7.toString(), "-380.4");

            Decimal d8{ 0.5 };
            CHECK_EQ(d8.toString(), "0.5");

            Decimal d9{ 0.25 };
            CHECK_EQ(d9.toString(), "0.25");

            Decimal d10{ 0.125 };
            CHECK_EQ(d10.toString(), "0.125");

            Decimal d11{ 0.0 };
            CHECK_EQ(d11.toString(), "0");

            Decimal d12{ -0.0 };
            CHECK_EQ(d12.toString(), "0");
        }

        TEST_CASE("construction - string")
        {
            Decimal d1{ "123.456" };
            CHECK_EQ(d1.toString(), "123.456");

            Decimal d2{ "-789.123" };
            CHECK(d2 < 0);

            Decimal d3{ "0" };
            CHECK(d3 == 0);

            Decimal d4{ "0.0001" };
            CHECK_FALSE(d4 == 0);

            Decimal d5{ "123.1234567890123456789" };
            CHECK_FALSE(d5 == 0);
        }

        TEST_CASE("construction - const char*")
        {
            const char* str1 = "123.456";
            Decimal d1{ str1 };
            CHECK_EQ(d1.toString(), "123.456");

            const char* str2 = "-789.123";
            Decimal d2{ str2 };
            CHECK(d2 < 0);

            const char* str3 = "0";
            Decimal d3{ str3 };
            CHECK(d3 == 0);

            const char* str4 = "0.0001";
            Decimal d4{ str4 };
            CHECK_FALSE(d4 == 0);

            const char* invalid = "invalid";
            CHECK_THROWS_AS(Decimal{ invalid }, std::invalid_argument);

            const char* empty = "";
            CHECK_THROWS_AS(Decimal{ empty }, std::invalid_argument);
        }

        TEST_CASE("assignment - compound")
        {
            Decimal d1{ "100" };
            Decimal d2{ "25" };

            d1 += d2;
            CHECK_FALSE(d1 == 0);

            d1 -= Decimal{ "25" };

            d1 *= Decimal{ "2" };

            d1 /= Decimal{ "4" };
            CHECK_EQ(d1.toString(), "50");
            CHECK_FALSE(d1 == 0);
        }

        TEST_CASE("assignment - unary minus")
        {
            Decimal d1{ "123.45" };
            Decimal d2{ -d1 };

            CHECK_FALSE(d1 < 0);
            CHECK(d2 < 0);
            CHECK_EQ(d1.toString(), "123.45");

            Decimal d3{ -d2 };
            CHECK_FALSE(d3 < 0);
            CHECK_EQ(d3.toString(), "123.45");
        }

        TEST_CASE("conversion - toDouble")
        {
            Decimal d1{ 123.456 };
            double converted{ d1.toDouble() };
            CHECK(converted > 123.455);
            CHECK(converted < 123.457);

            Decimal d2{ -789.123 };
            converted = d2.toDouble();
            CHECK(converted > -789.124);
            CHECK(converted < -789.122);

            Decimal d3{ 0 };
            converted = d3.toDouble();
            CHECK_EQ(converted, 0.0);
        }

        TEST_CASE("conversion - toString")
        {
            CHECK_EQ(Decimal{ 123 }.toString(), "123");
            CHECK_EQ(Decimal{ -456 }.toString(), "-456");
            CHECK_EQ(Decimal{ 0 }.toString(), "0");

            Decimal d1{ "123.456" };
            CHECK_EQ(d1.toString(), "123.456");

            Decimal d2{ "-0.001" };
            CHECK_EQ(d2.toString(), "-0.001");
        }

        TEST_CASE("conversion - toBits")
        {
            Decimal zero{ 0 };
            auto zeroBits{ zero.toBits() };
            CHECK_EQ(zeroBits.size(), 4u);
            CHECK_EQ(zeroBits[0], 0u);
            CHECK_EQ(zeroBits[1], 0u);
            CHECK_EQ(zeroBits[2], 0u);
            CHECK_EQ(zeroBits[3], 0u);

            Decimal simpleInt{ 123 };
            auto intBits{ simpleInt.toBits() };
            CHECK_EQ(intBits[0], 123u);
            CHECK_EQ(intBits[1], 0u);
            CHECK_EQ(intBits[2], 0u);
            CHECK_EQ(intBits[3], 0u);

            Decimal decimal{ "123.45" };
            auto decimalBits{ decimal.toBits() };
            CHECK_EQ(decimalBits[0], 12345u);
            CHECK_EQ(decimalBits[1], 0u);
            CHECK_EQ(decimalBits[2], 0u);
            CHECK_EQ((decimalBits[3] >> 16) & 0xFF, 2u);

            Decimal negative{ "-456.789" };
            auto negativeBits{ negative.toBits() };
            CHECK_EQ(negativeBits[0], 456789u);
            CHECK_EQ(negativeBits[1], 0u);
            CHECK_EQ(negativeBits[2], 0u);
            CHECK_NE(static_cast<std::uint32_t>(negativeBits[3]) & internal::constants::DECIMAL_SIGN_MASK, 0u);
            CHECK_EQ((negativeBits[3] >> 16) & 0xFF, 3u);

            Decimal large{ "123456789012345.678" };
            auto largeBits{ large.toBits() };
            CHECK_NE(largeBits[0], 0u);

            Decimal small{ "0.001" };
            auto smallBits{ small.toBits() };
            CHECK_EQ(smallBits[0], 1u);
            CHECK_EQ(smallBits[1], 0u);
            CHECK_EQ(smallBits[2], 0u);
            CHECK_EQ((smallBits[3] >> 16) & 0xFF, 3u);
            CHECK_EQ(static_cast<std::uint32_t>(smallBits[3]) & internal::constants::DECIMAL_SIGN_MASK, 0u);
        }

        TEST_CASE("conversion - fromString")
        {
            Decimal result;

            CHECK(Decimal::fromString("123.456", result));
            CHECK_EQ(result.toString(), "123.456");
            CHECK_FALSE(result < 0);
            CHECK_FALSE(result == 0);

            CHECK(Decimal::fromString("-789.123", result));
            CHECK(result < 0);
            CHECK_FALSE(result == 0);
        }

        TEST_CASE("arithmetic - addition")
        {
            Decimal d1{ "123.45" };
            Decimal d2{ "67.89" };
            auto result{ d1 + d2 };

            CHECK_FALSE(result == 0);
            CHECK_FALSE(result < 0);

            Decimal d3{ "100" };
            Decimal d4{ "-50" };
            result = d3 + d4;
            CHECK_FALSE(result == 0);
            CHECK_FALSE(result < 0);

            result = d1 + Decimal{ 0 };
            CHECK_EQ(result.toString(), d1.toString());
        }

        TEST_CASE("arithmetic - subtraction")
        {
            Decimal d1{ "100" };
            Decimal d2{ "30" };
            Decimal result{ d1 - d2 };

            CHECK_FALSE(result == 0);
            CHECK_FALSE(result < 0);

            result = d2 - d1;
            CHECK_FALSE(result == 0);
            CHECK(result < 0);

            result = d1 - d1;
            CHECK(result == 0);
        }

        TEST_CASE("arithmetic - multiplication")
        {
            Decimal d1{ "12.5" };
            Decimal d2{ "8" };
            Decimal result{ d1 * d2 };

            CHECK_FALSE(result == 0);
            CHECK_FALSE(result < 0);

            Decimal d3{ "-5" };
            result = d1 * d3;
            CHECK_FALSE(result == 0);
            CHECK(result < 0);

            result = d1 * Decimal{ 0 };
            CHECK(result == 0);
        }

        TEST_CASE("arithmetic - multiplication mantissa overflow trims and rounds precision")
        {
            Decimal left{ "792281625142643.37593543950335" };
            Decimal right{ "3" };
            Decimal result{ left * right };

            CHECK_EQ(result.toString(), "2376844875427930.1278063185101");
        }

        TEST_CASE("construction - string preserves all 29 significant digits of the max mantissa")
        {
            Decimal maxVal{ "79228162514264337593543950335" };
            CHECK_EQ(maxVal.toString(), "79228162514264337593543950335");
            CHECK_EQ(maxVal.scale(), 0);
        }

        TEST_CASE("arithmetic - division")
        {
            Decimal d1{ "100" };
            Decimal d2{ "4" };
            Decimal result{ d1 / d2 };

            CHECK_FALSE(result == 0);
            CHECK_FALSE(result < 0);
            CHECK_EQ(result.toString(), "25");

            Decimal d3{ "-20" };
            result = d1 / d3;
            CHECK_FALSE(result == 0);
            CHECK(result < 0);
            CHECK_EQ(result.toString(), "-5");

            Decimal d4{ "10" };
            Decimal d5{ "3" };
            result = d4 / d5;
            CHECK(result > Decimal{ "3.3" });
            CHECK(result < Decimal{ "3.4" });

            Decimal d6{ "2" };
            Decimal d7{ "1.5" };
            result = d6 / d7;
            CHECK(result > Decimal{ "1.3" });
            CHECK(result < Decimal{ "1.4" });

            CHECK_THROWS_AS(d1 / Decimal{ 0 }, std::overflow_error);
        }

        TEST_CASE("arithmetic - division when dividend scale-up hits the overflow guard early")
        {
            Decimal dividend{ "79228162514264337593543950335" }; // max 96-bit mantissa, scale 0
            Decimal divisor{ "0.9999999999999999999999999999" }; // scale 28

            Decimal result{ dividend / divisor };

            CHECK(result > Decimal{ "70000000000000000000000000000" });
            CHECK(result < Decimal{ "79228162514264337593543950335" });
        }

        TEST_CASE("free functions - abs")
        {
            Decimal pos{ "123.45" };
            CHECK_EQ(abs(pos), Decimal{ "123.45" });

            Decimal neg{ "-123.45" };
            CHECK_EQ(abs(neg), Decimal{ "123.45" });

            Decimal zero{ "0" };
            CHECK_EQ(abs(zero), Decimal{ "0" });

            Decimal smallNeg{ "-0.0001" };
            CHECK_EQ(abs(smallNeg), Decimal{ "0.0001" });
        }

        TEST_CASE("free functions - sqrt")
        {
            CHECK_EQ(sqrt(Decimal{ "4" }), Decimal{ "2" });
            CHECK_EQ(sqrt(Decimal{ "9" }), Decimal{ "3" });
            CHECK_EQ(sqrt(Decimal{ "100" }), Decimal{ "10" });

            Decimal sqrtTwo = sqrt(Decimal{ "2" });
            CHECK(sqrtTwo > Decimal{ "1.414213" });
            CHECK(sqrtTwo < Decimal{ "1.414214" });

            CHECK_EQ(sqrt(Decimal{ "0" }), Decimal{ "0" });

            CHECK_THROWS_AS([[maybe_unused]] auto r = sqrt(Decimal{ "-1" }), std::domain_error);
        }

        TEST_CASE("free functions - round")
        {
            CHECK_EQ(round(Decimal{ "123.456" }, 2), Decimal{ "123.46" });
            CHECK_EQ(round(Decimal{ "123.456" }, 1), Decimal{ "123.5" });
            CHECK_EQ(round(Decimal{ "123.456" }, 0), Decimal{ "123" });

            Decimal value{ "2.5" };
            CHECK_EQ(round(value, 0, Decimal::RoundingMode::ToNearest), Decimal{ "2" });
            CHECK_EQ(round(value, 0, Decimal::RoundingMode::ToNearestTiesAway), Decimal{ "3" });
        }

        TEST_CASE("free functions - floor")
        {
            CHECK_EQ(floor(Decimal{ "123.9" }), Decimal{ "123" });
            CHECK_EQ(floor(Decimal{ "123.1" }), Decimal{ "123" });
            CHECK_EQ(floor(Decimal{ "-123.1" }), Decimal{ "-124" });
            CHECK_EQ(floor(Decimal{ "-123.9" }), Decimal{ "-124" });
            CHECK_EQ(floor(Decimal{ "123" }), Decimal{ "123" });
        }

        TEST_CASE("free functions - ceil")
        {
            CHECK_EQ(ceil(Decimal{ "123.1" }), Decimal{ "124" });
            CHECK_EQ(ceil(Decimal{ "123.9" }), Decimal{ "124" });
            CHECK_EQ(ceil(Decimal{ "-123.9" }), Decimal{ "-123" });
            CHECK_EQ(ceil(Decimal{ "-123.1" }), Decimal{ "-123" });
            CHECK_EQ(ceil(Decimal{ "123" }), Decimal{ "123" });
        }

        TEST_CASE("free functions - trunc")
        {
            CHECK_EQ(trunc(Decimal{ "123.456" }), Decimal{ "123" });
            CHECK_EQ(trunc(Decimal{ "123.999" }), Decimal{ "123" });
            CHECK_EQ(trunc(Decimal{ "-123.456" }), Decimal{ "-123" });
            CHECK_EQ(trunc(Decimal{ "-123.999" }), Decimal{ "-123" });
            CHECK_EQ(trunc(Decimal{ "123" }), Decimal{ "123" });
        }

        TEST_CASE("free functions - ADL support")
        {
            Decimal value{ "123.456" };
            Decimal negative{ "-123.456" };

            auto absResult = abs(negative);
            CHECK_EQ(absResult, Decimal{ "123.456" });

            auto sqrtResult = sqrt(Decimal{ "4" });
            CHECK_EQ(sqrtResult, Decimal{ "2" });

            auto roundResult = round(value, 2);
            CHECK_EQ(roundResult, Decimal{ "123.46" });

            auto floorResult = floor(value);
            CHECK_EQ(floorResult, Decimal{ "123" });

            auto ceilingResult = ceil(value);
            CHECK_EQ(ceilingResult, Decimal{ "124" });

            auto truncResult = trunc(value);
            CHECK_EQ(truncResult, Decimal{ "123" });
        }

        TEST_CASE("comparison - equality")
        {
            Decimal d1{ "123.45" };
            Decimal d2{ "123.45" };
            Decimal d3{ "123.46" };

            CHECK(d1 == d2);
            CHECK_FALSE(d1 == d3);
            CHECK_FALSE(d1 != d2);
            CHECK(d1 != d3);

            Decimal zero1{ 0 };
            Decimal zero2{ "0.0" };
            CHECK(zero1 == zero2);
        }

        TEST_CASE("comparison - ordering")
        {
            Decimal d1{ "100" };
            Decimal d2{ "200" };
            Decimal d3{ "-50" };

            CHECK(d1 < d2);
            CHECK_FALSE(d2 < d1);
            CHECK(d3 < d1);

            CHECK(d2 > d1);
            CHECK_FALSE(d1 > d2);
            CHECK(d1 > d3);

            CHECK(d1 <= d2);
            CHECK(d1 <= Decimal{ "100" });
            CHECK_FALSE(d2 <= d1);

            CHECK(d2 >= d1);
            CHECK(d1 >= Decimal{ "100" });
            CHECK_FALSE(d1 >= d2);
        }

        TEST_CASE("comparison - floating-point")
        {
            Decimal d1{ 123.456 };
            Decimal d2{ -123.456 };
            Decimal zero{};

            CHECK(d1 == 123.456);
            CHECK(d2 == -123.456);
            CHECK(zero == 0.0);
            CHECK_FALSE(d1 == 123.457);

            CHECK_FALSE(d1 != 123.456);
            CHECK(d1 != 123.457);
            CHECK(d2 != 123.456);

            CHECK(d1 < 123.457);
            CHECK_FALSE(d1 < 123.456);
            CHECK(d2 < 0.0);

            CHECK(d1 <= 123.457);
            CHECK(d1 <= 123.456);
            CHECK_FALSE(d1 <= 123.455);

            CHECK_FALSE(d1 > 123.457);
            CHECK_FALSE(d1 > 123.456);
            CHECK(d1 > 123.455);

            CHECK_FALSE(d1 >= 123.457);
            CHECK(d1 >= 123.456);
            CHECK(d1 >= 123.455);
        }

        TEST_CASE("comparison - NaN always false")
        {
            Decimal d{ 123.456 };
            double nan = std::numeric_limits<double>::quiet_NaN();

            CHECK_FALSE(d == nan);
            CHECK(d != nan);
            CHECK_FALSE(d < nan);
            CHECK_FALSE(d <= nan);
            CHECK_FALSE(d > nan);
            CHECK_FALSE(d >= nan);
        }

        TEST_CASE("comparison - signed integer")
        {
            Decimal d1{ 42 };
            Decimal d2{ -42 };
            Decimal d3{ 42.5 };
            Decimal zero{};

            CHECK(d1 == std::int64_t{ 42 });
            CHECK(d2 == std::int64_t{ -42 });
            CHECK(zero == std::int64_t{ 0 });
            CHECK_FALSE(d1 == std::int64_t{ 43 });
            CHECK_FALSE(d3 == std::int64_t{ 42 });

            CHECK_FALSE(d1 != std::int64_t{ 42 });
            CHECK(d1 != std::int64_t{ 43 });
            CHECK(d3 != std::int64_t{ 42 });

            CHECK(d1 < std::int64_t{ 43 });
            CHECK_FALSE(d1 < std::int64_t{ 42 });
            CHECK_FALSE(d1 < std::int64_t{ 41 });
            CHECK(d2 < std::int64_t{ 0 });

            CHECK_FALSE(d1 > std::int64_t{ 43 });
            CHECK_FALSE(d1 > std::int64_t{ 42 });
            CHECK(d1 > std::int64_t{ 41 });

            CHECK(d1 == 42);
            CHECK(d1 < 43);
            CHECK(d1 > 41);
        }

        TEST_CASE("comparison - unsigned integer")
        {
            Decimal d1{ 42 };
            Decimal d2{ -42 };
            Decimal d3{ 42.5 };
            Decimal zero{};

            CHECK(d1 == std::uint64_t{ 42 });
            CHECK(zero == std::uint64_t{ 0 });
            CHECK_FALSE(d1 == std::uint64_t{ 43 });
            CHECK_FALSE(d2 == std::uint64_t{ 42 });
            CHECK_FALSE(d3 == std::uint64_t{ 42 });

            CHECK_FALSE(d1 != std::uint64_t{ 42 });
            CHECK(d1 != std::uint64_t{ 43 });
            CHECK(d2 != std::uint64_t{ 42 });
            CHECK(d3 != std::uint64_t{ 42 });

            CHECK(d1 < std::uint64_t{ 43 });
            CHECK_FALSE(d1 < std::uint64_t{ 42 });
            CHECK_FALSE(d1 < std::uint64_t{ 41 });
            CHECK(d2 < std::uint64_t{ 42 });

            CHECK_FALSE(d1 > std::uint64_t{ 43 });
            CHECK_FALSE(d1 > std::uint64_t{ 42 });
            CHECK(d1 > std::uint64_t{ 41 });
            CHECK_FALSE(d2 > std::uint64_t{ 42 });
        }

        TEST_CASE("comparison - edge cases")
        {
            Decimal large{ "999999999999999999999999" };
            Decimal small{ "0.000000000001" };
            Decimal zero{};

            CHECK_FALSE(large == std::numeric_limits<std::int64_t>::max());
            CHECK(large > std::numeric_limits<std::int64_t>::max());

            Decimal negative_large{ "-999999999999999999999999" };
            CHECK_FALSE(negative_large == std::numeric_limits<std::int64_t>::min());
            CHECK(negative_large < std::numeric_limits<std::int64_t>::min());

            CHECK_FALSE(small == 0);
            CHECK(small > 0);
            CHECK(small != std::uint64_t{ 0 });
            CHECK_FALSE(small == std::uint64_t{ 0 });

            CHECK(zero == 0);
            CHECK(zero == std::int64_t{ 0 });
            CHECK(zero == std::uint64_t{ 0 });
            CHECK(zero == 0.0);
            CHECK(zero == 0.0f);
        }

        TEST_CASE("precision - max precision")
        {
            std::string maxPrecision{ "1.2345678901234567890123456789" };
            Decimal d1;
            CHECK(Decimal::fromString(maxPrecision, d1));
            CHECK_FALSE(d1 == 0);

            std::string tooPrecise{ "1.23456789012345678901234567890" };
            CHECK(Decimal::fromString(tooPrecise, d1));
        }

        TEST_CASE("precision - large numbers")
        {
            Decimal d1{ std::numeric_limits<std::int64_t>::max() };
            CHECK_FALSE(d1 == 0);
            CHECK_FALSE(d1 < 0);

            Decimal d2{ std::numeric_limits<std::int64_t>::min() };
            CHECK_FALSE(d2 == 0);
            CHECK(d2 < 0);
        }

        TEST_CASE("precision - very small numbers")
        {
            Decimal d1{ "0.0000000000000000000000000001" };
            CHECK_FALSE(d1 == 0);
            CHECK_FALSE(d1 < 0);
            CHECK_EQ(d1.scale(), 28);
        }

        TEST_CASE("stress test - large operations")
        {
            Decimal accumulator{ "0" };
            Decimal increment{ "0.001" };

            for (int i{ 0 }; i < 1000; ++i)
                accumulator += increment;

            CHECK_FALSE(accumulator == 0);
            CHECK_FALSE(accumulator < 0);
        }

        TEST_CASE("utilities - decimalPlacesCount")
        {
            CHECK_EQ(Decimal{ "0" }.decimalPlacesCount(), 0);
            CHECK_EQ(Decimal{ "0.0" }.decimalPlacesCount(), 0);
            CHECK_EQ(Decimal{ "0.000" }.decimalPlacesCount(), 0);

            CHECK_EQ(Decimal{ "123" }.decimalPlacesCount(), 0);
            CHECK_EQ(Decimal{ "-456" }.decimalPlacesCount(), 0);

            CHECK_EQ(Decimal{ "123.456" }.decimalPlacesCount(), 3);
            CHECK_EQ(Decimal{ "-789.123" }.decimalPlacesCount(), 3);
            CHECK_EQ(Decimal{ "0.001" }.decimalPlacesCount(), 3);
            CHECK_EQ(Decimal{ "0.5" }.decimalPlacesCount(), 1);

            CHECK_EQ(Decimal{ "123.4500" }.decimalPlacesCount(), 2);
            CHECK_EQ(Decimal{ "123.000" }.decimalPlacesCount(), 0);
            CHECK_EQ(Decimal{ "456.7800" }.decimalPlacesCount(), 2);
            CHECK_EQ(Decimal{ "0.1000" }.decimalPlacesCount(), 1);

            CHECK_EQ(Decimal{ "123.4560" }.decimalPlacesCount(), 3);
            CHECK_EQ(Decimal{ "123.4000" }.decimalPlacesCount(), 1);
            CHECK_EQ(Decimal{ "999.9990" }.decimalPlacesCount(), 3);

            CHECK_EQ(Decimal{ "0.01" }.decimalPlacesCount(), 2);
            CHECK_EQ(Decimal{ "0.001" }.decimalPlacesCount(), 3);
            CHECK_EQ(Decimal{ "0.0001" }.decimalPlacesCount(), 4);

            CHECK_EQ(Decimal{ "123.123456789" }.decimalPlacesCount(), 9);
            CHECK_EQ(Decimal{ "0.123456789012345678901234567" }.decimalPlacesCount(), 27);
            CHECK_EQ(Decimal{ "1.1234567890123456789012345000" }.decimalPlacesCount(), 25);
            CHECK_EQ(Decimal{ "0.0000000000000000000000000001" }.decimalPlacesCount(), 28);

            Decimal d24{ "123.4500" };
            CHECK_EQ(d24.scale(), 2);
            CHECK_EQ(d24.decimalPlacesCount(), 2);

            Decimal d25{ "789.12300" };
            CHECK_EQ(d25.scale(), 3);
            CHECK_EQ(d25.decimalPlacesCount(), 3);

            Decimal d26{ 42 };
            CHECK_EQ(d26.scale(), 0);
            CHECK_EQ(d26.decimalPlacesCount(), 0);

            Decimal d27{ 123.456 };
            CHECK(d27.decimalPlacesCount() >= 3);

            CHECK_EQ(Decimal{ "-123.456" }.decimalPlacesCount(), 3);
            CHECK_EQ(Decimal{ "-0.001" }.decimalPlacesCount(), 3);
            CHECK_EQ(Decimal{ "-123.4500" }.decimalPlacesCount(), 2);
        }

        TEST_CASE("maths - binary representation")
        {
            Decimal d1{ 123.456 };
            auto bits{ d1.toBits() };
            CHECK_EQ(bits.size(), 4u);

            Decimal zero{ 0 };
            auto zeroBits{ zero.toBits() };
            CHECK_EQ(zeroBits[0], 0u);
            CHECK_EQ(zeroBits[1], 0u);
            CHECK_EQ(zeroBits[2], 0u);
            CHECK_EQ(zeroBits[3], 0u);

            Decimal negative{ -123.456 };
            auto negativeBits{ negative.toBits() };
            CHECK_NE(static_cast<std::uint32_t>(negativeBits[3]) & internal::constants::DECIMAL_SIGN_MASK, 0u);
        }

        TEST_CASE("maths - trunc method")
        {
            CHECK_EQ(Decimal{ "123.789" }.trunc().toString(), "123");
            CHECK_EQ(Decimal{ "-123.789" }.trunc().toString(), "-123");
            CHECK_EQ(Decimal{ "0.123" }.trunc().toString(), "0");
            CHECK_EQ(Decimal{ "123" }.trunc().toString(), "123");
            CHECK_EQ(Decimal{ "456.789" }.trunc().toString(), "456");
        }

        TEST_CASE("maths - floor method")
        {
            CHECK_EQ(Decimal{ "123.789" }.floor().toString(), "123");
            CHECK_EQ(Decimal{ "-123.789" }.floor().toString(), "-124");
            CHECK_EQ(Decimal{ "123.001" }.floor().toString(), "123");
            CHECK_EQ(Decimal{ "-123.001" }.floor().toString(), "-124");
            CHECK_EQ(Decimal{ "123" }.floor().toString(), "123");
        }

        TEST_CASE("maths - ceil method")
        {
            CHECK_EQ(Decimal{ "123.123" }.ceil().toString(), "124");
            CHECK_EQ(Decimal{ "-123.123" }.ceil().toString(), "-123");
            CHECK_EQ(Decimal{ "123.001" }.ceil().toString(), "124");
            CHECK_EQ(Decimal{ "-123.001" }.ceil().toString(), "-123");
            CHECK_EQ(Decimal{ "123" }.ceil().toString(), "123");
        }

        TEST_CASE("maths - round method basic")
        {
            CHECK_EQ(Decimal{ "123.4" }.round().toString(), "123");
            CHECK_EQ(Decimal{ "123.6" }.round().toString(), "124");
            CHECK_EQ(Decimal{ "123.5" }.round().toString(), "124");
            CHECK_EQ(Decimal{ "-123.4" }.round().toString(), "-123");
            CHECK_EQ(Decimal{ "-123.6" }.round().toString(), "-124");
        }

        TEST_CASE("maths - round with decimal places")
        {
            Decimal d1{ "123.4567" };
            CHECK_EQ(d1.round(2).toString(), "123.46");
            CHECK_EQ(d1.round(1).toString(), "123.5");
            CHECK_EQ(d1.round(0).toString(), "123");

            Decimal d2{ "-123.4567" };
            CHECK_EQ(d2.round(2).toString(), "-123.46");

            Decimal d4{ "123.45" };
            CHECK_EQ(d4.round(5).toString(), "123.45");
        }

        TEST_CASE("maths - abs method")
        {
            CHECK_EQ(Decimal{ "123.456" }.abs().toString(), "123.456");
            CHECK_FALSE(Decimal{ "123.456" }.abs() < 0);

            CHECK_EQ(Decimal{ "-123.456" }.abs().toString(), "123.456");
            CHECK_FALSE(Decimal{ "-123.456" }.abs() < 0);

            CHECK_EQ(Decimal{ "0" }.abs().toString(), "0");
            CHECK(Decimal{ "0" }.abs() == 0);

            CHECK_FALSE(Decimal{ "-0.0" }.abs() < 0);

            Decimal d6{ "-0.000000000000000000000000001" };
            CHECK_FALSE(d6.abs() < 0);
            CHECK_FALSE(d6.abs() == 0);
        }

        TEST_CASE("maths - sqrt method")
        {
            CHECK_EQ(Decimal{ "4" }.sqrt(), Decimal{ 2 });
            CHECK_EQ(Decimal{ "9" }.sqrt(), Decimal{ 3 });
            CHECK_EQ(Decimal{ "100" }.sqrt(), Decimal{ 10 });
            CHECK_EQ(Decimal{ "10000" }.sqrt(), Decimal{ 100 });
            CHECK_EQ(Decimal{ "0" }.sqrt().toString(), "0");
            CHECK_EQ(Decimal{ "1" }.sqrt().toString(), "1");

            Decimal sqrtTwo = Decimal{ "2" }.sqrt();
            CHECK(sqrtTwo > Decimal{ "1.414" });
            CHECK(sqrtTwo < Decimal{ "1.415" });

            Decimal sqrtThree = Decimal{ "3" }.sqrt();
            CHECK(sqrtThree > Decimal{ "1.732" });
            CHECK(sqrtThree < Decimal{ "1.733" });

            CHECK_EQ(Decimal{ "2.25" }.sqrt(), Decimal{ "1.5" });

            CHECK_THROWS_AS([[maybe_unused]] auto r = Decimal{ "-4" }.sqrt(), std::domain_error);
        }

        TEST_CASE("maths - sqrt edge cases small perfect squares")
        {
            CHECK_EQ(Decimal{ "0.01" }.sqrt(), Decimal{ "0.1" });
            CHECK_EQ(Decimal{ "0.04" }.sqrt(), Decimal{ "0.2" });
            CHECK_EQ(Decimal{ "0.09" }.sqrt(), Decimal{ "0.3" });
            CHECK_EQ(Decimal{ "0.16" }.sqrt(), Decimal{ "0.4" });
            CHECK_EQ(Decimal{ "0.25" }.sqrt(), Decimal{ "0.5" });
            CHECK_EQ(Decimal{ "0.36" }.sqrt(), Decimal{ "0.6" });
            CHECK_EQ(Decimal{ "0.49" }.sqrt(), Decimal{ "0.7" });
            CHECK_EQ(Decimal{ "0.64" }.sqrt(), Decimal{ "0.8" });
            CHECK_EQ(Decimal{ "0.81" }.sqrt(), Decimal{ "0.9" });

            CHECK_EQ(Decimal{ "1.44" }.sqrt(), Decimal{ "1.2" });
            CHECK_EQ(Decimal{ "1.69" }.sqrt(), Decimal{ "1.3" });
            CHECK_EQ(Decimal{ "6.25" }.sqrt(), Decimal{ "2.5" });
            CHECK_EQ(Decimal{ "12.25" }.sqrt(), Decimal{ "3.5" });
            CHECK_EQ(Decimal{ "20.25" }.sqrt(), Decimal{ "4.5" });

            CHECK_EQ(Decimal{ "400" }.sqrt(), Decimal{ "20" });
            CHECK_EQ(Decimal{ "625" }.sqrt(), Decimal{ "25" });
            CHECK_EQ(Decimal{ "900" }.sqrt(), Decimal{ "30" });
            CHECK_EQ(Decimal{ "2500" }.sqrt(), Decimal{ "50" });
            CHECK_EQ(Decimal{ "1000000" }.sqrt(), Decimal{ "1000" });
        }

        TEST_CASE("maths - sqrt non-perfect squares")
        {
            auto check = [](const char* val, const char* lo, const char* hi) {
                auto r = Decimal{ val }.sqrt();
                CHECK(r > Decimal{ lo });
                CHECK(r < Decimal{ hi });
            };

            check("0.5", "0.707", "0.708");
            check("1.5", "1.224", "1.225");
            check("2", "1.414", "1.415");
            check("3", "1.732", "1.733");
            check("5", "2.236", "2.237");
            check("6", "2.449", "2.450");
            check("7", "2.645", "2.647");
            check("8", "2.828", "2.829");
            check("10", "3.162", "3.163");
            check("50", "7.071", "7.072");
            check("99", "9.949", "9.950");
            check("101", "10.049", "10.050");
            check("200", "14.142", "14.143");
            check("500", "22.360", "22.361");
            check("1000", "31.622", "31.623");
            check("9999", "99.99", "100.00");
            check("10001", "100.00", "100.01");
            check("999999", "999.99", "1000.00");
            check("1000001", "1000.00", "1000.01");
        }

        TEST_CASE("maths - sqrt decimal precision")
        {
            CHECK_EQ(Decimal{ "4.41" }.sqrt(), Decimal{ "2.1" });
            CHECK_EQ(Decimal{ "5.29" }.sqrt(), Decimal{ "2.3" });
            CHECK_EQ(Decimal{ "7.84" }.sqrt(), Decimal{ "2.8" });
            CHECK_EQ(Decimal{ "9.61" }.sqrt(), Decimal{ "3.1" });
            CHECK_EQ(Decimal{ "11.56" }.sqrt(), Decimal{ "3.4" });
            CHECK_EQ(Decimal{ "14.44" }.sqrt(), Decimal{ "3.8" });
            CHECK_EQ(Decimal{ "17.64" }.sqrt(), Decimal{ "4.2" });

            CHECK_EQ(Decimal{ "0.0001" }.sqrt(), Decimal{ "0.01" });
            CHECK_EQ(Decimal{ "0.0004" }.sqrt(), Decimal{ "0.02" });
            CHECK_EQ(Decimal{ "0.0009" }.sqrt(), Decimal{ "0.03" });
            CHECK_EQ(Decimal{ "0.0016" }.sqrt(), Decimal{ "0.04" });
            CHECK_EQ(Decimal{ "0.0025" }.sqrt(), Decimal{ "0.05" });
        }

        TEST_CASE("rounding - ToNearest (banker's)")
        {
            CHECK_EQ(Decimal{ "2.4" }.round(0, Decimal::RoundingMode::ToNearest).toString(), "2");
            CHECK_EQ(Decimal{ "2.5" }.round(0, Decimal::RoundingMode::ToNearest).toString(), "2");
            CHECK_EQ(Decimal{ "2.6" }.round(0, Decimal::RoundingMode::ToNearest).toString(), "3");
            CHECK_EQ(Decimal{ "3.5" }.round(0, Decimal::RoundingMode::ToNearest).toString(), "4");
            CHECK_EQ(Decimal{ "4.5" }.round(0, Decimal::RoundingMode::ToNearest).toString(), "4");
            CHECK_EQ(Decimal{ "5.5" }.round(0, Decimal::RoundingMode::ToNearest).toString(), "6");

            CHECK_EQ(Decimal{ "-2.4" }.round(0, Decimal::RoundingMode::ToNearest).toString(), "-2");
            CHECK_EQ(Decimal{ "-2.5" }.round(0, Decimal::RoundingMode::ToNearest).toString(), "-2");
            CHECK_EQ(Decimal{ "-2.6" }.round(0, Decimal::RoundingMode::ToNearest).toString(), "-3");
            CHECK_EQ(Decimal{ "-3.5" }.round(0, Decimal::RoundingMode::ToNearest).toString(), "-4");
            CHECK_EQ(Decimal{ "-4.5" }.round(0, Decimal::RoundingMode::ToNearest).toString(), "-4");

            CHECK_EQ(Decimal{ "123.456" }.round(2, Decimal::RoundingMode::ToNearest).toString(), "123.46");
            CHECK_EQ(Decimal{ "123.455" }.round(2, Decimal::RoundingMode::ToNearest).toString(), "123.46");
            CHECK_EQ(Decimal{ "123.445" }.round(2, Decimal::RoundingMode::ToNearest).toString(), "123.44");
            CHECK_EQ(Decimal{ "123.454" }.round(2, Decimal::RoundingMode::ToNearest).toString(), "123.45");

            CHECK_EQ(Decimal{ "2.51" }.round(0, Decimal::RoundingMode::ToNearest).toString(), "3");
            CHECK_EQ(Decimal{ "-2.51" }.round(0, Decimal::RoundingMode::ToNearest).toString(), "-3");
        }

        TEST_CASE("rounding - ToNearestTiesAway")
        {
            CHECK_EQ(Decimal{ "2.4" }.round(0, Decimal::RoundingMode::ToNearestTiesAway).toString(), "2");
            CHECK_EQ(Decimal{ "2.5" }.round(0, Decimal::RoundingMode::ToNearestTiesAway).toString(), "3");
            CHECK_EQ(Decimal{ "2.6" }.round(0, Decimal::RoundingMode::ToNearestTiesAway).toString(), "3");
            CHECK_EQ(Decimal{ "3.5" }.round(0, Decimal::RoundingMode::ToNearestTiesAway).toString(), "4");
            CHECK_EQ(Decimal{ "4.5" }.round(0, Decimal::RoundingMode::ToNearestTiesAway).toString(), "5");

            CHECK_EQ(Decimal{ "-2.4" }.round(0, Decimal::RoundingMode::ToNearestTiesAway).toString(), "-2");
            CHECK_EQ(Decimal{ "-2.5" }.round(0, Decimal::RoundingMode::ToNearestTiesAway).toString(), "-3");
            CHECK_EQ(Decimal{ "-2.6" }.round(0, Decimal::RoundingMode::ToNearestTiesAway).toString(), "-3");
            CHECK_EQ(Decimal{ "-3.5" }.round(0, Decimal::RoundingMode::ToNearestTiesAway).toString(), "-4");

            CHECK_EQ(Decimal{ "123.455" }.round(2, Decimal::RoundingMode::ToNearestTiesAway).toString(), "123.46");
            CHECK_EQ(Decimal{ "123.445" }.round(2, Decimal::RoundingMode::ToNearestTiesAway).toString(), "123.45");
            CHECK_EQ(Decimal{ "-123.455" }.round(2, Decimal::RoundingMode::ToNearestTiesAway).toString(), "-123.46");
        }

        TEST_CASE("rounding - ToZero")
        {
            CHECK_EQ(Decimal{ "2.1" }.round(0, Decimal::RoundingMode::ToZero).toString(), "2");
            CHECK_EQ(Decimal{ "2.5" }.round(0, Decimal::RoundingMode::ToZero).toString(), "2");
            CHECK_EQ(Decimal{ "2.9" }.round(0, Decimal::RoundingMode::ToZero).toString(), "2");
            CHECK_EQ(Decimal{ "123.999" }.round(0, Decimal::RoundingMode::ToZero).toString(), "123");

            CHECK_EQ(Decimal{ "-2.1" }.round(0, Decimal::RoundingMode::ToZero).toString(), "-2");
            CHECK_EQ(Decimal{ "-2.5" }.round(0, Decimal::RoundingMode::ToZero).toString(), "-2");
            CHECK_EQ(Decimal{ "-2.9" }.round(0, Decimal::RoundingMode::ToZero).toString(), "-2");
            CHECK_EQ(Decimal{ "-123.999" }.round(0, Decimal::RoundingMode::ToZero).toString(), "-123");

            CHECK_EQ(Decimal{ "123.456" }.round(2, Decimal::RoundingMode::ToZero).toString(), "123.45");
            CHECK_EQ(Decimal{ "123.459" }.round(2, Decimal::RoundingMode::ToZero).toString(), "123.45");
            CHECK_EQ(Decimal{ "-123.456" }.round(2, Decimal::RoundingMode::ToZero).toString(), "-123.45");

            CHECK_EQ(
                Decimal{ "123.789" }.round(0, Decimal::RoundingMode::ToZero).toString(),
                Decimal{ "123.789" }.trunc().toString());
            CHECK_EQ(
                Decimal{ "-123.789" }.round(0, Decimal::RoundingMode::ToZero).toString(),
                Decimal{ "-123.789" }.trunc().toString());
        }

        TEST_CASE("rounding - ToPositiveInfinity")
        {
            CHECK_EQ(Decimal{ "2.1" }.round(0, Decimal::RoundingMode::ToPositiveInfinity).toString(), "3");
            CHECK_EQ(Decimal{ "2.5" }.round(0, Decimal::RoundingMode::ToPositiveInfinity).toString(), "3");
            CHECK_EQ(Decimal{ "2.9" }.round(0, Decimal::RoundingMode::ToPositiveInfinity).toString(), "3");
            CHECK_EQ(Decimal{ "2.0" }.round(0, Decimal::RoundingMode::ToPositiveInfinity).toString(), "2");

            CHECK_EQ(Decimal{ "-2.1" }.round(0, Decimal::RoundingMode::ToPositiveInfinity).toString(), "-2");
            CHECK_EQ(Decimal{ "-2.5" }.round(0, Decimal::RoundingMode::ToPositiveInfinity).toString(), "-2");
            CHECK_EQ(Decimal{ "-2.9" }.round(0, Decimal::RoundingMode::ToPositiveInfinity).toString(), "-2");
            CHECK_EQ(Decimal{ "-2.0" }.round(0, Decimal::RoundingMode::ToPositiveInfinity).toString(), "-2");

            CHECK_EQ(Decimal{ "123.451" }.round(2, Decimal::RoundingMode::ToPositiveInfinity).toString(), "123.46");
            CHECK_EQ(Decimal{ "-123.451" }.round(2, Decimal::RoundingMode::ToPositiveInfinity).toString(), "-123.45");

            CHECK_EQ(
                Decimal{ "123.001" }.round(0, Decimal::RoundingMode::ToPositiveInfinity).toString(),
                Decimal{ "123.001" }.ceil().toString());
            CHECK_EQ(
                Decimal{ "-123.001" }.round(0, Decimal::RoundingMode::ToPositiveInfinity).toString(),
                Decimal{ "-123.001" }.ceil().toString());
        }

        TEST_CASE("rounding - ToNegativeInfinity")
        {
            CHECK_EQ(Decimal{ "2.1" }.round(0, Decimal::RoundingMode::ToNegativeInfinity).toString(), "2");
            CHECK_EQ(Decimal{ "2.5" }.round(0, Decimal::RoundingMode::ToNegativeInfinity).toString(), "2");
            CHECK_EQ(Decimal{ "2.9" }.round(0, Decimal::RoundingMode::ToNegativeInfinity).toString(), "2");
            CHECK_EQ(Decimal{ "2.0" }.round(0, Decimal::RoundingMode::ToNegativeInfinity).toString(), "2");

            CHECK_EQ(Decimal{ "-2.1" }.round(0, Decimal::RoundingMode::ToNegativeInfinity).toString(), "-3");
            CHECK_EQ(Decimal{ "-2.5" }.round(0, Decimal::RoundingMode::ToNegativeInfinity).toString(), "-3");
            CHECK_EQ(Decimal{ "-2.9" }.round(0, Decimal::RoundingMode::ToNegativeInfinity).toString(), "-3");
            CHECK_EQ(Decimal{ "-2.0" }.round(0, Decimal::RoundingMode::ToNegativeInfinity).toString(), "-2");

            CHECK_EQ(Decimal{ "123.451" }.round(2, Decimal::RoundingMode::ToNegativeInfinity).toString(), "123.45");
            CHECK_EQ(Decimal{ "-123.451" }.round(2, Decimal::RoundingMode::ToNegativeInfinity).toString(), "-123.46");

            CHECK_EQ(
                Decimal{ "123.789" }.round(0, Decimal::RoundingMode::ToNegativeInfinity).toString(),
                Decimal{ "123.789" }.floor().toString());
            CHECK_EQ(
                Decimal{ "-123.789" }.round(0, Decimal::RoundingMode::ToNegativeInfinity).toString(),
                Decimal{ "-123.789" }.floor().toString());
        }

        TEST_CASE("rounding - round with decimal places all modes")
        {
            Decimal value{ "123.456789" };

            CHECK_EQ(value.round(0, Decimal::RoundingMode::ToNearest).toString(), "123");
            CHECK_EQ(value.round(1, Decimal::RoundingMode::ToNearest).toString(), "123.5");
            CHECK_EQ(value.round(2, Decimal::RoundingMode::ToNearest).toString(), "123.46");
            CHECK_EQ(value.round(3, Decimal::RoundingMode::ToNearest).toString(), "123.457");
            CHECK_EQ(value.round(4, Decimal::RoundingMode::ToNearest).toString(), "123.4568");

            CHECK_EQ(value.round(0, Decimal::RoundingMode::ToZero).toString(), "123");
            CHECK_EQ(value.round(1, Decimal::RoundingMode::ToZero).toString(), "123.4");
            CHECK_EQ(value.round(2, Decimal::RoundingMode::ToZero).toString(), "123.45");
            CHECK_EQ(value.round(3, Decimal::RoundingMode::ToZero).toString(), "123.456");

            CHECK_EQ(value.round(0, Decimal::RoundingMode::ToPositiveInfinity).toString(), "124");
            CHECK_EQ(value.round(1, Decimal::RoundingMode::ToPositiveInfinity).toString(), "123.5");
            CHECK_EQ(value.round(2, Decimal::RoundingMode::ToPositiveInfinity).toString(), "123.46");

            CHECK_EQ(value.round(0, Decimal::RoundingMode::ToNegativeInfinity).toString(), "123");
            CHECK_EQ(value.round(1, Decimal::RoundingMode::ToNegativeInfinity).toString(), "123.4");
            CHECK_EQ(value.round(2, Decimal::RoundingMode::ToNegativeInfinity).toString(), "123.45");
        }

        TEST_CASE("rounding - edge cases")
        {
            CHECK_EQ(Decimal{ "0" }.round(0, Decimal::RoundingMode::ToNearest).toString(), "0");
            CHECK_EQ(Decimal{ "0.0" }.round(0, Decimal::RoundingMode::ToNearest).toString(), "0");

            CHECK_EQ(Decimal{ "123" }.round(0, Decimal::RoundingMode::ToNearest).toString(), "123");
            CHECK_EQ(Decimal{ "-123" }.round(0, Decimal::RoundingMode::ToNearest).toString(), "-123");

            CHECK_EQ(Decimal{ "123.456" }.round(-1, Decimal::RoundingMode::ToNearest).toString(), "123");

            CHECK_EQ(Decimal{ "123.45" }.round(5, Decimal::RoundingMode::ToNearest).toString(), "123.45");

            CHECK_EQ(Decimal{ "0.001" }.round(0, Decimal::RoundingMode::ToNearest).toString(), "0");
            CHECK_EQ(Decimal{ "0.001" }.round(2, Decimal::RoundingMode::ToNearest).toString(), "0");
            CHECK_EQ(Decimal{ "0.001" }.round(3, Decimal::RoundingMode::ToNearest).toString(), "0.001");
        }

        TEST_CASE("maths - consistency (a+b)-a == b")
        {
            Decimal a{ "123.45" };
            Decimal b{ "67.89" };
            Decimal sum{ a + b };
            Decimal diff{ sum - a };
            CHECK(diff == b);
        }

        TEST_CASE("maths - precision preservation 0.1+0.2==0.3")
        {
            Decimal a{ "0.1" };
            Decimal b{ "0.2" };
            Decimal c{ "0.3" };
            Decimal sum{ a + b };
            CHECK(sum == c);

            Decimal precise1{ "0.1234567890123456789012345678" };
            Decimal precise2{ "0.0000000000000000000000000001" };
            Decimal preciseSum{ precise1 + precise2 };
            CHECK_FALSE(preciseSum == precise1);
            CHECK(preciseSum > precise1);
        }

        TEST_CASE("maths - pi times e")
        {
            Decimal pi{ "3.1415926535897932384626433832" };
            Decimal e{ "2.7182818284590452353602874713" };
            auto product = pi * e;

            CHECK(product > Decimal{ "8.5" });
            CHECK(product < Decimal{ "8.6" });
            CHECK(product > Decimal{ "8.539" });
            CHECK(product < Decimal{ "8.540" });
            CHECK(product > Decimal{ "8.5397" });
            CHECK(product < Decimal{ "8.5398" });

            auto sum = pi + e;
            CHECK(sum > Decimal{ "5.859" });
            CHECK(sum < Decimal{ "5.860" });
        }

        TEST_CASE("maths - rounding consistency")
        {
            Decimal value{ "123.456789" };
            CHECK_EQ(value.trunc().toString(), "123");
            CHECK_EQ(value.floor().toString(), "123");
            CHECK_EQ(value.ceil().toString(), "124");
            CHECK_EQ(value.round().toString(), "123");

            Decimal negValue{ "-123.456789" };
            CHECK_EQ(negValue.trunc().toString(), "-123");
            CHECK_EQ(negValue.floor().toString(), "-124");
            CHECK_EQ(negValue.ceil().toString(), "-123");
            CHECK_EQ(negValue.round().toString(), "-123");
        }

        TEST_CASE("maths - sqrt clang regression (irrational numbers)")
        {
            Decimal two{ "2" };
            auto sqrtTwo = two.sqrt();
            CHECK(sqrtTwo > Decimal{ "1.4" });
            CHECK(sqrtTwo < Decimal{ "1.5" });
            CHECK(sqrtTwo.scale() > 0);
            CHECK(sqrtTwo > Decimal{ "1.414" });
            CHECK(sqrtTwo < Decimal{ "1.415" });

            Decimal three{ "3" };
            auto sqrtThree = three.sqrt();
            CHECK(sqrtThree > Decimal{ "1.732" });
            CHECK(sqrtThree < Decimal{ "1.733" });
            CHECK(sqrtThree.scale() > 0);

            Decimal five{ "5" };
            auto sqrtFive = five.sqrt();
            CHECK(sqrtFive > Decimal{ "2.236" });
            CHECK(sqrtFive < Decimal{ "2.237" });
            CHECK(sqrtFive.scale() > 0);

            Decimal dividend{ "10" };
            Decimal divisor{ "3" };
            auto quotient = dividend / divisor;
            CHECK(quotient > Decimal{ "3.3" });
            CHECK(quotient < Decimal{ "3.4" });
            CHECK(quotient.scale() > 0);
        }

        TEST_CASE("operator+ sign: negative lhs smaller in magnitude than positive rhs")
        {
            CHECK_EQ((Decimal{ "-3.2" } + Decimal{ "273.15" }).toString(), "269.95");
            CHECK_EQ((Decimal{ "273.15" } + Decimal{ "-3.2" }).toString(), "269.95");

            // Commutativity must hold in all sign combinations
            CHECK_EQ((Decimal{ "-1" } + Decimal{ "10" }).toString(), "9");
            CHECK_EQ((Decimal{ "10" } + Decimal{ "-1" }).toString(), "9");

            CHECK_EQ((Decimal{ "-10" } + Decimal{ "1" }).toString(), "-9");
            CHECK_EQ((Decimal{ "1" } + Decimal{ "-10" }).toString(), "-9");

            CHECK_EQ((Decimal{ "-5" } + Decimal{ "5" }).toString(), "0");
            CHECK_EQ((Decimal{ "5" } + Decimal{ "-5" }).toString(), "0");

            Decimal celsiusToKelvin{ "273.15" };
            Decimal airTemp{ "-3.2" };
            CHECK_EQ((celsiusToKelvin + airTemp).toString(), "269.95");
            CHECK_EQ((airTemp + celsiusToKelvin).toString(), "269.95");
        }
    }
} // namespace dnv::vista::sdk::tests
