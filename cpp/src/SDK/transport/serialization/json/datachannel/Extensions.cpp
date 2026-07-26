#include "dnv/vista/sdk/transport/serialization/json/datachannel/Extensions.h"

#include "JSON/Json.h"

#include <stdexcept>

namespace dnv::vista::sdk::transport::serialization::json::datachannel
{
    namespace
    {
        using namespace dnv::vista::sdk::json;
        namespace domain = dnv::vista::sdk::transport::datachannel;
        using Ser = serialization::json::SerializableDocument;

        std::string_view whiteSpaceToString(domain::Restriction::WhiteSpace ws) noexcept
        {
            switch (ws)
            {
                case domain::Restriction::WhiteSpace::Preserve:
                    return "Preserve";
                case domain::Restriction::WhiteSpace::Replace:
                    return "Replace";
                case domain::Restriction::WhiteSpace::Collapse:
                    return "Collapse";
            }
            return "Preserve";
        }

        std::optional<domain::Restriction::WhiteSpace> whiteSpaceFromString(std::string_view s) noexcept
        {
            if (s == "Preserve")
            {
                return domain::Restriction::WhiteSpace::Preserve;
            }
            if (s == "Replace")
            {
                return domain::Restriction::WhiteSpace::Replace;
            }
            if (s == "Collapse")
            {
                return domain::Restriction::WhiteSpace::Collapse;
            }
            return std::nullopt;
        }

        double readDouble(const Document& d)
        {
            if (auto v = d.root<double>())
            {
                return *v;
            }
            if (auto v = d.root<std::int64_t>())
            {
                return static_cast<double>(*v);
            }
            if (auto v = d.root<std::uint64_t>())
            {
                return static_cast<double>(*v);
            }
            throw std::invalid_argument{ "Expected numeric value" };
        }

        Ser documentToSerializable(const Document& doc)
        {
            switch (doc.type())
            {
                case Type::Null:
                    return Ser{};
                case Type::Boolean:
                    return Ser{ doc.root<bool>().value() };
                case Type::Integer:
                    return Ser{ doc.root<std::int64_t>().value() };
                case Type::UnsignedInteger:
                    return Ser{ static_cast<std::int64_t>(doc.root<std::uint64_t>().value()) };
                case Type::Double:
                    return Ser{ doc.root<double>().value() };
                case Type::String:
                    return Ser{ doc.root<std::string>().value() };
                case Type::Array:
                {
                    Ser::Array arr;
                    arr.reserve(doc.size());
                    for (std::size_t i = 0; i < doc.size(); ++i)
                    {
                        arr.push_back(documentToSerializable(doc.at(i)));
                    }
                    return Ser{ std::move(arr) };
                }
                case Type::Object:
                {
                    auto obj = Ser::object();
                    for (auto it = doc.objectBegin(); it != doc.objectEnd(); ++it)
                    {
                        obj.set(it.key(), documentToSerializable(it.value()));
                    }
                    return obj;
                }
            }
            return Ser{};
        }

        void serializableToBuilder(Builder& b, const Ser& doc)
        {
            switch (doc.kind())
            {
                case Ser::Kind::Null:
                    b.write(nullptr);
                    break;
                case Ser::Kind::Boolean:
                    b.write(doc.asBoolean());
                    break;
                case Ser::Kind::Integer:
                    b.write(doc.asInteger());
                    break;
                case Ser::Kind::Double:
                    b.write(doc.asDouble());
                    break;
                case Ser::Kind::String:
                    b.write(doc.asString());
                    break;
                case Ser::Kind::Array:
                    b.writeStartArray();
                    for (const auto& elem : doc.asArray())
                    {
                        serializableToBuilder(b, elem);
                    }
                    b.writeEndArray();
                    break;
                case Ser::Kind::Object:
                    b.writeStartObject();
                    for (const auto& [key, val] : doc.asObject())
                    {
                        b.writeKey(key);
                        serializableToBuilder(b, val);
                    }
                    b.writeEndObject();
                    break;
            }
        }

