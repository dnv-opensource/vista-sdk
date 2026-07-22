#include <doctest/doctest.h>

#include <dnv/vista/sdk/transport/serialization/json/datachannel/Extensions.h>

#include <JSON/Json.h>

#include <EmbeddedTestData.h>

namespace dnv::vista::sdk::tests
{
    namespace jdc = transport::serialization::json::datachannel;
    using JsonDocument = sdk::json::Document;

    namespace
    {
        std::string loadJson()
        {
            auto text = EmbeddedTestData::text("DataChannelList.json");
            REQUIRE_FALSE(text.empty());
            return text;
        }
    } // namespace

    TEST_SUITE("DataChannelList::Json")
    {
        TEST_CASE("fromJsonString - malformed JSON returns nullopt instead of throwing")
        {
            std::string missingShipId = R"({
                "Package": {
                    "Header": {
                        "DataChannelListID": { "ID": "x", "TimeStamp": "2016-01-01T00:00:00Z" }
                    },
                    "DataChannelList": { "DataChannel": [] }
                }
            })";

            std::optional<jdc::DataChannelListPackageDto> dto;
            CHECK_NOTHROW(dto = jdc::fromJsonString(missingShipId));
            CHECK_FALSE(dto.has_value());
        }

        TEST_CASE("fromJsonString - parses reference file")
        {
            auto json = loadJson();
            auto dto = jdc::fromJsonString(json);
            REQUIRE(dto.has_value());

            CHECK_EQ(dto->package.header.shipId, "IMO1234567");
            CHECK_EQ(dto->package.header.dataChannelListId.id, "DataChannelList.xml");
            CHECK_EQ(dto->package.header.dataChannelListId.timeStamp, "2016-01-01T00:00:00Z");
            REQUIRE(dto->package.header.dataChannelListId.version.has_value());
            CHECK_EQ(*dto->package.header.dataChannelListId.version, "1.0");

            REQUIRE(dto->package.header.versionInformation.has_value());
            CHECK_EQ(dto->package.header.versionInformation->namingRule, "some_naming_rule");
            CHECK_EQ(dto->package.header.versionInformation->namingSchemeVersion, "2.0");
            REQUIRE(dto->package.header.versionInformation->referenceUrl.has_value());
            CHECK_EQ(*dto->package.header.versionInformation->referenceUrl, "http://somewhere.net");

            REQUIRE(dto->package.header.author.has_value());
            CHECK_EQ(*dto->package.header.author, "Author1");

            // Custom header (xs:any)
            REQUIRE(dto->package.header.customHeaders.has_value());
            const auto* customElem = dto->package.header.customHeaders->find("nr:CustomHeaderElement");
            REQUIRE(customElem != nullptr);
            CHECK_EQ(customElem->asString(), "Vender specific headers");

            CHECK_EQ(dto->package.dataChannelList.dataChannels.size(), 4u);
        }

        TEST_CASE("toDomain - constructs valid domain model from reference file")
        {
            auto json = loadJson();
            auto dto = jdc::fromJsonString(json);
            REQUIRE(dto.has_value());

            auto domain = jdc::toDomain(*dto);

            CHECK_EQ(domain.package().header().shipId().toString(), "IMO1234567");
            CHECK_EQ(domain.package().dataChannelList().size(), 4u);

            // First channel
            const auto& ch0 = domain.package().dataChannelList()[0];
            CHECK_FALSE(ch0.dataChannelId().localId().toString().empty());
            REQUIRE(ch0.dataChannelId().shortId().has_value());
            CHECK_EQ(*ch0.dataChannelId().shortId(), "0010");

            REQUIRE(ch0.dataChannelId().nameObject().has_value());
            CHECK_EQ(ch0.dataChannelId().nameObject()->namingRule(), "Naming_Rule");
            REQUIRE(ch0.dataChannelId().nameObject()->customNameObjects().has_value());
            const auto* customName = ch0.dataChannelId().nameObject()->customNameObjects()->find("nr:CustomNameObject");
            REQUIRE(customName != nullptr);
            CHECK_EQ(customName->asString(), "Vender specific NameObject");

            CHECK_EQ(ch0.property().dataChannelType().type(), "Inst");
            CHECK_EQ(ch0.property().format().type(), "Decimal");

            REQUIRE(ch0.property().format().restriction().has_value());
            CHECK_EQ(ch0.property().format().restriction()->fractionDigits(), std::optional<std::uint32_t>{ 1u });

            // Third channel - Alert with Enumeration
            const auto& ch2 = domain.package().dataChannelList()[2];
            CHECK_EQ(ch2.property().dataChannelType().type(), "Alert");
            REQUIRE(ch2.property().alertPriority().has_value());
            CHECK_EQ(*ch2.property().alertPriority(), "Alarm");
            REQUIRE(ch2.property().format().restriction().has_value());
            REQUIRE(ch2.property().format().restriction()->enumeration().has_value());
            const auto& enums = *ch2.property().format().restriction()->enumeration();
            REQUIRE(enums.size() == 3u);
            CHECK_EQ(enums[0], "High");
            CHECK_EQ(enums[1], "Low");
            CHECK_EQ(enums[2], "Normal");

            // Fourth channel - Average with CalculationPeriod + WhiteSpace
            const auto& ch1 = domain.package().dataChannelList()[1];
            CHECK_EQ(ch1.property().dataChannelType().type(), "Average");
            REQUIRE(ch1.property().dataChannelType().calculationPeriod().has_value());
            CHECK_EQ(*ch1.property().dataChannelType().calculationPeriod(), 3600.0);
            REQUIRE(ch1.property().format().restriction().has_value());
            REQUIRE(ch1.property().format().restriction()->whiteSpace().has_value());
            CHECK_EQ(
                *ch1.property().format().restriction()->whiteSpace(),
                transport::datachannel::Restriction::WhiteSpace::Preserve);
        }

        TEST_CASE("roundtrip - domain -> JSON -> domain")
        {
            auto json = loadJson();
            auto dto = jdc::fromJsonString(json);
            REQUIRE(dto.has_value());
            auto domain = jdc::toDomain(*dto);

            // domain -> JSON -> domain
            StringBuilder sb;
            jdc::toJsonString(sb, domain);
            auto dto2 = jdc::fromJsonString(sb.view());
            REQUIRE(dto2.has_value());
            auto domain2 = jdc::toDomain(*dto2);

            const auto& list1 = domain.package().dataChannelList();
            const auto& list2 = domain2.package().dataChannelList();
            REQUIRE_EQ(list1.size(), list2.size());

            for (std::size_t i = 0; i < list1.size(); ++i)
            {
                CAPTURE(i);
                CHECK_EQ(list1[i].dataChannelId().localId().toString(), list2[i].dataChannelId().localId().toString());
                CHECK_EQ(list1[i].dataChannelId().shortId(), list2[i].dataChannelId().shortId());
                CHECK_EQ(list1[i].property().dataChannelType().type(), list2[i].property().dataChannelType().type());
                CHECK_EQ(list1[i].property().format().type(), list2[i].property().format().type());
            }

            // Header
            CHECK_EQ(domain.package().header().shipId().toString(), domain2.package().header().shipId().toString());
            CHECK_EQ(
                domain.package().header().dataChannelListId().id(),
                domain2.package().header().dataChannelListId().id());
        }

        TEST_CASE("roundtrip - JSON string comparison with reference file")
        {
            auto originalJson = loadJson();

            // Parse reference -> domain -> re-serialize
            auto dto = jdc::fromJsonString(originalJson);
            REQUIRE(dto.has_value());
            auto domain = jdc::toDomain(*dto);
            StringBuilder reserializedBuffer;
            jdc::toJsonString(reserializedBuffer, domain);

            // Both must parse to equivalent documents
            auto doc1 = dnv::vista::sdk::json::Document::fromString(originalJson);
            auto doc2 = dnv::vista::sdk::json::Document::fromString(reserializedBuffer.view());
            REQUIRE(doc1.has_value());
            REQUIRE(doc2.has_value());

            // Key fields must match
            const auto& h1 = (*doc1)["Package"]["Header"];
            const auto& h2 = (*doc2)["Package"]["Header"];
            CHECK_EQ(h1["ShipID"].root<std::string>(), h2["ShipID"].root<std::string>());
            CHECK_EQ(
                h1["DataChannelListID"]["ID"].root<std::string>(), h2["DataChannelListID"]["ID"].root<std::string>());
            CHECK_EQ(
                h1["VersionInformation"]["NamingRule"].root<std::string>(),
                h2["VersionInformation"]["NamingRule"].root<std::string>());

            const auto& dcList1 = (*doc1)["Package"]["DataChannelList"]["DataChannel"];
            const auto& dcList2 = (*doc2)["Package"]["DataChannelList"]["DataChannel"];
            CHECK_EQ(dcList1.size(), dcList2.size());

            for (std::size_t i = 0; i < dcList1.size(); ++i)
            {
                CAPTURE(i);
                CHECK_EQ(
                    dcList1.at(i)["DataChannelID"]["LocalID"].root<std::string>(),
                    dcList2.at(i)["DataChannelID"]["LocalID"].root<std::string>());
                CHECK_EQ(
                    dcList1.at(i)["Property"]["DataChannelType"]["Type"].root<std::string>(),
                    dcList2.at(i)["Property"]["DataChannelType"]["Type"].root<std::string>());
            }
        }

        TEST_CASE("roundtrip - prettyPrint output parses back correctly")
        {
            auto json = loadJson();
            auto dto = jdc::fromJsonString(json);
            REQUIRE(dto.has_value());
            auto domain = jdc::toDomain(*dto);

            StringBuilder prettyBuffer;
            jdc::toJsonString(prettyBuffer, domain, true);

            CHECK(prettyBuffer.view().find('\n') != std::string_view::npos);

            auto dto2 = jdc::fromJsonString(prettyBuffer.view());
            REQUIRE(dto2.has_value());
            auto domain2 = jdc::toDomain(*dto2);

            const auto& list1 = domain.package().dataChannelList();
            const auto& list2 = domain2.package().dataChannelList();
            REQUIRE_EQ(list1.size(), list2.size());

            for (std::size_t i = 0; i < list1.size(); ++i)
            {
                CAPTURE(i);
                CHECK_EQ(list1[i].dataChannelId().localId().toString(), list2[i].dataChannelId().localId().toString());
                CHECK_EQ(list1[i].property().dataChannelType().type(), list2[i].property().dataChannelType().type());
                CHECK_EQ(list1[i].property().format().type(), list2[i].property().format().type());
            }

            CHECK_EQ(domain.package().header().shipId().toString(), domain2.package().header().shipId().toString());
            CHECK_EQ(
                domain.package().header().dataChannelListId().id(),
                domain2.package().header().dataChannelListId().id());
        }

        TEST_CASE("toDomain - invalid Restriction.WhiteSpace value throws instead of being silently dropped")
        {
            std::string json = R"({
                "Package": {
                    "Header": {
                        "ShipID": "IMO1234567",
                        "DataChannelListID": { "ID": "x", "TimeStamp": "2016-01-01T00:00:00Z" }
                    },
                    "DataChannelList": {
                        "DataChannel": [
                            {
                                "DataChannelID": {
                                    "LocalID": "/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/meta/qty-temperature/cnt-cooling.air"
                                },
                                "Property": {
                                    "DataChannelType": { "Type": "Inst" },
                                    "Format": {
                                        "Type": "String",
                                        "Restriction": { "WhiteSpace": "preserve" }
                                    }
                                }
                            }
                        ]
                    }
                }
            })";

            auto dto = jdc::fromJsonString(json);
            REQUIRE(dto.has_value());

            CHECK_THROWS_AS(static_cast<void>(jdc::toDomain(*dto)), std::invalid_argument);
        }

        TEST_CASE("fromJsonString(json, nullptr) propagates toDomain exceptions instead of returning nullopt")
        {
            std::string json = R"({
                "Package": {
                    "Header": {
                        "ShipID": "IMO1234567",
                        "DataChannelListID": { "ID": "x", "TimeStamp": "2016-01-01T00:00:00Z" }
                    },
                    "DataChannelList": {
                        "DataChannel": [
                            {
                                "DataChannelID": {
                                    "LocalID": "/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/meta/qty-temperature/cnt-cooling.air"
                                },
                                "Property": {
                                    "DataChannelType": { "Type": "Inst" },
                                    "Format": { "Type": "NotARealFormatType" }
                                }
                            }
                        ]
                    }
                }
            })";

            std::optional<transport::datachannel::DataChannelListPackage> domain;
            CHECK_NOTHROW(domain = jdc::fromJsonString(json, nullptr));
            CHECK_FALSE(domain.has_value());
        }
    }
} // namespace dnv::vista::sdk::tests
