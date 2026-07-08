#include <doctest/doctest.h>

#include <SDK/core/dto/CodebooksDto.h>
#include <SDK/core/dto/GmodDto.h>
#include <SDK/core/dto/GmodVersioningDto.h>
#include <SDK/core/dto/ISO19848Dtos.h>
#include <SDK/core/dto/LocationsDto.h>

namespace dnv::vista::sdk::tests
{
    TEST_SUITE("Dto::Codebooks")
    {
        TEST_CASE("codebooksDtoFromJson parses valid document")
        {
            auto doc = json::Document::fromString(R"({
                "visRelease": "3-4a",
                "items": [
                    {
                        "name": "calculations",
                        "values": {
                            "group1": ["val1", "val2"],
                            "group2": ["val3"]
                        }
                    }
                ]
            })");
            REQUIRE(doc.has_value());

            auto dto = codebooksDtoFromJson(*doc);

            CHECK_EQ(dto.visVersion, "3-4a");
            REQUIRE_EQ(dto.items.size(), 1u);
            CHECK_EQ(dto.items[0].name, "calculations");
            CHECK_EQ(dto.items[0].values.at("group1").size(), 2u);
            CHECK_EQ(dto.items[0].values.at("group1")[0], "val1");
            CHECK_EQ(dto.items[0].values.at("group2").size(), 1u);
        }

        TEST_CASE("codebooksDtoFromJson throws on missing visRelease")
        {
            auto doc = json::Document::fromString(R"({"items":[]})");
            REQUIRE(doc.has_value());
            CHECK_THROWS_AS(codebooksDtoFromJson(*doc), std::runtime_error);
        }

        TEST_CASE("codebooksDtoFromJson throws on missing items")
        {
            auto doc = json::Document::fromString(R"({"visRelease":"3-4a"})");
            REQUIRE(doc.has_value());
            CHECK_THROWS_AS(codebooksDtoFromJson(*doc), std::runtime_error);
        }

        TEST_CASE("codebookDtoFromJson throws on missing name")
        {
            auto doc = json::Document::fromString(R"({"values":{}})");
            REQUIRE(doc.has_value());
            CHECK_THROWS_AS(codebookDtoFromJson(*doc), std::runtime_error);
        }

        TEST_CASE("codebookDtoFromJson throws on missing values")
        {
            auto doc = json::Document::fromString(R"({"name":"calculations"})");
            REQUIRE(doc.has_value());
            CHECK_THROWS_AS(codebookDtoFromJson(*doc), std::runtime_error);
        }

