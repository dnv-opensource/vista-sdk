#include <doctest/doctest.h>

#include <dnv/VistaSDK.h>

using dnv::vista::sdk::transport::ShipId;

namespace dnv::vista::sdk::tests
{
    namespace ts = transport::timeseries;
    namespace dc = transport::datachannel;

    static dc::DataChannelListPackage createValidFullyCustomDataChannelList()
    {
        auto timeStamp = DateTimeOffset{ "2016-01-01T00:00:00Z" };
        dc::ConfigurationReference dataChannelListId{ "DataChannelList.xml", timeStamp, "1.0" };

        dc::VersionInformation versionInfo{ "some_naming_rule", "2.0", "http://somewhere.net" };

        dc::Header header{ ShipId::fromString("IMO1234567").value(), dataChannelListId, versionInfo, "Author1",
                           DateTimeOffset{ "2015-12-01T00:00:00Z" }, std::nullopt };

        dc::DataChannelList dataChannelList;

        // First DataChannel - Temperature sensor (Decimal format)
        {
            auto localIdOpt = LocalIdBuilder::fromString(
                "/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/meta/qty-temperature/cnt-cooling.air");
            REQUIRE(localIdOpt.has_value());

            dc::DataChannelId dataChannelId{ *localIdOpt, "0010" };

            dc::NameObject nameObject;
            nameObject.setNamingRule("Naming_Rule");
            dataChannelId.setNameObject(nameObject);

            dc::DataChannelType dataChannelType{ "Inst" };
            dataChannelType.setUpdateCycle(1.0);

            dc::Format format{ "Decimal" };
            dc::Restriction restriction;
            restriction.setFractionDigits(1);
            restriction.setMaxInclusive(200.0);
            restriction.setMinInclusive(-150.0);
            format.setRestriction(restriction);

            dc::Range range{ 0.0, 150.0 };
            dc::Unit unit{ "°C", "Temperature" };

            dc::Property property{ dataChannelType,
                                   format,
                                   range,
                                   unit,
                                   std::string{ "OPC_QUALITY" },
                                   std::nullopt,
                                   std::string{ "M/E #1 Air Cooler CFW OUT Temp" },
                                   std::string{ " Location: ECR, Manufacturer: AAA Company, Type: TYPE-AAA " },
                                   std::nullopt };

            dataChannelList.add(dc::DataChannel{ dataChannelId, property });
        }

        // Second DataChannel - Alert type (String format)
        {
            auto localIdOpt = LocalIdBuilder::fromString("/dnv-v2/vis-3-4a/511.15-1/E32/meta/qty-power");
            REQUIRE(localIdOpt.has_value());

            dc::DataChannelId dataChannelId{ *localIdOpt, "0020" };

            dc::DataChannelType dataChannelType{ "Alert" };

            dc::Format format{ "String" };
            dc::Restriction restriction;
            restriction.setMaxLength(100);
            restriction.setMinLength(0);
            format.setRestriction(restriction);

            dc::Property property{ dataChannelType,          format,       std::nullopt, std::nullopt, std::nullopt,
                                   std::string{ "Warning" }, std::nullopt, std::nullopt, std::nullopt };

            dataChannelList.add(dc::DataChannel{ dataChannelId, property });
        }

        return dc::DataChannelListPackage{ dc::Package{ header, std::move(dataChannelList) } };
    }

