#include <doctest/doctest.h>

#include <dnv/vista/sdk/types/datetime/DateTimeOffset.h>

#include <cstdlib>
#include <ctime>
#include <format>
#include <optional>
#include <sstream>

#if defined(_WIN32)
    #define setenv(name, value, overwrite) _putenv_s(name, value)
    #define unsetenv(name) _putenv_s(name, "")
    #if !defined(__MINGW32__)
        #define tzset _tzset
    #endif
#endif

namespace
{
    std::optional<std::string> getEnvVar(const char* name)
    {
#if defined(_WIN32) && !defined(__MINGW32__)
        char* buffer{ nullptr };
        size_t size{ 0 };
        if (_dupenv_s(&buffer, &size, name) != 0 || !buffer)
        {
            return std::nullopt;
        }
        std::string value{ buffer };
        free(buffer);
        return value;
#else
        const char* value{ std::getenv(name) };
        return value ? std::optional<std::string>{ value } : std::nullopt;
#endif
    }
} // namespace

namespace dnv::vista::sdk::tests
{
    TEST_SUITE("DateTimeOffset")
    {
        TEST_CASE("construction - default")
        {
            DateTimeOffset dto;
            CHECK_EQ(dto.ticks(), DateTime::min().ticks());
            CHECK_EQ(dto.offset().ticks(), 0);
        }

        TEST_CASE("construction - from DateTime and offset")
        {
            DateTime dt{ 2024, 1, 15, 12, 30, 45 };
            TimeSpan offset{ TimeSpan::fromHours(2.0) };
            DateTimeOffset dto{ dt, offset };

            CHECK_EQ(dto.dateTime(), dt);
            CHECK_EQ(dto.offset().ticks(), offset.ticks());
            CHECK_EQ(dto.year(), 2024);
            CHECK_EQ(dto.month(), 1);
            CHECK_EQ(dto.day(), 15);
            CHECK_EQ(dto.hour(), 12);
            CHECK_EQ(dto.minute(), 30);
            CHECK_EQ(dto.second(), 45);
        }

        TEST_CASE("construction - from ticks and offset")
        {
            std::int64_t ticks{ 638400000000000000LL };
            TimeSpan offset{ TimeSpan::fromHours(2) };
            DateTimeOffset dto{ ticks, offset };
            CHECK_EQ(dto.ticks(), ticks);
            CHECK_EQ(dto.offset().ticks(), offset.ticks());
        }

        TEST_CASE("construction - from date components and offset")
        {
            TimeSpan offset{ TimeSpan::fromHours(3.0) };
            DateTimeOffset dto{ 2024, 6, 15, offset };
            CHECK_EQ(dto.year(), 2024);
            CHECK_EQ(dto.month(), 6);
            CHECK_EQ(dto.day(), 15);
            CHECK_EQ(dto.hour(), 0);
            CHECK_EQ(dto.offset().ticks(), offset.ticks());
        }

        TEST_CASE("construction - from datetime components and offset")
        {
            TimeSpan offset{ TimeSpan::fromHours(-8.0) };
            DateTimeOffset dto{ 2024, 12, 25, 18, 45, 30, offset };
            CHECK_EQ(dto.year(), 2024);
            CHECK_EQ(dto.month(), 12);
            CHECK_EQ(dto.day(), 25);
            CHECK_EQ(dto.hour(), 18);
            CHECK_EQ(dto.minute(), 45);
            CHECK_EQ(dto.second(), 30);
            CHECK_EQ(dto.offset().ticks(), offset.ticks());
        }

        TEST_CASE("construction - from datetime with milliseconds and offset")
        {
            TimeSpan offset{ TimeSpan::fromHours(5.5) };
            DateTimeOffset dto{ 2024, 3, 10, 9, 15, 22, 500, offset };
            CHECK_EQ(dto.second(), 22);
            CHECK_EQ(dto.millisecond(), 500);
        }

        TEST_CASE("construction - from string")
        {
            DateTimeOffset dto{ "2024-06-20T08:15:30+02:00" };
            CHECK_EQ(dto.year(), 2024);
            CHECK_EQ(dto.month(), 6);
            CHECK_EQ(dto.day(), 20);
            CHECK_EQ(dto.hour(), 8);
            CHECK_EQ(dto.minute(), 15);
            CHECK_EQ(dto.second(), 30);
            CHECK_EQ(dto.offset().hours(), 2.0);
        }

        TEST_CASE("comparison - same UTC time, different offsets are equal")
        {
            DateTimeOffset dto1{ 2024, 1, 15, 12, 0, 0, TimeSpan::fromHours(2.0) };
            DateTimeOffset dto2{ 2024, 1, 15, 10, 0, 0, TimeSpan::fromHours(0.0) };
            CHECK(dto1 == dto2);

            DateTimeOffset dto3{ 2024, 1, 15, 13, 0, 0, TimeSpan::fromHours(2.0) };
            CHECK_FALSE(dto1 == dto3);
        }

        TEST_CASE("comparison operators")
        {
            DateTimeOffset dto1{ 2024, 1, 15, 12, 0, 0, TimeSpan::fromHours(0.0) };
            DateTimeOffset dto2{ 2024, 1, 15, 13, 0, 0, TimeSpan::fromHours(0.0) };

            CHECK(dto1 != dto2);
            CHECK(dto1 < dto2);
            CHECK(dto1 <= dto1);
            CHECK(dto1 <= dto2);
            CHECK(dto2 > dto1);
            CHECK(dto2 >= dto1);
            CHECK(dto1 >= dto1);
        }

        TEST_CASE("arithmetic - add/subtract TimeSpan")
        {
            DateTimeOffset dto{ 2024, 1, 15, 12, 0, 0, TimeSpan::fromHours(2.0) };

            DateTimeOffset result1{ dto + TimeSpan::fromHours(3.0) };
            CHECK_EQ(result1.hour(), 15);
            CHECK_EQ(result1.offset().ticks(), dto.offset().ticks());

            DateTimeOffset result2{ dto - TimeSpan::fromHours(2.0) };
            CHECK_EQ(result2.hour(), 10);

            dto += TimeSpan::fromMinutes(45.0);
            CHECK_EQ(dto.minute(), 45);

            DateTimeOffset dto2{ 2024, 1, 15, 12, 30, 0, TimeSpan::fromHours(2.0) };
            dto2 -= TimeSpan::fromMinutes(30.0);
            CHECK_EQ(dto2.minute(), 0);
        }

        TEST_CASE("arithmetic - subtract DateTimeOffset")
        {
            DateTimeOffset dto1{ 2024, 1, 15, 15, 0, 0, TimeSpan::fromHours(2.0) };
            DateTimeOffset dto2{ 2024, 1, 15, 12, 0, 0, TimeSpan::fromHours(2.0) };
            TimeSpan diff{ dto1 - dto2 };
            CHECK_EQ(diff.hours(), 3.0);
        }

        TEST_CASE("accessors - components")
        {
            DateTimeOffset dto{ 2024, 3, 15, 14, 30, 45, 123, TimeSpan::fromHours(-5.0) };
            CHECK_EQ(dto.year(), 2024);
            CHECK_EQ(dto.month(), 3);
            CHECK_EQ(dto.day(), 15);
            CHECK_EQ(dto.hour(), 14);
            CHECK_EQ(dto.minute(), 30);
            CHECK_EQ(dto.second(), 45);
            CHECK_EQ(dto.millisecond(), 123);
        }

        TEST_CASE("accessors - offset")
        {
            TimeSpan offset{ TimeSpan::fromHours(5.5) };
            DateTimeOffset dto{ 2024, 1, 15, 12, 0, 0, offset };
            CHECK_EQ(dto.offset().ticks(), offset.ticks());
            CHECK_EQ(dto.totalOffsetMinutes(), 330);
        }

        TEST_CASE("accessors - utcDateTime")
        {
            DateTimeOffset dto{ 2024, 1, 15, 12, 0, 0, TimeSpan::fromHours(2.0) };
            DateTime utc{ dto.utcDateTime() };
            CHECK_EQ(utc.hour(), 10);
            CHECK_EQ(utc.day(), 15);
        }

        TEST_CASE("accessors - dayOfWeek and dayOfYear")
        {
            DateTimeOffset dto{ 2024, 1, 15, 12, 0, 0, TimeSpan::fromHours(0.0) };
            CHECK_EQ(dto.dayOfWeek(), 1);
            CHECK_EQ(dto.dayOfYear(), 15);
        }

        TEST_CASE("conversion - toEpochSeconds / toEpochMilliseconds")
        {
            DateTimeOffset epoch{ DateTimeOffset::epoch() };
            CHECK_EQ(epoch.toEpochSeconds(), 0);
            CHECK_EQ(epoch.toEpochMilliseconds(), 0);

            DateTimeOffset dto1{ 1970, 1, 1, 1, 0, 0, TimeSpan::fromHours(0.0) };
            CHECK_EQ(dto1.toEpochSeconds(), 3600);

            DateTimeOffset dto2{ 1970, 1, 1, 0, 0, 1, 500, TimeSpan::fromHours(0.0) };
            CHECK_EQ(dto2.toEpochMilliseconds(), 1500);
        }

        TEST_CASE("conversion - toOffset")
        {
            DateTimeOffset dto{ 2024, 1, 15, 12, 0, 0, TimeSpan::fromHours(2.0) };
            DateTimeOffset converted{ dto.toOffset(TimeSpan::fromHours(-5.0)) };
            CHECK_EQ(converted.hour(), 5);
            CHECK_EQ(converted.offset().hours(), -5.0);
            CHECK_EQ(dto.utcTicks(), converted.utcTicks());
        }

        TEST_CASE("conversion - toUniversalTime")
        {
            DateTimeOffset dto{ 2024, 1, 15, 12, 0, 0, TimeSpan::fromHours(3.0) };
            DateTimeOffset utc{ dto.toUniversalTime() };
            CHECK_EQ(utc.hour(), 9);
            CHECK_EQ(utc.offset().ticks(), 0);
        }

        TEST_CASE("conversion - date and timeOfDay")
        {
            DateTimeOffset dto{ 2024, 3, 15, 14, 30, 45, TimeSpan::fromHours(1.0) };

            DateTimeOffset dateOnly{ dto.date() };
            CHECK_EQ(dateOnly.hour(), 0);
            CHECK_EQ(dateOnly.minute(), 0);
            CHECK_EQ(dateOnly.second(), 0);
            CHECK_EQ(dateOnly.day(), 15);

            TimeSpan timeOfDay{ dto.timeOfDay() };
            CHECK_EQ(timeOfDay.hours(), doctest::Approx(14.0 + 30.0 / 60.0 + 45.0 / 3600.0));
        }

        TEST_CASE("add methods")
        {
            DateTimeOffset dto{ 2024, 1, 15, 12, 0, 0, TimeSpan::fromHours(0.0) };

            CHECK_EQ(dto.addDays(5.0).day(), 20);
            CHECK_EQ(dto.addHours(25.0).day(), 16);
            CHECK_EQ(dto.addHours(25.0).hour(), 13);
            CHECK_EQ(dto.addMonths(2).month(), 3);
            CHECK_EQ(dto.addYears(1).year(), 2025);
        }

        TEST_CASE("toString - Iso8601")
        {
            DateTimeOffset dto{ 2024, 1, 15, 12, 30, 45, TimeSpan::fromHours(2.0) };
            std::string str{ dto.toString() };
            CHECK(str.find("2024-01-15") != std::string::npos);
            CHECK(str.find("12:30:45") != std::string::npos);
            CHECK(str.find("+02:00") != std::string::npos);
        }

        TEST_CASE("toString - negative offset")
        {
            DateTimeOffset dto{ 2024, 6, 20, 18, 45, 30, TimeSpan::fromHours(-5.0) };
            CHECK(dto.toString().find("-05:00") != std::string::npos);
        }

        TEST_CASE("toString - Iso8601Millis")
        {
            CHECK_EQ(
                DateTimeOffset{ 2024, 1, 15, 10, 30, 12, 123, TimeSpan::fromHours(2.0) }.toString(
                    DateTime::Format::Iso8601Millis),
                "2024-01-15T10:30:12.123+02:00");
            CHECK_EQ(
                DateTimeOffset{ 2024, 1, 15, 10, 30, 12, 0, TimeSpan::fromHours(-5.0) }.toString(
                    DateTime::Format::Iso8601Millis),
                "2024-01-15T10:30:12.000-05:00");
            CHECK_EQ(
                DateTimeOffset{ 2024, 6, 20, 8, 15, 0, 999, TimeSpan::fromHours(5.5) }.toString(
                    DateTime::Format::Iso8601Millis),
                "2024-06-20T08:15:00.999+05:30");
        }

        TEST_CASE("toString - Iso8601PreciseTrimmed")
        {
            CHECK_EQ(
                DateTimeOffset{ 2024, 1, 15, 10, 30, 12, 123, TimeSpan::fromHours(2.0) }.toString(
                    DateTime::Format::Iso8601PreciseTrimmed),
                "2024-01-15T10:30:12.123+02:00");
            CHECK_EQ(
                DateTimeOffset{ 2024, 1, 15, 10, 30, 12, 100, TimeSpan::fromHours(-5.0) }.toString(
                    DateTime::Format::Iso8601PreciseTrimmed),
                "2024-01-15T10:30:12.1-05:00");
            CHECK_EQ(
                DateTimeOffset{ 2024, 1, 15, 10, 30, 12, 0, TimeSpan::fromHours(0.0) }.toString(
                    DateTime::Format::Iso8601PreciseTrimmed),
                "2024-01-15T10:30:12.0Z");
        }

        TEST_CASE("toString - Iso8601Basic")
        {
            CHECK_EQ(
                DateTimeOffset{ 2024, 1, 15, 10, 30, 12, TimeSpan::fromHours(2.0) }.toString(
                    DateTime::Format::Iso8601Basic),
                "20240115T103012+0200");
            CHECK_EQ(
                DateTimeOffset{ 2024, 6, 20, 14, 45, 30, TimeSpan::fromHours(-5.0) }.toString(
                    DateTime::Format::Iso8601Basic),
                "20240620T144530-0500");
            CHECK_EQ(
                DateTimeOffset{ 2024, 12, 25, 0, 0, 0, TimeSpan::fromHours(0.0) }.toString(
                    DateTime::Format::Iso8601Basic),
                "20241225T000000+0000");
            CHECK_EQ(
                DateTimeOffset{ 2024, 3, 10, 9, 15, 45, TimeSpan::fromHours(5.5) }.toString(
                    DateTime::Format::Iso8601Basic),
                "20240310T091545+0530");
        }

        TEST_CASE("toString - Iso8601Micros")
        {
            DateTime dt1{ 2024, 1, 15, 10, 30, 12, 0 };
            dt1 = DateTime{ dt1.ticks() + 1234560LL };
            CHECK_EQ(
                DateTimeOffset{ dt1, TimeSpan::fromHours(2.0) }.toString(DateTime::Format::Iso8601Micros),
                "2024-01-15T10:30:12.123456+02:00");

            CHECK_EQ(
                DateTimeOffset{ 2024, 1, 15, 10, 30, 12, 0, TimeSpan::fromHours(-5.0) }.toString(
                    DateTime::Format::Iso8601Micros),
                "2024-01-15T10:30:12.000000-05:00");
            CHECK_EQ(
                DateTimeOffset{ 2024, 1, 15, 10, 30, 12, 123, TimeSpan::fromHours(0.0) }.toString(
                    DateTime::Format::Iso8601Micros),
                "2024-01-15T10:30:12.123000Z");
        }

        TEST_CASE("equals / equalsExact")
        {
            DateTimeOffset dto1{ 2024, 1, 15, 12, 0, 0, TimeSpan::fromHours(2.0) };
            DateTimeOffset dto2{ 2024, 1, 15, 10, 0, 0, TimeSpan::fromHours(0.0) };
            DateTimeOffset dto3{ 2024, 1, 15, 12, 0, 0, TimeSpan::fromHours(2.0) };

            CHECK(dto1.equals(dto2));
            CHECK_FALSE(dto1.equalsExact(dto2));
            CHECK(dto1.equalsExact(dto3));
        }

        TEST_CASE("spaceship operator")
        {
            DateTimeOffset dto1{ 2024, 1, 15, 12, 0, 0, TimeSpan::fromHours(0.0) };
            DateTimeOffset dto2{ 2024, 1, 15, 13, 0, 0, TimeSpan::fromHours(0.0) };

            CHECK(std::is_lt(dto1 <=> dto2));
            CHECK(std::is_gt(dto2 <=> dto1));
            CHECK(std::is_eq(dto1 <=> dto1));
        }

        TEST_CASE("factory - now / utcNow / today")
        {
            DateTimeOffset now{ DateTimeOffset::now() };
            CHECK_GT(now.ticks(), 0);
            CHECK_GE(now.year(), 2024);

            DateTimeOffset utc{ DateTimeOffset::utcNow() };
            CHECK_EQ(utc.offset().ticks(), 0);
            CHECK_GE(utc.year(), 2024);

            // now() and utcNow() represent the same instant
            auto diff{ now - utc };
            CHECK(std::abs(diff.seconds()) < 2.0);

            DateTimeOffset today{ DateTimeOffset::today() };
            CHECK_EQ(today.hour(), 0);
            CHECK_EQ(today.minute(), 0);
            CHECK_EQ(today.second(), 0);
        }

        TEST_CASE("factory - min / max / epoch")
        {
            DateTimeOffset minVal{ DateTimeOffset::min() };
            CHECK_EQ(minVal.year(), 1);
            CHECK_EQ(minVal.month(), 1);
            CHECK_EQ(minVal.day(), 1);

            DateTimeOffset maxVal{ DateTimeOffset::max() };
            CHECK_EQ(maxVal.year(), 9999);
            CHECK_EQ(maxVal.month(), 12);
            CHECK_EQ(maxVal.day(), 31);

            DateTimeOffset epoch{ DateTimeOffset::epoch() };
            CHECK_EQ(epoch.year(), 1970);
            CHECK_EQ(epoch.month(), 1);
            CHECK_EQ(epoch.day(), 1);
            CHECK_EQ(epoch.offset().ticks(), 0);
        }

        TEST_CASE("factory - fromEpochSeconds / fromEpochMilliseconds")
        {
            DateTimeOffset dto1{ DateTimeOffset::fromEpochSeconds(3600) };
            CHECK_EQ(dto1.year(), 1970);
            CHECK_EQ(dto1.hour(), 1);
            CHECK_EQ(dto1.offset().ticks(), 0);

            DateTimeOffset dto2{ DateTimeOffset::fromEpochMilliseconds(1500) };
            CHECK_EQ(dto2.second(), 1);
            CHECK_EQ(dto2.millisecond(), 500);
        }

        TEST_CASE("parsing - with positive offset")
        {
            DateTimeOffset dto{ "2024-01-15T12:30:45+02:00" };
            CHECK_EQ(dto.year(), 2024);
            CHECK_EQ(dto.hour(), 12);
            CHECK_EQ(dto.offset().hours(), 2.0);
        }

        TEST_CASE("parsing - with negative offset")
        {
            DateTimeOffset dto{ "2024-06-20T18:45:30-05:00" };
            CHECK_EQ(dto.year(), 2024);
            CHECK_EQ(dto.hour(), 18);
            CHECK_EQ(dto.offset().hours(), -5.0);
        }

        TEST_CASE("parsing - Zulu (UTC)")
        {
            DateTimeOffset dto{ "2024-03-10T09:15:22Z" };
            CHECK_EQ(dto.year(), 2024);
            CHECK_EQ(dto.hour(), 9);
            CHECK_EQ(dto.offset().ticks(), 0);
        }

        TEST_CASE("parsing - fractional seconds")
        {
            DateTimeOffset dto{ "2024-01-15T12:30:45.123+01:00" };
            CHECK_EQ(dto.second(), 45);
            CHECK_EQ(dto.millisecond(), 123);
        }

        TEST_CASE("fromString - optional overload")
        {
            auto dto1{ DateTimeOffset::fromString("2024-01-15T12:30:45+02:00") };
            CHECK(dto1.has_value());
            CHECK_EQ(dto1->year(), 2024);
            CHECK_EQ(dto1->hour(), 12);
            CHECK_EQ(dto1->offset().hours(), 2.0);

            CHECK_FALSE(DateTimeOffset::fromString("invalid-date-string").has_value());
        }

        TEST_CASE("fromString - out-parameter overload")
        {
            DateTimeOffset dto;
            CHECK(DateTimeOffset::fromString("2024-01-15T12:30:45+02:00", dto));
            CHECK_EQ(dto.year(), 2024);
            CHECK_EQ(dto.hour(), 12);

            CHECK_FALSE(DateTimeOffset::fromString("invalid-date-string", dto));
        }

        TEST_CASE("parsing - constructor throws on invalid")
        {
            CHECK_THROWS_AS((void)DateTimeOffset{ "not-a-date" }, std::invalid_argument);
        }

        TEST_CASE("parsing - rejects invalid offsets")
        {
            CHECK_FALSE(DateTimeOffset::fromString("2024-01-15T12:00:00+15:00").has_value());
            CHECK_FALSE(DateTimeOffset::fromString("2024-01-15T12:00:00-15:00").has_value());
            CHECK_FALSE(DateTimeOffset::fromString("2024-01-15T12:00:00+14:01").has_value());
            CHECK_FALSE(DateTimeOffset::fromString("2024-01-15T12:00:00+01:60").has_value());
            CHECK_THROWS_AS((void)DateTimeOffset{ "2024-01-15T12:00:00+15:00" }, std::invalid_argument);
        }

        TEST_CASE("parsing - accepts valid offset edge cases")
        {
            auto dto1{ DateTimeOffset::fromString("2024-01-15T12:00:00+14:00") };
            CHECK(dto1.has_value());
            CHECK_EQ(dto1->offset().hours(), 14.0);

            auto dto2{ DateTimeOffset::fromString("2024-01-15T12:00:00-14:00") };
            CHECK(dto2.has_value());
            CHECK_EQ(dto2->offset().hours(), -14.0);

            CHECK(DateTimeOffset::fromString("2024-01-15T12:00:00Z").has_value());
            CHECK(DateTimeOffset::fromString("2024-01-15T12:00:00+00:00").has_value());
        }

        TEST_CASE("stream output operator")
        {
            DateTimeOffset dto{ 2024, 1, 15, 12, 30, 45, TimeSpan::fromHours(2.0) };
            std::ostringstream oss;
            oss << dto;
            CHECK(oss.str().find("2024") != std::string::npos);
            CHECK(oss.str().find("12:30:45") != std::string::npos);
        }

        TEST_CASE("stream input operator")
        {
            std::istringstream iss{ "2024-01-15T12:30:45+02:00" };
            DateTimeOffset dto;
            iss >> dto;
            CHECK_EQ(dto.year(), 2024);
            CHECK_EQ(dto.hour(), 12);
            CHECK_EQ(dto.offset().hours(), 2.0);
        }

        TEST_CASE("std::format support")
        {
            DateTimeOffset dto{ 2024, 1, 15, 12, 30, 45, TimeSpan::fromHours(2.0) };
            std::string formatted{ std::format("{}", dto) };
            CHECK(formatted.find("2024-01-15") != std::string::npos);
            CHECK(formatted.find("12:30:45") != std::string::npos);
            CHECK(formatted.find("+02:00") != std::string::npos);

            DateTimeOffset neg{ 2024, 6, 20, 18, 45, 30, TimeSpan::fromHours(-5.0) };
            CHECK(std::format("{}", neg).find("-05:00") != std::string::npos);
        }

        TEST_CASE("edge case - crossing day boundary")
        {
            DateTimeOffset dto{ 2024, 1, 15, 23, 0, 0, TimeSpan::fromHours(0.0) };
            DateTimeOffset result{ dto.addHours(2.0) };
            CHECK_EQ(result.day(), 16);
            CHECK_EQ(result.hour(), 1);
        }

        TEST_CASE("edge case - offset conversion across days")
        {
            DateTimeOffset dto{ 2024, 1, 16, 1, 0, 0, TimeSpan::fromHours(10.0) };
            DateTimeOffset converted{ dto.toOffset(TimeSpan::fromHours(-9.0)) };
            CHECK_EQ(converted.day(), 15);
        }

        TEST_CASE("edge case - leap year")
        {
            DateTimeOffset dto{ 2024, 2, 29, 12, 0, 0, TimeSpan::fromHours(0.0) };
            CHECK_EQ(dto.month(), 2);
            CHECK_EQ(dto.day(), 29);
        }

        TEST_CASE("chrono - round-trip toChrono / fromChrono without offset")
        {
            DateTimeOffset original{ 2024, 3, 10, 14, 30, 22, TimeSpan{ 0 } };
            auto tp{ original.toChrono() };
            DateTimeOffset roundTrip{ DateTimeOffset::fromChrono(tp) };
            CHECK_EQ(roundTrip.year(), original.year());
            CHECK_EQ(roundTrip.month(), original.month());
            CHECK_EQ(roundTrip.day(), original.day());
            CHECK_EQ(roundTrip.hour(), original.hour());
            CHECK_EQ(roundTrip.minute(), original.minute());
            CHECK_EQ(roundTrip.second(), original.second());
            CHECK_EQ(roundTrip.totalOffsetMinutes(), 0);
        }

        TEST_CASE("chrono - toChrono returns UTC instant regardless of offset")
        {
            DateTimeOffset tokyo{ 2024, 3, 10, 23, 30, 0, TimeSpan::fromHours(9.0) };
            DateTimeOffset utc{ 2024, 3, 10, 14, 30, 0, TimeSpan{ 0 } };
            CHECK_EQ(tokyo.toChrono(), utc.toChrono());
        }

        TEST_CASE("chrono - fromChrono with non-zero offset carries offset and UTC instant")
        {
            DateTimeOffset utc{ 2024, 3, 10, 14, 30, 0, TimeSpan{ 0 } };
            auto tp{ utc.toChrono() };
            DateTimeOffset tokyo{ DateTimeOffset::fromChrono(tp, std::chrono::seconds{ 9 * 3600 }) };
            CHECK_EQ(tokyo.totalOffsetMinutes(), 540);
            CHECK_EQ(tokyo.toChrono(), tp);
        }

        TEST_CASE("system timezone offset is correct when local/UTC day-of-month split crosses a month boundary")
        {
            std::optional<std::string> originalTz{ getEnvVar("TZ") };

            setenv("TZ", "GMT-2", 1);
            tzset();

            // 2024-01-31 23:00:00 UTC -> in UTC+2 this is 2024-02-01 01:00:00 local: exercises the
            // case where the local and UTC calendar dates fall in different months.
            DateTime utcBoundary{ 2024, 1, 31, 23, 0, 0 };
            DateTimeOffset dto{ utcBoundary };

            if (originalTz.has_value())
            {
                setenv("TZ", originalTz->c_str(), 1);
            }
            else
            {
                unsetenv("TZ");
            }
            tzset();

            CHECK_EQ(dto.offset().hours(), doctest::Approx(2.0));
        }
    }
} // namespace dnv::vista::sdk::tests
