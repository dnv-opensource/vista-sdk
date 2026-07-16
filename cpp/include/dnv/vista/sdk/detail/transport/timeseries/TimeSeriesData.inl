#include <algorithm>

namespace dnv::vista::sdk::transport::timeseries
{

    inline ConfigurationReference::ConfigurationReference(std::string id, DateTimeOffset timeStamp)
        : m_id{ std::move(id) },
          m_timeStamp{ std::move(timeStamp) }
    {}

    inline const std::string& ConfigurationReference::id() const noexcept
    {
        return m_id;
    }

    inline const DateTimeOffset& ConfigurationReference::timeStamp() const noexcept
    {
        return m_timeStamp;
    }

    inline void ConfigurationReference::setId(std::string id)
    {
        m_id = std::move(id);
    }

    inline void ConfigurationReference::setTimeStamp(DateTimeOffset timeStamp)
    {
        m_timeStamp = std::move(timeStamp);
    }

    inline TimeSpan::TimeSpan(DateTimeOffset start, DateTimeOffset end)
        : m_start{ std::move(start) },
          m_end{ std::move(end) }
    {
        if (m_start > m_end)
        {
            throw std::invalid_argument{ "TimeSpan start must be before or equal to end" };
        }
    }

    inline const DateTimeOffset& TimeSpan::start() const noexcept
    {
        return m_start;
    }

    inline const DateTimeOffset& TimeSpan::end() const noexcept
    {
        return m_end;
    }

    inline void TimeSpan::setStart(DateTimeOffset start)
    {
        if (start > m_end)
        {
            throw std::invalid_argument{ "TimeSpan start must be before or equal to end" };
        }
        m_start = std::move(start);
    }

    inline void TimeSpan::setEnd(DateTimeOffset end)
    {
        if (m_start > end)
        {
            throw std::invalid_argument{ "TimeSpan start must be before or equal to end" };
        }
        m_end = std::move(end);
    }

    inline Header::Header(
        ShipId shipId,
        std::optional<TimeSpan> timeSpan,
        std::optional<DateTimeOffset> dateCreated,
        std::optional<DateTimeOffset> dateModified,
        std::optional<std::string> author,
        std::optional<std::vector<ConfigurationReference>> systemConfiguration,
        std::optional<CustomHeaders> customHeaders)
        : m_shipId{ std::move(shipId) },
          m_timeSpan{ std::move(timeSpan) },
          m_dateCreated{ std::move(dateCreated) },
          m_dateModified{ std::move(dateModified) },
          m_author{ std::move(author) },
          m_systemConfiguration{ std::move(systemConfiguration) },
          m_customHeaders{ std::move(customHeaders) }
    {}

    inline const ShipId& Header::shipId() const noexcept
    {
        return m_shipId;
    }

    inline const std::optional<TimeSpan>& Header::timeSpan() const noexcept
    {
        return m_timeSpan;
    }

    inline const std::optional<DateTimeOffset>& Header::dateCreated() const noexcept
    {
        return m_dateCreated;
    }

    inline const std::optional<DateTimeOffset>& Header::dateModified() const noexcept
    {
        return m_dateModified;
    }

    inline const std::optional<std::string>& Header::author() const noexcept
    {
        return m_author;
    }

    inline const std::optional<std::vector<ConfigurationReference>>& Header::systemConfiguration() const noexcept
    {
        return m_systemConfiguration;
    }

    inline const std::optional<CustomHeaders>& Header::customHeaders() const noexcept
    {
        return m_customHeaders;
    }

    inline void Header::setShipId(ShipId shipId)
    {
        m_shipId = std::move(shipId);
    }

    inline void Header::setTimeSpan(std::optional<TimeSpan> timeSpan)
    {
        m_timeSpan = std::move(timeSpan);
    }

    inline void Header::setDateCreated(std::optional<DateTimeOffset> dateCreated)
    {
        m_dateCreated = std::move(dateCreated);
    }

    inline void Header::setDateModified(std::optional<DateTimeOffset> dateModified)
    {
        m_dateModified = std::move(dateModified);
    }

    inline void Header::setAuthor(std::optional<std::string> author)
    {
        m_author = std::move(author);
    }