        void writeRestriction(Builder& b, const RestrictionDto& r)
        {
            b.writeStartObject();
            if (r.fractionDigits)
            {
                b.writeTrustedKey("FractionDigits");
                b.write(static_cast<std::int64_t>(*r.fractionDigits));
            }
            if (r.length)
            {
                b.writeTrustedKey("Length");
                b.write(static_cast<std::int64_t>(*r.length));
            }
            if (r.maxExclusive)
            {
                b.writeTrustedKey("MaxExclusive");
                b.write(*r.maxExclusive);
            }
            if (r.maxInclusive)
            {
                b.writeTrustedKey("MaxInclusive");
                b.write(*r.maxInclusive);
            }
            if (r.maxLength)
            {
                b.writeTrustedKey("MaxLength");
                b.write(static_cast<std::int64_t>(*r.maxLength));
            }
            if (r.minExclusive)
            {
                b.writeTrustedKey("MinExclusive");
                b.write(*r.minExclusive);
            }
            if (r.minInclusive)
            {
                b.writeTrustedKey("MinInclusive");
                b.write(*r.minInclusive);
            }
            if (r.minLength)
            {
                b.writeTrustedKey("MinLength");
                b.write(static_cast<std::int64_t>(*r.minLength));
            }
            if (r.pattern)
            {
                b.writeTrustedKey("Pattern");
                b.write(*r.pattern);
            }
            if (r.totalDigits)
            {
                b.writeTrustedKey("TotalDigits");
                b.write(static_cast<std::int64_t>(*r.totalDigits));
            }
            if (r.whiteSpace)
            {
                b.writeTrustedKey("WhiteSpace");
                b.write(*r.whiteSpace);
            }
            if (r.enumeration)
            {
                b.writeTrustedKey("Enumeration");
                b.writeStartArray();
                for (const auto& v : *r.enumeration)
                {
                    b.write(v);
                }
                b.writeEndArray();
            }
            b.writeEndObject();
        }

        void writeDataChannelDto(Builder& b, const DataChannelDto& dc)
        {
            b.writeStartObject();

            // DataChannelID
            b.writeTrustedKey("DataChannelID");
            b.writeStartObject();
            b.writeTrustedKey("LocalID");
            b.write(dc.dataChannelId.localId);
            if (dc.dataChannelId.shortId)
            {
                b.writeTrustedKey("ShortID");
                b.write(*dc.dataChannelId.shortId);
            }
            if (dc.dataChannelId.nameObject)
            {
                b.writeTrustedKey("NameObject");
                b.writeStartObject();
                b.writeTrustedKey("NamingRule");
                b.write(dc.dataChannelId.nameObject->namingRule);
                if (dc.dataChannelId.nameObject->customNameObjects)
                {
                    for (const auto& [key, val] : dc.dataChannelId.nameObject->customNameObjects->asObject())
                    {
                        b.writeKey(key);
                        serializableToBuilder(b, val);
                    }
                }
                b.writeEndObject();
            }
            b.writeEndObject();

            // Property
            const auto& p = dc.property;
            b.writeTrustedKey("Property");
            b.writeStartObject();

            b.writeTrustedKey("DataChannelType");
            b.writeStartObject();
            b.writeTrustedKey("Type");
            b.write(p.dataChannelType.type);
            if (p.dataChannelType.updateCycle)
            {
                b.writeTrustedKey("UpdateCycle");
                b.write(*p.dataChannelType.updateCycle);
            }
            if (p.dataChannelType.calculationPeriod)
            {
                b.writeTrustedKey("CalculationPeriod");
                b.write(*p.dataChannelType.calculationPeriod);
            }
            b.writeEndObject();

            b.writeTrustedKey("Format");
            b.writeStartObject();
            b.writeTrustedKey("Type");
            b.write(p.format.type);
            if (p.format.restriction)
            {
                b.writeTrustedKey("Restriction");
                writeRestriction(b, *p.format.restriction);
            }
            b.writeEndObject();

            if (p.range)
            {
                b.writeTrustedKey("Range");
                b.writeStartObject();
                b.writeTrustedKey("High");
                b.write(p.range->high);
                b.writeTrustedKey("Low");
                b.write(p.range->low);
                b.writeEndObject();
            }
            if (p.unit)
            {
                b.writeTrustedKey("Unit");
                b.writeStartObject();
                b.writeTrustedKey("UnitSymbol");
                b.write(p.unit->unitSymbol);
                if (p.unit->quantityName)
                {
                    b.writeTrustedKey("QuantityName");
                    b.write(*p.unit->quantityName);
                }
                if (p.unit->customElements)
                {
                    for (const auto& [key, val] : p.unit->customElements->asObject())
                    {
                        b.writeKey(key);
                        serializableToBuilder(b, val);
                    }
                }
                b.writeEndObject();
            }
            if (p.qualityCoding)
            {
                b.writeTrustedKey("QualityCoding");
                b.write(*p.qualityCoding);
            }
            if (p.alertPriority)
            {
                b.writeTrustedKey("AlertPriority");
                b.write(*p.alertPriority);
            }
            if (p.name)
            {
                b.writeTrustedKey("Name");
                b.write(*p.name);
            }
            if (p.remarks)
            {
                b.writeTrustedKey("Remarks");
                b.write(*p.remarks);
            }
            if (p.customProperties)
            {
                for (const auto& [key, val] : p.customProperties->asObject())
                {
                    b.writeKey(key);
                    serializableToBuilder(b, val);
                }
            }
            b.writeEndObject(); // Property

            b.writeEndObject(); // DataChannel
        }

