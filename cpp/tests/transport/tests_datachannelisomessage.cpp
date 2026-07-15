#include <doctest/doctest.h>

#include <dnv/vista/sdk/transport/datachannel/DataChannel.h>
#include <dnv/vista/sdk/transport/ShipId.h>

namespace dnv::vista::sdk::tests
{
    transport::datachannel::DataChannelListPackage createValidDataChannelList()
    {
        transport::datachannel::ConfigurationReference dataChannelListId{ "some-id",
                                                                          DateTimeOffset{ "2016-01-01T00:00:00Z" } };

        transport::datachannel::Header header{ transport::ShipId::fromString("IMO1234567").value(),
                                               dataChannelListId,
                                               std::nullopt,
                                               std::string{ "some-author" },
                                               DateTimeOffset::utcNow(),
                                               std::nullopt };

        transport::datachannel::DataChannelList dataChannelList;

        {
            auto localId = LocalId::fromString("/dnv-v2/vis-3-4a/511.15-1/E32/meta/qty-power").value();
            transport::datachannel::DataChannelId dataChannelId{ localId, std::string{ "0010" } };

            transport::datachannel::DataChannelType dataChannelType{ "Inst" };
            transport::datachannel::Format format{ "String" };

            transport::datachannel::Property property{ dataChannelType, format,       std::nullopt,
                                                       std::nullopt,    std::nullopt, std::nullopt,
                                                       std::nullopt,    std::nullopt, std::nullopt };

            dataChannelList.add(transport::datachannel::DataChannel{ dataChannelId, property });
        }

        transport::datachannel::Package package{ header, std::move(dataChannelList) };
        return transport::datachannel::DataChannelListPackage{ std::move(package) };
    }