    inline void Header::setSystemConfiguration(std::optional<std::vector<ConfigurationReference>> systemConfiguration)
    {
        m_systemConfiguration = std::move(systemConfiguration);
    }

    inline void Header::setCustomHeaders(std::optional<CustomHeaders> customHeaders)
    {
        m_customHeaders = std::move(customHeaders);
    }

    inline TabularDataSet::TabularDataSet(
        DateTimeOffset timeStamp, std::vector<std::string> value, std::optional<std::vector<std::string>> quality)
        : m_timeStamp{ std::move(timeStamp) },
          m_value{ std::move(value) },
          m_quality{ std::move(quality) }
    {}

    inline const DateTimeOffset& TabularDataSet::timeStamp() const noexcept
    {
        return m_timeStamp;
    }

    inline const std::vector<std::string>& TabularDataSet::value() const noexcept
    {
        return m_value;
    }

    inline const std::optional<std::vector<std::string>>& TabularDataSet::quality() const noexcept
    {
        return m_quality;
    }

    inline void TabularDataSet::setTimeStamp(DateTimeOffset timeStamp)
    {
        m_timeStamp = std::move(timeStamp);
    }

    inline void TabularDataSet::setValue(std::vector<std::string> value)
    {
        m_value = std::move(value);
    }

    inline void TabularDataSet::setQuality(std::optional<std::vector<std::string>> quality)
    {
        m_quality = std::move(quality);
    }

    inline TabularData::TabularData(std::vector<DataChannelId> dataChannelIds, std::vector<TabularDataSet> dataSets)
        : m_dataChannelIds{ std::move(dataChannelIds) },
          m_dataSets{ std::move(dataSets) }
    {}

    inline const std::vector<DataChannelId>& TabularData::dataChannelIds() const noexcept
    {
        return m_dataChannelIds;
    }

    inline const std::vector<TabularDataSet>& TabularData::dataSets() const noexcept
    {
        return m_dataSets;
    }

    inline std::size_t TabularData::numberOfDataSets() const noexcept
    {
        return m_dataSets.size();
    }

    inline std::size_t TabularData::numberOfDataChannels() const noexcept
    {
        return m_dataChannelIds.size();
    }

    inline void TabularData::setDataChannelIds(std::vector<DataChannelId> dataChannelIds)
    {
        m_dataChannelIds = std::move(dataChannelIds);
    }

    inline void TabularData::setDataSets(std::vector<TabularDataSet> dataSets)
    {
        m_dataSets = std::move(dataSets);
    }

    inline ValidateResult<> TabularData::validate() const
    {
        if (m_dataChannelIds.empty())
        {
            return ValidateResult<>::invalid("Tabular data has no data channels");
        }

        if (m_dataSets.empty())
        {
            return ValidateResult<>::invalid("Tabular data has no data sets");
        }

        for (std::size_t i = 0; i < m_dataSets.size(); ++i)
        {
            const auto& dataset = m_dataSets[i];
            if (dataset.value().size() != m_dataChannelIds.size())
            {
                return ValidateResult<>::invalid(
                    "Tabular data set " + std::to_string(i) + " expects " + std::to_string(m_dataChannelIds.size()) +
                    " values, but " + std::to_string(dataset.value().size()) + " values are provided");
            }

            if (dataset.quality().has_value() && dataset.quality()->size() != m_dataChannelIds.size())
            {
                return ValidateResult<>::invalid(
                    "Tabular data set " + std::to_string(i) + " expects " + std::to_string(m_dataChannelIds.size()) +
                    " quality values, but " + std::to_string(dataset.quality()->size()) +
                    " quality values are provided");
            }
        }

        return ValidateResult<>::ok();
    }

    inline EventDataSet::EventDataSet(
        DateTimeOffset timeStamp, DataChannelId dataChannelId, std::string value, std::optional<std::string> quality)
        : m_timeStamp{ std::move(timeStamp) },
          m_dataChannelId{ std::move(dataChannelId) },
          m_value{ std::move(value) },
          m_quality{ std::move(quality) }
    {}

    inline const DateTimeOffset& EventDataSet::timeStamp() const noexcept
    {
        return m_timeStamp;
    }

    inline const DataChannelId& EventDataSet::dataChannelId() const noexcept
    {
        return m_dataChannelId;
    }

