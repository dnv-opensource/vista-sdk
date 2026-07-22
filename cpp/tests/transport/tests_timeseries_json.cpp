#include <doctest/doctest.h>

#include <dnv/vista/sdk/transport/serialization/json/timeseries/Extensions.h>

#include <JSON/Json.h>

#include <EmbeddedTestData.h>

namespace dnv::vista::sdk::tests
{
    namespace jts = transport::serialization::json::timeseries;
    using JsonDocument = sdk::json::Document;

    namespace
    {
        std::string loadJson()
        {
            auto text = EmbeddedTestData::text("TimeSeriesData.json");
            REQUIRE_FALSE(text.empty());
            return text;
        }
    } // namespace

    TEST_SUITE("TimeSeriesData::Json")
    {
        TEST_CASE("fromJsonString - malformed JSON returns nullopt instead of throwing")
        {
            std::string missingShipId = R"({
                "Package": {
                    "Header": {
                        "TimeSpan": { "Start": "2016-01-01T00:00:00Z", "End": "2016-01-01T01:00:00Z" }
                    }
                }
            })";

            std::optional<jts::TimeSeriesDataPackageDto> dto;
            CHECK_NOTHROW(dto = jts::fromJsonString(missingShipId));
            CHECK_FALSE(dto.has_value());
        }

        TEST_CASE("fromJsonString - parses reference file")
        {
            auto json = loadJson();
            auto dto = jts::fromJsonString(json);
            REQUIRE(dto.has_value());

            // Header
            REQUIRE(dto->package.header.has_value());
            CHECK_EQ(dto->package.header->shipId, "IMO1234567");

            REQUIRE(dto->package.header->timeSpan.has_value());
            CHECK_EQ(dto->package.header->timeSpan->start, "2016-01-01T12:00:00Z");
            CHECK_EQ(dto->package.header->timeSpan->end, "2016-01-03T12:00:00Z");

            REQUIRE(dto->package.header->dateCreated.has_value());
            CHECK_EQ(*dto->package.header->dateCreated, "2016-01-03T12:00:00Z");

            REQUIRE(dto->package.header->author.has_value());
            CHECK_EQ(*dto->package.header->author, "Shipboard data server");

            REQUIRE(dto->package.header->systemConfiguration.has_value());
            REQUIRE_EQ(dto->package.header->systemConfiguration->size(), 2u);
            CHECK_EQ((*dto->package.header->systemConfiguration)[0].id, "SystemConfiguration.xml");
            CHECK_EQ((*dto->package.header->systemConfiguration)[0].timeStamp, "2016-01-01T00:00:00Z");

            // TimeSeriesData entries
            REQUIRE_EQ(dto->package.timeSeriesData.size(), 2u);

            // First entry - TabularData + EventData
            const auto& ts0 = dto->package.timeSeriesData[0];
            REQUIRE(ts0.dataConfiguration.has_value());
            CHECK_EQ(ts0.dataConfiguration->id, "DataChannelList.xml");
            CHECK_EQ(ts0.dataConfiguration->timeStamp, "2016-01-01T00:00:00Z");

            REQUIRE(ts0.tabularData.has_value());
            REQUIRE_EQ(ts0.tabularData->size(), 2u);

            const auto& tab0 = (*ts0.tabularData)[0];
            REQUIRE(tab0.numberOfDataSet.has_value());
            CHECK_EQ(*tab0.numberOfDataSet, 2u);
            REQUIRE(tab0.numberOfDataChannel.has_value());
            CHECK_EQ(*tab0.numberOfDataChannel, 2u);
            REQUIRE(tab0.dataChannelIds.has_value());
            REQUIRE_EQ(tab0.dataChannelIds->size(), 2u);
            CHECK_EQ((*tab0.dataChannelIds)[0], "0010");
            CHECK_EQ((*tab0.dataChannelIds)[1], "0020");

            REQUIRE(tab0.dataSets.has_value());
            REQUIRE_EQ(tab0.dataSets->size(), 2u);
            CHECK_EQ((*tab0.dataSets)[0].timeStamp, "2016-01-01T12:00:00Z");
            REQUIRE_EQ((*tab0.dataSets)[0].value.size(), 2u);
            CHECK_EQ((*tab0.dataSets)[0].value[0], "100.0");
            REQUIRE((*tab0.dataSets)[0].quality.has_value());
            CHECK_EQ((*(*tab0.dataSets)[0].quality)[0], "0");

            REQUIRE(ts0.eventData.has_value());
            REQUIRE(ts0.eventData->numberOfDataSet.has_value());
            CHECK_EQ(*ts0.eventData->numberOfDataSet, 3u);
            REQUIRE(ts0.eventData->dataSet.has_value());
            REQUIRE_EQ(ts0.eventData->dataSet->size(), 3u);
            CHECK_EQ((*ts0.eventData->dataSet)[0].dataChannelId, "0011");
            CHECK_EQ((*ts0.eventData->dataSet)[0].value, "HIGH");

            // Second entry - TabularData only, no EventData
            const auto& ts1 = dto->package.timeSeriesData[1];
            CHECK_FALSE(ts1.eventData.has_value());
            REQUIRE(ts1.tabularData.has_value());
            REQUIRE_EQ(ts1.tabularData->size(), 2u);
        }

        TEST_CASE("toDomain - constructs valid domain model from reference file")
        {
            auto json = loadJson();
            auto dto = jts::fromJsonString(json);
            REQUIRE(dto.has_value());

            auto domain = jts::toDomain(*dto);

            // Header
            CHECK_EQ(domain.package().header()->shipId().toString(), "IMO1234567");

            REQUIRE(domain.package().header()->timeSpan().has_value());
            CHECK_FALSE(domain.package().header()->timeSpan()->start().toString().empty());
            CHECK_FALSE(domain.package().header()->timeSpan()->end().toString().empty());

            REQUIRE(domain.package().header()->systemConfiguration().has_value());
            CHECK_EQ(domain.package().header()->systemConfiguration()->size(), 2u);

            // TimeSeriesData
            REQUIRE_EQ(domain.package().timeSeriesData().size(), 2u);

            const auto& ts0 = domain.package().timeSeriesData()[0];
            REQUIRE(ts0.dataConfiguration().has_value());
            CHECK_EQ(ts0.dataConfiguration()->id(), "DataChannelList.xml");

            REQUIRE(ts0.tabularData().has_value());
            REQUIRE_EQ(ts0.tabularData()->size(), 2u);

            const auto& tab0 = (*ts0.tabularData())[0];
            CHECK_EQ(tab0.numberOfDataChannels(), 2u);
            CHECK_EQ(tab0.numberOfDataSets(), 2u);
            REQUIRE_EQ(tab0.dataChannelIds().size(), 2u);
            CHECK_EQ(tab0.dataChannelIds()[0].toString(), "0010");

            REQUIRE_EQ(tab0.dataSets().size(), 2u);
            REQUIRE_EQ(tab0.dataSets()[0].value().size(), 2u);
            CHECK_EQ(tab0.dataSets()[0].value()[0], "100.0");

            REQUIRE(ts0.eventData().has_value());
            REQUIRE(ts0.eventData()->dataSet().has_value());
            REQUIRE_EQ(ts0.eventData()->dataSet()->size(), 3u);
            CHECK_EQ((*ts0.eventData()->dataSet())[0].dataChannelId().toString(), "0011");
            CHECK_EQ((*ts0.eventData()->dataSet())[0].value(), "HIGH");

            // Second entry has no EventData
            CHECK_FALSE(domain.package().timeSeriesData()[1].eventData().has_value());
        }

        TEST_CASE("roundtrip - domain -> JSON -> domain")
        {
            auto json = loadJson();
            auto dto = jts::fromJsonString(json);
            REQUIRE(dto.has_value());
            auto domain = jts::toDomain(*dto);

            StringBuilder sb;
            jts::toJsonString(sb, domain);
            auto dto2 = jts::fromJsonString(sb.view());
            REQUIRE(dto2.has_value());
            auto domain2 = jts::toDomain(*dto2);

            CHECK_EQ(domain.package().header()->shipId().toString(), domain2.package().header()->shipId().toString());

            REQUIRE_EQ(domain.package().timeSeriesData().size(), domain2.package().timeSeriesData().size());

            for (std::size_t i = 0; i < domain.package().timeSeriesData().size(); ++i)
            {
                CAPTURE(i);
                const auto& ts1 = domain.package().timeSeriesData()[i];
                const auto& ts2 = domain2.package().timeSeriesData()[i];

                CHECK_EQ(ts1.dataConfiguration().has_value(), ts2.dataConfiguration().has_value());
                if (ts1.dataConfiguration())
                    CHECK_EQ(ts1.dataConfiguration()->id(), ts2.dataConfiguration()->id());

                CHECK_EQ(ts1.tabularData().has_value(), ts2.tabularData().has_value());
                if (ts1.tabularData())
                {
                    REQUIRE_EQ(ts1.tabularData()->size(), ts2.tabularData()->size());
                    for (std::size_t j = 0; j < ts1.tabularData()->size(); ++j)
                    {
                        CAPTURE(j);
                        CHECK_EQ(
                            (*ts1.tabularData())[j].numberOfDataChannels(),
                            (*ts2.tabularData())[j].numberOfDataChannels());
                        CHECK_EQ(
                            (*ts1.tabularData())[j].dataChannelIds().size(),
                            (*ts2.tabularData())[j].dataChannelIds().size());
                    }
                }

                CHECK_EQ(ts1.eventData().has_value(), ts2.eventData().has_value());
                if (ts1.eventData() && ts1.eventData()->dataSet())
                {
                    REQUIRE_EQ(ts1.eventData()->dataSet()->size(), ts2.eventData()->dataSet()->size());
                }
            }
        }

        TEST_CASE("roundtrip - JSON string key comparison with reference file")
        {
            auto originalJson = loadJson();

            auto dto = jts::fromJsonString(originalJson);
            REQUIRE(dto.has_value());
            auto domain = jts::toDomain(*dto);
            StringBuilder reserializedBuffer;
            jts::toJsonString(reserializedBuffer, domain);

            auto doc1 = JsonDocument::fromString(originalJson);
            auto doc2 = JsonDocument::fromString(reserializedBuffer.view());
            REQUIRE(doc1.has_value());
            REQUIRE(doc2.has_value());

            const auto& h1 = (*doc1)["Package"]["Header"];
            const auto& h2 = (*doc2)["Package"]["Header"];
            CHECK_EQ(h1["ShipID"].root<std::string>(), h2["ShipID"].root<std::string>());

            const auto& ts1 = (*doc1)["Package"]["TimeSeriesData"];
            const auto& ts2 = (*doc2)["Package"]["TimeSeriesData"];
            REQUIRE_EQ(ts1.size(), ts2.size());

            // TabularData channel IDs preserved
            const auto& tab1 = ts1.at(0)["TabularData"].at(0)["DataChannelID"];
            const auto& tab2 = ts2.at(0)["TabularData"].at(0)["DataChannelID"];
            REQUIRE_EQ(tab1.size(), tab2.size());
            for (std::size_t i = 0; i < tab1.size(); ++i)
            {
                CAPTURE(i);
                CHECK_EQ(tab1.at(i).root<std::string>(), tab2.at(i).root<std::string>());
            }

            // EventData channel IDs preserved
            const auto& ev1 = ts1.at(0)["EventData"]["DataSet"];
            const auto& ev2 = ts2.at(0)["EventData"]["DataSet"];
            REQUIRE_EQ(ev1.size(), ev2.size());
            for (std::size_t i = 0; i < ev1.size(); ++i)
            {
                CAPTURE(i);
                CHECK_EQ(
                    ev1.at(i)["DataChannelID"].root<std::string>(), ev2.at(i)["DataChannelID"].root<std::string>());
                CHECK_EQ(ev1.at(i)["Value"].root<std::string>(), ev2.at(i)["Value"].root<std::string>());
            }
        }
        TEST_CASE("toDomain - empty ShipID throws invalid_argument")
        {
            std::string json = R"({
                "Package": {
                    "Header": {
                        "ShipID": "   "
                    },
                    "TimeSeriesData": []
                }
            })";

            auto dto = jts::fromJsonString(json);
            REQUIRE(dto.has_value());

            CHECK_THROWS_AS(static_cast<void>(jts::toDomain(*dto)), std::invalid_argument);
        }

        TEST_CASE("fromJsonString(json, nullptr) - empty ShipID returns nullopt instead of throwing")
        {
            std::string json = R"({
                "Package": {
                    "Header": {
                        "ShipID": "   "
                    },
                    "TimeSeriesData": []
                }
            })";

            std::optional<transport::timeseries::TimeSeriesDataPackage> domain;
            CHECK_NOTHROW(domain = jts::fromJsonString(json, nullptr));
            CHECK_FALSE(domain.has_value());
        }

        TEST_CASE("roundtrip - prettyPrint output parses back correctly")
        {
            auto json = loadJson();
            auto dto = jts::fromJsonString(json);
            REQUIRE(dto.has_value());
            auto domain = jts::toDomain(*dto);

            StringBuilder prettyBuffer;
            jts::toJsonString(prettyBuffer, domain, true);

            CHECK(prettyBuffer.view().find('\n') != std::string_view::npos);

            auto dto2 = jts::fromJsonString(prettyBuffer.view());
            REQUIRE(dto2.has_value());
            auto domain2 = jts::toDomain(*dto2);

            CHECK_EQ(domain.package().header()->shipId().toString(), domain2.package().header()->shipId().toString());

            REQUIRE_EQ(domain.package().timeSeriesData().size(), domain2.package().timeSeriesData().size());

            for (std::size_t i = 0; i < domain.package().timeSeriesData().size(); ++i)
            {
                CAPTURE(i);
                const auto& ts1 = domain.package().timeSeriesData()[i];
                const auto& ts2 = domain2.package().timeSeriesData()[i];

                CHECK_EQ(ts1.dataConfiguration().has_value(), ts2.dataConfiguration().has_value());
                if (ts1.dataConfiguration())
                {
                    CHECK_EQ(ts1.dataConfiguration()->id(), ts2.dataConfiguration()->id());
                }

                if (ts1.tabularData() && ts2.tabularData())
                {
                    REQUIRE_EQ(ts1.tabularData()->size(), ts2.tabularData()->size());
                    if (!ts1.tabularData()->empty())
                    {
                        CHECK_EQ(
                            (*ts1.tabularData())[0].dataChannelIds().size(),
                            (*ts2.tabularData())[0].dataChannelIds().size());
                    }
                }
            }
        }
    }
} // namespace dnv::vista::sdk::tests
