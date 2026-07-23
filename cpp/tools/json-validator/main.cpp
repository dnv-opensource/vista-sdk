/**
 * @file main.cpp
 * @brief CLI tool for validating ISO 19848 JSON files against schemas
 * @details Command-line interface to validate DataChannelList and TimeSeriesData JSON files,
 *          and to cross-validate a TimeSeriesData file against its DataChannelList
 *
 * Usage:
 *   json-validator-cli datachannel <file.json>     # Validate DataChannelList
 *   json-validator-cli timeseries <file.json>      # Validate TimeSeriesData
 *   json-validator-cli cross <dcl.json> <tsd.json> # Cross-validate both
 *   json-validator-cli --help                      # Show help
 */

#include "SchemaValidator.h"

#include <SchemasRegistry.h>

#include <dnv/VistaSDK.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

using dnv::vista::sdk::json::Document;
using dnv::vista::sdk::json::SchemaValidator;

namespace jdc = dnv::vista::sdk::transport::serialization::json::datachannel;
namespace jts = dnv::vista::sdk::transport::serialization::json::timeseries;

// Forward declarations
void printHelp();
bool validateJson(const std::filesystem::path& jsonFile, std::string_view schemaName);
bool validateCross(const std::filesystem::path& dclFile, const std::filesystem::path& tsdFile);
std::optional<std::string> readFile(const std::filesystem::path& file);

std::optional<dnv::vista::sdk::transport::datachannel::DataChannelListPackage> buildDataChannelListBestEffort(
    const jdc::DataChannelListPackageDto& dto, std::vector<std::string>& skippedErrors);

std::optional<dnv::vista::sdk::transport::timeseries::TimeSeriesDataPackage> buildTimeSeriesDataBestEffort(
    const jts::TimeSeriesDataPackageDto& dto, std::vector<std::string>& skippedErrors);

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printHelp();
        return 1;
    }

    std::string_view command = argv[1];

    if (command == "--help" || command == "-h")
    {
        printHelp();
        return 0;
    }

    if (command == "datachannel" || command == "timeseries")
    {
        if (argc < 3)
        {
            std::cerr << "Error: Missing JSON file argument\n\n";
            printHelp();
            return 1;
        }

        std::filesystem::path jsonFile = argv[2];

        if (!std::filesystem::exists(jsonFile))
        {
            std::cerr << "Error: File not found: " << jsonFile << "\n";
            return 1;
        }

        std::string_view schemaName =
            (command == "datachannel") ? "DataChannelList.schema.json" : "TimeSeriesData.schema.json";

        bool valid = validateJson(jsonFile, schemaName);
        return valid ? 0 : 1;
    }

    if (command == "cross")
    {
        if (argc < 4)
        {
            std::cerr << "Error: Missing DataChannelList and/or TimeSeriesData file argument\n\n";
            printHelp();
            return 1;
        }

        std::filesystem::path dclFile = argv[2];
        std::filesystem::path tsdFile = argv[3];

        if (!std::filesystem::exists(dclFile))
        {
            std::cerr << "Error: File not found: " << dclFile << "\n";
            return 1;
        }
        if (!std::filesystem::exists(tsdFile))
        {
            std::cerr << "Error: File not found: " << tsdFile << "\n";
            return 1;
        }

        bool valid = validateCross(dclFile, tsdFile);
        return valid ? 0 : 1;
    }

    std::cerr << "Error: Unknown command '" << command << "'\n\n";
    printHelp();
    return 1;
}

void printHelp()
{
    std::cout << "Vista SDK Validator - JSON Schema Validation Tool\n";
    std::cout << "Vista SDK version: " << dnv::vista::sdk::version() << "\n";
    std::cout << R"(
USAGE:
    json-validator-cli <command> <file.json>

COMMANDS:
    datachannel <file.json>            Validate DataChannelList JSON file
    timeseries <file.json>             Validate TimeSeriesData JSON file
    cross <dcl.json> <tsd.json>        Cross-validate TimeSeriesData against DataChannelList
    --help, -h                         Show this help message

EXAMPLES:
    json-validator-cli datachannel DataChannelList.sample.json
    json-validator-cli timeseries TimeSeriesData.sample.json
    json-validator-cli cross DataChannelList.json TimeSeriesData.json

DESCRIPTION:
    Validates ISO 19848 JSON files against their official JSON schemas.

    The 'cross' command always runs every validation step to completion and
    reports the full list of errors found - it never stops at the first
    error. Schema validation, DataChannel construction, TimeSeriesData
    construction, and the semantic cross-check all run unconditionally;
    individual malformed items (a single bad DataChannel, a single bad
    TabularData block) are logged and excluded, while everything else that
    can be built and validated still is.

EXIT CODES:
    0    Validation succeeded
    1    Validation failed or error occurred
)";
}