        TEST_CASE("codebooksDtoFromJson ignores schemaVersion and buildId")
        {
            auto doc = json::Document::fromString(R"({
                "schemaVersion": "1",
                "buildId": "abc",
                "visRelease": "3-4a",
                "items": []
            })");
            REQUIRE(doc.has_value());
            auto dto = codebooksDtoFromJson(*doc);
            CHECK_EQ(dto.visVersion, "3-4a");
            CHECK(dto.items.empty());
        }
    }

    TEST_SUITE("Dto::Gmod")
    {
        TEST_CASE("gmodDtoFromJson parses valid document")
        {
            auto doc = json::Document::fromString(R"({
                "visRelease": "3-4a",
                "items": [
                    {
                        "category": "PRODUCT",
                        "type": "SELECTION",
                        "code": "VE",
                        "name": "Vessel",
                        "commonName": "Ship"
                    },
                    {
                        "category": "ASSET",
                        "type": "TYPE",
                        "code": "400",
                        "installSubstructure": true,
                        "normalAssignmentNames": {"1": "propeller"}
                    }
                ],
                "relations": [["VE", "400"]]
            })");
            REQUIRE(doc.has_value());

            auto dto = gmodDtoFromJson(*doc);

            CHECK_EQ(dto.visVersion, "3-4a");
            REQUIRE_EQ(dto.items.size(), 2u);

            const auto& node0 = dto.items[0];
            CHECK_EQ(node0.category, "PRODUCT");
            CHECK_EQ(node0.type, "SELECTION");
            CHECK_EQ(node0.code, "VE");
            REQUIRE(node0.name.has_value());
            CHECK_EQ(*node0.name, "Vessel");
            REQUIRE(node0.commonName.has_value());
            CHECK_EQ(*node0.commonName, "Ship");
            CHECK_FALSE(node0.definition.has_value());
            CHECK_FALSE(node0.installSubstructure.has_value());
            CHECK_FALSE(node0.normalAssignmentNames.has_value());

            const auto& node1 = dto.items[1];
            REQUIRE(node1.installSubstructure.has_value());
            CHECK(*node1.installSubstructure);
            REQUIRE(node1.normalAssignmentNames.has_value());
            CHECK_EQ(node1.normalAssignmentNames->at("1"), "propeller");

            REQUIRE_EQ(dto.relations.size(), 1u);
            CHECK_EQ(dto.relations[0][0], "VE");
            CHECK_EQ(dto.relations[0][1], "400");
        }

        TEST_CASE("gmodDtoFromJson throws on missing visRelease")
        {
            auto doc = json::Document::fromString(R"({"items":[],"relations":[]})");
            REQUIRE(doc.has_value());
            CHECK_THROWS_AS(gmodDtoFromJson(*doc), std::runtime_error);
        }

        TEST_CASE("gmodDtoFromJson throws on missing relations")
        {
            auto doc = json::Document::fromString(R"({"visRelease":"3-4a","items":[]})");
            REQUIRE(doc.has_value());
            CHECK_THROWS_AS(gmodDtoFromJson(*doc), std::runtime_error);
        }

        TEST_CASE("gmodNodeDtoFromJson throws on missing category")
        {
            auto doc = json::Document::fromString(R"({"type":"SELECTION","code":"VE"})");
            REQUIRE(doc.has_value());
            CHECK_THROWS_AS(gmodNodeDtoFromJson(*doc), std::runtime_error);
        }

        TEST_CASE("gmodNodeDtoFromJson throws on missing code")
        {
            auto doc = json::Document::fromString(R"({"category":"PRODUCT","type":"SELECTION"})");
            REQUIRE(doc.has_value());
            CHECK_THROWS_AS(gmodNodeDtoFromJson(*doc), std::runtime_error);
        }
    }

    TEST_SUITE("Dto::GmodVersioning")
    {
        TEST_CASE("gmodVersioningDtoFromJson parses valid document")
        {
            auto doc = json::Document::fromString(R"({
                "visRelease": "3-4a",
                "items": {
                    "VE": {
                        "operations": ["changeCode"],
                        "source": "VE",
                        "target": "VE2"
                    },
                    "400": {
                        "operations": ["assignmentChanged"],
                        "source": "400",
                        "oldAssignment": "propeller",
                        "newAssignment": "screw"
                    }
                }
            })");
            REQUIRE(doc.has_value());

            auto dto = gmodVersioningDtoFromJson(*doc);

            CHECK_EQ(dto.visVersion, "3-4a");
            REQUIRE_EQ(dto.items.size(), 2u);

            const auto& ve = dto.items.at("VE");
            CHECK(ve.operations.count("changeCode") == 1u);
            CHECK_EQ(ve.source, "VE");
            REQUIRE(ve.target.has_value());
            CHECK_EQ(*ve.target, "VE2");
            CHECK_FALSE(ve.oldAssignment.has_value());
            CHECK_FALSE(ve.deleteAssignment);

            const auto& n400 = dto.items.at("400");
            CHECK(n400.operations.count("assignmentChanged") == 1u);
            REQUIRE(n400.oldAssignment.has_value());
            CHECK_EQ(*n400.oldAssignment, "propeller");
            REQUIRE(n400.newAssignment.has_value());
            CHECK_EQ(*n400.newAssignment, "screw");
            CHECK_FALSE(n400.target.has_value());
        }

        TEST_CASE("gmodVersioningDtoFromJson throws on missing visRelease")
        {
            auto doc = json::Document::fromString(R"({"items":{}})");
            REQUIRE(doc.has_value());
            CHECK_THROWS_AS(gmodVersioningDtoFromJson(*doc), std::runtime_error);
        }

        TEST_CASE("gmodNodeConversionDtoFromJson throws on missing operations")
        {
            auto doc = json::Document::fromString(R"({"source":"VE"})");
            REQUIRE(doc.has_value());
            CHECK_THROWS_AS(gmodNodeConversionDtoFromJson(*doc), std::runtime_error);
        }

        TEST_CASE("gmodNodeConversionDtoFromJson throws on missing source")
        {
            auto doc = json::Document::fromString(R"({"operations":["changeCode"]})");
            REQUIRE(doc.has_value());
            CHECK_THROWS_AS(gmodNodeConversionDtoFromJson(*doc), std::runtime_error);
        }

        TEST_CASE("gmodVersioningAssignmentChangeDtoFromJson parses valid document")
        {
            auto doc = json::Document::fromString(R"({
                "oldAssignment": "propeller",
                "currentAssignment": "screw"
            })");
            REQUIRE(doc.has_value());

            auto dto = gmodVersioningAssignmentChangeDtoFromJson(*doc);

            CHECK_EQ(dto.oldAssignment, "propeller");
            CHECK_EQ(dto.currentAssignment, "screw");
        }

        TEST_CASE("gmodVersioningAssignmentChangeDtoFromJson throws on missing oldAssignment")
        {
            auto doc = json::Document::fromString(R"({"currentAssignment":"screw"})");
            REQUIRE(doc.has_value());
            CHECK_THROWS_AS(gmodVersioningAssignmentChangeDtoFromJson(*doc), std::runtime_error);
        }
    }

    TEST_SUITE("Dto::ISO19848")
    {
        TEST_CASE("dataChannelTypeNamesDtoFromJson parses valid document")
        {
            auto doc = json::Document::fromString(R"({
                "values": [
                    {"type": "Inst", "description": "Instantaneous"},
                    {"type": "Avg",  "description": "Average"}
                ]
            })");
            REQUIRE(doc.has_value());

            auto dto = dataChannelTypeNamesDtoFromJson(*doc);

            REQUIRE_EQ(dto.values.size(), 2u);
            CHECK_EQ(dto.values[0].type, "Inst");
            CHECK_EQ(dto.values[0].description, "Instantaneous");
            CHECK_EQ(dto.values[1].type, "Avg");
        }

        TEST_CASE("dataChannelTypeNamesDtoFromJson throws on missing values")
        {
            auto doc = json::Document::fromString(R"({})");
            REQUIRE(doc.has_value());
            CHECK_THROWS_AS(dataChannelTypeNamesDtoFromJson(*doc), std::runtime_error);
        }

        TEST_CASE("formatDataTypesDtoFromJson parses valid document")
        {
            auto doc = json::Document::fromString(R"({
                "values": [
                    {"type": "Boolean", "description": "True or false"},
                    {"type": "String",  "description": "UTF-8 text"}
                ]
            })");
            REQUIRE(doc.has_value());

            auto dto = formatDataTypesDtoFromJson(*doc);

            REQUIRE_EQ(dto.values.size(), 2u);
            CHECK_EQ(dto.values[0].type, "Boolean");
            CHECK_EQ(dto.values[1].type, "String");
        }

        TEST_CASE("formatDataTypesDtoFromJson throws on missing values")
        {
            auto doc = json::Document::fromString(R"({})");
            REQUIRE(doc.has_value());
            CHECK_THROWS_AS(formatDataTypesDtoFromJson(*doc), std::runtime_error);
        }

        TEST_CASE("dataChannelTypeNameDtoFromJson throws on missing type")
        {
            auto doc = json::Document::fromString(R"({"description":"Instantaneous"})");
            REQUIRE(doc.has_value());
            CHECK_THROWS_AS(dataChannelTypeNameDtoFromJson(*doc), std::runtime_error);
        }

        TEST_CASE("dataChannelTypeNameDtoFromJson throws on missing description")
        {
            auto doc = json::Document::fromString(R"({"type":"Inst"})");
            REQUIRE(doc.has_value());
            CHECK_THROWS_AS(dataChannelTypeNameDtoFromJson(*doc), std::runtime_error);
        }
    }

    TEST_SUITE("Dto::Locations")
    {
        TEST_CASE("locationsDtoFromJson parses valid document")
        {
            auto doc = json::Document::fromString(R"({
                "visRelease": "3-4a",
                "items": [
                    {"code": "N", "name": "Number"},
                    {"code": "C", "name": "Centre", "definition": "The centre position"}
                ]
            })");
            REQUIRE(doc.has_value());

            auto dto = locationsDtoFromJson(*doc);

            CHECK_EQ(dto.visVersion, "3-4a");
            REQUIRE_EQ(dto.items.size(), 2u);
            CHECK_EQ(dto.items[0].code, 'N');
            CHECK_EQ(dto.items[0].name, "Number");
            CHECK_FALSE(dto.items[0].definition.has_value());
            CHECK_EQ(dto.items[1].code, 'C');
            CHECK_EQ(dto.items[1].name, "Centre");
            REQUIRE(dto.items[1].definition.has_value());
            CHECK_EQ(*dto.items[1].definition, "The centre position");
        }

        TEST_CASE("locationsDtoFromJson throws on missing visRelease")
        {
            auto doc = json::Document::fromString(R"({"items":[]})");
            REQUIRE(doc.has_value());
            CHECK_THROWS_AS(locationsDtoFromJson(*doc), std::runtime_error);
        }

        TEST_CASE("locationsDtoFromJson throws on missing items")
        {
            auto doc = json::Document::fromString(R"({"visRelease":"3-4a"})");
            REQUIRE(doc.has_value());
            CHECK_THROWS_AS(locationsDtoFromJson(*doc), std::runtime_error);
        }

        TEST_CASE("relativeLocationDtoFromJson throws on missing code")
        {
            auto doc = json::Document::fromString(R"({"name":"Number"})");
            REQUIRE(doc.has_value());
            CHECK_THROWS_AS(relativeLocationDtoFromJson(*doc), std::runtime_error);
        }

        TEST_CASE("relativeLocationDtoFromJson throws on multi-character code")
        {
            auto doc = json::Document::fromString(R"({"code":"NC","name":"Number"})");
            REQUIRE(doc.has_value());
            CHECK_THROWS_AS(relativeLocationDtoFromJson(*doc), std::runtime_error);
        }

        TEST_CASE("relativeLocationDtoFromJson throws on empty code")
        {
            auto doc = json::Document::fromString(R"({"code":"","name":"Number"})");
            REQUIRE(doc.has_value());
            CHECK_THROWS_AS(relativeLocationDtoFromJson(*doc), std::runtime_error);
        }

        TEST_CASE("relativeLocationDtoFromJson throws on missing name")
        {
            auto doc = json::Document::fromString(R"({"code":"N"})");
            REQUIRE(doc.has_value());
            CHECK_THROWS_AS(relativeLocationDtoFromJson(*doc), std::runtime_error);
        }
    }
} // namespace dnv::vista::sdk::tests