        void writeRestrictionDomain(Builder& b, const domain::Restriction& r)
        {
            b.writeStartObject();
            if (r.fractionDigits())
            {
                b.writeTrustedKey("FractionDigits");
                b.write(static_cast<std::int64_t>(*r.fractionDigits()));
            }
            if (r.length())
            {
                b.writeTrustedKey("Length");
                b.write(static_cast<std::int64_t>(*r.length()));
            }
            if (r.maxExclusive())
            {
                b.writeTrustedKey("MaxExclusive");
                b.write(*r.maxExclusive());
            }
            if (r.maxInclusive())
            {
                b.writeTrustedKey("MaxInclusive");
                b.write(*r.maxInclusive());
            }
            if (r.maxLength())
            {
                b.writeTrustedKey("MaxLength");
                b.write(static_cast<std::int64_t>(*r.maxLength()));
            }
            if (r.minExclusive())
            {
                b.writeTrustedKey("MinExclusive");
                b.write(*r.minExclusive());
            }
            if (r.minInclusive())
            {
                b.writeTrustedKey("MinInclusive");
                b.write(*r.minInclusive());
            }
            if (r.minLength())
            {
                b.writeTrustedKey("MinLength");
                b.write(static_cast<std::int64_t>(*r.minLength()));
            }
            if (r.pattern())
            {
                b.writeTrustedKey("Pattern");
                b.write(*r.pattern());
            }
            if (r.totalDigits())
            {
                b.writeTrustedKey("TotalDigits");
                b.write(static_cast<std::int64_t>(*r.totalDigits()));
            }
            if (r.whiteSpace())
            {
                b.writeTrustedKey("WhiteSpace");
                b.write(whiteSpaceToString(*r.whiteSpace()));
            }
            if (r.enumeration())
            {
                b.writeTrustedKey("Enumeration");
                b.writeStartArray();
                for (const auto& v : *r.enumeration())
                {
                    b.write(v);
                }
                b.writeEndArray();
            }
            b.writeEndObject();
        }

