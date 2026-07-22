/**
 * @file datachannellist.cpp
 * @brief Demonstrates usage of vista-sdk DataChannelList API with ISO 19848 support
 * @details This sample shows how to create DataChannelList messages with headers, data channels,
 *          custom properties, name objects, and JSON serialization per ISO 19848:2024
 */

#include <dnv/VistaSDK.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

int main()
{
    using namespace dnv::vista::sdk;
    using namespace dnv::vista::sdk::transport;
    namespace jdc = dnv::vista::sdk::transport::serialization::json::datachannel;

    std::cout << "=== vista-sdk DataChannelList Sample ===\n\n";

    {
        std::cout << "1. Header: Creating DataChannelList headers\n";
        std::cout << "----------------------------------------------\n";

        // Create ShipId from IMO number
        auto shipId = ShipId::fromString("IMO9074729").value();

        // Create ConfigurationReference
        auto configRef = datachannel::ConfigurationReference{ "vessel-config-2026-v1",
                                                              DateTimeOffset{ "2026-07-16T10:00:00Z" },
                                                              "v1" };

        // Create optional metadata
        auto versionInfo = datachannel::VersionInformation{ "dnv-v2", "3.0a" };
        auto author = std::string{ "Vista SDK Sample" };
        auto dateCreated = DateTimeOffset{ "2026-07-16T10:00:00Z" };
        datachannel::CustomHeaders customHeaders;
        customHeaders.set("vesselType", std::string{ "Container Ship" });
        customHeaders.set("operator", std::string{ "DNV" });
        customHeaders.set("lastModified", std::string{ "2026-07-16T14:30:00Z" });
        auto header =
            datachannel::Header{ shipId, configRef, versionInfo, author, dateCreated, std::move(customHeaders) };

        std::cout << "Created header:\n";
        std::cout << "  Ship ID         : " << header.shipId().toString() << "\n";
        std::cout << "  Config Reference: " << header.dataChannelListId().id() << "\n";
        std::cout << "  Version         : " << header.versionInformation()->namingRule() << "\n";
        std::cout << "  Author          : " << header.author().value() << "\n";
        std::cout << "  Date Created    : " << header.dateCreated()->toString() << "\n";
        std::cout << "  Custom Headers  : Present\n";
        std::cout << "\n";
    }

    {
        std::cout << "2. DataChannelId: Creating channel identifiers\n";
        std::cout << "-------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());
        const auto& locations = vis.locations(vis.latest());
        const auto& codebooks = vis.codebooks(vis.latest());

        // Create LocalId for GPS latitude
        auto primaryItem = GmodPath::fromShortPath("411.1/C101.63-1", gmod, locations).value();
        auto qtyTag = codebooks[CodebookName::Quantity].createTag("angle").value();

        auto localId =
            LocalIdBuilder::create(vis.latest()).withPrimaryItem(primaryItem).withMetadataTag(qtyTag).build();

        // Create NameObject
        datachannel::NameObject nameObject;
        nameObject.setNamingRule("dnv-v2");

        // Create DataChannelId
        auto dataChannelId = datachannel::DataChannelId{ localId, "GPSLatitude", nameObject };

        std::cout << "Created DataChannelId:\n";
        std::cout << "  LocalId    : " << dataChannelId.localId().toString() << "\n";
        std::cout << "  Short ID   : " << *dataChannelId.shortId() << "\n";
        std::cout << "  Naming Rule: " << dataChannelId.nameObject()->namingRule() << "\n";

        std::cout << "\n";
    }

    {
        std::cout << "3. Property: Creating properties with custom fields\n";
        std::cout << "------------------------------------------------------\n";

        // Create standard property fields
        auto dataChannelType = datachannel::DataChannelType{ "Inst" };
        auto format = datachannel::Format{ "Decimal" };
        auto unit = datachannel::Unit{ "deg" };
        auto range = datachannel::Range{ -90.0, 90.0 };

        // Create custom property fields
        datachannel::CustomProperties customProperties;
        customProperties.set("coordinateSystem", std::string{ "WGS84" });
        customProperties.set("precision", Decimal{ 0.000001 });
        customProperties.set("range", std::string{ "-90 to 90" });
        customProperties.set("updateRate", int64_t{ 1 });
        customProperties.set("calibrated", true);
        // Build property
        auto property = datachannel::Property{ dataChannelType,
                                               format,
                                               range,
                                               unit,
                                               std::nullopt, // qualityCoding
                                               std::nullopt, // alertPriority
                                               std::nullopt, // name
                                               std::nullopt, // remarks
                                               std::move(customProperties) };

        std::cout << "Created Property:\n";
        std::cout << "  Type             : " << property.dataChannelType().type() << "\n";
        std::cout << "  Format           : " << property.format().type() << "\n";
        std::cout << "  Unit             : " << property.unit()->unitSymbol() << "\n";
        std::cout << "  Custom Properties: Present\n";
        std::cout << "\n";
    }

    {
        std::cout << "4. DataChannel: Creating complete data channels\n";
        std::cout << "--------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());
        const auto& locations = vis.locations(vis.latest());
        const auto& codebooks = vis.codebooks(vis.latest());

        // Create DataChannelId
        auto primaryItem = GmodPath::fromShortPath("411.1/C101.63-1", gmod, locations).value();
        auto qtyTag = codebooks[CodebookName::Quantity].createTag("angle").value();
        auto localId =
            LocalIdBuilder::create(vis.latest()).withPrimaryItem(primaryItem).withMetadataTag(qtyTag).build();

        datachannel::NameObject nameObject;
        nameObject.setNamingRule("dnv-v2");

        auto dataChannelId = datachannel::DataChannelId{ localId, "GPSLatitude", nameObject };

        // Create Property
        datachannel::CustomProperties customProperties;
        customProperties.set("coordinateSystem", std::string{ "WGS84" });
        customProperties.set("precision", Decimal{ 0.000001 });
        auto property = datachannel::Property{ datachannel::DataChannelType{ "Inst" },
                                               datachannel::Format{ "Decimal" },
                                               datachannel::Range{ -90.0, 90.0 },
                                               datachannel::Unit{ "deg" },
                                               std::nullopt,
                                               std::nullopt,
                                               std::nullopt,
                                               std::nullopt,
                                               std::move(customProperties) };

        // Build DataChannel
        auto dataChannel = datachannel::DataChannel{ dataChannelId, property };

        std::cout << "Created DataChannel:\n";
        if (dataChannel.dataChannelId().shortId())
        {
            std::cout << "  Short ID: " << *dataChannel.dataChannelId().shortId() << "\n";
        }
        std::cout << "  LocalId : " << dataChannel.dataChannelId().localId().toString() << "\n";
        std::cout << "  Type    : " << dataChannel.property().dataChannelType().type() << "\n";
        std::cout << "  Format  : " << dataChannel.property().format().type() << "\n";
        std::cout << "  Unit    : " << dataChannel.property().unit()->unitSymbol() << "\n";

        std::cout << "\n";
    }

    {
        std::cout << "5. DataChannelList: Creating complete channel lists\n";
        std::cout << "------------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());
        const auto& locations = vis.locations(vis.latest());
        const auto& codebooks = vis.codebooks(vis.latest());

        // Create header
        auto shipId = ShipId::fromString("IMO9074729").value();
        auto configRef = datachannel::ConfigurationReference{ "vessel-config-2026-v1",
                                                              DateTimeOffset{ "2026-07-16T10:00:00Z" },
                                                              "v1" };

        datachannel::CustomHeaders customHeaders;
        customHeaders.set("vesselType", std::string{ "Container Ship" });

        auto header = datachannel::Header{ shipId,
                                           configRef,
                                           datachannel::VersionInformation{ "dnv-v2", "3.0a" },
                                           std::string{ "Vista SDK Sample" },
                                           DateTimeOffset{ "2026-07-16T10:00:00Z" },
                                           std::move(customHeaders) };

        // Create multiple data channels
        std::vector<datachannel::DataChannel> channels;

        // Channel 1: GPS Latitude
        {
            auto primaryItem = GmodPath::fromShortPath("411.1/C101.63-1", gmod, locations).value();
            auto qtyTag = codebooks[CodebookName::Quantity].createTag("angle").value();
            auto localId =
                LocalIdBuilder::create(vis.latest()).withPrimaryItem(primaryItem).withMetadataTag(qtyTag).build();

            datachannel::NameObject nameObject;
            nameObject.setNamingRule("dnv-v2");

            auto dataChannelId = datachannel::DataChannelId{ localId, "GPSLatitude", nameObject };

            datachannel::CustomProperties customProps;
            customProps.set("coordinateSystem", std::string{ "WGS84" });

            auto property = datachannel::Property{ datachannel::DataChannelType{ "Inst" },
                                                   datachannel::Format{ "Decimal" },
                                                   datachannel::Range{ -90.0, 90.0 },
                                                   datachannel::Unit{ "deg" },
                                                   std::nullopt,
                                                   std::nullopt,
                                                   std::nullopt,
                                                   std::nullopt,
                                                   std::move(customProps) };

            channels.push_back(datachannel::DataChannel{ dataChannelId, property });
        }

        // Channel 2: GPS Longitude
        {
            auto primaryItem = GmodPath::fromShortPath("411.1/C101.63-2", gmod, locations).value();
            auto qtyTag = codebooks[CodebookName::Quantity].createTag("angle").value();
            auto localId =
                LocalIdBuilder::create(vis.latest()).withPrimaryItem(primaryItem).withMetadataTag(qtyTag).build();

            datachannel::NameObject nameObject;
            nameObject.setNamingRule("dnv-v2");

            auto dataChannelId = datachannel::DataChannelId{ localId, "GPSLongitude", nameObject };

            datachannel::CustomProperties customProps;
            customProps.set("coordinateSystem", std::string{ "WGS84" });

            auto property = datachannel::Property{ datachannel::DataChannelType{ "Inst" },
                                                   datachannel::Format{ "Decimal" },
                                                   datachannel::Range{ -180.0, 180.0 },
                                                   datachannel::Unit{ "deg" },
                                                   std::nullopt,
                                                   std::nullopt,
                                                   std::nullopt,
                                                   std::nullopt,
                                                   std::move(customProps) };

            channels.push_back(datachannel::DataChannel{ dataChannelId, property });
        }

        // Channel 3: Engine Temperature
        {
            auto primaryItem = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations).value();
            auto qtyTag = codebooks[CodebookName::Quantity].createTag("temperature").value();
            auto cntTag = codebooks[CodebookName::Content].createTag("exhaust.gas").value();
            auto localId = LocalIdBuilder::create(vis.latest())
                               .withPrimaryItem(primaryItem)
                               .withMetadataTag(qtyTag)
                               .withMetadataTag(cntTag)
                               .build();

            datachannel::NameObject nameObject;
            nameObject.setNamingRule("dnv-v2");

            auto dataChannelId = datachannel::DataChannelId{ localId, "EngineTemp", nameObject };

            datachannel::CustomProperties customProps;
            customProps.set("sensor", std::string{ "K-type thermocouple" });
            customProps.set("maxValue", Decimal{ 1200.0 });

            auto property = datachannel::Property{ datachannel::DataChannelType{ "Average" },
                                                   datachannel::Format{ "Decimal" },
                                                   datachannel::Range{ -50.0, 1200.0 },
                                                   datachannel::Unit{ "°C" },
                                                   std::nullopt,
                                                   std::nullopt,
                                                   std::nullopt,
                                                   std::nullopt,
                                                   std::move(customProps) };

            channels.push_back(datachannel::DataChannel{ dataChannelId, property });
        }

        // Channel 4: Fuel Level
        {
            auto primaryItem = GmodPath::fromShortPath("621.21/S90", gmod, locations).value();
            auto qtyTag = codebooks[CodebookName::Quantity].createTag("volume").value();
            auto cntTag = codebooks[CodebookName::Content].createTag("fuel.oil").value();
            auto localId = LocalIdBuilder::create(vis.latest())
                               .withPrimaryItem(primaryItem)
                               .withMetadataTag(qtyTag)
                               .withMetadataTag(cntTag)
                               .build();

            datachannel::NameObject nameObject;
            nameObject.setNamingRule("dnv-v2");

            auto dataChannelId = datachannel::DataChannelId{ localId, "FuelLevel", nameObject };

            datachannel::CustomProperties customProps;
            customProps.set("tankCapacity", Decimal{ 5000.0 });
            customProps.set("alarmLevel", Decimal{ 500.0 });

            auto property = datachannel::Property{ datachannel::DataChannelType{ "Inst" },
                                                   datachannel::Format{ "Decimal" },
                                                   datachannel::Range{ 0.0, 5000.0 },
                                                   datachannel::Unit{ "m3" },
                                                   std::nullopt,
                                                   std::nullopt,
                                                   std::nullopt,
                                                   std::nullopt,
                                                   std::move(customProps) };

            channels.push_back(datachannel::DataChannel{ dataChannelId, property });
        }

        // Build Package (combines Header + DataChannelList)
        auto dataChannelList = datachannel::DataChannelList{ channels };
        auto package = datachannel::Package{ header, dataChannelList };

        std::cout << "Created Package with DataChannelList:\n";
        std::cout << "  Ship ID      : " << package.header().shipId().toString() << "\n";
        std::cout << "  Config Ref   : " << package.header().dataChannelListId().id() << "\n";
        std::cout << "  Channel Count: " << package.dataChannelList().dataChannels().size() << "\n\n";

        std::cout << "Channels:\n";
        for (size_t i = 0; i < package.dataChannelList().dataChannels().size(); ++i)
        {
            const auto& channel = package.dataChannelList().dataChannels()[i];
            std::cout << "  [" << (i + 1) << "]";
            if (channel.dataChannelId().shortId())
            {
                std::cout << " " << *channel.dataChannelId().shortId();
            }
            std::cout << " (" << channel.property().dataChannelType().type() << ", "
                      << channel.property().format().type() << ")\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "6. Value Types: Working with different data types\n";
        std::cout << "----------------------------------------------------\n";

        // Create values of different types
        transport::Value stringValue = transport::Value::String{ "test string" };
        transport::Value intValue = transport::Value::Integer{ 42 };
        transport::Value decimalValue = transport::Value::Decimal{ 3.14159 };
        transport::Value boolValue = transport::Value::Boolean{ true };
        transport::Value dateTimeValue = transport::Value::DateTime{ "2026-07-16T15:30:00Z" };

        std::cout << std::boolalpha;
        std::cout << "Value type checks:\n";
        std::cout << "  String   value is string  : " << stringValue.string().has_value() << "\n";
        std::cout << "  Integer  value is integer : " << intValue.integer().has_value() << "\n";
        std::cout << "  Decimal  value is decimal : " << decimalValue.decimal().has_value() << "\n";
        std::cout << "  Boolean  value is boolean : " << boolValue.boolean().has_value() << "\n";
        std::cout << "  DateTime value is dateTime: " << dateTimeValue.dateTime().has_value() << "\n";

        std::cout << "\nExtracting values:\n";
        if (auto str = stringValue.string())
        {
            std::cout << "  String   : " << *str << "\n";
        }
        if (auto i = intValue.integer())
        {
            std::cout << "  Integer  : " << *i << "\n";
        }
        if (auto dec = decimalValue.decimal())
        {
            std::cout << "  Decimal  : " << dec->toString() << "\n";
        }
        if (auto b = boolValue.boolean())
        {
            std::cout << "  Boolean  : " << *b << "\n";
        }
        if (auto dt = dateTimeValue.dateTime())
        {
            std::cout << "  DateTime : " << dt->toString() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "7. Validation: ISO 19848 field validation\n";
        std::cout << "--------------------------------------------\n";

        // Valid DataChannelType
        try
        {
            auto validType = datachannel::DataChannelType{ "Inst" };
            std::cout << "[OK] Created valid DataChannelType: " << validType.type() << "\n";
        }
        catch (const std::exception& e)
        {
            std::cout << "[ERROR] Failed to create DataChannelType: " << e.what() << "\n";
        }

        // Invalid DataChannelType
        try
        {
            auto invalidType = datachannel::DataChannelType{ "InvalidType" };
            std::cout << "[ERROR] Created invalid DataChannelType (should have thrown)\n";
        }
        catch (const std::invalid_argument& e)
        {
            std::cout << "[OK] Correctly rejected invalid DataChannelType: " << e.what() << "\n";
        }

        // Valid Format
        try
        {
            auto validFormat = datachannel::Format{ "Decimal" };
            std::cout << "[OK] Created valid Format: " << validFormat.type() << "\n";
        }
        catch (const std::exception& e)
        {
            std::cout << "[ERROR] Failed to create Format: " << e.what() << "\n";
        }

        // Invalid Format
        try
        {
            auto invalidFormat = datachannel::Format{ "InvalidFormat" };
            std::cout << "[ERROR] Created invalid Format (should have thrown)\n";
        }
        catch (const std::invalid_argument& e)
        {
            std::cout << "[OK] Correctly rejected invalid Format: " << e.what() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "8. JSON Serialization: Converting to/from JSON\n";
        std::cout << "------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());
        const auto& locations = vis.locations(vis.latest());
        const auto& codebooks = vis.codebooks(vis.latest());

        // Create a simple DataChannelList
        auto shipId = ShipId::fromString("IMO1234567").value();
        auto header = datachannel::Header{
            shipId,
            datachannel::ConfigurationReference{ "demo-config-v1", DateTimeOffset{ "2026-07-16T10:00:00Z" } },
            std::nullopt,
            std::nullopt,
            std::nullopt,
            std::nullopt
        };

        std::vector<datachannel::DataChannel> channels;

        // GPS Latitude
        {
            auto primaryItem = GmodPath::fromShortPath("710.1/F211.11", gmod, locations).value();
            auto qtyTag = codebooks[CodebookName::Quantity].createTag("latitude").value();
            auto localId =
                LocalIdBuilder::create(vis.latest()).withPrimaryItem(primaryItem).withMetadataTag(qtyTag).build();

            datachannel::NameObject nameObject;
            nameObject.setNamingRule("dnv-v2");

            auto dataChannelId = datachannel::DataChannelId{ localId, "GPSLatitude", nameObject };

            datachannel::CustomProperties customProps;
            customProps.set("coordinateSystem", std::string{ "WGS84" });
            customProps.set("geodeticDatum", std::string{ "WGS84" });

            auto property = datachannel::Property{ datachannel::DataChannelType{ "Inst" },
                                                   datachannel::Format{ "Decimal" },
                                                   datachannel::Range{ -90.0, 90.0 },
                                                   datachannel::Unit{ "deg" },
                                                   std::nullopt,
                                                   std::nullopt,
                                                   "GPS Latitude",
                                                   "Primary GPS latitude position",
                                                   std::move(customProps) };

            channels.push_back(datachannel::DataChannel{ dataChannelId, property });
        }

        // GPS Longitude
        {
            auto primaryItem = GmodPath::fromShortPath("710.1/F211.12", gmod, locations).value();
            auto qtyTag = codebooks[CodebookName::Quantity].createTag("longitude").value();
            auto localId =
                LocalIdBuilder::create(vis.latest()).withPrimaryItem(primaryItem).withMetadataTag(qtyTag).build();

            datachannel::NameObject nameObject;
            nameObject.setNamingRule("dnv-v2");

            auto dataChannelId = datachannel::DataChannelId{ localId, "GPSLongitude", nameObject };

            datachannel::CustomProperties customProps;
            customProps.set("coordinateSystem", std::string{ "WGS84" });
            customProps.set("geodeticDatum", std::string{ "WGS84" });

            auto property = datachannel::Property{ datachannel::DataChannelType{ "Inst" },
                                                   datachannel::Format{ "Decimal" },
                                                   datachannel::Range{ -180.0, 180.0 },
                                                   datachannel::Unit{ "deg" },
                                                   std::nullopt,
                                                   std::nullopt,
                                                   "GPS Longitude",
                                                   "Primary GPS longitude position",
                                                   std::move(customProps) };

            channels.push_back(datachannel::DataChannel{ dataChannelId, property });
        }

        auto dataChannelList = datachannel::DataChannelList{ channels };
        auto package = datachannel::Package{ header, dataChannelList };
        auto dataChannelListPackage = datachannel::DataChannelListPackage{ package };

        StringBuilder sb;
        jdc::toJsonString(sb, dataChannelListPackage, true);

        std::cout << "Serialization result: Success\n";
        std::cout << "\nJSON output (formatted):\n";
        std::cout << sb.view() << std::endl;

        std::cout << "\n";
    }

    {
        std::cout << "9. Advanced: Main Engine Monitoring System\n";
        std::cout << "--------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& gmod = vis.gmod(vis.latest());
        const auto& locations = vis.locations(vis.latest());
        const auto& codebooks = vis.codebooks(vis.latest());

        // Create header with engine monitoring system metadata
        auto shipId = ShipId::fromString("IMO9074729").value();
        auto configRef = datachannel::ConfigurationReference{ "main-engine-monitor-v3",
                                                              DateTimeOffset{ "2026-07-16T10:00:00Z" },
                                                              "3.0" };
        auto versionInfo = datachannel::VersionInformation{ "dnv-v2", "3.10a" };

        datachannel::CustomHeaders customHeaders;
        customHeaders.set("monitoringSystem", std::string{ "Vista Engine Monitor" });
        customHeaders.set("engineManufacturer", std::string{ "MAN Energy Solutions" });
        customHeaders.set("engineModel", std::string{ "ME-C9.5-175" });
        customHeaders.set("samplingRate", Decimal{ 10.0 });

        auto header = datachannel::Header{ shipId,
                                           configRef,
                                           versionInfo,
                                           std::string{ "Chief Engineer" },
                                           DateTimeOffset{ "2026-07-16T10:00:00Z" },
                                           std::move(customHeaders) };

        std::vector<datachannel::DataChannel> channels;

        // Channel 1: Engine Speed (RPM)
        {
            auto primaryItem = GmodPath::fromShortPath("411.1/C101.41", gmod, locations).value();
            auto qtyTag = codebooks[CodebookName::Quantity].createTag("rotational.speed").value();
            auto localId =
                LocalIdBuilder::create(vis.latest()).withPrimaryItem(primaryItem).withMetadataTag(qtyTag).build();

            datachannel::CustomNameObjects customNameObjects;
            customNameObjects.set("sensorType", std::string{ "Magnetic pickup" });
            customNameObjects.set("sensorManufacturer", std::string{ "Wärtsilä" });
            customNameObjects.set("calibrationDate", std::string{ "2026-07-16T10:00:00Z" });
            customNameObjects.set("isRedundant", true);

            datachannel::NameObject nameObject;
            nameObject.setNamingRule("dnv-v2");
            nameObject.setCustomNameObjects(customNameObjects);

            auto dataChannelId = datachannel::DataChannelId{ localId, "MainEngineRPM", nameObject };

            datachannel::CustomElements unitCustomElements;
            unitCustomElements.set("siEquivalent", std::string{ "hertz" });
            unitCustomElements.set("conversionFactor", Decimal{ 60.0 });

            auto unit = datachannel::Unit{ "rpm", "rotational speed", std::move(unitCustomElements) };

            datachannel::CustomProperties customProperties;
            customProperties.set("nominalSpeed", Decimal{ 150.0 });
            customProperties.set("maxSpeed", Decimal{ 175.0 });
            customProperties.set("idleSpeed", Decimal{ 60.0 });
            customProperties.set("criticalAlarm", true);

            auto property = datachannel::Property{ datachannel::DataChannelType{ "Inst" },
                                                   datachannel::Format{ "Decimal" },
                                                   datachannel::Range{ 0.0, 200.0 },
                                                   unit,
                                                   "A",
                                                   "High",
                                                   "Main Engine Rotational Speed",
                                                   "Primary propulsion engine RPM",
                                                   std::move(customProperties) };

            channels.push_back(datachannel::DataChannel{ dataChannelId, property });
        }

        // Channel 2: Lube Oil Pressure
        {
            auto primaryItem = GmodPath::fromShortPath("411.1/C101.663i", gmod, locations).value();
            auto qtyTag = codebooks[CodebookName::Quantity].createTag("pressure").value();
            auto cntTag = codebooks[CodebookName::Content].createTag("lubricating.oil").value();
            auto posTag = codebooks[CodebookName::Position].createTag("inlet").value();
            auto localId = LocalIdBuilder::create(vis.latest())
                               .withPrimaryItem(primaryItem)
                               .withMetadataTag(qtyTag)
                               .withMetadataTag(cntTag)
                               .withMetadataTag(posTag)
                               .build();

            datachannel::CustomNameObjects customNameObjects;
            customNameObjects.set("sensorType", std::string{ "Pressure transducer" });
            customNameObjects.set("sensorManufacturer", std::string{ "WIKA" });
            customNameObjects.set("sensorModel", std::string{ "A-10" });
            customNameObjects.set("accuracy", Decimal{ 0.5 });

            datachannel::NameObject nameObject;
            nameObject.setNamingRule("dnv-v2");
            nameObject.setCustomNameObjects(customNameObjects);

            auto dataChannelId = datachannel::DataChannelId{ localId, "LubeOilPressure", nameObject };

            datachannel::CustomElements unitCustomElements;
            unitCustomElements.set("pressureType", std::string{ "Gauge" });
            unitCustomElements.set("siEquivalent", std::string{ "pascal" });
            unitCustomElements.set("conversionFactor", Decimal{ 100000.0 });

            auto unit = datachannel::Unit{ "bar", "pressure", std::move(unitCustomElements) };

            datachannel::CustomProperties customProperties;
            customProperties.set("normalPressure", Decimal{ 4.2 });
            customProperties.set("minPressure", Decimal{ 2.5 });
            customProperties.set("alarmThreshold", Decimal{ 2.0 });
            customProperties.set("shutdownThreshold", Decimal{ 1.5 });

            auto property = datachannel::Property{ datachannel::DataChannelType{ "Average", 5.0, 1.0 },
                                                   datachannel::Format{ "Decimal" },
                                                   datachannel::Range{ 0.0, 10.0 },
                                                   unit,
                                                   "A",
                                                   "High",
                                                   "Lubricating Oil Inlet Pressure",
                                                   "Main engine lube oil system inlet pressure",
                                                   std::move(customProperties) };

            channels.push_back(datachannel::DataChannel{ dataChannelId, property });
        }

        // Channel 3: Exhaust Gas Temperature
        {
            auto primaryItem = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations).value();
            auto qtyTag = codebooks[CodebookName::Quantity].createTag("temperature").value();
            auto cntTag = codebooks[CodebookName::Content].createTag("exhaust.gas").value();
            auto localId = LocalIdBuilder::create(vis.latest())
                               .withPrimaryItem(primaryItem)
                               .withMetadataTag(qtyTag)
                               .withMetadataTag(cntTag)
                               .build();

            datachannel::CustomNameObjects customNameObjects;
            customNameObjects.set("sensorType", std::string{ "K-type thermocouple" });
            customNameObjects.set("cylinderNumber", static_cast<int64_t>(2));
            customNameObjects.set("maxTemp", Decimal{ 600.0 });
            customNameObjects.set("isCritical", true);

            datachannel::NameObject nameObject;
            nameObject.setNamingRule("dnv-v2");
            nameObject.setCustomNameObjects(customNameObjects);

            auto dataChannelId = datachannel::DataChannelId{ localId, "ExhaustTemp_Cyl2", nameObject };

            datachannel::CustomElements unitCustomElements;
            unitCustomElements.set("siEquivalent", std::string{ "kelvin" });
            unitCustomElements.set("conversionOffset", Decimal{ 273.15 });

            auto unit = datachannel::Unit{ "degC", "temperature", std::move(unitCustomElements) };

            datachannel::CustomProperties customProperties;
            customProperties.set("normalTemp", Decimal{ 380.0 });
            customProperties.set("maxTemp", Decimal{ 450.0 });
            customProperties.set("alarmThreshold", Decimal{ 420.0 });
            customProperties.set("deviationAlarm", Decimal{ 30.0 });
            auto property = datachannel::Property{ datachannel::DataChannelType{ "Average", 10.0, 10.0 },
                                                   datachannel::Format{ "Decimal" },
                                                   datachannel::Range{ 0.0, 600.0 },
                                                   unit,
                                                   "A",
                                                   "High",
                                                   "Exhaust Gas Temperature Cylinder 2",
                                                   "Main engine cylinder 2 exhaust temperature",
                                                   std::move(customProperties) };

            channels.push_back(datachannel::DataChannel{ dataChannelId, property });
        }

        // Channel 4: Fuel Consumption
        {
            auto primaryItem = GmodPath::fromShortPath("411.1/C101", gmod, locations).value();
            auto secItem = GmodPath::fromShortPath("620.1/M201.32", gmod, locations).value();
            auto qtyTag = codebooks[CodebookName::Quantity].createTag("volume.flow.rate").value();
            auto localId = LocalIdBuilder::create(vis.latest())
                               .withPrimaryItem(primaryItem)
                               .withSecondaryItem(secItem)
                               .withMetadataTag(qtyTag)
                               .build();

            datachannel::CustomNameObjects customNameObjects;
            customNameObjects.set("meterType", std::string{ "Coriolis flow meter" });
            customNameObjects.set("meterManufacturer", std::string{ "Endress+Hauser" });
            customNameObjects.set("meterModel", std::string{ "Promass 83F" });
            customNameObjects.set("accuracy", Decimal{ 0.1 });

            datachannel::NameObject nameObject;
            nameObject.setNamingRule("dnv-v2");
            nameObject.setCustomNameObjects(customNameObjects);

            auto dataChannelId = datachannel::DataChannelId{ localId, "FuelConsumption", nameObject };

            datachannel::CustomElements unitCustomElements;
            unitCustomElements.set("flowType", std::string{ "Volumetric" });
            unitCustomElements.set("fluidType", std::string{ "HFO380" });
            unitCustomElements.set("refTemperature", Decimal{ 15.0 });

            auto unit = datachannel::Unit{ "l/h", "volume flow rate", std::move(unitCustomElements) };

            datachannel::CustomProperties customProperties;
            customProperties.set("fuelGrade", std::string{ "IFO380" });
            customProperties.set("fuelDensity", Decimal{ 991.0 });
            customProperties.set("nominalConsumption", Decimal{ 2800.0 });
            customProperties.set("co2Factor", Decimal{ 3.114 });

            auto property = datachannel::Property{ datachannel::DataChannelType{ "Average", 60.0, 60.0 },
                                                   datachannel::Format{ "Decimal" },
                                                   datachannel::Range{ 0.0, 5000.0 },
                                                   unit,
                                                   "A",
                                                   "Normal",
                                                   "Main Engine Fuel Consumption",
                                                   "Hourly fuel consumption monitoring",
                                                   std::move(customProperties) };

            channels.push_back(datachannel::DataChannel{ dataChannelId, property });
        }

        auto dataChannelList = datachannel::DataChannelList{ channels };
        auto package = datachannel::Package{ header, dataChannelList };
        auto dataChannelListPackage = datachannel::DataChannelListPackage{ package };

        // Serialize
        StringBuilder engineJsonBuffer;
        jdc::toJsonString(engineJsonBuffer, dataChannelListPackage, true);

        std::cout << "Main Engine Monitoring System:\n";
        std::cout << "  Engine       : MAN Energy Solutions ME-C9.5-175\n";
        std::cout << "  Channels     : " << dataChannelList.dataChannels().size() << "\n";
        std::cout << "  Custom fields: Present\n";
        std::cout << "\nMonitored parameters:\n";
        for (size_t i = 0; i < dataChannelList.dataChannels().size(); ++i)
        {
            const auto& channel = dataChannelList.dataChannels()[i];
            std::cout << "  [" << (i + 1) << "] " << *channel.dataChannelId().shortId();
            if (channel.property().name().has_value())
            {
                std::cout << " - " << *channel.property().name();
            }
            std::cout << "\n";
        }

        std::cout << "\nEngine Monitoring JSON:\n";
        std::cout << engineJsonBuffer.view() << std::endl;

        std::cout << "\n";
    }

    {
        std::cout << "10. Advanced: DTO-level manipulation before serialization\n";
        std::cout << "---------------------------------------------------------\n";

        // Build a minimal package
        auto shipId = ShipId::fromString("IMO8027781").value();
        auto configRef =
            datachannel::ConfigurationReference{ "dto-patch-demo", DateTimeOffset{ "2026-07-16T00:00:00Z" }, "1.0" };
        auto header = datachannel::Header{ shipId, configRef };
        auto package = datachannel::Package{ header, datachannel::DataChannelList{} };
        auto domainPackage = datachannel::DataChannelListPackage{ package };

        // Convert to the DTO (the serialization-facing representation) so the package can be
        // patched and serialized directly, without rebuilding a validated domain package
        auto dto = jdc::toDto(domainPackage);
        dto.package.header.author = "export-pipeline";
        if (!dto.package.header.customHeaders)
        {
            dto.package.header.customHeaders = serialization::json::SerializableDocument{};
        }
        dto.package.header.customHeaders->set("exportedBy", std::string{ "vista-sdk-sample" });
        dto.package.header.customHeaders->set("exportTimestamp", DateTimeOffset::utcNow().toString());

        StringBuilder patchedJsonBuffer;
        jdc::toJsonString(patchedJsonBuffer, dto, true);

        std::cout << "DTO patched with author and exportTimestamp custom header:\n";
        std::cout << patchedJsonBuffer.view() << std::endl;

        std::cout << "\n";
    }

    return 0;
}