std::optional<std::string> readFile(const std::filesystem::path& file)
{
    std::ifstream stream(file);
    if (!stream)
    {
        return std::nullopt;
    }

    return std::string{ std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>() };
}

std::optional<dnv::vista::sdk::transport::datachannel::DataChannelListPackage> buildDataChannelListBestEffort(
    const jdc::DataChannelListPackageDto& dto, std::vector<std::string>& skippedErrors)
{
    namespace domain = dnv::vista::sdk::transport::datachannel;
    using dnv::vista::sdk::DateTimeOffset;
    using dnv::vista::sdk::LocalId;
    using dnv::vista::sdk::transport::ShipId;

    const auto& p = dto.package;
    const auto& h = p.header;

    domain::ConfigurationReference confRef{ h.dataChannelListId.id,
                                            DateTimeOffset{ h.dataChannelListId.timeStamp },
                                            h.dataChannelListId.version };

    std::optional<domain::VersionInformation> vi;
    if (h.versionInformation)
    {
        vi = domain::VersionInformation{ h.versionInformation->namingRule,
                                         h.versionInformation->namingSchemeVersion,
                                         h.versionInformation->referenceUrl };
    }

    std::optional<DateTimeOffset> dateCreated;
    if (h.dateCreated)
    {
        dateCreated = DateTimeOffset{ *h.dateCreated };
    }

    auto shipIdResult = ShipId::fromString(h.shipId);
    if (!shipIdResult)
    {
        skippedErrors.push_back("Header: invalid ShipID '" + h.shipId + "' - cannot build DataChannelList at all");
        return std::nullopt;
    }

    domain::Header header{ std::move(*shipIdResult), confRef, vi, h.author, dateCreated, h.customHeaders };
    domain::DataChannelList list;

    for (size_t i = 0; i < p.dataChannelList.dataChannels.size(); ++i)
    {
        const auto& dc = p.dataChannelList.dataChannels[i];
        try
        {
            auto localIdResult = LocalId::fromString(dc.dataChannelId.localId);
            if (!localIdResult)
            {
                throw std::invalid_argument{ "Invalid LocalID: " + dc.dataChannelId.localId };
            }

            std::optional<domain::NameObject> nameObject;
            if (dc.dataChannelId.nameObject)
            {
                nameObject = domain::NameObject{ dc.dataChannelId.nameObject->namingRule,
                                                 dc.dataChannelId.nameObject->customNameObjects };
            }

            domain::DataChannelId dataChannelId{ *localIdResult, dc.dataChannelId.shortId, nameObject };

            domain::DataChannelType dct{ dc.property.dataChannelType.type,
                                         dc.property.dataChannelType.updateCycle,
                                         dc.property.dataChannelType.calculationPeriod };

            std::optional<domain::Restriction> restriction;
            if (dc.property.format.restriction)
            {
                const auto& r = *dc.property.format.restriction;
                domain::Restriction dr;
                dr.setEnumeration(r.enumeration);
                dr.setFractionDigits(r.fractionDigits);
                dr.setLength(r.length);
                dr.setMaxExclusive(r.maxExclusive);
                dr.setMaxInclusive(r.maxInclusive);
                dr.setMaxLength(r.maxLength);
                dr.setMinExclusive(r.minExclusive);
                dr.setMinInclusive(r.minInclusive);
                dr.setMinLength(r.minLength);
                dr.setPattern(r.pattern);
                dr.setTotalDigits(r.totalDigits);
                restriction = std::move(dr);
            }

            domain::Format format{ dc.property.format.type, restriction };

            std::optional<domain::Range> range;
            if (dc.property.range)
            {
                range = domain::Range{ dc.property.range->low, dc.property.range->high };
            }

            std::optional<domain::Unit> unit;
            if (dc.property.unit)
            {
                unit = domain::Unit{ dc.property.unit->unitSymbol,
                                     dc.property.unit->quantityName,
                                     dc.property.unit->customElements };
            }

            domain::Property property{ dct,
                                       format,
                                       range,
                                       unit,
                                       dc.property.qualityCoding,
                                       dc.property.alertPriority,
                                       dc.property.name,
                                       dc.property.remarks,
                                       dc.property.customProperties };

            list.add(domain::DataChannel{ dataChannelId, property });
        }
        catch (const std::exception& ex)
        {
            std::string label = dc.dataChannelId.shortId.value_or(dc.dataChannelId.localId);
            skippedErrors.push_back("DataChannel[" + std::to_string(i) + "] (" + label + "): " + ex.what());
        }
    }

    domain::Package package{ header, std::move(list) };
    return domain::DataChannelListPackage{ std::move(package) };
}