    transport::datachannel::DataChannelListPackage createValidFullyCustomDataChannelList()
    {
        transport::datachannel::ConfigurationReference dataChannelListId{ "DataChannelList.xml",
                                                                          DateTimeOffset{ "2016-01-01T00:00:00Z" },
                                                                          std::string{ "1.0" } };

        transport::datachannel::VersionInformation versionInfo{ "some_naming_rule",
                                                                "2.0",
                                                                std::string{ "http://somewhere.net" } };

        transport::datachannel::CustomHeaders customHeaders =
            transport::serialization::json::SerializableDocument::object();
        customHeaders.set(
            "nr:CustomHeaderElement",
            transport::serialization::json::SerializableDocument{ std::string{ "Vendor specific headers" } });
        customHeaders.set("sampleRate", transport::serialization::json::SerializableDocument{ std::int64_t{ 1000 } });
        customHeaders.set("isValidated", transport::serialization::json::SerializableDocument{ true });
        customHeaders.set("tolerance", transport::serialization::json::SerializableDocument{ std::string{ "0.001" } });
        customHeaders.set(
            "certificationDate",
            transport::serialization::json::SerializableDocument{ std::string{ "2024-01-15T00:00:00Z" } });

        transport::datachannel::Header header{ transport::ShipId::fromString("IMO1234567").value(),
                                               dataChannelListId,
                                               versionInfo,
                                               std::string{ "Author1" },
                                               DateTimeOffset{ "2015-12-01T00:00:00Z" },
                                               std::move(customHeaders) };

        transport::datachannel::DataChannelList dataChannelList;

        // Temperature sensor
        {
            auto localId =
                LocalId::fromString("/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/meta/qty-temperature/cnt-cooling.air")
                    .value();

            transport::datachannel::DataChannelId dataChannelId{ localId, std::string{ "0010" } };

            transport::datachannel::CustomNameObjects customNameObjs =
                transport::serialization::json::SerializableDocument::object();
            customNameObjs.set(
                "nr:CustomNameObject",
                transport::serialization::json::SerializableDocument{ std::string{ "Vendor specific NameObject" } });
            customNameObjs.set("priority", transport::serialization::json::SerializableDocument{ std::int64_t{ 10 } });
            customNameObjs.set("isActive", transport::serialization::json::SerializableDocument{ false });
            customNameObjs.set("offset", transport::serialization::json::SerializableDocument{ 2.5 });
            customNameObjs.set(
                "lastModified",
                transport::serialization::json::SerializableDocument{ std::string{ "2024-01-10T12:30:00Z" } });

            transport::datachannel::NameObject nameObject{ "Naming_Rule", std::move(customNameObjs) };
            dataChannelId.setNameObject(nameObject);

            transport::datachannel::DataChannelType dataChannelType{ "Inst" };
            dataChannelType.setUpdateCycle(1.0);

            transport::datachannel::Format format{ "Decimal" };
            transport::datachannel::Restriction restriction;
            restriction.setFractionDigits(1);
            restriction.setMaxInclusive(200.0);
            restriction.setMinInclusive(-150.0);
            format.setRestriction(restriction);

            transport::datachannel::Range range{ 0.0, 150.0 };

            transport::datachannel::CustomElements customElements =
                transport::serialization::json::SerializableDocument::object();
            customElements.set(
                "nr:CustomUnitElement",
                transport::serialization::json::SerializableDocument{ std::string{ "Vendor specific unit element" } });
            customElements.set(
                "conversionFactor", transport::serialization::json::SerializableDocument{ std::int64_t{ 100 } });
            customElements.set("isMetric", transport::serialization::json::SerializableDocument{ false });
            customElements.set("multiplier", transport::serialization::json::SerializableDocument{ 1.8 });
            customElements.set(
                "standardizedDate",
                transport::serialization::json::SerializableDocument{ std::string{ "2020-05-01T00:00:00Z" } });

            transport::datachannel::Unit unit{ "°C", std::string{ "Temperature" }, std::move(customElements) };

            transport::datachannel::CustomProperties customProperties =
                transport::serialization::json::SerializableDocument::object();
            customProperties.set(
                "nr:CustomPropertyElement",
                transport::serialization::json::SerializableDocument{
                    std::string{ "Vendor specific property element" } });
            customProperties.set(
                "maxSamples", transport::serialization::json::SerializableDocument{ std::int64_t{ 500 } });
            customProperties.set("requiresCalibration", transport::serialization::json::SerializableDocument{ false });
            customProperties.set("accuracy", transport::serialization::json::SerializableDocument{ 0.05 });
            customProperties.set(
                "installationDate",
                transport::serialization::json::SerializableDocument{ std::string{ "2023-06-15T08:00:00Z" } });

            transport::datachannel::Property property{ dataChannelType,
                                                       format,
                                                       range,
                                                       unit,
                                                       std::string{ "OPC_QUALITY" },
                                                       std::nullopt,
                                                       std::string{ "M/E #1 Air Cooler CFW OUT Temp" },
                                                       std::string{
                                                           "Location: ECR, Manufacturer: AAA Company, Type: TYPE-AAA" },
                                                       customProperties };

            dataChannelList.add(transport::datachannel::DataChannel{ dataChannelId, property });
        }

        // Alert type
        {
            auto localId = LocalId::fromString("/dnv-v2/vis-3-4a/511.15-1/E32/meta/qty-power").value();
            transport::datachannel::DataChannelId dataChannelId{ localId, std::string{ "0020" } };

            transport::datachannel::DataChannelType dataChannelType{ "Alert" };

            transport::datachannel::Format format{ "String" };
            transport::datachannel::Restriction restriction;
            restriction.setMaxLength(100);
            restriction.setMinLength(0);
            format.setRestriction(restriction);

            transport::datachannel::Property property{ dataChannelType, format,       std::nullopt,
                                                       std::nullopt,    std::nullopt, std::string{ "Warning" },
                                                       std::nullopt,    std::nullopt, std::nullopt };

            dataChannelList.add(transport::datachannel::DataChannel{ dataChannelId, property });
        }

        transport::datachannel::Package package{ header, std::move(dataChannelList) };
        return transport::datachannel::DataChannelListPackage{ std::move(package) };
    }

