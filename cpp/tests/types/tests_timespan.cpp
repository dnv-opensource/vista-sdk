#include <doctest/doctest.h>

#include <dnv/vista/sdk/types/datetime/TimeSpan.h>

#include <format>
#include <limits>
#include <sstream>

namespace dnv::vista::sdk::tests
{
    TEST_SUITE("TimeSpan")
    {
        TEST_CASE("construction - default (zero duration)")
        {
            TimeSpan ts;
            CHECK_EQ(ts.ticks(), 0);
            CHECK_EQ(ts.seconds(), 0.0);
        }

        TEST_CASE("construction - from ticks")
        {
            TimeSpan ts{ 10000000LL };
            CHECK_EQ(ts.ticks(), 10000000LL);
            CHECK_EQ(ts.seconds(), 1.0);
        }

        TEST_CASE("construction - from chrono")
        {
            TimeSpan ts1{ std::chrono::seconds(60) };
            CHECK_EQ(ts1.seconds(), 60.0);

            TimeSpan ts2{ std::chrono::milliseconds(1500) };
            CHECK_EQ(ts2.milliseconds(), 1500.0);
            CHECK_EQ(ts2.seconds(), 1.5);

            TimeSpan ts3{ std::chrono::hours(2) };
            CHECK_EQ(ts3.hours(), 2.0);
        }

        TEST_CASE("comparison operators")
        {
            TimeSpan ts1{ TimeSpan::fromSeconds(10) };
            TimeSpan ts2{ TimeSpan::fromSeconds(20) };
            TimeSpan ts3{ TimeSpan::fromSeconds(10) };

            CHECK(ts1 == ts3);
            CHECK_FALSE(ts1 == ts2);
            CHECK(ts1 != ts2);
            CHECK(ts1 < ts2);
            CHECK(ts1 <= ts2);
            CHECK(ts1 <= ts3);
            CHECK(ts2 > ts1);
            CHECK(ts2 >= ts1);
            CHECK(ts1 >= ts3);
        }

        TEST_CASE("arithmetic - addition and subtraction")
        {
            TimeSpan ts1{ TimeSpan::fromHours(2) };
            TimeSpan ts2{ TimeSpan::fromMinutes(30) };

            TimeSpan sum{ ts1 + ts2 };
            CHECK_EQ(sum.hours(), 2.5);
            CHECK_EQ(sum.minutes(), 150.0);

            TimeSpan diff{ ts1 - ts2 };
            CHECK_EQ(diff.hours(), 1.5);
            CHECK_EQ(diff.minutes(), 90.0);
        }

        TEST_CASE("arithmetic - negation")
        {
            TimeSpan ts{ TimeSpan::fromHours(2.5) };
            TimeSpan neg{ -ts };
            CHECK_EQ(neg.hours(), -2.5);
            CHECK_EQ(neg.ticks(), -ts.ticks());
        }

        TEST_CASE("arithmetic - compound assignment")
        {
            TimeSpan ts1{ TimeSpan::fromHours(2) };
            ts1 += TimeSpan::fromMinutes(30);
            CHECK_EQ(ts1.hours(), 2.5);

            TimeSpan ts2{ TimeSpan::fromHours(2) };
            ts2 -= TimeSpan::fromMinutes(30);
            CHECK_EQ(ts2.hours(), 1.5);
        }

        TEST_CASE("arithmetic - multiplication")
        {
            TimeSpan ts{ TimeSpan::fromHours(2) };

            CHECK_EQ((ts * 3).hours(), 6.0);
            CHECK_EQ((3 * ts).hours(), 6.0);
            CHECK_EQ((ts * 2.5).hours(), 5.0);
            CHECK_EQ((2.5 * ts).hours(), 5.0);
            CHECK_EQ((ts * 0).ticks(), 0);
            CHECK_EQ((ts * -1).hours(), -2.0);
        }

        TEST_CASE("arithmetic - division")
        {
            TimeSpan ts1{ TimeSpan::fromHours(6) };
            CHECK_EQ((ts1 / 3).hours(), 2.0);
            CHECK_EQ((ts1 / -2).hours(), -3.0);

            TimeSpan ts2{ TimeSpan::fromHours(5) };
            CHECK_EQ((ts2 / 2.5).hours(), 2.0);

            TimeSpan ts3{ TimeSpan::fromHours(6) };
            TimeSpan ts4{ TimeSpan::fromHours(2) };
            CHECK_EQ(ts3 / ts4, 3.0);
        }

        TEST_CASE("arithmetic - complex chaining")
        {
            TimeSpan ts{ TimeSpan::fromHours(2) };
            TimeSpan result{ (ts * 2 + TimeSpan::fromMinutes(30)) / 2 };
            CHECK_EQ(result.hours(), 2.25);
        }

        TEST_CASE("accessors")
        {
            TimeSpan ts{ TimeSpan::fromHours(2.5) };

            CHECK_EQ(ts.ticks(), static_cast<std::int64_t>(2.5 * 60 * 60 * 10000000));
            CHECK_EQ(ts.days(), 2.5 / 24.0);
            CHECK_EQ(ts.hours(), 2.5);
            CHECK_EQ(ts.minutes(), 150.0);
            CHECK_EQ(ts.seconds(), 9000.0);
            CHECK_EQ(ts.milliseconds(), 9000000.0);

            TimeSpan neg{ TimeSpan::fromMinutes(-45) };
            CHECK_EQ(neg.minutes(), -45.0);
            CHECK_EQ(neg.hours(), -0.75);
        }

        TEST_CASE("factory methods")
        {
            CHECK_EQ(TimeSpan::fromDays(1.5).hours(), 36.0);
            CHECK_EQ(TimeSpan::fromHours(2.5).minutes(), 150.0);
            CHECK_EQ(TimeSpan::fromMinutes(90).hours(), 1.5);
            CHECK_EQ(TimeSpan::fromSeconds(3600).hours(), 1.0);
            CHECK_EQ(TimeSpan::fromMilliseconds(5000).seconds(), 5.0);
            CHECK_EQ(TimeSpan::fromMicroseconds(5000).milliseconds(), 5.0);

            TimeSpan ticks{ TimeSpan::fromTicks(10000000LL) };
            CHECK_EQ(ticks.ticks(), 10000000LL);
            CHECK_EQ(ticks.seconds(), 1.0);
        }

        TEST_CASE("toString - compact ISO 8601")
        {
            CHECK_EQ(TimeSpan::fromHours(1).toString(), "PT1H");
            CHECK_EQ(TimeSpan::fromMinutes(30).toString(), "PT30M");
            CHECK_EQ(TimeSpan::fromSeconds(45).toString(), "PT45S");

            CHECK_EQ((TimeSpan::fromHours(1) + TimeSpan::fromMinutes(30)).toString(), "PT1H30M");
            CHECK_EQ((TimeSpan::fromHours(2) + TimeSpan::fromSeconds(15)).toString(), "PT2H15S");
            CHECK_EQ(
                (TimeSpan::fromHours(1) + TimeSpan::fromMinutes(30) + TimeSpan::fromSeconds(45)).toString(),
                "PT1H30M45S");

            CHECK_EQ(TimeSpan{}.toString(), "PT0S");
        }

        TEST_CASE("toString - negative durations")
        {
            CHECK_EQ(TimeSpan::fromHours(-1).toString(), "-PT1H");
            CHECK_EQ(TimeSpan::fromMinutes(-30).toString(), "-PT30M");
            CHECK_EQ((TimeSpan::fromHours(-2) + TimeSpan::fromMinutes(-15)).toString(), "-PT2H15M");
        }

        TEST_CASE("toString - fractional seconds")
        {
            std::string result{ TimeSpan::fromSeconds(1.5).toString() };
            CHECK(result.find("PT1.") != std::string::npos);
            CHECK(result.back() == 'S');
        }

        TEST_CASE("parsing - compact ISO 8601")
        {
            CHECK_EQ(TimeSpan{ "PT1H" }.hours(), 1.0);
            CHECK_EQ(TimeSpan{ "PT30M" }.minutes(), 30.0);
            CHECK_EQ(TimeSpan{ "PT45S" }.seconds(), 45.0);
            CHECK_EQ(TimeSpan{ "PT1H30M" }.minutes(), 90.0);
            CHECK_EQ(TimeSpan{ "PT0S" }.seconds(), 0.0);
        }

        TEST_CASE("parsing - verbose ISO 8601")
        {
            CHECK_EQ(TimeSpan{ "PT1H0M0S" }.hours(), 1.0);
            CHECK_EQ(TimeSpan{ "PT0H30M0S" }.minutes(), 30.0);
            CHECK_EQ(TimeSpan{ "PT1H30M45S" }.seconds(), doctest::Approx(3600.0 + 30 * 60 + 45));
        }

        TEST_CASE("parsing - negative")
        {
            CHECK_EQ(TimeSpan{ "-PT1H" }.hours(), -1.0);
            CHECK_EQ(TimeSpan{ "-PT30M" }.minutes(), -30.0);
            CHECK_EQ(TimeSpan{ "-PT1H30M45S" }.seconds(), doctest::Approx(-(3600.0 + 30 * 60 + 45)));
        }

        TEST_CASE("parsing - with days")
        {
            CHECK_EQ(TimeSpan{ "P1D" }.days(), 1.0);
            CHECK_EQ(TimeSpan{ "P1DT12H" }.hours(), 36.0);
            CHECK_EQ(TimeSpan{ "P2DT3H30M45S" }.seconds(), doctest::Approx(2 * 86400.0 + 3 * 3600 + 30 * 60 + 45));
        }

        TEST_CASE("parsing - fractional seconds")
        {
            CHECK_EQ(TimeSpan{ "PT1.5S" }.seconds(), 1.5);
            CHECK(doctest::Approx(TimeSpan{ "PT0.123S" }.seconds()).epsilon(0.001) == 0.123);
        }

        TEST_CASE("fromString - optional overload")
        {
            auto ts1{ TimeSpan::fromString("PT1H30M45S") };
            CHECK(ts1.has_value());
            CHECK_EQ(ts1->minutes(), doctest::Approx(90.75));

            CHECK_FALSE(TimeSpan::fromString("invalid").has_value());
            CHECK_FALSE(TimeSpan::fromString("").has_value());
            CHECK_FALSE(TimeSpan::fromString("PT").has_value());
        }

        TEST_CASE("fromString - out-parameter overload")
        {
            TimeSpan result;
            CHECK(TimeSpan::fromString("PT1H", result));
            CHECK_EQ(result.hours(), 1.0);

            CHECK(TimeSpan::fromString("-PT1H", result));
            CHECK_EQ(result.hours(), -1.0);

            CHECK_FALSE(TimeSpan::fromString("invalid", result));
        }

        TEST_CASE("parsing - constructor throws on invalid")
        {
            CHECK_THROWS_AS((void)TimeSpan{ "invalid" }, std::invalid_argument);
            CHECK_THROWS_AS((void)TimeSpan{ "" }, std::invalid_argument);
            CHECK_THROWS_AS((void)TimeSpan{ "PT" }, std::invalid_argument);
        }

        TEST_CASE("parsing - rejects invalid component order")
        {
            CHECK_FALSE(TimeSpan::fromString("PT30M2H").has_value());
            CHECK_FALSE(TimeSpan::fromString("PT45S30M").has_value());
            CHECK_THROWS_AS((void)TimeSpan{ "PT30M2H" }, std::invalid_argument);
        }

        TEST_CASE("parsing - rejects duplicate components")
        {
            CHECK_FALSE(TimeSpan::fromString("PT1H2H").has_value());
            CHECK_FALSE(TimeSpan::fromString("PT30M15M").has_value());
        }

        TEST_CASE("parsing - round-trip")
        {
            TimeSpan parsed{ TimeSpan{ "PT1H0M0S" } };
            std::string output{ parsed.toString() };
            TimeSpan reparsed{ TimeSpan{ output } };
            CHECK_EQ(parsed.ticks(), reparsed.ticks());
        }

        TEST_CASE("chrono interop - toChrono / fromChrono")
        {
            TimeSpan ts1{ TimeSpan::fromSeconds(90) };
            auto chrono1{ ts1.toChrono() };
            auto secs{ std::chrono::duration_cast<std::chrono::seconds>(chrono1) };
            CHECK_EQ(secs.count(), 90);

            TimeSpan ts2{ TimeSpan::fromSeconds(1.5) };
            auto chrono2{ ts2.toChrono() };
            auto ms{ std::chrono::duration_cast<std::chrono::milliseconds>(chrono2) };
            CHECK_EQ(ms.count(), 1500);

            TimeSpan original{ TimeSpan::fromSeconds(45.5) };
            TimeSpan roundTrip{ TimeSpan::fromChrono(original.toChrono()) };
            CHECK(doctest::Approx(roundTrip.seconds()).epsilon(0.001) == original.seconds());
        }

        TEST_CASE("stream output operator")
        {
            TimeSpan ts{ TimeSpan::fromHours(1.5) };
            std::ostringstream oss;
            oss << ts;

            std::string output{ oss.str() };
            CHECK_FALSE(output.empty());
            CHECK(output.find("PT") != std::string::npos);
        }

        TEST_CASE("stream input operator")
        {
            std::istringstream iss{ "PT1H30M45S" };
            TimeSpan ts;
            iss >> ts;
            CHECK_FALSE(iss.fail());
            CHECK_EQ(ts.minutes(), doctest::Approx(90.75));

            std::istringstream iss2{ "invalid" };
            TimeSpan ts2;
            iss2 >> ts2;
            CHECK(iss2.fail());
        }

        TEST_CASE("std::format support")
        {
            TimeSpan ts{ TimeSpan::fromHours(2.5) };
            std::string formatted{ std::format("{}", ts) };
            CHECK(formatted.find("PT") != std::string::npos);
            CHECK(formatted.find("2H30M") != std::string::npos);

            TimeSpan neg{ TimeSpan::fromMinutes(-45.0) };
            CHECK(std::format("{}", neg).find("-PT45M") != std::string::npos);

            TimeSpan zero;
            CHECK(std::format("{}", zero).find("PT0S") != std::string::npos);
        }

        TEST_CASE("literals")
        {
            using namespace dnv::vista::sdk::literals;

            CHECK_EQ((1_h).hours(), 1.0);
            CHECK_EQ((24_h).days(), 1.0);
            CHECK_EQ((2.5_h).minutes(), 150.0);
            CHECK_EQ((30_min).seconds(), 60.0 * 30);
            CHECK_EQ((1_s).milliseconds(), 1000.0);
            CHECK_EQ((1000_ms).seconds(), 1.0);
            CHECK_EQ((1_d).hours(), 24.0);

            CHECK_EQ((1_h + 30_min).minutes(), 90.0);
            CHECK_EQ((2_h - 30_min).minutes(), 90.0);

            CHECK(1_h < 2_h);
            CHECK(30_min == 0.5_h);
            CHECK(60_s == 1_min);
            CHECK(1000_ms == 1_s);

            auto lit{ 2.5_h };
            auto fact{ TimeSpan::fromHours(2.5) };
            CHECK_EQ(lit.ticks(), fact.ticks());
        }

        TEST_CASE("literals - toString")
        {
            using namespace dnv::vista::sdk::literals;
            CHECK_EQ((2_h + 30_min).toString(), "PT2H30M");
        }

        TEST_CASE("edge cases - zero duration")
        {
            TimeSpan zero;
            CHECK_EQ(zero.ticks(), 0);
            CHECK_EQ(zero.toString(), "PT0S");
        }

        TEST_CASE("edge cases - negative duration")
        {
            TimeSpan neg{ TimeSpan::fromHours(-2.5) };
            CHECK_EQ(neg.hours(), -2.5);
            CHECK_LT(neg.ticks(), 0);

            TimeSpan result{ TimeSpan::fromHours(3) + neg };
            CHECK_EQ(result.hours(), 0.5);
        }

        TEST_CASE("integration - round-trip serialization")
        {
            TimeSpan original{ TimeSpan::fromHours(2) + TimeSpan::fromMinutes(30) + TimeSpan::fromSeconds(45.5) };
            std::string serialized{ original.toString() };
            TimeSpan deserialized{ TimeSpan{ serialized } };
            CHECK(doctest::Approx(deserialized.seconds()).epsilon(0.001) == original.seconds());
        }

        TEST_CASE("edge cases - toString() at INT64_MIN ticks")
        {
            TimeSpan minSpan{ TimeSpan::fromTicks(std::numeric_limits<std::int64_t>::min()) };
            CHECK_EQ(minSpan.ticks(), std::numeric_limits<std::int64_t>::min());
            CHECK_EQ(minSpan.toString(), "-P10675199DT2H48M5.4775808S");
        }
    }
} // namespace dnv::vista::sdk::tests