std::optional<dnv::vista::sdk::transport::timeseries::TimeSeriesDataPackage> buildTimeSeriesDataBestEffort(
    const jts::TimeSeriesDataPackageDto& dto, std::vector<std::string>& skippedErrors)
{
    namespace domain = dnv::vista::sdk::transport::timeseries;
    using dnv::vista::sdk::DateTimeOffset;
    using dnv::vista::sdk::transport::ShipId;

    const auto& p = dto.package;

    std::optional<domain::Header> header;
    if (p.header)
    {
        const auto& h = *p.header;

        std::optional<domain::TimeSpan> timeSpan;
        if (h.timeSpan)
        {
            timeSpan = domain::TimeSpan{ DateTimeOffset{ h.timeSpan->start }, DateTimeOffset{ h.timeSpan->end } };
        }

        std::optional<DateTimeOffset> dateCreated;
        if (h.dateCreated)
        {
            dateCreated = DateTimeOffset{ *h.dateCreated };
        }
        std::optional<DateTimeOffset> dateModified;
        if (h.dateModified)
        {
            dateModified = DateTimeOffset{ *h.dateModified };
        }
        std::optional<std::vector<domain::ConfigurationReference>> sysConf;
        if (h.systemConfiguration)
        {
            std::vector<domain::ConfigurationReference> refs;
            for (const auto& c : *h.systemConfiguration)
            {
                refs.emplace_back(c.id, DateTimeOffset{ c.timeStamp });
            }
            sysConf = std::move(refs);
        }

        auto shipIdResult = ShipId::fromString(h.shipId);
        if (!shipIdResult)
        {
            skippedErrors.push_back("Header: invalid ShipID '" + h.shipId + "' - cannot build TimeSeriesData at all");
            return std::nullopt;
        }

        header = domain::Header{ std::move(*shipIdResult), timeSpan, dateCreated, dateModified, h.author, sysConf,
                                 h.customHeaders };
    }

    std::vector<domain::TimeSeriesData> tsList;

    for (size_t tsIndex = 0; tsIndex < p.timeSeriesData.size(); ++tsIndex)
    {
        const auto& ts = p.timeSeriesData[tsIndex];

        std::optional<domain::ConfigurationReference> dataConfig;
        if (ts.dataConfiguration)
        {
            dataConfig = domain::ConfigurationReference{ ts.dataConfiguration->id,
                                                         DateTimeOffset{ ts.dataConfiguration->timeStamp } };
        }

        std::optional<std::vector<domain::TabularData>> tabularData;
        if (ts.tabularData)
        {
            std::vector<domain::TabularData> tabList;
            for (size_t tdIndex = 0; tdIndex < ts.tabularData->size(); ++tdIndex)
            {
                const auto& td = (*ts.tabularData)[tdIndex];
                try
                {
                    std::vector<domain::DataChannelId> ids;
                    if (td.dataChannelIds)
                    {
                        for (const auto& s : *td.dataChannelIds)
                        {
                            auto idResult = domain::DataChannelId::fromString(s);
                            if (!idResult)
                            {
                                throw std::invalid_argument{ "Invalid DataChannelID: " + s };
                            }
                            ids.push_back(std::move(*idResult));
                        }
                    }

                    if (td.numberOfDataChannel && *td.numberOfDataChannel != ids.size())
                    {
                        throw std::invalid_argument{ "NumberOfDataChannel (" + std::to_string(*td.numberOfDataChannel) +
                                                     ") does not match actual DataChannelID count (" +
                                                     std::to_string(ids.size()) + ")" };
                    }

                    std::vector<domain::TabularDataSet> sets;
                    if (td.dataSets)
                    {
                        for (const auto& ds : *td.dataSets)
                        {
                            sets.emplace_back(DateTimeOffset{ ds.timeStamp }, ds.value, ds.quality);
                        }
                    }

                    if (td.numberOfDataSet && *td.numberOfDataSet != sets.size())
                    {
                        throw std::invalid_argument{ "NumberOfDataSet (" + std::to_string(*td.numberOfDataSet) +
                                                     ") does not match actual DataSet count (" +
                                                     std::to_string(sets.size()) + ")" };
                    }

                    tabList.emplace_back(std::move(ids), std::move(sets));
                }
                catch (const std::exception& ex)
                {
                    skippedErrors.push_back(
                        "TimeSeriesData[" + std::to_string(tsIndex) + "]/TabularData[" + std::to_string(tdIndex) +
                        "]: " + ex.what());
                }
            }
            tabularData = std::move(tabList);
        }

        std::optional<domain::EventData> eventData;
        if (ts.eventData)
        {
            std::optional<std::vector<domain::EventDataSet>> sets;
            if (ts.eventData->dataSet)
            {
                std::vector<domain::EventDataSet> evSets;
                for (size_t edIndex = 0; edIndex < ts.eventData->dataSet->size(); ++edIndex)
                {
                    const auto& ed = (*ts.eventData->dataSet)[edIndex];
                    try
                    {
                        auto idResult = domain::DataChannelId::fromString(ed.dataChannelId);
                        if (!idResult)
                        {
                            throw std::invalid_argument{ "Invalid DataChannelID: " + ed.dataChannelId };
                        }
                        evSets.emplace_back(DateTimeOffset{ ed.timeStamp }, std::move(*idResult), ed.value, ed.quality);
                    }
                    catch (const std::exception& ex)
                    {
                        skippedErrors.push_back(
                            "TimeSeriesData[" + std::to_string(tsIndex) + "]/EventData[" + std::to_string(edIndex) +
                            "]: " + ex.what());
                    }
                }
                sets = std::move(evSets);
            }
            eventData = domain::EventData{ std::move(sets) };
        }

        tsList.emplace_back(dataConfig, tabularData, eventData, ts.customDataKinds);
    }

    domain::Package package{ header, std::move(tsList) };
    return domain::TimeSeriesDataPackage{ std::move(package) };
}