        void writeDataChannelDomain(Builder& b, const domain::DataChannel& dc)
        {
            b.writeStartObject();

            // DataChannelID
            const auto& id = dc.dataChannelId();
            b.writeTrustedKey("DataChannelID");
            b.writeStartObject();
            b.writeTrustedKey("LocalID");
            b.write(id.localId().toString());
            if (id.shortId())
            {
                b.writeTrustedKey("ShortID");
                b.write(*id.shortId());
            }
            if (id.nameObject())
            {
                b.writeTrustedKey("NameObject");
                b.writeStartObject();
                b.writeTrustedKey("NamingRule");
                b.write(id.nameObject()->namingRule());
                if (id.nameObject()->customNameObjects())
                {
                    for (const auto& [key, val] : id.nameObject()->customNameObjects()->asObject())
                    {
                        b.writeKey(key);
                        serializableToBuilder(b, val);
                    }
                }
                b.writeEndObject();
            }
            b.writeEndObject();

            // Property
            const auto& p = dc.property();
            b.writeTrustedKey("Property");
            b.writeStartObject();

            b.writeTrustedKey("DataChannelType");
            b.writeStartObject();
            b.writeTrustedKey("Type");
            b.write(p.dataChannelType().type());
            if (p.dataChannelType().updateCycle())
            {
                b.writeTrustedKey("UpdateCycle");
                b.write(*p.dataChannelType().updateCycle());
            }
            if (p.dataChannelType().calculationPeriod())
            {
                b.writeTrustedKey("CalculationPeriod");
                b.write(*p.dataChannelType().calculationPeriod());
            }
            b.writeEndObject();

            b.writeTrustedKey("Format");
            b.writeStartObject();
            b.writeTrustedKey("Type");
            b.write(p.format().type());
            if (p.format().restriction())
            {
                b.writeTrustedKey("Restriction");
                writeRestrictionDomain(b, *p.format().restriction());
            }
            b.writeEndObject();

            if (p.range())
            {
                b.writeTrustedKey("Range");
                b.writeStartObject();
                b.writeTrustedKey("High");
                b.write(p.range()->high());
                b.writeTrustedKey("Low");
                b.write(p.range()->low());
                b.writeEndObject();
            }
            if (p.unit())
            {
                b.writeTrustedKey("Unit");
                b.writeStartObject();
                b.writeTrustedKey("UnitSymbol");
                b.write(p.unit()->unitSymbol());
                if (p.unit()->quantityName())
                {
                    b.writeTrustedKey("QuantityName");
                    b.write(*p.unit()->quantityName());
                }
                if (p.unit()->customElements())
                {
                    for (const auto& [key, val] : p.unit()->customElements()->asObject())
                    {
                        b.writeKey(key);
                        serializableToBuilder(b, val);
                    }
                }
                b.writeEndObject();
            }
            if (p.qualityCoding())
            {
                b.writeTrustedKey("QualityCoding");
                b.write(*p.qualityCoding());
            }
            if (p.alertPriority())
            {
                b.writeTrustedKey("AlertPriority");
                b.write(*p.alertPriority());
            }
            if (p.name())
            {
                b.writeTrustedKey("Name");
                b.write(*p.name());
            }
            if (p.remarks())
            {
                b.writeTrustedKey("Remarks");
                b.write(*p.remarks());
            }
            if (p.customProperties())
            {
                for (const auto& [key, val] : p.customProperties()->asObject())
                {
                    b.writeKey(key);
                    serializableToBuilder(b, val);
                }
            }
            b.writeEndObject(); // Property

            b.writeEndObject(); // DataChannel
        }

        RestrictionDto restrictionDtoFromJson(const Document& doc)
        {
            RestrictionDto r;
            if (doc.contains("FractionDigits"))
            {
                r.fractionDigits = static_cast<std::uint32_t>(doc["FractionDigits"].root<std::int64_t>().value());
            }
            if (doc.contains("Length"))
            {
                r.length = static_cast<std::uint32_t>(doc["Length"].root<std::int64_t>().value());
            }
            if (doc.contains("MaxExclusive"))
            {
                r.maxExclusive = readDouble(doc["MaxExclusive"]);
            }
            if (doc.contains("MaxInclusive"))
            {
                r.maxInclusive = readDouble(doc["MaxInclusive"]);
            }
            if (doc.contains("MaxLength"))
            {
                r.maxLength = static_cast<std::uint32_t>(doc["MaxLength"].root<std::int64_t>().value());
            }
            if (doc.contains("MinExclusive"))
            {
                r.minExclusive = readDouble(doc["MinExclusive"]);
            }
            if (doc.contains("MinInclusive"))
            {
                r.minInclusive = readDouble(doc["MinInclusive"]);
            }
            if (doc.contains("MinLength"))
            {
                r.minLength = static_cast<std::uint32_t>(doc["MinLength"].root<std::int64_t>().value());
            }
            if (doc.contains("Pattern"))
            {
                r.pattern = doc["Pattern"].root<std::string>().value();
            }
            if (doc.contains("TotalDigits"))
            {
                r.totalDigits = static_cast<std::uint32_t>(doc["TotalDigits"].root<std::int64_t>().value());
            }
            if (doc.contains("WhiteSpace"))
            {
                r.whiteSpace = doc["WhiteSpace"].root<std::string>().value();
            }
            if (doc.contains("Enumeration"))
            {
                const auto& arr = doc["Enumeration"];
                std::vector<std::string> enums;
                enums.reserve(arr.size());
                for (std::size_t i = 0; i < arr.size(); ++i)
                {
                    enums.push_back(arr.at(i).root<std::string>().value());
                }
                r.enumeration = std::move(enums);
            }
            return r;
        }

