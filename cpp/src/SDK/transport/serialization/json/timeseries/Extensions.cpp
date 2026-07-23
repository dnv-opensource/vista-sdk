#include "dnv/vista/sdk/transport/serialization/json/timeseries/Extensions.h"

#include "JSON/Json.h"

#include <stdexcept>

namespace dnv::vista::sdk::transport::serialization::json::timeseries
{
    namespace
    {
        using namespace dnv::vista::sdk::json;
        namespace domain = dnv::vista::sdk::transport::timeseries;
        using Ser = serialization::json::SerializableDocument;

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
    } // namespace

    //=========================================================================
    // Domain <-> DTO
    //=========================================================================

    TimeSeriesDataPackageDto toDto(const domain::TimeSeriesDataPackage& pkg)
    {
        TimeSeriesDataPackageDto result;
        auto& p = result.package;

        const auto& h = pkg.package().header();
        if (h)
        {
            HeaderDto hDto;
            hDto.shipId = h->shipId().toString();

            StringBuilder timeStampBuffer;
            if (h->timeSpan())
            {
                h->timeSpan()->start().toString(timeStampBuffer);
                std::string startStr{ timeStampBuffer.toString() };
                timeStampBuffer.clear();

                h->timeSpan()->end().toString(timeStampBuffer);
                std::string endStr{ timeStampBuffer.toString() };
                timeStampBuffer.clear();

                hDto.timeSpan = TimeSpanDto{ std::move(startStr), std::move(endStr) };
            }
            if (h->dateCreated())
            {
                h->dateCreated()->toString(timeStampBuffer);
                hDto.dateCreated = timeStampBuffer.toString();
                timeStampBuffer.clear();
            }
            if (h->dateModified())
            {
                h->dateModified()->toString(timeStampBuffer);
                hDto.dateModified = timeStampBuffer.toString();
                timeStampBuffer.clear();
            }
            if (h->author())
            {
                hDto.author = *h->author();
            }
            if (h->systemConfiguration())
            {
                std::vector<ConfigurationReferenceDto> sysConf;
                sysConf.reserve(h->systemConfiguration()->size());
                for (const auto& c : *h->systemConfiguration())
                {
                    c.timeStamp().toString(timeStampBuffer);
                    sysConf.push_back({ c.id(), timeStampBuffer.toString() });
                    timeStampBuffer.clear();
                }
                hDto.systemConfiguration = std::move(sysConf);
            }
            if (h->customHeaders())
            {
                hDto.customHeaders = *h->customHeaders();
            }
            p.header = std::move(hDto);
        }

        StringBuilder tsTimeStampBuffer;
        for (const auto& ts : pkg.package().timeSeriesData())
        {
            TimeSeriesDataDto tsDto;

            if (ts.dataConfiguration())
            {
                ts.dataConfiguration()->timeStamp().toString(tsTimeStampBuffer);
                tsDto.dataConfiguration =
                    ConfigurationReferenceDto{ ts.dataConfiguration()->id(), tsTimeStampBuffer.toString() };
                tsTimeStampBuffer.clear();
            }
            if (ts.tabularData())
            {
                std::vector<TabularDataDto> tabList;
                for (const auto& td : *ts.tabularData())
                {
                    TabularDataDto tdDto;
                    tdDto.numberOfDataSet = td.numberOfDataSets();
                    tdDto.numberOfDataChannel = td.numberOfDataChannels();

                    std::vector<std::string> ids;
                    for (const auto& id : td.dataChannelIds())
                    {
                        ids.push_back(id.toString());
                    }
                    tdDto.dataChannelIds = std::move(ids);

                    std::vector<TabularDataSetDto> sets;
                    sets.reserve(td.dataSets().size());
                    StringBuilder timeStampBuffer;
                    for (const auto& ds : td.dataSets())
                    {
                        TabularDataSetDto dsDto;
                        ds.timeStamp().toString(timeStampBuffer);
                        dsDto.timeStamp = timeStampBuffer.toString();
                        timeStampBuffer.clear();
                        dsDto.value = ds.value();
                        dsDto.quality = ds.quality();
                        sets.push_back(std::move(dsDto));
                    }
                    tdDto.dataSets = std::move(sets);
                    tabList.push_back(std::move(tdDto));
                }
                tsDto.tabularData = std::move(tabList);
            }

            if (ts.eventData())
            {
                EventDataDto edDto;
                edDto.numberOfDataSet = ts.eventData()->numberOfDataSet();
                if (ts.eventData()->dataSet())
                {
                    std::vector<EventDataSetDto> sets;
                    sets.reserve(ts.eventData()->dataSet()->size());
                    StringBuilder timeStampBuffer;
                    for (const auto& ed : *ts.eventData()->dataSet())
                    {
                        EventDataSetDto edSetDto;
                        ed.timeStamp().toString(timeStampBuffer);
                        edSetDto.timeStamp = timeStampBuffer.toString();
                        timeStampBuffer.clear();
                        edSetDto.dataChannelId = ed.dataChannelId().toString();
                        edSetDto.value = ed.value();
                        edSetDto.quality = ed.quality();
                        sets.push_back(std::move(edSetDto));
                    }
                    edDto.dataSet = std::move(sets);
                }
                tsDto.eventData = std::move(edDto);
            }

            if (ts.customDataKinds())
            {
                tsDto.customDataKinds = *ts.customDataKinds();
            }

            p.timeSeriesData.push_back(std::move(tsDto));
        }

        return result;
    }