    TEST_SUITE("IsoMessageTests")
    {
        TEST_CASE("DataChannelList - basic construction")
        {
            auto message = createValidDataChannelList();
            CHECK_FALSE(message.package().dataChannelList().size() == 0);

            const auto& first = message.package().dataChannelList().dataChannels().front();
            CHECK_FALSE(first.dataChannelId().localId().toString().empty());
            CHECK_NOTHROW((void)first.property());
        }

        TEST_CASE("DataChannelList - LocalId lookup")
        {
            auto message = createValidDataChannelList();
            const auto& dataChannel = message.package().dataChannelList()[0];
            const auto& localId = dataChannel.dataChannelId().localId();

            const auto& lookup = message.package().dataChannelList()[localId];
            auto lookupOpt = message.package().dataChannelList().from(localId);

            REQUIRE(lookupOpt.has_value());
            CHECK_EQ(&dataChannel, &lookup);
            CHECK_EQ(&dataChannel, &lookupOpt->get());
        }

        TEST_CASE("DataChannelList - ShortId lookup")
        {
            auto message = createValidDataChannelList();
            const auto& dataChannel = message.package().dataChannelList()[0];
            const auto& shortId = dataChannel.dataChannelId().shortId();

            REQUIRE(shortId.has_value());
            const auto& lookup = message.package().dataChannelList()[*shortId];
            auto lookupOpt = message.package().dataChannelList().from(*shortId);

            REQUIRE(lookupOpt.has_value());
            CHECK_EQ(&dataChannel, &lookup);
            CHECK_EQ(&dataChannel, &lookupOpt->get());
        }

        TEST_CASE("DataChannelList - iteration")
        {
            auto message = createValidDataChannelList();
            const auto expectedSize = message.package().dataChannelList().size();

            std::size_t counter = 0;
            for (const auto& dc : message.package().dataChannelList())
            {
                (void)dc;
                ++counter;
            }
            CHECK_EQ(expectedSize, counter);

            counter = 0;
            for (auto it = message.package().dataChannelList().begin(); it != message.package().dataChannelList().end();
                 ++it)
            {
                ++counter;
            }
            CHECK_EQ(expectedSize, counter);
        }
    }