    inline const std::string& EventDataSet::value() const noexcept
    {
        return m_value;
    }

    inline const std::optional<std::string>& EventDataSet::quality() const noexcept
    {
        return m_quality;
    }

    inline void EventDataSet::setTimeStamp(DateTimeOffset timeStamp)
    {
        m_timeStamp = std::move(timeStamp);
    }

    inline void EventDataSet::setDataChannelId(DataChannelId dataChannelId)
    {
        m_dataChannelId = std::move(dataChannelId);
    }

    inline void EventDataSet::setValue(std::string value)
    {
        m_value = std::move(value);
    }

    inline void EventDataSet::setQuality(std::optional<std::string> quality)
    {
        m_quality = std::move(quality);
    }

    inline EventData::EventData(std::optional<std::vector<EventDataSet>> dataSet)
        : m_dataSet{ std::move(dataSet) }
    {}

    inline const std::optional<std::vector<EventDataSet>>& EventData::dataSet() const noexcept
    {
        return m_dataSet;
    }

    inline std::size_t EventData::numberOfDataSet() const noexcept
    {
        return m_dataSet.has_value() ? m_dataSet->size() : 0;
    }

    inline void EventData::setDataSet(std::optional<std::vector<EventDataSet>> dataSet)
    {
        m_dataSet = std::move(dataSet);
    }

    inline TimeSeriesData::TimeSeriesData(
        std::optional<ConfigurationReference> dataConfiguration,
        std::optional<std::vector<TabularData>> tabularData,
        std::optional<EventData> eventData,
        std::optional<CustomData> customDataKinds)
        : m_dataConfiguration{ std::move(dataConfiguration) },
          m_tabularData{ std::move(tabularData) },
          m_eventData{ std::move(eventData) },
          m_customDataKinds{ std::move(customDataKinds) }
    {}

    inline const std::optional<ConfigurationReference>& TimeSeriesData::dataConfiguration() const noexcept
    {
        return m_dataConfiguration;
    }

    inline const std::optional<std::vector<TabularData>>& TimeSeriesData::tabularData() const noexcept
    {
        return m_tabularData;
    }

    inline const std::optional<EventData>& TimeSeriesData::eventData() const noexcept
    {
        return m_eventData;
    }

    inline const std::optional<CustomData>& TimeSeriesData::customDataKinds() const noexcept
    {
        return m_customDataKinds;
    }

    inline void TimeSeriesData::setDataConfiguration(std::optional<ConfigurationReference> dataConfiguration)
    {
        m_dataConfiguration = std::move(dataConfiguration);
    }

    inline void TimeSeriesData::setTabularData(std::optional<std::vector<TabularData>> tabularData)
    {
        m_tabularData = std::move(tabularData);
    }

    inline void TimeSeriesData::setEventData(std::optional<EventData> eventData)
    {
        m_eventData = std::move(eventData);
    }

    inline void TimeSeriesData::setCustomDataKinds(std::optional<CustomData> customDataKinds)
    {
        m_customDataKinds = std::move(customDataKinds);
    }

    inline Package::Package(std::optional<Header> header, std::vector<TimeSeriesData> timeSeriesData)
        : m_header{ std::move(header) },
          m_timeSeriesData{ std::move(timeSeriesData) }
    {}

    inline const std::optional<Header>& Package::header() const noexcept
    {
        return m_header;
    }

    inline const std::vector<TimeSeriesData>& Package::timeSeriesData() const noexcept
    {
        return m_timeSeriesData;
    }

    inline void Package::setHeader(std::optional<Header> header)
    {
        m_header = std::move(header);
    }

    inline void Package::setTimeSeriesData(std::vector<TimeSeriesData> timeSeriesData)
    {
        m_timeSeriesData = std::move(timeSeriesData);
    }

    inline TimeSeriesDataPackage::TimeSeriesDataPackage(Package package)
        : m_package{ std::move(package) }
    {}

    inline const Package& TimeSeriesDataPackage::package() const noexcept
    {
        return m_package;
    }

    inline void TimeSeriesDataPackage::setPackage(Package package)
    {
        m_package = std::move(package);
    }
} // namespace dnv::vista::sdk::transport::timeseries