        Ser customFieldsFromJson(const Document& doc, const std::vector<std::string_view>& knownKeys)
        {
            auto result = Ser::object();
            for (auto it = doc.objectBegin(); it != doc.objectEnd(); ++it)
            {
                bool known = false;
                for (const auto& k : knownKeys)
                {
                    if (it.key() == k)
                    {
                        known = true;
                        break;
                    }
                }
                if (!known)
                {
                    result.set(it.key(), documentToSerializable(it.value()));
                }
            }
            if (result.asObject().empty())
            {
                return Ser{};
            }
            return result;
        }

        DataChannelDto dataChannelDtoFromJson(const Document& doc)
        {
            DataChannelDto dc;

            const auto& idDoc = doc["DataChannelID"];
            dc.dataChannelId.localId = idDoc["LocalID"].root<std::string>().value();
            if (idDoc.contains("ShortID"))
            {
                dc.dataChannelId.shortId = idDoc["ShortID"].root<std::string>();
            }
            if (idDoc.contains("NameObject"))
            {
                const auto& noDoc = idDoc["NameObject"];
                NameObjectDto no;
                no.namingRule = noDoc["NamingRule"].root<std::string>().value();
                auto custom = customFieldsFromJson(noDoc, { "NamingRule" });
                if (!custom.isNull())
                {
                    no.customNameObjects = std::move(custom);
                }
                dc.dataChannelId.nameObject = std::move(no);
            }

            const auto& propDoc = doc["Property"];

            const auto& dctDoc = propDoc["DataChannelType"];
            dc.property.dataChannelType.type = dctDoc["Type"].root<std::string>().value();
            if (dctDoc.contains("UpdateCycle"))
            {
                dc.property.dataChannelType.updateCycle = readDouble(dctDoc["UpdateCycle"]);
            }
            if (dctDoc.contains("CalculationPeriod"))
            {
                dc.property.dataChannelType.calculationPeriod = readDouble(dctDoc["CalculationPeriod"]);
            }

            const auto& fmtDoc = propDoc["Format"];
            dc.property.format.type = fmtDoc["Type"].root<std::string>().value();
            if (fmtDoc.contains("Restriction"))
            {
                dc.property.format.restriction = restrictionDtoFromJson(fmtDoc["Restriction"]);
            }

            if (propDoc.contains("Range"))
            {
                const auto& rDoc = propDoc["Range"];
                dc.property.range = RangeDto{ readDouble(rDoc["Low"]), readDouble(rDoc["High"]) };
            }
            if (propDoc.contains("Unit"))
            {
                const auto& uDoc = propDoc["Unit"];
                UnitDto u;
                u.unitSymbol = uDoc["UnitSymbol"].root<std::string>().value();
                if (uDoc.contains("QuantityName"))
                {
                    u.quantityName = uDoc["QuantityName"].root<std::string>();
                }
                auto custom = customFieldsFromJson(uDoc, { "UnitSymbol", "QuantityName" });
                if (!custom.isNull())
                {
                    u.customElements = std::move(custom);
                }
                dc.property.unit = std::move(u);
            }
            if (propDoc.contains("QualityCoding"))
            {
                dc.property.qualityCoding = propDoc["QualityCoding"].root<std::string>();
            }
            if (propDoc.contains("AlertPriority"))
            {
                dc.property.alertPriority = propDoc["AlertPriority"].root<std::string>();
            }
            if (propDoc.contains("Name"))
            {
                dc.property.name = propDoc["Name"].root<std::string>();
            }
            if (propDoc.contains("Remarks"))
            {
                dc.property.remarks = propDoc["Remarks"].root<std::string>();
            }

            auto customProps = customFieldsFromJson(
                propDoc,
                { "DataChannelType", "Format", "Range", "Unit", "QualityCoding", "AlertPriority", "Name", "Remarks" });
            if (!customProps.isNull())
            {
                dc.property.customProperties = std::move(customProps);
            }

            return dc;
        }
    } // namespace

    //=========================================================================
    // Domain <-> DTO
    //=========================================================================