bool validateCross(const std::filesystem::path& dclFile, const std::filesystem::path& tsdFile)
{
    std::cout << "DataChannelList: " << dclFile.filename() << "\n";
    std::cout << "TimeSeriesData : " << tsdFile.filename() << "\n\n";

    bool allValid = true;

    std::cout << "--- Schema validation (DataChannelList) ---\n";
    if (!validateJson(dclFile, "DataChannelList.schema.json"))
    {
        allValid = false;
    }
    std::cout << "\n";

    std::cout << "--- Schema validation (TimeSeriesData) ---\n";
    if (!validateJson(tsdFile, "TimeSeriesData.schema.json"))
    {
        allValid = false;
    }
    std::cout << "\n";

    std::cout << "--- Cross-check ---\n";

    auto dclContent = readFile(dclFile);
    if (!dclContent)
    {
        std::cerr << "[ERROR] Failed to read: " << dclFile.filename() << "\n";
        return false;
    }
    auto tsdContent = readFile(tsdFile);
    if (!tsdContent)
    {
        std::cerr << "[ERROR] Failed to read: " << tsdFile.filename() << "\n";
        return false;
    }

    auto dclDtoOpt = jdc::fromJsonString(*dclContent);
    if (!dclDtoOpt.has_value())
    {
        std::cerr << "[ERROR] Failed to parse DataChannelList JSON: " << dclFile.filename() << "\n";
        std::cerr << "        (see schema errors above for details)\n";
        return false;
    }

    auto tsdDtoOpt = jts::fromJsonString(*tsdContent);
    if (!tsdDtoOpt.has_value())
    {
        std::cerr << "[ERROR] Failed to parse TimeSeriesData JSON: " << tsdFile.filename() << "\n";
        std::cerr << "        (see schema errors above for details)\n";
        return false;
    }

    std::vector<std::string> skippedChannels;
    auto dclPackageOpt = buildDataChannelListBestEffort(*dclDtoOpt, skippedChannels);
    if (!dclPackageOpt.has_value())
    {
        std::cerr << "[ERROR] Cannot build DataChannelList: " << dclFile.filename() << "\n";
        for (const auto& err : skippedChannels)
        {
            std::cerr << "    - " << err << "\n";
        }
        return false;
    }

    if (!skippedChannels.empty())
    {
        allValid = false;
        std::cerr << "[FAIL] " << skippedChannels.size()
                  << " DataChannel(s) could not be built and were excluded from the cross-check:\n";
        for (const auto& err : skippedChannels)
        {
            std::cerr << "    - " << err << "\n";
        }
        std::cerr << "\n";
    }

    std::vector<std::string> skippedBlocks;
    auto tsdPackageOpt = buildTimeSeriesDataBestEffort(*tsdDtoOpt, skippedBlocks);
    if (!tsdPackageOpt.has_value())
    {
        std::cerr << "[ERROR] Cannot build TimeSeriesData: " << tsdFile.filename() << "\n";
        for (const auto& err : skippedBlocks)
        {
            std::cerr << "    - " << err << "\n";
        }
        return false;
    }

    if (!skippedBlocks.empty())
    {
        allValid = false;
        std::cerr << "[FAIL] " << skippedBlocks.size()
                  << " TimeSeriesData block(s) could not be built and were excluded from the cross-check:\n";
        for (const auto& err : skippedBlocks)
        {
            std::cerr << "    - " << err << "\n";
        }
        std::cerr << "\n";
    }

    const auto& dclPackage = dclPackageOpt.value();
    const auto& tsdPackage = tsdPackageOpt.value();

    std::cout << "DataChannelList channels: " << dclPackage.dataChannelList().size() << " built OK, "
              << skippedChannels.size() << " skipped\n\n";

    auto onData = [](const dnv::vista::sdk::DateTimeOffset&,
                     const dnv::vista::sdk::transport::datachannel::DataChannel&,
                     const dnv::vista::sdk::transport::Value&,
                     const std::optional<std::string>&) { return dnv::vista::sdk::transport::ValidateResult<>::ok(); };

    size_t index = 0;

    for (const auto& timeSeriesData : tsdPackage.package().timeSeriesData())
    {
        ++index;
        auto result = timeSeriesData.validate(dclPackage, onData, onData);

        if (result)
        {
            std::cout << "[OK] TimeSeriesData[" << index << "]: cross-validation succeeded\n";
        }
        else
        {
            allValid = false;
            std::cerr << "[FAIL] TimeSeriesData[" << index << "]: " << result.errors().size() << " error(s):\n";
            for (const auto& error : result.errors())
            {
                std::cerr << "    - " << error << "\n";
            }
        }
    }

    return allValid;
}