    static ts::TimeSeriesDataPackage createTestTimeSeriesDataPackage()
    {
        auto dcPackage = createValidFullyCustomDataChannelList();

        std::vector<ts::DataChannelId> allDataChannelIds;
        for (const auto& dc : dcPackage.dataChannelList().dataChannels())
        {
            auto idOpt = ts::DataChannelId::fromString(
                dc.dataChannelId().shortId().has_value() ? *dc.dataChannelId().shortId()
                                                         : dc.dataChannelId().localId().toString());
            if (idOpt.has_value())
                allDataChannelIds.push_back(*idOpt);
        }

        std::vector<ts::DataChannelId> firstId{ allDataChannelIds[0] };

        auto start = DateTimeOffset{ "2016-01-01T12:00:00Z" };
        auto end = DateTimeOffset{ "2016-01-03T12:00:00Z" };
        auto timestamp1 = DateTimeOffset{ "2016-01-01T12:00:00Z" };
        auto timestamp2 = DateTimeOffset{ "2016-01-02T12:00:00Z" };
        auto timestamp3 = DateTimeOffset{ "2016-01-03T12:00:00Z" };
        auto eventTimestamp = DateTimeOffset{ "2016-01-01T12:00:01Z" };
        auto createdModified = DateTimeOffset{ "2016-01-03T12:00:00Z" };
        auto configTimestamp = DateTimeOffset{ "2016-01-03T00:00:00Z" };

        std::vector<ts::TabularDataSet> dataSets1{
            ts::TabularDataSet(timestamp1, { "100.0", "200.0" }, std::vector<std::string>{ "0", "0" }),
            ts::TabularDataSet(timestamp2, { "105.0", "210.0" }, std::vector<std::string>{ "0", "0" })
        };
        ts::TabularData tabularData1(allDataChannelIds, dataSets1);

        std::vector<ts::TabularDataSet> dataSets2{ ts::TabularDataSet(timestamp1, { "100.0" }, std::nullopt),
                                                   ts::TabularDataSet(timestamp2, { "100.1" }, std::nullopt),
                                                   ts::TabularDataSet(timestamp3, { "100.2" }, std::nullopt) };
        ts::TabularData tabularData2(firstId, dataSets2);

        std::vector<ts::EventDataSet> eventDataSets{ ts::EventDataSet{
            eventTimestamp, allDataChannelIds[1], "HIGH", std::string{ "0" } } };
        ts::EventData eventData{ eventDataSets };

        ts::ConfigurationReference dataConfig{ "DataChannelList.xml", DateTimeOffset{ "2016-01-01T00:00:00Z" } };

        std::vector<ts::ConfigurationReference> systemConfigs{
            ts::ConfigurationReference{ "SystemConfiguration.xml", configTimestamp },
            ts::ConfigurationReference{ "SystemConfiguration.xml", configTimestamp }
        };

        ts::CustomData customDataKinds1;
        customDataKinds1.set("dataQuality", std::string{ "high" });
        customDataKinds1.set("recordCount", std::int64_t{ 42 });
        customDataKinds1.set("validated", true);
        customDataKinds1.set("accuracy", 0.01);
        customDataKinds1.set("processingTime", std::string{ "2024-01-15T10:30:00Z" });

        ts::TimeSeriesData tsData1{
            dataConfig, std::vector<ts::TabularData>{ tabularData1, tabularData2 }, eventData, customDataKinds1
        };

        ts::CustomData customDataKinds2;
        customDataKinds2.set("source", std::string{ "sensor_array_1" });
        customDataKinds2.set("version", std::int64_t{ 2 });

        ts::TimeSeriesData tsData2{
            dataConfig, std::vector<ts::TabularData>{ tabularData1, tabularData2 }, eventData, customDataKinds2
        };

        ts::CustomHeaders customHeaders;
        customHeaders.set("temperatureUnit", std::string{ "Celsius" });
        customHeaders.set("sampleRate", std::int64_t{ 1000 });
        customHeaders.set("isCompressed", true);
        customHeaders.set("precision", 0.001);
        customHeaders.set("calibrationDate", std::string{ "2024-01-15T00:00:00Z" });

        ts::Header header{ ShipId::fromString("IMO1234567").value(),
                           ts::TimeSpan(start, end),
                           createdModified,
                           createdModified,
                           "Shipboard data server",
                           systemConfigs,
                           customHeaders };

        return ts::TimeSeriesDataPackage{ ts::Package{ header, std::vector<ts::TimeSeriesData>{ tsData1, tsData2 } } };
    }

    TEST_SUITE("IsoMessageTests::TimeSeries")
    {
        TEST_CASE("TimeSeriesData package is non-empty")
        {
            auto message = createTestTimeSeriesDataPackage();
            CHECK_FALSE(message.package().timeSeriesData().empty());
        }

        TEST_CASE("TimeSeriesData structure validation")
        {
            auto message = createTestTimeSeriesDataPackage();

            for (const auto& tsData : message.package().timeSeriesData())
            {
                REQUIRE(tsData.tabularData().has_value());
                CHECK_FALSE(tsData.tabularData()->empty());
                for (const auto& td : *tsData.tabularData())
                {
                    CHECK_FALSE(td.dataChannelIds().empty());
                    CHECK_FALSE(td.dataSets().empty());
                }

                REQUIRE(tsData.eventData().has_value());
                REQUIRE(tsData.eventData()->dataSet().has_value());
                CHECK_FALSE(tsData.eventData()->dataSet()->empty());
            }
        }
    }