    TEST_SUITE("DataChannelTests")
    {
        TEST_CASE("FullyCustomDataChannel - structure")
        {
            auto pkg = createValidFullyCustomDataChannelList();
            CHECK_EQ(pkg.package().dataChannelList().size(), 2u);

            // Custom headers
            const auto& header = pkg.package().header();
            REQUIRE(header.customHeaders().has_value());
            const auto& customHeaders = *header.customHeaders();

            const auto* strVal = customHeaders.find("nr:CustomHeaderElement");
            REQUIRE(strVal != nullptr);
            CHECK_EQ(strVal->asString(), "Vendor specific headers");

            const auto* sampleRate = customHeaders.find("sampleRate");
            REQUIRE(sampleRate != nullptr);
            CHECK_EQ(sampleRate->asInteger(), 1000);

            const auto* isValidated = customHeaders.find("isValidated");
            REQUIRE(isValidated != nullptr);
            CHECK(isValidated->asBoolean());

            const auto* tolerance = customHeaders.find("tolerance");
            REQUIRE(tolerance != nullptr);
            CHECK_EQ(tolerance->asString(), "0.001");
        }

        TEST_CASE("FullyCustomDataChannel - temperature sensor")
        {
            auto pkg = createValidFullyCustomDataChannelList();
            const auto& tempChannel = pkg.package().dataChannelList()[0];
            const auto& prop = tempChannel.property();

            CHECK_EQ(prop.dataChannelType().type(), "Inst");
            REQUIRE(prop.dataChannelType().updateCycle().has_value());
            CHECK_EQ(*prop.dataChannelType().updateCycle(), 1.0);

            CHECK_EQ(prop.format().type(), "Decimal");
            REQUIRE(prop.format().restriction().has_value());
            const auto& restriction = *prop.format().restriction();
            REQUIRE(restriction.fractionDigits().has_value());
            CHECK_EQ(*restriction.fractionDigits(), 1u);
            REQUIRE(restriction.maxInclusive().has_value());
            CHECK_EQ(*restriction.maxInclusive(), 200.0);
            REQUIRE(restriction.minInclusive().has_value());
            CHECK_EQ(*restriction.minInclusive(), -150.0);

            REQUIRE(prop.range().has_value());
            CHECK_EQ(prop.range()->low(), 0.0);
            CHECK_EQ(prop.range()->high(), 150.0);

            REQUIRE(prop.unit().has_value());
            CHECK_EQ(prop.unit()->unitSymbol(), "°C");
            REQUIRE(prop.unit()->quantityName().has_value());
            CHECK_EQ(*prop.unit()->quantityName(), "Temperature");

            REQUIRE(prop.qualityCoding().has_value());
            CHECK_EQ(*prop.qualityCoding(), "OPC_QUALITY");

            REQUIRE(prop.name().has_value());
            CHECK_EQ(*prop.name(), "M/E #1 Air Cooler CFW OUT Temp");

            REQUIRE(prop.remarks().has_value());
            CHECK_EQ(*prop.remarks(), "Location: ECR, Manufacturer: AAA Company, Type: TYPE-AAA");

            // NameObject custom fields
            REQUIRE(tempChannel.dataChannelId().nameObject().has_value());
            const auto& nameObject = *tempChannel.dataChannelId().nameObject();
            CHECK_EQ(nameObject.namingRule(), "Naming_Rule");

            REQUIRE(nameObject.customNameObjects().has_value());
            const auto& customNameObjs = *nameObject.customNameObjects();

            const auto* nameObjStr = customNameObjs.find("nr:CustomNameObject");
            REQUIRE(nameObjStr != nullptr);
            CHECK_EQ(nameObjStr->asString(), "Vendor specific NameObject");

            const auto* priority = customNameObjs.find("priority");
            REQUIRE(priority != nullptr);
            CHECK_EQ(priority->asInteger(), 10);

            const auto* isActive = customNameObjs.find("isActive");
            REQUIRE(isActive != nullptr);
            CHECK_FALSE(isActive->asBoolean());

            const auto* offset = customNameObjs.find("offset");
            REQUIRE(offset != nullptr);
            CHECK_EQ(offset->asDouble(), doctest::Approx(2.5));

            // Unit custom elements
            REQUIRE(prop.unit()->customElements().has_value());
            const auto& customElements = *prop.unit()->customElements();

            const auto* unitElem = customElements.find("nr:CustomUnitElement");
            REQUIRE(unitElem != nullptr);
            CHECK_EQ(unitElem->asString(), "Vendor specific unit element");

            const auto* convFactor = customElements.find("conversionFactor");
            REQUIRE(convFactor != nullptr);
            CHECK_EQ(convFactor->asInteger(), 100);

            const auto* isMetric = customElements.find("isMetric");
            REQUIRE(isMetric != nullptr);
            CHECK_FALSE(isMetric->asBoolean());

            const auto* multiplier = customElements.find("multiplier");
            REQUIRE(multiplier != nullptr);
            CHECK_EQ(multiplier->asDouble(), doctest::Approx(1.8));

            // Custom properties
            REQUIRE(prop.customProperties().has_value());
            const auto& customProps = *prop.customProperties();

            const auto* propElem = customProps.find("nr:CustomPropertyElement");
            REQUIRE(propElem != nullptr);
            CHECK_EQ(propElem->asString(), "Vendor specific property element");

            const auto* maxSamples = customProps.find("maxSamples");
            REQUIRE(maxSamples != nullptr);
            CHECK_EQ(maxSamples->asInteger(), 500);

            const auto* requiresCalib = customProps.find("requiresCalibration");
            REQUIRE(requiresCalib != nullptr);
            CHECK_FALSE(requiresCalib->asBoolean());

            const auto* accuracy = customProps.find("accuracy");
            REQUIRE(accuracy != nullptr);
            CHECK_EQ(accuracy->asDouble(), doctest::Approx(0.05));
        }

        TEST_CASE("FullyCustomDataChannel - alert type")
        {
            auto pkg = createValidFullyCustomDataChannelList();
            const auto& alertChannel = pkg.package().dataChannelList()[1];
            const auto& prop = alertChannel.property();

            CHECK_EQ(prop.dataChannelType().type(), "Alert");
            CHECK_EQ(prop.format().type(), "String");

            REQUIRE(prop.alertPriority().has_value());
            CHECK_EQ(*prop.alertPriority(), "Warning");

            REQUIRE(prop.format().restriction().has_value());
            const auto& restriction = *prop.format().restriction();
            REQUIRE(restriction.maxLength().has_value());
            CHECK_EQ(*restriction.maxLength(), 100u);
            REQUIRE(restriction.minLength().has_value());
            CHECK_EQ(*restriction.minLength(), 0u);

            CHECK_FALSE(prop.range().has_value());
            CHECK_FALSE(prop.unit().has_value());
            CHECK_FALSE(prop.qualityCoding().has_value());
            CHECK_FALSE(prop.name().has_value());
            CHECK_FALSE(prop.remarks().has_value());
            CHECK_FALSE(prop.customProperties().has_value());
        }

        TEST_CASE("Property - validation valid decimal")
        {
            transport::datachannel::DataChannelType type{ "Inst" };
            transport::datachannel::Format format{ "Decimal" };
            transport::datachannel::Range range{ 0.0, 100.0 };
            transport::datachannel::Unit unit{ "°C" };

            transport::datachannel::Property property{ type,         format,       range,
                                                       unit,         std::nullopt, std::nullopt,
                                                       std::nullopt, std::nullopt, std::nullopt };

            CHECK(property.validate());
        }

        TEST_CASE("Property - validation valid alert")
        {
            transport::datachannel::DataChannelType type{ "Alert" };
            transport::datachannel::Format format{ "String" };

            transport::datachannel::Property property{ type,         format,       std::nullopt,
                                                       std::nullopt, std::nullopt, std::string{ "Critical" },
                                                       std::nullopt, std::nullopt, std::nullopt };

            CHECK(property.validate());
        }

        TEST_CASE("Restriction - length validation")
        {
            transport::datachannel::Restriction restriction;
            transport::datachannel::Format format{ "String" };
            restriction.setLength(5);

            CHECK(restriction.validateValue("hello", format));
            CHECK_FALSE(restriction.validateValue("hi", format));
            CHECK_FALSE(restriction.validateValue("toolong", format));
        }

        TEST_CASE("Restriction - min/max length validation")
        {
            transport::datachannel::Restriction restriction;
            transport::datachannel::Format format{ "String" };
            restriction.setMinLength(2);
            restriction.setMaxLength(10);

            CHECK(restriction.validateValue("ok", format));
            CHECK_FALSE(restriction.validateValue("x", format));
            CHECK_FALSE(restriction.validateValue("thisstringistoolong", format));
        }

        TEST_CASE("Restriction - totalDigits validation for Integer values")
        {
            transport::datachannel::Restriction restriction;
            transport::datachannel::Format format{ "Integer" };
            restriction.setTotalDigits(3);

            CHECK(restriction.validateValue("123", format));
            CHECK(restriction.validateValue("-123", format));
            CHECK_FALSE(restriction.validateValue("12", format));
            CHECK_FALSE(restriction.validateValue("1234", format));
        }

        TEST_CASE("Restriction - totalDigits validation for Decimal values")
        {
            transport::datachannel::Restriction restriction;
            transport::datachannel::Format format{ "Decimal" };
            restriction.setTotalDigits(4);

            CHECK(restriction.validateValue("123.4", format));
            CHECK(restriction.validateValue("123.400", format));
            CHECK_FALSE(restriction.validateValue("12.3", format));
            CHECK_FALSE(restriction.validateValue("12345", format));
        }

        TEST_CASE("Range - construction and validation")
        {
            transport::datachannel::Range range{ 0.0, 100.0 };
            CHECK_EQ(range.low(), 0.0);
            CHECK_EQ(range.high(), 100.0);

            CHECK_THROWS_AS((transport::datachannel::Range{ 100.0, 0.0 }), std::invalid_argument);

            range.setLow(10.0);
            CHECK_EQ(range.low(), 10.0);

            range.setHigh(90.0);
            CHECK_EQ(range.high(), 90.0);

            CHECK_THROWS_AS(range.setLow(200.0), std::invalid_argument);
            CHECK_THROWS_AS(range.setHigh(-10.0), std::invalid_argument);
        }

        TEST_CASE("DataChannelList - add, lookup, clear")
        {
            transport::datachannel::DataChannelList dataChannelList;
            CHECK_EQ(dataChannelList.size(), 0u);

            auto singleList = createValidDataChannelList();
            transport::datachannel::DataChannel channelCopy = singleList.package().dataChannelList()[0];
            dataChannelList.add(channelCopy);
            CHECK_EQ(dataChannelList.size(), 1u);

            const auto& localId = channelCopy.dataChannelId().localId();
            auto found = dataChannelList.from(localId);
            CHECK(found.has_value());

            dataChannelList.clear();
            CHECK_EQ(dataChannelList.size(), 0u);
        }

        TEST_CASE("Error handling - invalid DataChannelType")
        {
            CHECK_THROWS_AS((transport::datachannel::DataChannelType{ "InvalidType" }), std::invalid_argument);
        }

        TEST_CASE("Error handling - invalid Format type")
        {
            CHECK_THROWS_AS((transport::datachannel::Format{ "InvalidFormat" }), std::invalid_argument);
        }

        TEST_CASE("Error handling - duplicate LocalId")
        {
            transport::datachannel::DataChannelList dataChannelList;
            auto validList = createValidDataChannelList();
            transport::datachannel::DataChannel first = validList.package().dataChannelList()[0];
            dataChannelList.add(first);

            transport::datachannel::DataChannel duplicate = validList.package().dataChannelList()[0];
            CHECK_THROWS_AS(dataChannelList.add(duplicate), std::invalid_argument);
        }

        TEST_CASE("Format - constructor stores the canonical trimmed type, not the raw input")
        {
            transport::datachannel::Format format{ " Decimal" };

            CHECK_EQ(format.type(), "Decimal");
        }

        TEST_CASE("DataChannelType - constructor stores the canonical trimmed type, not the raw input")
        {
            transport::datachannel::DataChannelType dataChannelType{ " Alert" };

            CHECK_EQ(dataChannelType.type(), "Alert");
        }

        TEST_CASE("Format::setType rejects an invalid type, matching the constructor's validation")
        {
            transport::datachannel::Format format{ "Decimal" };

            CHECK_THROWS_AS(format.setType("NotARealFormatType"), std::invalid_argument);
        }

        TEST_CASE("DataChannelType::setType rejects an invalid type, matching the constructor's validation")
        {
            transport::datachannel::DataChannelType dataChannelType{ "Inst" };

            CHECK_THROWS_AS(dataChannelType.setType("NotARealDataChannelType"), std::invalid_argument);
        }
    }
} // namespace dnv::vista::sdk::tests
