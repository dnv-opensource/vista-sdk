/**
 * @file sensorsdataflow.cpp
 * @brief Demonstrates usage of vista-sdk for transforming proprietary sensor data to ISO19848
 * @details This sample shows how to use DataChannelList as a mapping layer between
 *          proprietary system IDs (short_id) and ISO19848 LocalIds, then aggregate sensor
 *          readings into TabularData groups based on updateCycle (ISO19848 optimization)
 */

#include <dnv/vista/sdk/core/VIS.h>
#include <dnv/vista/sdk/transport/serialization/json/datachannel/Extensions.h>
#include <dnv/vista/sdk/transport/serialization/json/timeseries/Extensions.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

int main()
{
    using namespace dnv::vista::sdk;
    using namespace dnv::vista::sdk::transport;
    namespace jdc = dnv::vista::sdk::transport::serialization::json::datachannel;
    namespace jts = dnv::vista::sdk::transport::serialization::json::timeseries;

    struct SensorReading
    {
        std::string systemId;
        double value;
        DateTimeOffset timestamp;
        std::string quality;
    };

    auto formatDouble = [](double value) {
        std::string str = std::to_string(value);
        str.erase(str.find_last_not_of('0') + 1, std::string::npos);
        if (str.back() == '.')
        {
            str.pop_back();
        }
        return str;
    };

    auto createDataChannelList = []() {
        datachannel::DataChannelList dcList;

        auto localId1Opt =
            LocalId::fromString("/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/meta/qty-temperature/cnt-cooling.air");
        if (!localId1Opt.has_value())
        {
            throw std::runtime_error{ "Failed to parse localId1" };
        }
        auto localId1 = localId1Opt.value();

        auto dcId1 = datachannel::DataChannelId{ localId1, std::optional<std::string>{ "TEMP001" }, std::nullopt };

        auto restriction1 = datachannel::Restriction{};
        restriction1.setFractionDigits(1);
        restriction1.setMaxInclusive(200.0);
        restriction1.setMinInclusive(-50.0);

        auto format1 = datachannel::Format{ "Decimal", restriction1 };
        auto dataChannelType1 = datachannel::DataChannelType{ "Inst", 1.0 };
        auto range1 = datachannel::Range{ 0.0, 150.0 };
        auto unit1 = datachannel::Unit{ "°C", "Temperature" };

        auto prop1 = datachannel::Property{
            dataChannelType1, format1, range1, unit1, "OPC_QUALITY", std::nullopt, "Main Engine Air Cooler Temperature"
        };
        dcList.add(datachannel::DataChannel{ dcId1, prop1 });

        auto localId2Opt = LocalId::fromString("/dnv-v2/vis-3-4a/511.15-1/E32/meta/qty-power");
        if (!localId2Opt.has_value())
        {
            throw std::runtime_error{ "Failed to parse localId2" };
        }
        auto localId2 = localId2Opt.value();

        auto dcId2 = datachannel::DataChannelId{ localId2, std::optional<std::string>{ "PWR001" }, std::nullopt };
        auto format2 = datachannel::Format{ "Decimal", datachannel::Restriction{} };
        auto dataChannelType2 = datachannel::DataChannelType{ "Inst", 1.0 };
        auto range2 = datachannel::Range{ 0.0, 10000.0 };
        auto unit2 = datachannel::Unit{ "kW", "Power" };

        auto prop2 = datachannel::Property{ dataChannelType2,        format2, range2, unit2, std::nullopt, std::nullopt,
                                            "Generator Power Output" };
        dcList.add(datachannel::DataChannel{ dcId2, prop2 });

        auto localId3Opt = LocalId::fromString("/dnv-v2/vis-3-4a/621.22i/S110/meta/cnt-diesel.oil/cmd-stop");
        if (!localId3Opt.has_value())
        {
            throw std::runtime_error{ "Failed to parse localId3" };
        }
        auto localId3 = localId3Opt.value();

        auto dcId3 = datachannel::DataChannelId{ localId3, std::optional<std::string>{ "ALT001" }, std::nullopt };
        auto format3 = datachannel::Format{ "Boolean", datachannel::Restriction{} };
        auto dataChannelType3 = datachannel::DataChannelType{ "Command" };

        auto prop3 = datachannel::Property{
            dataChannelType3, format3, std::nullopt, std::nullopt, std::nullopt, std::nullopt, "Diesel Oil Stop Command"
        };
        dcList.add(datachannel::DataChannel{ dcId3, prop3 });

        auto shipIdOpt = ShipId::fromString("IMO1234567");
        if (!shipIdOpt.has_value())
        {
            throw std::runtime_error{ "Failed to parse shipId" };
        }
        auto shipId = shipIdOpt.value();
        auto configRef = datachannel::ConfigurationReference{ "DataChannelList-v1",
                                                              DateTimeOffset{ "2026-07-17T00:00:00Z" },
                                                              "1.0" };
        auto versionInfo = datachannel::VersionInformation{ "dnv", "v2", "https://docs.vista.dnv.com" };
        auto header = datachannel::Header{
            shipId, configRef, versionInfo, "Vista SDK Sample", DateTimeOffset{ "2026-07-17T00:00:00Z" }
        };
        auto package = datachannel::Package{ header, dcList };
        return datachannel::DataChannelListPackage{ package };
    };

    std::cout << "=== vista-sdk SensorDataFlow Sample ===\n\n";

    {
        std::cout << "1. DataChannelList: Creating sample data channel list\n";
        std::cout << "--------------------------------------------------------\n";

        auto dclPackage = createDataChannelList();

        std::cout << "Created DataChannelList with " << dclPackage.dataChannelList().size() << " channels:\n";

        for (const auto& dc : dclPackage.dataChannelList())
        {
            auto cycle = dc.property().dataChannelType().updateCycle();
            std::string cycleStr = cycle.has_value() ? formatDouble(cycle.value()) + "s" : "None";
            std::cout << "  " << dc.dataChannelId().shortId().value_or("<no-short-id>") << " (updateCycle=" << cycleStr
                      << ")\n";
        }

        {
            std::cout << "\n";
            auto jsonStr = jdc::toJsonString(dclPackage, true);
            std::cout << "Serialized to JSON: " << jsonStr.size() << " bytes\n";
            std::cout << "Package:\n" << jsonStr << "\n";
        }
        std::cout << "\n";

        std::cout << "2. Proprietary sensor data: Simulating sensor readings\n";
        std::cout << "---------------------------------------------------------\n";

        auto baseTime = DateTimeOffset{ 2026, 7, 17, 10, 0, 0, TimeSpan{} };

        std::vector<SensorReading> readings{
            { "TEMP001", 45.2, baseTime, "Good" },  { "TEMP001", 46.1, baseTime + TimeSpan::fromMinutes(5), "Good" },
            { "PWR001", 2500.0, baseTime, "Good" }, { "PWR001", 2650.5, baseTime + TimeSpan::fromMinutes(5), "Good" },
            { "ALT001", 1.0, baseTime, "Good" },    { "UNKNOWN", 123.4, baseTime, "Good" },
        };

        std::cout << "Created " << readings.size() << " sensor readings from proprietary system\n";
        std::cout << "Sample readings:\n";
        for (size_t i = 0; i < readings.size(); ++i)
        {
            std::cout << "  SystemID=" << readings[i].systemId << ", Value=" << readings[i].value
                      << ", Timestamp=" << readings[i].timestamp.toString() << "\n";
        }
        std::cout << "\n";

        std::cout << "3. Validation: Filter readings against DataChannelList\n";
        std::cout << "--------------------------------------------------------\n";

        std::map<std::string, std::vector<SensorReading>> validReadings;

        for (const auto& reading : readings)
        {
            auto dcOpt = dclPackage.dataChannelList().from(reading.systemId);
            if (dcOpt.has_value())
            {
                validReadings[reading.systemId].push_back(reading);
                std::cout << "  Accepted: " << reading.systemId << " = " << reading.value << "\n";
            }
            else
            {
                std::cout << "  Rejected: " << reading.systemId << " (unknown channel)\n";
            }
        }

        std::cout << "\nAccepted " << validReadings.size() << " unique channels with valid data\n";
        std::cout << "\n";

        std::cout << "4. Grouping: Group channels by updateCycle (ISO19848 optimization)\n";
        std::cout << "---------------------------------------------------------------------\n";

        std::map<std::string, std::vector<std::pair<datachannel::DataChannel, std::string>>> channelsByCycle;

        for (const auto& [systemId, _] : validReadings)
        {
            auto dcOpt = dclPackage.dataChannelList().from(systemId);
            if (dcOpt.has_value())
            {
                const auto& dc = dcOpt.value().get();
                auto updateCycleOpt = dc.property().dataChannelType().updateCycle();
                std::string cycleKey = updateCycleOpt.has_value() ? formatDouble(updateCycleOpt.value()) : "none";
                channelsByCycle[cycleKey].emplace_back(dc, systemId);
            }
        }

        std::cout << "Grouped channels into " << channelsByCycle.size() << " updateCycle groups:\n";
        for (const auto& [cycleKey, channels] : channelsByCycle)
        {
            std::string cycleStr = (cycleKey == "none") ? "None" : cycleKey + "s";
            std::cout << "  updateCycle=" << cycleStr << ": " << channels.size() << " channels\n";
        }
        std::cout << "\n";

        std::cout << "5. TimeSeriesData: Build TabularData for each group\n";
        std::cout << "------------------------------------------------------\n";

        std::vector<timeseries::TabularData> tabularDataList;

        for (const auto& [cycleKey, channelPairs] : channelsByCycle)
        {
            std::set<DateTimeOffset> allTimestamps;
            for (const auto& [_, systemId] : channelPairs)
            {
                for (const auto& reading : validReadings[systemId])
                {
                    allTimestamps.insert(reading.timestamp);
                }
            }

            std::vector<timeseries::DataChannelId> dataChannelIds;
            for (const auto& [dc, _] : channelPairs)
            {
                auto dcIdOpt = timeseries::DataChannelId::fromString(dc.dataChannelId().localId().toString());
                if (dcIdOpt.has_value())
                {
                    dataChannelIds.push_back(std::move(dcIdOpt.value()));
                }
            }

            std::vector<timeseries::TabularDataSet> dataSets;
            for (const auto& ts : allTimestamps)
            {
                std::vector<std::string> values;
                std::vector<std::string> qualities;

                for (const auto& [dc, systemId] : channelPairs)
                {
                    auto it = std::find_if(
                        validReadings[systemId].begin(), validReadings[systemId].end(), [&ts](const SensorReading& r) {
                            return r.timestamp == ts;
                        });

                    if (it != validReadings[systemId].end())
                    {
                        if (dc.property().format().type() == "Boolean")
                        {
                            values.push_back(it->value != 0.0 ? "True" : "False");
                        }
                        else
                        {
                            values.push_back(formatDouble(it->value));
                        }
                        qualities.push_back(it->quality);
                    }
                    else
                    {
                        values.push_back("");
                        qualities.push_back("Bad");
                    }
                }

                dataSets.push_back(timeseries::TabularDataSet{ ts, values, std::optional{ qualities } });
            }

            std::string cycleStr = (cycleKey == "none") ? "None" : cycleKey + "s";
            std::cout << "  TabularData: " << channelPairs.size() << " channels, " << dataSets.size()
                      << " rows (updateCycle=" << cycleStr << ")\n";

            tabularDataList.push_back(timeseries::TabularData{ dataChannelIds, dataSets });
        }
        std::cout << "\n";

        std::cout << "6. Package: Create TimeSeriesDataPackage with header\n";
        std::cout << "-------------------------------------------------------\n";

        std::vector<DateTimeOffset> allTimestampsList;
        for (const auto& [_, readings_vec] : validReadings)
        {
            for (const auto& reading : readings_vec)
            {
                allTimestampsList.push_back(reading.timestamp);
            }
        }

        if (allTimestampsList.empty())
        {
            std::cout << "No valid readings to build a TimeSeriesData package from.\n";
            return 0;
        }

        auto minTs = *std::min_element(allTimestampsList.begin(), allTimestampsList.end());
        auto maxTs = *std::max_element(allTimestampsList.begin(), allTimestampsList.end());

        auto shipIdOpt = ShipId::fromString("IMO1234567");
        if (!shipIdOpt.has_value())
        {
            throw std::runtime_error{ "Failed to parse shipId" };
        }
        auto shipId = shipIdOpt.value();

        const auto& dcHeader = dclPackage.package().header();
        auto configuration = timeseries::ConfigurationReference{ dcHeader.dataChannelListId().id(),
                                                                 dcHeader.dataChannelListId().timeStamp() };

        timeseries::Header tsHeader{ shipId,
                                     std::optional<timeseries::TimeSpan>{ timeseries::TimeSpan{ minTs, maxTs } },
                                     std::nullopt,
                                     std::nullopt,
                                     std::optional<std::string>{ "Vista SDK Sample" } };

        std::vector<timeseries::TimeSeriesData> timeSeriesDataVec;
        timeSeriesDataVec.push_back(
            timeseries::TimeSeriesData{ std::optional{ configuration }, std::optional{ tabularDataList } });

        timeseries::Package pkg{ std::optional{ tsHeader }, timeSeriesDataVec };
        auto tsPackage = timeseries::TimeSeriesDataPackage{ pkg };

        std::cout << "Created TimeSeriesDataPackage:\n";
        std::cout << "  Ship ID  : " << tsHeader.shipId().toString() << "\n";
        std::cout << "  TimeSpan : " << minTs.toString() << " to " << maxTs.toString() << "\n";
        std::cout << "  TabularData blocks: " << tabularDataList.size() << "\n";
        std::cout << "\n";

        std::cout << "7. Serialization: Convert to ISO19848 JSON format\n";
        std::cout << "----------------------------------------------------\n";

        auto jsonStr = jts::toJsonString(tsPackage, true);
        std::cout << "Serialized to JSON: " << jsonStr.size() << " bytes\n";
        std::cout << "Package:\n" << jsonStr << "\n";
        std::cout << "\n";

        std::cout << "8. Validation: Cross-check TimeSeriesData against DataChannelList\n";
        std::cout << "-----------------------------------------------------------------------\n";

        auto onData = [](const DateTimeOffset&,
                         const datachannel::DataChannel&,
                         const transport::Value&,
                         const std::optional<std::string>&) { return ValidateResult<>::ok(); };

        auto validation = timeSeriesDataVec.front().validate(dclPackage, onData, onData);

        std::cout << "  Is valid: " << std::boolalpha << bool{ validation } << "\n";
        if (!validation)
        {
            std::cout << "  Errors:\n";
            for (const auto& error : validation.errors())
            {
                std::cout << "    - " << error << "\n";
            }
        }
        std::cout << "\n";

        std::cout << "9. Validation failure: Out-of-range sensor value rejected\n";
        std::cout << "---------------------------------------------------------------\n";

        auto faultyChannelId = timeseries::DataChannelId::fromString(
                                   dclPackage.dataChannelList()["TEMP001"].dataChannelId().localId().toString())
                                   .value();

        auto faultyTabularData = timeseries::TabularData{
            std::vector<timeseries::DataChannelId>{ faultyChannelId },
            std::vector<timeseries::TabularDataSet>{ timeseries::TabularDataSet{
                baseTime, std::vector<std::string>{ "999.9" }, std::optional{ std::vector<std::string>{ "Good" } } } }
        };

        auto faultyTimeSeriesData =
            timeseries::TimeSeriesData{ std::optional{ configuration },
                                        std::optional{ std::vector<timeseries::TabularData>{ faultyTabularData } } };

        std::cout << "  Reporting TEMP001 = 999.9 degC (Restriction.MaxInclusive is 200)\n";

        auto faultyValidation = faultyTimeSeriesData.validate(dclPackage, onData, onData);

        std::cout << "  Is valid: " << std::boolalpha << bool{ faultyValidation } << "\n";
        if (!faultyValidation)
        {
            std::cout << "  Errors:\n";
            for (const auto& error : faultyValidation.errors())
            {
                std::cout << "    - " << error << "\n";
            }
        }
        std::cout << "\n";
    }

    return 0;
}