bool validateJson(const std::filesystem::path& jsonFile, std::string_view schemaName)
{
    std::cout << "Validating: " << jsonFile.filename() << "\n";
    std::cout << "Schema: " << schemaName << "\n\n";

    const auto* schemaBlob = dnv::vista::sdk::schemas::find(schemaName);
    if (!schemaBlob)
    {
        std::cerr << "Error: Schema not found: " << schemaName << "\n";
        return false;
    }

    auto schemaDocOpt = Document::fromString(schemaBlob->str());
    if (!schemaDocOpt.has_value())
    {
        std::cerr << "Error: Failed to parse embedded schema\n";
        return false;
    }

    SchemaValidator validator;
    if (!validator.load(*schemaDocOpt))
    {
        std::cerr << "Error: Failed to load schema into validator\n";
        return false;
    }

    std::ifstream file(jsonFile);
    if (!file)
    {
        std::cerr << "Error: Failed to open file: " << jsonFile << "\n";
        return false;
    }

    std::string jsonContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    auto documentOpt = Document::fromString(jsonContent);
    if (!documentOpt.has_value())
    {
        std::cerr << "[ERROR] JSON Parse Error:\n";
        std::cerr << "        Failed to parse " << jsonFile.filename() << "\n";
        return false;
    }

    auto result = validator.validate(*documentOpt);

    if (result.isValid())
    {
        std::cout << "[OK] Validation succeeded\n";
        std::cout << "     File conforms to " << schemaName << "\n";
        std::cout << "     Schema: " << validator.title() << " (" << validator.draftString() << ")\n";
        return true;
    }
    else
    {
        std::cerr << "[FAIL] Validation Failed (" << result.errorCount() << " errors):\n\n";

        for (size_t i = 0; i < result.errorCount(); ++i)
        {
            const auto& error = result.error(i);
            std::cerr << "  [" << (i + 1) << "] " << error.path() << "\n";
            std::cerr << "      " << error.message() << "\n";

            if (!error.constraint().empty())
            {
                std::cerr << "      Constraint: " << error.constraint() << "\n";
            }
            if (!error.expectedValue().empty())
            {
                std::cerr << "      Expected: " << error.expectedValue() << "\n";
            }
            if (!error.actualValue().empty())
            {
                std::cerr << "      Actual: " << error.actualValue() << "\n";
            }
            std::cerr << "\n";
        }

        return false;
    }
}