    DataChannelListPackageDto toDto(const domain::DataChannelListPackage& pkg)
    {
        DataChannelListPackageDto result;
        auto& p = result.package;

        const auto& h = pkg.package().header();
        p.header.shipId = h.shipId().toString();
        p.header.dataChannelListId.id = h.dataChannelListId().id();

        StringBuilder timeStampBuffer;
        h.dataChannelListId().timeStamp().toString(timeStampBuffer);
        p.header.dataChannelListId.timeStamp = timeStampBuffer.toString();
        if (h.dataChannelListId().version())
        {
            p.header.dataChannelListId.version = *h.dataChannelListId().version();
        }
        if (h.versionInformation())
        {
            VersionInformationDto vi;
            vi.namingRule = h.versionInformation()->namingRule();
            vi.namingSchemeVersion = h.versionInformation()->namingSchemeVersion();
            if (h.versionInformation()->referenceUrl())
            {
                vi.referenceUrl = *h.versionInformation()->referenceUrl();
            }
            p.header.versionInformation = std::move(vi);
        }
        if (h.author())
        {
            p.header.author = *h.author();
        }
        if (h.dateCreated())
        {
            timeStampBuffer.clear();
            h.dateCreated()->toString(timeStampBuffer);
            p.header.dateCreated = timeStampBuffer.toString();
        }
        if (h.customHeaders())
        {
            p.header.customHeaders = *h.customHeaders();
        }

        for (const auto& dc : pkg.package().dataChannelList())
        {
            DataChannelDto dto;
            dto.dataChannelId.localId = dc.dataChannelId().localId().toString();
            if (dc.dataChannelId().shortId())
            {
                dto.dataChannelId.shortId = *dc.dataChannelId().shortId();
            }
            if (dc.dataChannelId().nameObject())
            {
                NameObjectDto no;
                no.namingRule = dc.dataChannelId().nameObject()->namingRule();
                if (dc.dataChannelId().nameObject()->customNameObjects())
                {
                    no.customNameObjects = *dc.dataChannelId().nameObject()->customNameObjects();
                }
                dto.dataChannelId.nameObject = std::move(no);
            }

            const auto& prop = dc.property();
            dto.property.dataChannelType.type = prop.dataChannelType().type();
            dto.property.dataChannelType.updateCycle = prop.dataChannelType().updateCycle();
            dto.property.dataChannelType.calculationPeriod = prop.dataChannelType().calculationPeriod();

            dto.property.format.type = prop.format().type();
            if (prop.format().restriction())
            {
                const auto& r = *prop.format().restriction();
                RestrictionDto rd;
                rd.enumeration = r.enumeration();
                rd.fractionDigits = r.fractionDigits();
                rd.length = r.length();
                rd.maxExclusive = r.maxExclusive();
                rd.maxInclusive = r.maxInclusive();
                rd.maxLength = r.maxLength();
                rd.minExclusive = r.minExclusive();
                rd.minInclusive = r.minInclusive();
                rd.minLength = r.minLength();
                rd.pattern = r.pattern();
                rd.totalDigits = r.totalDigits();
                if (r.whiteSpace())
                {
                    rd.whiteSpace = std::string{ whiteSpaceToString(*r.whiteSpace()) };
                }
                dto.property.format.restriction = std::move(rd);
            }
            if (prop.range())
            {
                dto.property.range = RangeDto{ prop.range()->low(), prop.range()->high() };
            }
            if (prop.unit())
            {
                UnitDto u;
                u.unitSymbol = prop.unit()->unitSymbol();
                u.quantityName = prop.unit()->quantityName();
                u.customElements = prop.unit()->customElements();
                dto.property.unit = std::move(u);
            }
            dto.property.qualityCoding = prop.qualityCoding();
            dto.property.alertPriority = prop.alertPriority();
            dto.property.name = prop.name();
            dto.property.remarks = prop.remarks();
            dto.property.customProperties = prop.customProperties();

            p.dataChannelList.dataChannels.push_back(std::move(dto));
        }

        return result;
    }

    domain::DataChannelListPackage toDomain(const DataChannelListPackageDto& dto)
    {
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
            throw std::invalid_argument{ "Invalid ShipID: " + h.shipId };
        }

        domain::Header header{ std::move(*shipIdResult), confRef, vi, h.author, dateCreated, h.customHeaders };

