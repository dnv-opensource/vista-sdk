#include <doctest/doctest.h>

#include <dnv/VistaSDK.h>

#include <format>
#include <sstream>

namespace dnv::vista::sdk::tests
{
    TEST_SUITE("DateTime")
    {
        TEST_CASE("construction - default")
        {
            DateTime dt;
            CHECK_EQ(dt.ticks(), DateTime::min().ticks());
            CHECK_EQ(dt.year(), 1);
            CHECK_EQ(dt.month(), 1);
            CHECK_EQ(dt.day(), 1);
        }

        TEST_CASE("construction - from ticks")
        {
            std::int64_t ticks{ 638400000000000000LL };
            DateTime dt{ ticks };
            CHECK_EQ(dt.ticks(), ticks);
        }

        TEST_CASE("construction - from date components")
        {
            DateTime dt{ 2024, 6, 15 };
            CHECK_EQ(dt.year(), 2024);
            CHECK_EQ(dt.month(), 6);
            CHECK_EQ(dt.day(), 15);
            CHECK_EQ(dt.hour(), 0);
            CHECK_EQ(dt.minute(), 0);
            CHECK_EQ(dt.second(), 0);
        }

        TEST_CASE("construction - from datetime components")
        {
            DateTime dt{ 2024, 12, 25, 18, 45, 30 };
            CHECK_EQ(dt.year(), 2024);
            CHECK_EQ(dt.month(), 12);
            CHECK_EQ(dt.day(), 25);
            CHECK_EQ(dt.hour(), 18);
            CHECK_EQ(dt.minute(), 45);
            CHECK_EQ(dt.second(), 30);
        }

        TEST_CASE("construction - from datetime with milliseconds")
        {
            DateTime dt{ 2024, 3, 10, 9, 15, 22, 500 };
            CHECK_EQ(dt.second(), 22);
            CHECK_EQ(dt.millisecond(), 500);
        }

        TEST_CASE("construction - from ISO 8601 string")
        {
            DateTime dt{ "2024-01-15T12:30:45Z" };
            CHECK_EQ(dt.year(), 2024);
            CHECK_EQ(dt.month(), 1);
            CHECK_EQ(dt.day(), 15);
            CHECK_EQ(dt.hour(), 12);
            CHECK_EQ(dt.minute(), 30);
            CHECK_EQ(dt.second(), 45);
        }

        TEST_CASE("construction - from chrono")
        {
            auto now{ std::chrono::system_clock::now() };
            DateTime dt{ now };
            CHECK_GT(dt.ticks(), 0);
        }

        TEST_CASE("comparison operators")
        {
            DateTime dt1{ 2024, 1, 15, 12, 0, 0 };
            DateTime dt2{ 2024, 1, 15, 12, 0, 0 };
            DateTime dt3{ 2024, 1, 15, 13, 0, 0 };

            CHECK(dt1 == dt2);
            CHECK_FALSE(dt1 == dt3);
            CHECK(dt1 != dt3);
            CHECK(dt1 < dt3);
            CHECK(dt1 <= dt2);
            CHECK(dt1 <= dt3);
            CHECK(dt3 > dt1);
            CHECK(dt3 >= dt1);
            CHECK(dt1 >= dt2);
        }

        TEST_CASE("arithmetic - add/subtract TimeSpan")
        {
            DateTime dt{ 2024, 1, 15, 12, 0, 0 };

            DateTime result1{ dt + TimeSpan::fromHours(3.0) };
            CHECK_EQ(result1.hour(), 15);
            CHECK_EQ(result1.day(), 15);

            DateTime result2{ dt - TimeSpan::fromHours(2.0) };
            CHECK_EQ(result2.hour(), 10);

            dt += TimeSpan::fromMinutes(45.0);
            CHECK_EQ(dt.minute(), 45);

            DateTime dt2{ 2024, 1, 15, 12, 30, 0 };
            dt2 -= TimeSpan::fromMinutes(30.0);
            CHECK_EQ(dt2.minute(), 0);
        }

        TEST_CASE("arithmetic - subtract DateTime")
        {
            DateTime dt1{ 2024, 1, 15, 15, 0, 0 };
            DateTime dt2{ 2024, 1, 15, 12, 0, 0 };
            TimeSpan diff{ dt1 - dt2 };
            CHECK_EQ(diff.hours(), 3.0);
        }

        TEST_CASE("accessors")
        {
            DateTime dt{ 2024, 3, 15, 14, 30, 45, 123 };
            CHECK_EQ(dt.year(), 2024);
            CHECK_EQ(dt.month(), 3);
            CHECK_EQ(dt.day(), 15);
            CHECK_EQ(dt.hour(), 14);
            CHECK_EQ(dt.minute(), 30);
            CHECK_EQ(dt.second(), 45);
            CHECK_EQ(dt.millisecond(), 123);
        }

        TEST_CASE("accessors - dayOfWeek and dayOfYear")
        {
            DateTime dt{ 2024, 1, 15, 12, 0, 0 };
            CHECK_EQ(dt.dayOfWeek(), 1); // Monday
            CHECK_EQ(dt.dayOfYear(), 15);
        }

        TEST_CASE("conversion - toEpochSeconds / toEpochMilliseconds")
        {
            DateTime epoch{ DateTime::epoch() };
            CHECK_EQ(epoch.toEpochSeconds(), 0);
            CHECK_EQ(epoch.toEpochMilliseconds(), 0);

            DateTime dt1{ 1970, 1, 1, 1, 0, 0 };
            CHECK_EQ(dt1.toEpochSeconds(), 3600);

            DateTime dt2{ 1970, 1, 1, 0, 0, 1, 500 };
            CHECK_EQ(dt2.toEpochMilliseconds(), 1500);
        }

        TEST_CASE("conversion - date and timeOfDay")
        {
            DateTime dt{ 2024, 3, 15, 14, 30, 45 };

            DateTime dateOnly{ dt.date() };
            CHECK_EQ(dateOnly.hour(), 0);
            CHECK_EQ(dateOnly.minute(), 0);
            CHECK_EQ(dateOnly.second(), 0);
            CHECK_EQ(dateOnly.day(), 15);

            TimeSpan timeOfDay{ dt.timeOfDay() };
            CHECK(doctest::Approx(timeOfDay.hours()).epsilon(0.001) == 14.0 + 30.0 / 60.0 + 45.0 / 3600.0);
        }

        TEST_CASE("add methods")
        {
            DateTime dt{ 2024, 1, 15, 12, 0, 0 };

            CHECK_EQ(dt.addDays(5.0).day(), 20);
            CHECK_EQ(dt.addHours(25.0).day(), 16);
            CHECK_EQ(dt.addHours(25.0).hour(), 13);
            CHECK_EQ(dt.addMinutes(90.0).hour(), 13);
            CHECK_EQ(dt.addMinutes(90.0).minute(), 30);
            CHECK_EQ(dt.addSeconds(3661.0).hour(), 13);
            CHECK_EQ(dt.addSeconds(3661.0).minute(), 1);
            CHECK_EQ(dt.addMilliseconds(1500.0).second(), 1);
            CHECK_EQ(dt.addMonths(2).month(), 3);
            CHECK_EQ(dt.addYears(1).year(), 2025);

            // addMonths clamps day overflow (Jan 31 + 1 month = Feb 28/29)
            DateTime jan31{ 2024, 1, 31, 0, 0, 0 };
            CHECK_EQ(jan31.addMonths(1).month(), 2);
            CHECK_EQ(jan31.addMonths(1).day(), 29); // 2024 is a leap year

            // addMonths wraps across years
            CHECK_EQ(dt.addMonths(13).year(), 2025);
            CHECK_EQ(dt.addMonths(13).month(), 2);

            // negative
            CHECK_EQ(dt.addMonths(-1).month(), 12);
            CHECK_EQ(dt.addMonths(-1).year(), 2023);
        }

        TEST_CASE("toString - Iso8601 basic")
        {
            DateTime dt{ 2024, 1, 15, 12, 30, 45 };
            std::string str{ dt.toString() };
            CHECK(str.find("2024-01-15") != std::string::npos);
            CHECK(str.find("12:30:45") != std::string::npos);
            CHECK(str.find("Z") != std::string::npos);
        }

        TEST_CASE("toString - Iso8601Millis")
        {
            CHECK_EQ(
                DateTime{ 2024, 1, 15, 10, 30, 12, 123 }.toString(DateTime::Format::Iso8601Millis),
                "2024-01-15T10:30:12.123Z");
            CHECK_EQ(
                DateTime{ 2024, 1, 15, 10, 30, 12, 0 }.toString(DateTime::Format::Iso8601Millis),
                "2024-01-15T10:30:12.000Z");
            CHECK_EQ(
                DateTime{ 2024, 1, 15, 10, 30, 12, 7 }.toString(DateTime::Format::Iso8601Millis),
                "2024-01-15T10:30:12.007Z");
        }

        TEST_CASE("toString - Iso8601PreciseTrimmed")
        {
            CHECK_EQ(
                DateTime{ 2024, 1, 15, 10, 30, 12, 123 }.toString(DateTime::Format::Iso8601PreciseTrimmed),
                "2024-01-15T10:30:12.123Z");
            CHECK_EQ(
                DateTime{ 2024, 1, 15, 10, 30, 12, 100 }.toString(DateTime::Format::Iso8601PreciseTrimmed),
                "2024-01-15T10:30:12.1Z");
            CHECK_EQ(
                DateTime{ 2024, 1, 15, 10, 30, 12, 0 }.toString(DateTime::Format::Iso8601PreciseTrimmed),
                "2024-01-15T10:30:12.0Z");
        }

        TEST_CASE("toString - Iso8601Micros")
        {
            DateTime dt1{ 2024, 1, 15, 10, 30, 12, 0 };
            dt1 = DateTime{ dt1.ticks() + 1234560LL };
            CHECK_EQ(dt1.toString(DateTime::Format::Iso8601Micros), "2024-01-15T10:30:12.123456Z");

            DateTime dt2{ 2024, 1, 15, 10, 30, 12, 0 };
            CHECK_EQ(dt2.toString(DateTime::Format::Iso8601Micros), "2024-01-15T10:30:12.000000Z");

            DateTime dt3{ 2024, 1, 15, 10, 30, 12, 123 };
            CHECK_EQ(dt3.toString(DateTime::Format::Iso8601Micros), "2024-01-15T10:30:12.123000Z");
        }

        TEST_CASE("toString - date only / time only")
        {
            DateTime dt{ 2024, 6, 20, 18, 45, 30 };
            std::string dateStr{ dt.toString(DateTime::Format::Iso8601Date) };
            CHECK(dateStr.find("2024-06-20") != std::string::npos);
            CHECK(dateStr.find("T") == std::string::npos);

            std::string timeStr{ dt.toString(DateTime::Format::Iso8601Time) };
            CHECK(timeStr.find("18:45:30") != std::string::npos);
            CHECK(timeStr.find("2024") == std::string::npos);
        }

        TEST_CASE("isLeapYear")
        {
            CHECK(DateTime::isLeapYear(2024));
            CHECK_FALSE(DateTime::isLeapYear(2023));
            CHECK_FALSE(DateTime::isLeapYear(1900));
            CHECK(DateTime::isLeapYear(2000));
        }

        TEST_CASE("daysInMonth")
        {
            CHECK_EQ(DateTime::daysInMonth(2024, 1), 31);
            CHECK_EQ(DateTime::daysInMonth(2024, 4), 30);
            CHECK_EQ(DateTime::daysInMonth(2024, 2), 29);
            CHECK_EQ(DateTime::daysInMonth(2023, 2), 28);
        }

        TEST_CASE("factory - min / max / epoch")
        {
            DateTime minVal{ DateTime::min() };
            CHECK_EQ(minVal.year(), 1);
            CHECK_EQ(minVal.month(), 1);
            CHECK_EQ(minVal.day(), 1);

            DateTime maxVal{ DateTime::max() };
            CHECK_EQ(maxVal.year(), 9999);
            CHECK_EQ(maxVal.month(), 12);
            CHECK_EQ(maxVal.day(), 31);

            DateTime epoch{ DateTime::epoch() };
            CHECK_EQ(epoch.year(), 1970);
            CHECK_EQ(epoch.month(), 1);
            CHECK_EQ(epoch.day(), 1);
            CHECK_EQ(epoch.hour(), 0);
        }

        TEST_CASE("factory - fromEpochSeconds / fromEpochMilliseconds")
        {
            DateTime dt1{ DateTime::fromEpochSeconds(3600) };
            CHECK_EQ(dt1.year(), 1970);
            CHECK_EQ(dt1.hour(), 1);

            DateTime dt2{ DateTime::fromEpochMilliseconds(1500) };
            CHECK_EQ(dt2.second(), 1);
            CHECK_EQ(dt2.millisecond(), 500);
        }

        TEST_CASE("factory - utcNow")
        {
            DateTime now{ DateTime::utcNow() };
            CHECK_GT(now.ticks(), 0);
            CHECK_GE(now.year(), 2024);
        }

        TEST_CASE("parsing - ISO 8601 with fractional seconds")
        {
            DateTime dt{ "2024-01-15T12:30:45.123Z" };
            CHECK_EQ(dt.second(), 45);
            CHECK_EQ(dt.millisecond(), 123);
        }

        TEST_CASE("parsing - date only")
        {
            DateTime dt{ "2024-06-20" };
            CHECK_EQ(dt.year(), 2024);
            CHECK_EQ(dt.month(), 6);
            CHECK_EQ(dt.day(), 20);
            CHECK_EQ(dt.hour(), 0);
        }

        TEST_CASE("fromString - optional overload")
        {
            auto dt1{ DateTime::fromString("2024-01-15T12:30:45Z") };
            CHECK(dt1.has_value());
            CHECK_EQ(dt1->year(), 2024);
            CHECK_EQ(dt1->hour(), 12);

            CHECK_FALSE(DateTime::fromString("invalid-date-string").has_value());
        }

        TEST_CASE("fromString - out-parameter overload")
        {
            DateTime dt;
            CHECK(DateTime::fromString("2024-01-15T12:30:45Z", dt));
            CHECK_EQ(dt.year(), 2024);
            CHECK_EQ(dt.hour(), 12);

            CHECK_FALSE(DateTime::fromString("invalid-date-string", dt));
        }

        TEST_CASE("parsing - constructor throws on invalid")
        {
            CHECK_THROWS_AS((void)DateTime{ "not-a-date" }, std::invalid_argument);
        }

        TEST_CASE("chrono - round-trip toChrono / fromChrono")
        {
            DateTime original{ 2024, 3, 10, 14, 30, 22 };
            auto tp{ original.toChrono() };
            DateTime roundTrip{ DateTime::fromChrono(tp) };
            CHECK_EQ(roundTrip.year(), original.year());
            CHECK_EQ(roundTrip.month(), original.month());
            CHECK_EQ(roundTrip.day(), original.day());
            CHECK_EQ(roundTrip.hour(), original.hour());
            CHECK_EQ(roundTrip.minute(), original.minute());
            CHECK_EQ(roundTrip.second(), original.second());
        }

        TEST_CASE("stream output operator")
        {
            DateTime dt{ 2024, 1, 15, 12, 30, 45 };
            std::ostringstream oss;
            oss << dt;
            CHECK(oss.str().find("2024") != std::string::npos);
            CHECK(oss.str().find("12:30:45") != std::string::npos);
        }

        TEST_CASE("stream input operator")
        {
            std::istringstream iss{ "2024-01-15T12:30:45Z" };
            DateTime dt;
            iss >> dt;
            CHECK_EQ(dt.year(), 2024);
            CHECK_EQ(dt.hour(), 12);
        }

        TEST_CASE("std::format support")
        {
            DateTime dt{ 2024, 1, 15, 12, 30, 45 };
            std::string formatted{ std::format("{}", dt) };
            CHECK(formatted.find("2024-01-15") != std::string::npos);
            CHECK(formatted.find("12:30:45") != std::string::npos);
            CHECK(formatted.find("Z") != std::string::npos);
        }

        TEST_CASE("edge case - crossing day boundary")
        {
            DateTime dt{ 2024, 1, 15, 23, 0, 0 };
            DateTime result{ dt + TimeSpan::fromHours(2.0) };
            CHECK_EQ(result.day(), 16);
            CHECK_EQ(result.hour(), 1);
        }

        TEST_CASE("edge case - crossing month boundary")
        {
            DateTime dt{ 2024, 1, 31, 12, 0, 0 };
            DateTime result{ dt + TimeSpan::fromDays(1.0) };
            CHECK_EQ(result.month(), 2);
            CHECK_EQ(result.day(), 1);
        }

        TEST_CASE("edge case - crossing year boundary")
        {
            DateTime dt{ 2023, 12, 31, 23, 59, 59 };
            DateTime result{ dt + TimeSpan::fromSeconds(1.0) };
            CHECK_EQ(result.year(), 2024);
            CHECK_EQ(result.month(), 1);
            CHECK_EQ(result.day(), 1);
            CHECK_EQ(result.hour(), 0);
            CHECK_EQ(result.minute(), 0);
            CHECK_EQ(result.second(), 0);
        }

        TEST_CASE("edge case - leap year February 29")
        {
            DateTime dt{ 2024, 2, 29, 12, 0, 0 };
            CHECK_EQ(dt.month(), 2);
            CHECK_EQ(dt.day(), 29);
            CHECK(DateTime::isLeapYear(2024));
        }

        TEST_CASE("integration - round-trip serialization")
        {
            DateTime original{ 2024, 3, 15, 14, 30, 45, 123 };
            std::string serialized{ original.toString(DateTime::Format::Iso8601PreciseTrimmed) };
            DateTime deserialized{ DateTime{ serialized } };
            CHECK_EQ(deserialized.year(), original.year());
            CHECK_EQ(deserialized.month(), original.month());
            CHECK_EQ(deserialized.day(), original.day());
            CHECK_EQ(deserialized.hour(), original.hour());
            CHECK_EQ(deserialized.minute(), original.minute());
            CHECK_EQ(deserialized.second(), original.second());
            CHECK_EQ(deserialized.millisecond(), original.millisecond());
            CHECK_EQ(serialized, "2024-03-15T14:30:45.123Z");
        }

        TEST_CASE("integration - Unix timestamp round-trip")
        {
            std::int64_t epochSeconds{ 1704110400LL };
            DateTime dt{ DateTime::fromEpochSeconds(epochSeconds) };
            CHECK_EQ(dt.toEpochSeconds(), epochSeconds);
        }

        TEST_CASE("integration - arithmetic chaining")
        {
            DateTime dt{ 2024, 1, 1, 0, 0, 0 };
            DateTime result{ dt + TimeSpan::fromDays(5.0) + TimeSpan::fromHours(3.0) + TimeSpan::fromMinutes(30.0) };
            CHECK_EQ(result.day(), 6);
            CHECK_EQ(result.hour(), 3);
            CHECK_EQ(result.minute(), 30);
        }
    }
} // namespace dnv::vista::sdk::tests