    TEST_SUITE("TimeSeriesTests")
    {
        TEST_CASE("TimeSpan - valid construction and setters")
        {
            auto start = DateTimeOffset{ "2016-01-01T12:00:00Z" };
            auto end = DateTimeOffset{ "2016-01-03T12:00:00Z" };

            ts::TimeSpan timeSpan(start, end);
            CHECK_EQ(timeSpan.start().toString(), start.toString());
            CHECK_EQ(timeSpan.end().toString(), end.toString());

            auto newStart = DateTimeOffset{ "2016-01-02T12:00:00Z" };
            timeSpan.setStart(newStart);
            CHECK_EQ(timeSpan.start().toString(), newStart.toString());

            auto newEnd = DateTimeOffset{ "2016-01-04T12:00:00Z" };
            timeSpan.setEnd(newEnd);
            CHECK_EQ(timeSpan.end().toString(), newEnd.toString());
        }

        TEST_CASE("TimeSpan - invalid range throws")
        {
            auto start = DateTimeOffset{ "2016-01-03T12:00:00Z" };
            auto end = DateTimeOffset{ "2016-01-01T12:00:00Z" };
            CHECK_THROWS_AS(ts::TimeSpan(start, end), std::invalid_argument);

            auto validStart = DateTimeOffset{ "2016-01-01T12:00:00Z" };
            auto validEnd = DateTimeOffset{ "2016-01-03T12:00:00Z" };
            ts::TimeSpan timeSpan(validStart, validEnd);

            CHECK_THROWS_AS(timeSpan.setStart(DateTimeOffset{ "2016-01-04T12:00:00Z" }), std::invalid_argument);
            CHECK_THROWS_AS(timeSpan.setEnd(DateTimeOffset{ "2015-12-31T12:00:00Z" }), std::invalid_argument);
        }

        TEST_CASE("TabularData - valid validation")
        {
            auto id1 = ts::DataChannelId::fromString("0010");
            auto id2 = ts::DataChannelId::fromString("0020");
            REQUIRE(id1.has_value());
            REQUIRE(id2.has_value());

            auto timestamp = DateTimeOffset{ "2016-01-01T12:00:00Z" };
            std::vector<ts::TabularDataSet> dataSets{ ts::TabularDataSet(
                timestamp, { "100.0", "200.0" }, std::vector<std::string>{ "0", "0" }) };

            ts::TabularData tabularData({ *id1, *id2 }, dataSets);
            auto result = tabularData.validate();
            CHECK(result);
        }

        TEST_CASE("TabularData - dimension mismatch fails validation")
        {
            auto id1 = ts::DataChannelId::fromString("0010");
            auto id2 = ts::DataChannelId::fromString("0020");
            REQUIRE(id1.has_value());
            REQUIRE(id2.has_value());

            // 3 values but only 2 channels
            auto timestamp = DateTimeOffset{ "2016-01-01T12:00:00Z" };
            std::vector<ts::TabularDataSet> dataSets{ ts::TabularDataSet(
                timestamp, { "100.0", "200.0", "300.0" }, std::vector<std::string>{ "0", "0", "0" }) };

            ts::TabularData tabularData({ *id1, *id2 }, dataSets);
            auto result = tabularData.validate();
            CHECK_FALSE(result);
        }

        TEST_CASE("EventData - operations")
        {
            ts::EventData eventData{ std::nullopt };
            CHECK_FALSE(eventData.dataSet().has_value());
            CHECK_EQ(eventData.numberOfDataSet(), 0U);

            auto timestamp = DateTimeOffset{ "2016-01-01T12:00:01Z" };
            auto dataChannelIdOpt = ts::DataChannelId::fromString("0010");
            REQUIRE(dataChannelIdOpt.has_value());

            std::vector<ts::EventDataSet> sets{ ts::EventDataSet{
                timestamp, *dataChannelIdOpt, "HIGH", std::string{ "0" } } };
            eventData.setDataSet(sets);
            REQUIRE(eventData.dataSet().has_value());
            CHECK_EQ(eventData.numberOfDataSet(), 1U);

            sets.push_back(ts::EventDataSet{ timestamp, *dataChannelIdOpt, "LOW", std::string{ "0" } });
            eventData.setDataSet(sets);
            CHECK_EQ(eventData.numberOfDataSet(), 2U);

            eventData.setDataSet(std::nullopt);
            CHECK_FALSE(eventData.dataSet().has_value());
            CHECK_EQ(eventData.numberOfDataSet(), 0U);
        }
    }