        domain::DataChannelList list;
        for (const auto& dc : p.dataChannelList.dataChannels)
        {
            auto localIdResult = LocalId::fromString(dc.dataChannelId.localId);
            if (!localIdResult)
            {
                throw std::invalid_argument{ "Invalid LocalID: " + dc.dataChannelId.localId };
            }
            auto localId = std::move(*localIdResult);

            std::optional<domain::NameObject> nameObject;
            if (dc.dataChannelId.nameObject)
            {
                nameObject = domain::NameObject{ dc.dataChannelId.nameObject->namingRule,
                                                 dc.dataChannelId.nameObject->customNameObjects };
            }

            domain::DataChannelId dataChannelId{ localId, dc.dataChannelId.shortId, nameObject };

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
                if (r.whiteSpace)
                {
                    auto whiteSpace = whiteSpaceFromString(*r.whiteSpace);
                    if (!whiteSpace.has_value())
                    {
                        throw std::invalid_argument{ "Invalid WhiteSpace value: " + *r.whiteSpace };
                    }
                    dr.setWhiteSpace(whiteSpace);
                }
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

        domain::Package package{ header, std::move(list) };
        return domain::DataChannelListPackage{ std::move(package) };
    }

    //=========================================================================
    // DTO <-> JSON
    //=========================================================================

    void toJsonString(dnv::vista::sdk::StringBuilder& buffer, const DataChannelListPackageDto& dto, bool prettyPrint)
    {
        Builder b{ buffer, Builder::Options{ prettyPrint ? 2 : 0 } };
        b.writeStartObject();
        b.writeTrustedKey("Package");
        b.writeStartObject();

        // Header
        const auto& h = dto.package.header;
        b.writeTrustedKey("Header");
        b.writeStartObject();
        b.writeTrustedKey("ShipID");
        b.write(h.shipId);
        b.writeTrustedKey("DataChannelListID");
        b.writeStartObject();
        b.writeTrustedKey("ID");
        b.write(h.dataChannelListId.id);
        if (h.dataChannelListId.version)
        {
            b.writeTrustedKey("Version");
            b.write(*h.dataChannelListId.version);
        }
        b.writeTrustedKey("TimeStamp");
        b.write(h.dataChannelListId.timeStamp);
        b.writeEndObject();
        if (h.versionInformation)
        {
            b.writeTrustedKey("VersionInformation");
            b.writeStartObject();
            b.writeTrustedKey("NamingRule");
            b.write(h.versionInformation->namingRule);
            b.writeTrustedKey("NamingSchemeVersion");
            b.write(h.versionInformation->namingSchemeVersion);
            if (h.versionInformation->referenceUrl)
            {
                b.writeTrustedKey("ReferenceURL");
                b.write(*h.versionInformation->referenceUrl);
            }
            b.writeEndObject();
        }
        if (h.author)
        {
            b.writeTrustedKey("Author");
            b.write(*h.author);
        }
        if (h.dateCreated)
        {
            b.writeTrustedKey("DateCreated");
            b.write(*h.dateCreated);
        }
        if (h.customHeaders)
        {
            for (const auto& [key, val] : h.customHeaders->asObject())
            {
                b.writeKey(key);
                serializableToBuilder(b, val);
            }
        }
        b.writeEndObject(); // Header

        // DataChannelList
        b.writeTrustedKey("DataChannelList");
        b.writeStartObject();
        b.writeTrustedKey("DataChannel");
        b.writeStartArray();
        for (const auto& dc : dto.package.dataChannelList.dataChannels)
        {
            writeDataChannelDto(b, dc);
        }
        b.writeEndArray();
        b.writeEndObject(); // DataChannelList

        b.writeEndObject(); // Package
        b.writeEndObject(); // root
    }