    domain::TimeSeriesDataPackage toDomain(const TimeSeriesDataPackageDto& dto)
    {
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
                throw std::invalid_argument{ "Invalid ShipID: " + h.shipId };
            }

            header = domain::Header{ std::move(*shipIdResult), timeSpan, dateCreated, dateModified, h.author, sysConf,
                                     h.customHeaders };
        }

        std::vector<domain::TimeSeriesData> tsList;
        for (const auto& ts : p.timeSeriesData)
        {
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
                for (const auto& td : *ts.tabularData)
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
                        throw std::invalid_argument{ "Number of data channels does not match the expected count" };
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
                        throw std::invalid_argument{ "Number of data sets does not match the expected count" };
                    }
                    tabList.emplace_back(std::move(ids), std::move(sets));
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
                    for (const auto& ed : *ts.eventData->dataSet)
                    {
                        auto idResult = domain::DataChannelId::fromString(ed.dataChannelId);
                        if (!idResult)
                        {
                            throw std::invalid_argument{ "Invalid DataChannelID: " + ed.dataChannelId };
                        }
                        evSets.emplace_back(DateTimeOffset{ ed.timeStamp }, std::move(*idResult), ed.value, ed.quality);
                    }
                    sets = std::move(evSets);
                }
                eventData = domain::EventData{ std::move(sets) };
            }

            tsList.emplace_back(dataConfig, tabularData, eventData, ts.customDataKinds);
        }

        domain::Package pkg{ std::move(header), std::move(tsList) };
        return domain::TimeSeriesDataPackage{ std::move(pkg) };
    }

    //=========================================================================
    // DTO <-> JSON
    //=========================================================================

    void toJsonString(dnv::vista::sdk::StringBuilder& buffer, const TimeSeriesDataPackageDto& dto, bool prettyPrint)
    {
        Builder b{ buffer, Builder::Options{ prettyPrint ? 2 : 0 } };
        b.writeStartObject();
        b.writeTrustedKey("Package");
        b.writeStartObject();

        // Header
        if (dto.package.header)
        {
            const auto& h = *dto.package.header;
            b.writeTrustedKey("Header");
            b.writeStartObject();
            b.writeTrustedKey("ShipID");
            b.write(h.shipId);
            if (h.timeSpan)
            {
                b.writeTrustedKey("TimeSpan");
                b.writeStartObject();
                b.writeTrustedKey("Start");
                b.write(h.timeSpan->start);
                b.writeTrustedKey("End");
                b.write(h.timeSpan->end);
                b.writeEndObject();
            }
            if (h.dateCreated)
            {
                b.writeTrustedKey("DateCreated");
                b.write(*h.dateCreated);
            }
            if (h.dateModified)
            {
                b.writeTrustedKey("DateModified");
                b.write(*h.dateModified);
            }
            if (h.author)
            {
                b.writeTrustedKey("Author");
                b.write(*h.author);
            }
            if (h.systemConfiguration)
            {
                b.writeTrustedKey("SystemConfiguration");
                b.writeStartArray();
                for (const auto& c : *h.systemConfiguration)
                {
                    b.writeStartObject();
                    b.writeTrustedKey("ID");
                    b.write(c.id);
                    b.writeTrustedKey("TimeStamp");
                    b.write(c.timeStamp);
                    b.writeEndObject();
                }
                b.writeEndArray();
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
        }

        // TimeSeriesData
        b.writeTrustedKey("TimeSeriesData");
        b.writeStartArray();
        for (const auto& ts : dto.package.timeSeriesData)
        {
            b.writeStartObject();

            if (ts.dataConfiguration)
            {
                b.writeTrustedKey("DataConfiguration");
                b.writeStartObject();
                b.writeTrustedKey("ID");
                b.write(ts.dataConfiguration->id);
                b.writeTrustedKey("TimeStamp");
                b.write(ts.dataConfiguration->timeStamp);
                b.writeEndObject();
            }

            if (ts.tabularData)
            {
                b.writeTrustedKey("TabularData");
                b.writeStartArray();
                for (const auto& td : *ts.tabularData)
                {
                    b.writeStartObject();
                    if (td.numberOfDataSet)
                    {
                        b.writeTrustedKey("NumberOfDataSet");
                        b.write(static_cast<std::int64_t>(*td.numberOfDataSet));
                    }
                    if (td.numberOfDataChannel)
                    {
                        b.writeTrustedKey("NumberOfDataChannel");
                        b.write(static_cast<std::int64_t>(*td.numberOfDataChannel));
                    }
                    if (td.dataChannelIds)
                    {
                        b.writeTrustedKey("DataChannelID");
                        b.writeStartArray();
                        for (const auto& id : *td.dataChannelIds)
                        {
                            b.write(id);
                        }
                        b.writeEndArray();
                    }
                    if (td.dataSets)
                    {
                        b.writeTrustedKey("DataSet");
                        b.writeStartArray();
                        for (const auto& ds : *td.dataSets)
                        {
                            b.writeStartObject();
                            b.writeTrustedKey("TimeStamp");
                            b.write(ds.timeStamp);
                            b.writeTrustedKey("Value");
                            b.writeStartArray();
                            for (const auto& v : ds.value)
                            {
                                b.write(v);
                            }
                            b.writeEndArray();
                            if (ds.quality)
                            {
                                b.writeTrustedKey("Quality");
                                b.writeStartArray();
                                for (const auto& q : *ds.quality)
                                {
                                    b.write(q);
                                }
                                b.writeEndArray();
                            }
                            b.writeEndObject();
                        }
                        b.writeEndArray();
                    }
                    b.writeEndObject();
                }
                b.writeEndArray();
            }

            if (ts.eventData)
            {
                b.writeTrustedKey("EventData");
                b.writeStartObject();
                if (ts.eventData->numberOfDataSet)
                {
                    b.writeTrustedKey("NumberOfDataSet");
                    b.write(static_cast<std::int64_t>(*ts.eventData->numberOfDataSet));
                }
                if (ts.eventData->dataSet)
                {
                    b.writeTrustedKey("DataSet");
                    b.writeStartArray();
                    for (const auto& ed : *ts.eventData->dataSet)
                    {
                        b.writeStartObject();
                        b.writeTrustedKey("TimeStamp");
                        b.write(ed.timeStamp);
                        b.writeTrustedKey("DataChannelID");
                        b.write(ed.dataChannelId);
                        b.writeTrustedKey("Value");
                        b.write(ed.value);
                        if (ed.quality)
                        {
                            b.writeTrustedKey("Quality");
                            b.write(*ed.quality);
                        }
                        b.writeEndObject();
                    }
                    b.writeEndArray();
                }
                b.writeEndObject(); // EventData
            }

            if (ts.customDataKinds)
            {
                for (const auto& [key, val] : ts.customDataKinds->asObject())
                {
                    b.writeKey(key);
                    serializableToBuilder(b, val);
                }
            }

            b.writeEndObject(); // TimeSeriesData entry
        }
        b.writeEndArray(); // TimeSeriesData

        b.writeEndObject(); // Package
        b.writeEndObject(); // root
    }

    std::optional<TimeSeriesDataPackageDto> fromJsonString(std::string_view json)
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
            TimeSeriesDataPackageDto result;
            const auto& pkgDoc = root["Package"];

            // Header (optional)
            if (pkgDoc.contains("Header"))
            {
                const auto& hDoc = pkgDoc["Header"];
                HeaderDto hDto;
                hDto.shipId = hDoc["ShipID"].root<std::string>().value();

                if (hDoc.contains("TimeSpan"))
                {
                    const auto& tsDoc = hDoc["TimeSpan"];
                    hDto.timeSpan = TimeSpanDto{ tsDoc["Start"].root<std::string>().value(),
                                                 tsDoc["End"].root<std::string>().value() };
                }
                if (hDoc.contains("DateCreated"))
                {
                    hDto.dateCreated = hDoc["DateCreated"].root<std::string>();
                }
                if (hDoc.contains("DateModified"))
                {
                    hDto.dateModified = hDoc["DateModified"].root<std::string>();
                }
                if (hDoc.contains("Author"))
                {
                    hDto.author = hDoc["Author"].root<std::string>();
                }
                if (hDoc.contains("SystemConfiguration"))
                {
                    const auto& scArr = hDoc["SystemConfiguration"];
                    std::vector<ConfigurationReferenceDto> refs;
                    refs.reserve(scArr.size());
                    for (std::size_t i = 0; i < scArr.size(); ++i)
                    {
                        const auto& c = scArr.at(i);
                        refs.push_back(
                            { c["ID"].root<std::string>().value(), c["TimeStamp"].root<std::string>().value() });
                    }
                    hDto.systemConfiguration = std::move(refs);
                }
                auto customHeaders = customFieldsFromJson(
                    hDoc, { "ShipID", "TimeSpan", "DateCreated", "DateModified", "Author", "SystemConfiguration" });
                if (!customHeaders.isNull())
                {
                    hDto.customHeaders = std::move(customHeaders);
                }

                result.package.header = std::move(hDto);
            }

            // TimeSeriesData array
            if (!pkgDoc.contains("TimeSeriesData"))
            {
                return result;
            }

            const auto& tsArr = pkgDoc["TimeSeriesData"];
            result.package.timeSeriesData.reserve(tsArr.size());
            for (std::size_t i = 0; i < tsArr.size(); ++i)
            {
                const auto& tsDoc = tsArr.at(i);
                TimeSeriesDataDto tsDto;

                if (tsDoc.contains("DataConfiguration"))
                {
                    const auto& dcDoc = tsDoc["DataConfiguration"];
                    tsDto.dataConfiguration =
                        ConfigurationReferenceDto{ dcDoc["ID"].root<std::string>().value(),
                                                   dcDoc["TimeStamp"].root<std::string>().value() };
                }

                if (tsDoc.contains("TabularData"))
                {
                    const auto& tabArr = tsDoc["TabularData"];
                    std::vector<TabularDataDto> tabList;
                    tabList.reserve(tabArr.size());
                    for (std::size_t j = 0; j < tabArr.size(); ++j)
                    {
                        const auto& tdDoc = tabArr.at(j);
                        TabularDataDto tdDto;

                        if (tdDoc.contains("NumberOfDataSet"))
                        {
                            tdDto.numberOfDataSet =
                                static_cast<std::size_t>(tdDoc["NumberOfDataSet"].root<std::int64_t>().value());
                        }
                        if (tdDoc.contains("NumberOfDataChannel"))
                        {
                            tdDto.numberOfDataChannel =
                                static_cast<std::size_t>(tdDoc["NumberOfDataChannel"].root<std::int64_t>().value());
                        }

                        if (tdDoc.contains("DataChannelID"))
                        {
                            const auto& idsArr = tdDoc["DataChannelID"];
                            std::vector<std::string> ids;
                            ids.reserve(idsArr.size());
                            for (std::size_t k = 0; k < idsArr.size(); ++k)
                            {
                                ids.push_back(idsArr.at(k).root<std::string>().value());
                            }
                            tdDto.dataChannelIds = std::move(ids);
                        }

                        if (tdDoc.contains("DataSet"))
                        {
                            const auto& dsArr = tdDoc["DataSet"];
                            std::vector<TabularDataSetDto> sets;
                            sets.reserve(dsArr.size());
                            for (std::size_t k = 0; k < dsArr.size(); ++k)
                            {
                                const auto& dsDoc = dsArr.at(k);
                                TabularDataSetDto dsDto;
                                dsDto.timeStamp = dsDoc["TimeStamp"].root<std::string>().value();

                                const auto& valArr = dsDoc["Value"];
                                dsDto.value.reserve(valArr.size());
                                for (std::size_t m = 0; m < valArr.size(); ++m)
                                {
                                    dsDto.value.push_back(valArr.at(m).root<std::string>().value());
                                }

                                if (dsDoc.contains("Quality"))
                                {
                                    const auto& qArr = dsDoc["Quality"];
                                    std::vector<std::string> qual;
                                    qual.reserve(qArr.size());
                                    for (std::size_t m = 0; m < qArr.size(); ++m)
                                    {
                                        qual.push_back(qArr.at(m).root<std::string>().value());
                                    }
                                    dsDto.quality = std::move(qual);
                                }
                                sets.push_back(std::move(dsDto));
                            }
                            tdDto.dataSets = std::move(sets);
                        }
                        tabList.push_back(std::move(tdDto));
                    }
                    tsDto.tabularData = std::move(tabList);
                }

                if (tsDoc.contains("EventData"))
                {
                    const auto& edDoc = tsDoc["EventData"];
                    EventDataDto edDto;

                    if (edDoc.contains("NumberOfDataSet"))
                    {
                        edDto.numberOfDataSet =
                            static_cast<std::size_t>(edDoc["NumberOfDataSet"].root<std::int64_t>().value());
                    }

                    if (edDoc.contains("DataSet"))
                    {
                        const auto& dsArr = edDoc["DataSet"];
                        std::vector<EventDataSetDto> sets;
                        sets.reserve(dsArr.size());
                        for (std::size_t k = 0; k < dsArr.size(); ++k)
                        {
                            const auto& dsDoc = dsArr.at(k);
                            EventDataSetDto dsDto;
                            dsDto.timeStamp = dsDoc["TimeStamp"].root<std::string>().value();
                            dsDto.dataChannelId = dsDoc["DataChannelID"].root<std::string>().value();
                            dsDto.value = dsDoc["Value"].root<std::string>().value();
                            if (dsDoc.contains("Quality"))
                            {
                                dsDto.quality = dsDoc["Quality"].root<std::string>();
                            }
                            sets.push_back(std::move(dsDto));
                        }
                        edDto.dataSet = std::move(sets);
                    }
                    tsDto.eventData = std::move(edDto);
                }

                auto customData = customFieldsFromJson(tsDoc, { "DataConfiguration", "TabularData", "EventData" });
                if (!customData.isNull())
                {
                    tsDto.customDataKinds = std::move(customData);
                }

                result.package.timeSeriesData.push_back(std::move(tsDto));
            }

            return result;
        }
        catch (const std::exception&)
        {
            return std::nullopt;
        }
    }
} // namespace dnv::vista::sdk::transport::serialization::json::timeseries