    TEST_SUITE("ValidationTests::TimeSeries")
    {
        TEST_CASE("ConfigurationReference - construction and setters")
        {
            auto timestamp = DateTimeOffset{ "2016-01-01T00:00:00Z" };
            ts::ConfigurationReference configRef("TestConfig.xml", timestamp);

            CHECK_EQ(configRef.id(), "TestConfig.xml");
            CHECK_EQ(configRef.timeStamp().toString(), timestamp.toString());

            configRef.setId("NewConfig.xml");
            CHECK_EQ(configRef.id(), "NewConfig.xml");

            auto newTs = DateTimeOffset{ "2016-01-02T00:00:00Z" };
            configRef.setTimeStamp(newTs);
            CHECK_EQ(configRef.timeStamp().toString(), newTs.toString());
        }

        TEST_CASE("TabularDataSet - construction with and without quality")
        {
            auto timestamp = DateTimeOffset{ "2016-01-01T12:00:00Z" };
            std::vector<std::string> values{ "100.0", "200.0" };
            std::vector<std::string> quality{ "0", "0" };

            ts::TabularDataSet dataSet(timestamp, values, quality);
            CHECK_EQ(dataSet.timeStamp().toString(), timestamp.toString());
            CHECK_EQ(dataSet.value().size(), 2U);
            CHECK_EQ(dataSet.value()[0], "100.0");
            CHECK_EQ(dataSet.value()[1], "200.0");
            REQUIRE(dataSet.quality().has_value());
            CHECK_EQ(dataSet.quality()->size(), 2U);
            CHECK_EQ((*dataSet.quality())[0], "0");

            ts::TabularDataSet dataSetNoQuality(timestamp, values);
            CHECK_FALSE(dataSetNoQuality.quality().has_value());
        }

        TEST_CASE("EventDataSet - construction with and without quality")
        {
            auto timestamp = DateTimeOffset{ "2016-01-01T12:00:01Z" };
            auto dataChannelIdOpt = ts::DataChannelId::fromString("0010");
            REQUIRE(dataChannelIdOpt.has_value());

            ts::EventDataSet eds(timestamp, *dataChannelIdOpt, "HIGH", std::string{ "0" });
            CHECK_EQ(eds.timeStamp().toString(), timestamp.toString());
            CHECK_EQ(eds.dataChannelId().toString(), dataChannelIdOpt->toString());
            CHECK_EQ(eds.value(), "HIGH");
            REQUIRE(eds.quality().has_value());
            CHECK_EQ(*eds.quality(), "0");

            ts::EventDataSet edsNoQuality(timestamp, *dataChannelIdOpt, "LOW");
            CHECK_FALSE(edsNoQuality.quality().has_value());
        }

        TEST_CASE("Package - empty TimeSeriesData is allowed")
        {
            ts::Header header{ ShipId::fromString("IMO1234567").value(),
                               std::nullopt,
                               std::nullopt,
                               std::nullopt,
                               std::nullopt,
                               std::nullopt,
                               std::nullopt };
            ts::Package package(header, {});
            CHECK(package.timeSeriesData().empty());
            CHECK(package.header().has_value());
        }
    }