    void toJsonString(
        dnv::vista::sdk::StringBuilder& buffer, const domain::DataChannelListPackage& pkg, bool prettyPrint)
    {
        Builder b{ buffer, Builder::Options{ prettyPrint ? 2 : 0 } };
        b.writeStartObject();
        b.writeTrustedKey("Package");
        b.writeStartObject();

        StringBuilder timeStampBuffer;

        // Header
        const auto& h = pkg.package().header();
        b.writeTrustedKey("Header");
        b.writeStartObject();
        b.writeTrustedKey("ShipID");
        b.write(h.shipId().toString());
        b.writeTrustedKey("DataChannelListID");
        b.writeStartObject();
        b.writeTrustedKey("ID");
        b.write(h.dataChannelListId().id());
        if (h.dataChannelListId().version())
        {
            b.writeTrustedKey("Version");
            b.write(*h.dataChannelListId().version());
        }
        b.writeTrustedKey("TimeStamp");
        h.dataChannelListId().timeStamp().toString(timeStampBuffer);
        b.write(timeStampBuffer.view());
        timeStampBuffer.clear();
        b.writeEndObject();
        if (h.versionInformation())
        {
            b.writeTrustedKey("VersionInformation");
            b.writeStartObject();
            b.writeTrustedKey("NamingRule");
            b.write(h.versionInformation()->namingRule());
            b.writeTrustedKey("NamingSchemeVersion");
            b.write(h.versionInformation()->namingSchemeVersion());
            if (h.versionInformation()->referenceUrl())
            {
                b.writeTrustedKey("ReferenceURL");
                b.write(*h.versionInformation()->referenceUrl());
            }
            b.writeEndObject();
        }
        if (h.author())
        {
            b.writeTrustedKey("Author");
            b.write(*h.author());
        }
        if (h.dateCreated())
        {
            b.writeTrustedKey("DateCreated");
            h.dateCreated()->toString(timeStampBuffer);
            b.write(timeStampBuffer.view());
            timeStampBuffer.clear();
        }
        if (h.customHeaders())
        {
            for (const auto& [key, val] : h.customHeaders()->asObject())
            {
                b.writeKey(key);
                serializableToBuilder(b, val);
            }
        }
        b.writeEndObject(); // Header

        // DataChannelList
        b.writeTrustedKey("DataChannelList");
        b.writeStartObject();
        b.writeTrustedKey("DataChannel");
        b.writeStartArray();
        for (const auto& dc : pkg.package().dataChannelList().dataChannels())
        {
            writeDataChannelDomain(b, dc);
        }
        b.writeEndArray();
        b.writeEndObject(); // DataChannelList

        b.writeEndObject(); // Package
        b.writeEndObject(); // root
    }

    std::optional<DataChannelListPackageDto> fromJsonString(std::string_view json)
    {
        auto docOpt = Document::fromString(json);
        if (!docOpt)
        {
            return std::nullopt;
        }

        const auto& root = *docOpt;
        if (!root.contains("Package"))
        {
            return std::nullopt;
        }

        try
        {
            DataChannelListPackageDto result;
            const auto& pkgDoc = root["Package"];

            const auto& hDoc = pkgDoc["Header"];
            result.package.header.shipId = hDoc["ShipID"].root<std::string>().value();

            const auto& clIdDoc = hDoc["DataChannelListID"];
            result.package.header.dataChannelListId.id = clIdDoc["ID"].root<std::string>().value();
            result.package.header.dataChannelListId.timeStamp = clIdDoc["TimeStamp"].root<std::string>().value();
            if (clIdDoc.contains("Version"))
            {
                result.package.header.dataChannelListId.version = clIdDoc["Version"].root<std::string>();
            }

            if (hDoc.contains("VersionInformation"))
            {
                const auto& viDoc = hDoc["VersionInformation"];
                VersionInformationDto vi;
                vi.namingRule = viDoc["NamingRule"].root<std::string>().value();
                vi.namingSchemeVersion = viDoc["NamingSchemeVersion"].root<std::string>().value();
                if (viDoc.contains("ReferenceURL"))
                {
                    vi.referenceUrl = viDoc["ReferenceURL"].root<std::string>();
                }
                result.package.header.versionInformation = std::move(vi);
            }
            if (hDoc.contains("Author"))
            {
                result.package.header.author = hDoc["Author"].root<std::string>();
            }
            if (hDoc.contains("DateCreated"))
            {
                result.package.header.dateCreated = hDoc["DateCreated"].root<std::string>();
            }

            auto customHeaders = customFieldsFromJson(
                hDoc, { "ShipID", "DataChannelListID", "VersionInformation", "Author", "DateCreated" });
            if (!customHeaders.isNull())
            {
                result.package.header.customHeaders = std::move(customHeaders);
            }

            const auto& dcListDoc = pkgDoc["DataChannelList"]["DataChannel"];
            result.package.dataChannelList.dataChannels.reserve(dcListDoc.size());
            for (std::size_t i = 0; i < dcListDoc.size(); ++i)
            {
                result.package.dataChannelList.dataChannels.push_back(dataChannelDtoFromJson(dcListDoc.at(i)));
            }

            return result;
        }
        catch (const std::exception&)
        {
            return std::nullopt;
        }
    }
} // namespace dnv::vista::sdk::transport::serialization::json::datachannel
