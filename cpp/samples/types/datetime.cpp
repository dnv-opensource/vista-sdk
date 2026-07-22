/**
 * @file datetime.cpp
 * @brief Demonstrates DateTime, TimeSpan, and DateTimeOffset usage
 * @details This sample shows how to use the Vista SDK datetime types for high-precision
 *          temporal operations, ISO 8601 parsing/formatting, and timezone handling
 */

#include <dnv/VistaSDK.h>

#include <iostream>
#include <string>

int main()
{
    using namespace dnv::vista::sdk;

    std::cout << "=== vista-sdk DateTime samples ===\n\n";

    {
        std::cout << "1. DateTime: Basic construction\n";
        std::cout << "--------------------------------\n";

        // DateTime is always UTC - sensor timestamps in a DataChannel are recorded in UTC,
        // use DateTimeOffset::now() for bridge/local-time display
        DateTime utcNow{ DateTime::utcNow() };
        DateTimeOffset localNow{ DateTimeOffset::now() };
        DateTimeOffset today{ DateTimeOffset::today() };
        DateTime sensorReading{ 2026, 7, 14, 14, 30, 45 };
        DateTime epoch{ DateTime::epoch() };

        std::cout << "Current UTC time:        " << utcNow.toString() << "\n";
        std::cout << "Current LOCAL time:      " << localNow.toString() << "\n";
        std::cout << "Today at midnight:       " << today.toString() << "\n";
        std::cout << "Engine sensor timestamp: " << sensorReading.toString() << "\n";
        std::cout << "Unix epoch:              " << epoch.toString() << "\n";
        std::cout << "Note: DateTime is always UTC - use DateTimeOffset for local time\n";
        std::cout << "\n";
    }

    {
        std::cout << "2. DateTime: Parsing ISO 8601 strings\n";
        std::cout << "--------------------------------------\n";

        // A TimeSeriesData payload timestamp and a scheduled dry-dock date
        DateTime dt1{ DateTime{ "2026-07-14T14:30:45Z" } };
        DateTime dt2{ DateTime{ "2026-07-14" } };

        std::cout << "Parsed sensor timestamp \"2026-07-14T14:30:45Z\": " << dt1.toString() << "\n";
        std::cout << "Parsed dry-dock date \"2026-07-14\":              " << dt2.toString() << "\n";

        // Safe parsing with fromString
        DateTime result;
        bool success{ DateTime::fromString("2026-07-14T00:00:00Z", result) };
        std::cout << "fromString success:             " << (success ? "true" : "false") << "\n";
        if (success)
        {
            std::cout << "  Parsed value:                 " << result.toString() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "3. DateTime: Accessing components\n";
        std::cout << "----------------------------------\n";

        // Timestamp of a high-precision engine temperature reading (qty-temperature)
        DateTime dt{ 2026, 7, 14, 14, 30, 45, 123 };
        std::cout << "Sensor timestamp: " << dt.toString(DateTime::Format::Iso8601Precise) << "\n";
        std::cout << "  Year:         " << dt.year() << "\n";
        std::cout << "  Month:        " << dt.month() << "\n";
        std::cout << "  Day:          " << dt.day() << "\n";
        std::cout << "  Hour:         " << dt.hour() << "\n";
        std::cout << "  Minute:       " << dt.minute() << "\n";
        std::cout << "  Second:       " << dt.second() << "\n";
        std::cout << "  Millisecond:  " << dt.millisecond() << "\n";
        std::cout << "  Day of week:  " << dt.dayOfWeek() << " (0=Sunday)\n";
        std::cout << "  Day of year:  " << dt.dayOfYear() << "\n";

        std::cout << "\n";
    }

    {
        std::cout << "4. DateTime: Arithmetic operations\n";
        std::cout << "-----------------------------------\n";

        // Voyage departure and estimated time of arrival
        DateTime departure{ 2026, 7, 14, 10, 0, 0 };
        std::cout << "Departure (UTC):         " << departure.toString() << "\n";

        DateTime eta{ departure + TimeSpan::fromHours(2.5) };
        std::cout << "ETA (+2.5h transit):     " << eta.toString() << "\n";

        DateTime delayedDeparture{ departure - TimeSpan::fromMinutes(30.0) };
        std::cout << "Departure moved -30 min: " << delayedDeparture.toString() << "\n";

        TimeSpan transitTime{ eta - departure };
        std::cout << "Transit time:            " << transitTime.toString() << " (" << transitTime.hours()
                  << " hours)\n";

        std::cout << "\n";
    }

    {
        std::cout << "5. DateTime: Comparisons\n";
        std::cout << "------------------------\n";

        // Comparing maintenance log timestamps for the same component
        DateTime inspectionStart{ 2026, 7, 14, 10, 0, 0 };
        DateTime inspectionEnd{ 2026, 7, 14, 14, 0, 0 };
        DateTime duplicateEntry{ 2026, 7, 14, 10, 0, 0 };

        std::cout << std::boolalpha;
        std::cout << "inspectionStart == duplicateEntry:  " << (inspectionStart == duplicateEntry) << "\n";
        std::cout << "inspectionStart != inspectionEnd:   " << (inspectionStart != inspectionEnd) << "\n";
        std::cout << "inspectionStart < inspectionEnd:    " << (inspectionStart < inspectionEnd) << "\n";
        std::cout << "inspectionEnd > inspectionStart:    " << (inspectionEnd > inspectionStart) << "\n";

        std::cout << "\n";
    }

    {
        std::cout << "6. DateTime: Epoch timestamp conversions\n";
        std::cout << "----------------------------------------\n";

        // DataChannelList timestamps are often exchanged as Unix epoch values
        DateTime dt{ 2026, 7, 14, 0, 0, 0 };
        std::cout << "DateTime:              " << dt.toString() << "\n";
        std::cout << "Unix seconds:          " << dt.toEpochSeconds() << "\n";
        std::cout << "Unix milliseconds:     " << dt.toEpochMilliseconds() << "\n";

        DateTime fromEpoch{ DateTime::fromEpochSeconds(1704067200) };
        std::cout << "From epoch 1704067200: " << fromEpoch.toString() << "\n";

        std::cout << "\n";
    }

    {
        std::cout << "7. DateTime: Leap year handling\n";
        std::cout << "--------------------------------\n";

        // Relevant for annual survey/certification scheduling that spans February
        std::cout << std::boolalpha;
        std::cout << "2024 is leap year:    " << DateTime::isLeapYear(2024) << "\n";
        std::cout << "2023 is leap year:    " << DateTime::isLeapYear(2023) << "\n";
        std::cout << "Days in Feb 2024:     " << DateTime::daysInMonth(2024, 2) << "\n";
        std::cout << "Days in Feb 2023:     " << DateTime::daysInMonth(2023, 2) << "\n";

        DateTime surveyDate{ 2024, 2, 29, 12, 0, 0 };
        std::cout << "Survey scheduled Feb 29, 2024: " << surveyDate.toString() << "\n";

        std::cout << "\n";
    }

    {
        std::cout << "8. TimeSpan: Creating durations\n";
        std::cout << "--------------------------------\n";

        // Typical durations used in voyage planning and maintenance intervals
        TimeSpan voyageDuration{ TimeSpan::fromDays(1.5) };
        TimeSpan bunkeringDuration{ TimeSpan::fromHours(2.5) };
        TimeSpan portStay{ TimeSpan::fromMinutes(90.0) };
        TimeSpan engineWarmup{ TimeSpan::fromSeconds(3600.0) };

        std::cout << "Voyage duration (1.5 days):    " << voyageDuration.toString() << "\n";
        std::cout << "Bunkering duration (2.5h):     " << bunkeringDuration.toString() << "\n";
        std::cout << "Port stay (90 min):            " << portStay.toString() << "\n";
        std::cout << "Engine warmup (3600s):         " << engineWarmup.toString() << "\n";
        std::cout << "Note: ISO 8601 duration format (PT = Period Time)\n";

        std::cout << "\n";
    }

    {
        std::cout << "9. TimeSpan: Parsing ISO 8601 durations\n";
        std::cout << "----------------------------------------\n";

        // Maintenance schedule intervals expressed as ISO 8601 durations
        TimeSpan ts1{ TimeSpan{ "PT1H" } };
        TimeSpan ts2{ TimeSpan{ "PT1H30M45S" } };
        TimeSpan ts3{ TimeSpan{ "P1DT12H" } };
        TimeSpan ts4{ TimeSpan{ "-PT2H30M" } };

        std::cout << "PT1H (watch rotation):        " << ts1.hours() << " hours\n";
        std::cout << "PT1H30M45S (engine test run):  " << ts2.seconds() << " seconds\n";
        std::cout << "P1DT12H (port turnaround):     " << ts3.hours() << " hours\n";
        std::cout << "-PT2H30M (schedule offset):    " << ts4.minutes() << " minutes\n";

        std::cout << "\n";
    }

    {
        std::cout << "10. TimeSpan: Unit conversions\n";
        std::cout << "-------------------------------\n";

        // Duration of a scheduled engine overhaul
        TimeSpan overhaulDuration{ TimeSpan::fromHours(2.5) };
        std::cout << "Overhaul duration: 2.5 hours\n";
        std::cout << "  Total days:          " << overhaulDuration.days() << "\n";
        std::cout << "  Total hours:         " << overhaulDuration.hours() << "\n";
        std::cout << "  Total minutes:       " << overhaulDuration.minutes() << "\n";
        std::cout << "  Total seconds:       " << overhaulDuration.seconds() << "\n";
        std::cout << "  Total milliseconds:  " << overhaulDuration.milliseconds() << "\n";
        std::cout << "  Ticks (100ns):       " << overhaulDuration.ticks() << "\n";

        std::cout << "\n";
    }

    {
        std::cout << "11. TimeSpan: Arithmetic operations\n";
        std::cout << "------------------------------------\n";

        // Combining watch durations for a bridge crew schedule
        TimeSpan morningWatch{ TimeSpan::fromHours(2.0) };
        TimeSpan handoverBuffer{ TimeSpan::fromMinutes(30.0) };

        std::cout << "Morning watch (2h):      " << morningWatch.toString() << "\n";
        std::cout << "Handover buffer (30m):   " << handoverBuffer.toString() << "\n";
        std::cout << "watch + buffer:          " << (morningWatch + handoverBuffer).toString() << "\n";
        std::cout << "watch - buffer:          " << (morningWatch - handoverBuffer).toString() << "\n";
        std::cout << "-watch:                  " << (-morningWatch).toString() << "\n";

        TimeSpan fullShift{ TimeSpan::fromDays(1.0) + TimeSpan::fromHours(3.0) + TimeSpan::fromMinutes(30.0) };
        std::cout << "1d + 3h + 30m shift:     " << fullShift.toString() << "\n";

        std::cout << "\n";
    }

    {
        std::cout << "12. TimeSpan: Comparisons\n";
        std::cout << "-------------------------\n";

        // Comparing two anchoring durations
        TimeSpan shortAnchorage{ TimeSpan::fromMinutes(30.0) };
        TimeSpan longAnchorage{ TimeSpan::fromHours(2.0) };

        std::cout << std::boolalpha;
        std::cout << "30 min == 30 min:  " << (shortAnchorage == TimeSpan::fromMinutes(30.0)) << "\n";
        std::cout << "30 min < 2 hours:  " << (shortAnchorage < longAnchorage) << "\n";
        std::cout << "2 hours > 30 min:  " << (longAnchorage > shortAnchorage) << "\n";

        std::cout << "\n";
    }

    {
        std::cout << "13. DateTimeOffset: Basic construction with timezone\n";
        std::cout << "-----------------------------------------------------\n";

        // Local bridge time versus port-local timezones for two of the vessel's calls
        DateTimeOffset now{ DateTimeOffset::now() };
        DateTimeOffset utcNow{ DateTimeOffset::utcNow() };
        DateTimeOffset yokohama{ 2026, 7, 14, 14, 30, 0, TimeSpan::fromHours(9.0) };
        DateTimeOffset newYork{ 2026, 7, 14, 14, 30, 0, TimeSpan::fromHours(-5.0) };

        std::cout << "Bridge local time:        " << now.toString() << "\n";
        std::cout << "UTC time:                 " << utcNow.toString() << "\n";
        std::cout << "Port of Yokohama (UTC+9): " << yokohama.toString() << "\n";
        std::cout << "Port of New York (UTC-5): " << newYork.toString() << "\n";
        std::cout << "Note: DateTimeOffset includes timezone offset\n";

        std::cout << "\n";
    }

    {
        std::cout << "14. DateTimeOffset: Parsing with timezone offsets\n";
        std::cout << "--------------------------------------------------\n";

        // Port call records arriving with different local timezone offsets
        DateTimeOffset dto1{ DateTimeOffset{ "2026-07-14T14:30:00+09:00" } };
        DateTimeOffset dto2{ DateTimeOffset{ "2026-07-14T14:30:00-05:00" } };
        DateTimeOffset dto3{ DateTimeOffset{ "2026-07-14T14:30:00Z" } };

        std::cout << "Parsed +09:00:  " << dto1.toString() << "\n";
        std::cout << "Parsed -05:00:  " << dto2.toString() << "\n";
        std::cout << "Parsed Z (UTC): " << dto3.toString() << "\n";

        std::cout << "\n";
    }

    {
        std::cout << "15. DateTimeOffset: Timezone conversions\n";
        std::cout << "-----------------------------------------\n";

        // Same arrival instant expressed in different port timezones
        DateTimeOffset yokohama{ 2026, 7, 14, 18, 0, 0, TimeSpan::fromHours(9.0) };
        std::cout << "Arrival, Yokohama (UTC+9):  " << yokohama.toString() << "\n";

        DateTimeOffset newYork{ yokohama.toOffset(TimeSpan::fromHours(-5.0)) };
        std::cout << "Same instant in New York:   " << newYork.toString() << "\n";

        DateTimeOffset utc{ yokohama.toUniversalTime() };
        std::cout << "Same instant in UTC:        " << utc.toString() << "\n";

        std::cout << "\nAll represent same instant: " << std::boolalpha << (yokohama.utcTicks() == newYork.utcTicks())
                  << "\n";

        std::cout << "\n";
    }

    {
        std::cout << "16. DateTimeOffset: Accessing components\n";
        std::cout << "-----------------------------------------\n";

        // A DataChannel reading recorded with a local (non-UTC) timezone offset
        DateTimeOffset dto{ 2026, 7, 14, 14, 30, 45, TimeSpan::fromHours(2.0) };
        std::cout << "DateTimeOffset:     " << dto.toString() << "\n";
        std::cout << "  Local time:       " << dto.toString(DateTime::Format::Iso8601Extended) << "\n";
        std::cout << "  UTC time:         " << dto.utcDateTime().toString() << "\n";
        std::cout << "  Offset:           " << dto.offset().toString() << "\n";
        std::cout << "  Offset minutes:   " << dto.totalOffsetMinutes() << "\n";

        std::cout << "\n";
    }

    {
        std::cout << "17. DateTimeOffset: Arithmetic operations\n";
        std::cout << "------------------------------------------\n";

        // Estimated berth availability window
        DateTimeOffset berthAvailable{ 2026, 7, 14, 10, 0, 0, TimeSpan::fromHours(2.0) };
        std::cout << "Berth available from:  " << berthAvailable.toString() << "\n";

        DateTimeOffset berthDeadline{ berthAvailable + TimeSpan::fromHours(3.0) };
        std::cout << "Berth deadline (+3h):  " << berthDeadline.toString() << "\n";

        // Difference between two port calls in different timezones (same UTC moment)
        DateTimeOffset yokohamaDeparture{ 2026, 7, 14, 18, 0, 0, TimeSpan::fromHours(9.0) };
        DateTimeOffset newYorkArrival{ 2026, 7, 14, 4, 0, 0, TimeSpan::fromHours(-5.0) };
        TimeSpan diff{ yokohamaDeparture - newYorkArrival };
        std::cout << "\nYokohama departure:    " << yokohamaDeparture.toString() << "\n";
        std::cout << "New York arrival:      " << newYorkArrival.toString() << "\n";
        std::cout << "Difference (UTC):      " << diff.toString() << " (same UTC moment)\n";

        std::cout << "\n";
    }

    {
        std::cout << "18. DateTimeOffset: Comparisons (UTC-based)\n";
        std::cout << "--------------------------------------------\n";

        // Same signal received by shore stations in different timezones
        DateTimeOffset yokohama{ 2026, 7, 14, 18, 0, 0, TimeSpan::fromHours(9.0) };
        DateTimeOffset newYork{ 2026, 7, 14, 4, 0, 0, TimeSpan::fromHours(-5.0) };

        std::cout << "Yokohama:                  " << yokohama.toString() << "\n";
        std::cout << "New York:                  " << newYork.toString() << "\n";
        std::cout << std::boolalpha;
        std::cout << "Equal (same UTC moment):   " << (yokohama == newYork) << "\n";
        std::cout << "Exact (incl offset):       " << yokohama.equalsExact(newYork) << "\n";

        std::cout << "\n";
    }

    {
        std::cout << "19. Integration: Maintenance task scheduling across timezones\n";
        std::cout << "-----------------------------------------------------------------\n";

        // A scheduled engine maintenance task recorded in UTC, reported to shore in
        // different timezones
        DateTime taskStart{ 2026, 7, 14, 9, 0, 0 };
        TimeSpan duration{ TimeSpan::fromHours(2.0) + TimeSpan::fromMinutes(30.0) };
        DateTime taskEnd{ taskStart + duration };

        std::cout << "Maintenance starts (UTC): " << taskStart.toString() << "\n";
        std::cout << "Duration:                 " << duration.toString() << "\n";
        std::cout << "Maintenance ends (UTC):   " << taskEnd.toString() << "\n";

        DateTimeOffset endUtc{ taskEnd, TimeSpan{} };
        DateTimeOffset endYokohama{ endUtc.toOffset(TimeSpan::fromHours(9.0)) };
        DateTimeOffset endNY{ endUtc.toOffset(TimeSpan::fromHours(-5.0)) };
        std::cout << "End reported to Yokohama: " << endYokohama.toString() << "\n";
        std::cout << "End reported to New York: " << endNY.toString() << "\n";

        std::cout << "\n";
    }

    {
        std::cout << "20. Integration: Port agent coordination across timezones\n";
        std::cout << "-------------------------------------------------------------\n";

        // Scheduling a port-agent call that must line up across three shore offices
        DateTimeOffset callYokohama{ 2026, 7, 14, 14, 0, 0, TimeSpan::fromHours(9.0) };
        std::cout << "Port agent call, Yokohama (UTC+9): " << callYokohama.toString() << "\n";

        DateTimeOffset callNY{ callYokohama.toOffset(TimeSpan::fromHours(-5.0)) };
        DateTimeOffset callLondon{ callYokohama.toOffset(TimeSpan::fromHours(1.0)) };
        DateTimeOffset callSydney{ callYokohama.toOffset(TimeSpan::fromHours(10.0)) };

        std::cout << "Same call in New York:             " << callNY.toString() << " (" << callNY.hour() << ":00)\n";
        std::cout << "Same call in London:               " << callLondon.toString() << " (" << callLondon.hour()
                  << ":00)\n";
        std::cout << "Same call in Sydney:               " << callSydney.toString() << " (" << callSydney.hour()
                  << ":00)\n";

        std::cout << "\n";
    }

    {
        using namespace dnv::vista::sdk::literals;

        std::cout << "21. TimeSpan: Using modern C++ user-defined literals\n";
        std::cout << "-----------------------------------------------------\n";

        auto duration1 = 2_h;
        auto duration2 = 30_min;
        auto duration3 = 45_s;
        auto duration4 = 500_ms;

        std::cout << "2 hours:        " << duration1.toString() << " (" << duration1.hours() << " hours)\n";
        std::cout << "30 minutes:     " << duration2.toString() << " (" << duration2.minutes() << " minutes)\n";
        std::cout << "45 seconds:     " << duration3.toString() << " (" << duration3.seconds() << " seconds)\n";
        std::cout << "500 millisec:   " << duration4.toString() << " (" << duration4.milliseconds() << " ms)\n";

        auto watchDuration = 8_h + 30_min;
        auto handoverBreak = 1_h;
        auto activeWatch = watchDuration - handoverBreak;

        std::cout << "\nBridge watch example:\n";
        std::cout << "  Full watch cycle: " << watchDuration.toString() << " (" << watchDuration.hours() << " hours)\n";
        std::cout << "  Handover break:   " << handoverBreak.toString() << " (" << handoverBreak.hours() << " hour)\n";
        std::cout << "  Active watch:     " << activeWatch.toString() << " (" << activeWatch.hours() << " hours)\n";

        auto engineTestRun = 1_h + 23_min + 45_s;
        auto crewRestPeriod = 8_h + 15_min;
        auto bunkeringOperation = 45_min + 30_s;

        std::cout << "\nShipboard examples:\n";
        std::cout << "  Engine test run:     " << engineTestRun.toString() << "\n";
        std::cout << "  Crew rest period:    " << crewRestPeriod.toString() << "\n";
        std::cout << "  Bunkering operation: " << bunkeringOperation.toString() << "\n";

        auto duration5 = 2.5_h;
        auto duration6 = 30.5_min;
        auto duration7 = 1.5_s;

        std::cout << "\nFractional values:\n";
        std::cout << "  2.5 hours:    " << duration5.toString() << " (" << duration5.minutes() << " minutes)\n";
        std::cout << "  30.5 minutes: " << duration6.toString() << " (" << duration6.seconds() << " seconds)\n";
        std::cout << "  1.5 seconds:  " << duration7.toString() << " (" << duration7.milliseconds() << " ms)\n";

        constexpr auto compileTime = 2_h + 30_min;
        std::cout << "\nCompile-time literal: " << compileTime.toString() << " (evaluated at compile-time!)\n";

        std::cout << "\n";
    }

    {
        std::cout << "22. DateTimeOffset: std::chrono interop (toChrono / fromChrono)\n";
        std::cout << "----------------------------------------------------------------\n";

        // UTC departure time stored in a system_clock::time_point
        DateTimeOffset departure{ 2026, 7, 14, 9, 0, 0, TimeSpan{} };
        auto tp{ departure.toChrono() };
        std::cout << "Departure (UTC):             " << departure.toString() << "\n";
        std::cout << "As time_point epoch seconds: "
                  << std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count() << "\n";

        // Reconstruct from time_point with no offset (UTC)
        DateTimeOffset recoveredUtc{ DateTimeOffset::fromChrono(tp) };
        std::cout << "Recovered (UTC):             " << recoveredUtc.toString() << "\n";

        // Same instant expressed in Tokyo time (+09:00)
        DateTimeOffset tokyoArrival{ DateTimeOffset::fromChrono(tp, std::chrono::seconds{ 9 * 3600 }) };
        std::cout << "Same instant in Tokyo:       " << tokyoArrival.toString() << "\n";
        std::cout << "UTC instant preserved:       " << (tokyoArrival.toChrono() == tp ? "yes" : "no") << "\n";

        std::cout << "\n";
    }

    return 0;
}