    TEST_SUITE("CrossValidationTests::TimeSeries")
    {
        TEST_CASE("Valid TimeSeriesData against DataChannelList")
        {
            auto dcPackage = createValidFullyCustomDataChannelList();
            auto tsPackage = createTestTimeSeriesDataPackage();

            const auto& tsData = tsPackage.package().timeSeriesData()[0];

            auto onTabularData = [](const auto&, const auto&, const auto&, const auto&) {
                return transport::ValidateResult<>::ok();
            };
            auto onEventData = [](const auto&, const auto&, const auto&, const auto&) {
                return transport::ValidateResult<>::ok();
            };

            auto result = tsData.validate(dcPackage, onTabularData, onEventData);
            CHECK(result);
        }

        TEST_CASE("Invalid DataChannelId not found in DataChannelList")
        {
            auto dcPackage = createValidFullyCustomDataChannelList();

            auto timestamp = DateTimeOffset{ "2016-01-01T12:00:00Z" };
            auto invalidIdOpt = ts::DataChannelId::fromString("9999");
            REQUIRE(invalidIdOpt.has_value());

            ts::TabularData tabularData{ { *invalidIdOpt },
                                         { ts::TabularDataSet(timestamp, { "100.0" }, std::nullopt) } };
            ts::ConfigurationReference dataConfig{ "DataChannelList.xml", DateTimeOffset{ "2016-01-01T00:00:00Z" } };
            ts::TimeSeriesData tsData{
                dataConfig, std::vector<ts::TabularData>{ tabularData }, std::nullopt, std::nullopt
            };

            auto onTabularData = [](const auto&, const auto&, const auto&, const auto&) {
                return transport::ValidateResult<>::ok();
            };
            auto onEventData = [](const auto&, const auto&, const auto&, const auto&) {
                return transport::ValidateResult<>::ok();
            };

            auto result = tsData.validate(dcPackage, onTabularData, onEventData);
            CHECK_FALSE(result);
            REQUIRE_FALSE(result.errors().empty());
            CHECK(result.errors()[0].find("not found") != std::string::npos);
        }

        TEST_CASE("Invalid value format mismatch")
        {
            auto dcPackage = createValidFullyCustomDataChannelList();

            const auto& firstChannel = dcPackage.dataChannelList().dataChannels()[0];
            auto channelIdOpt = ts::DataChannelId::fromString(
                firstChannel.dataChannelId().shortId().has_value() ? *firstChannel.dataChannelId().shortId()
                                                                   : firstChannel.dataChannelId().localId().toString());
            REQUIRE(channelIdOpt.has_value());

            auto timestamp = DateTimeOffset{ "2016-01-01T12:00:00Z" };
            ts::TabularData tabularData{ { *channelIdOpt },
                                         { ts::TabularDataSet(timestamp, { "invalid_not_a_number" }, std::nullopt) } };
            ts::ConfigurationReference dataConfig{ "DataChannelList.xml", DateTimeOffset{ "2016-01-01T00:00:00Z" } };
            ts::TimeSeriesData tsData{
                dataConfig, std::vector<ts::TabularData>{ tabularData }, std::nullopt, std::nullopt
            };

            auto onTabularData = [](const auto&, const auto&, const auto&, const auto&) {
                return transport::ValidateResult<>::ok();
            };
            auto onEventData = [](const auto&, const auto&, const auto&, const auto&) {
                return transport::ValidateResult<>::ok();
            };

            auto result = tsData.validate(dcPackage, onTabularData, onEventData);
            CHECK_FALSE(result);
            REQUIRE_FALSE(result.errors().empty());
            CHECK(result.errors()[0].find("invalid value") != std::string::npos);
        }

        TEST_CASE("Invalid DataConfiguration ID mismatch")
        {
            auto dcPackage = createValidFullyCustomDataChannelList();
            auto tsPackage = createTestTimeSeriesDataPackage();

            const auto& originalTsData = tsPackage.package().timeSeriesData()[0];

            ts::ConfigurationReference wrongConfig{ "WrongDataChannelList.xml",
                                                    DateTimeOffset{ "2016-01-01T00:00:00Z" } };
            ts::TimeSeriesData tsData{
                wrongConfig, originalTsData.tabularData(), originalTsData.eventData(), std::nullopt
            };

            auto onTabularData = [](const auto&, const auto&, const auto&, const auto&) {
                return transport::ValidateResult<>::ok();
            };
            auto onEventData = [](const auto&, const auto&, const auto&, const auto&) {
                return transport::ValidateResult<>::ok();
            };

            auto result = tsData.validate(dcPackage, onTabularData, onEventData);
            CHECK_FALSE(result);
            REQUIRE_FALSE(result.errors().empty());
            CHECK(result.errors()[0].find("does not match") != std::string::npos);
        }

        TEST_CASE("Invalid empty TimeSeriesData")
        {
            auto dcPackage = createValidFullyCustomDataChannelList();

            ts::ConfigurationReference dataConfig{ "DataChannelList.xml", DateTimeOffset{ "2016-01-01T00:00:00Z" } };
            ts::TimeSeriesData tsData{ dataConfig, std::nullopt, std::nullopt, std::nullopt };

            auto onTabularData = [](const auto&, const auto&, const auto&, const auto&) {
                return transport::ValidateResult<>::ok();
            };
            auto onEventData = [](const auto&, const auto&, const auto&, const auto&) {
                return transport::ValidateResult<>::ok();
            };

            auto result = tsData.validate(dcPackage, onTabularData, onEventData);
            CHECK_FALSE(result);
            REQUIRE_FALSE(result.errors().empty());
            CHECK(result.errors()[0].find("without data") != std::string::npos);
        }

        TEST_CASE("Custom callback rejection")
        {
            auto dcPackage = createValidFullyCustomDataChannelList();
            auto tsPackage = createTestTimeSeriesDataPackage();

            const auto& tsData = tsPackage.package().timeSeriesData()[0];

            auto onTabularData = [](const auto&, const auto&, const auto&, const auto&) {
                return transport::ValidateResult<>::invalid("Custom business rule violation");
            };
            auto onEventData = [](const auto&, const auto&, const auto&, const auto&) {
                return transport::ValidateResult<>::ok();
            };

            auto result = tsData.validate(dcPackage, onTabularData, onEventData);
            CHECK_FALSE(result);
            REQUIRE_FALSE(result.errors().empty());
            CHECK(result.errors()[0].find("Custom business rule violation") != std::string::npos);
        }
    }
} // namespace dnv::vista::sdk::tests
