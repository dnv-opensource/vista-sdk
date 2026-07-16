#include "dnv/vista/sdk/transport/timeseries/TimeSeriesData.h"

#include "dnv/vista/sdk/transport/datachannel/DataChannel.h"

#include <algorithm>
#include <vector>

namespace dnv::vista::sdk::transport::timeseries
{
    ValidateResult<> TimeSeriesData::validate(
        const datachannel::DataChannelListPackage& dcPackage,
        std::function<ValidateResult<>(
            const DateTimeOffset&, const datachannel::DataChannel&, const Value&, const std::optional<std::string>&)>
            onTabularData,
        std::function<ValidateResult<>(
            const DateTimeOffset&, const datachannel::DataChannel&, const Value&, const std::optional<std::string>&)>
            onEventData) const
    {
        if (m_dataConfiguration.has_value())
        {
            if (dcPackage.package().header().dataChannelListId().id() != m_dataConfiguration->id())
            {
                return ValidateResult<>::invalid("DataConfiguration Id does not match DataChannelList Id");
            }
        }

        // Ensure at least one data source contains data
        const bool hasTabularData = m_tabularData.has_value() && !m_tabularData->empty();
        const bool hasEventData =
            m_eventData.has_value() && m_eventData->dataSet().has_value() && !m_eventData->dataSet()->empty();

        if (!hasTabularData && !hasEventData)
        {
            return ValidateResult<>::invalid("Can't ingest timeseries data without data");
        }

        std::vector<std::string> errors;

        // Validate Tabular data
        if (hasTabularData)
        {
            for (const auto& table : *m_tabularData)
            {
                // Validate table structure
                auto tableValidation = table.validate();
                if (!tableValidation)
                {
                    return tableValidation;
                }

                // Validate each dataset
                const auto& dataSets = table.dataSets();
                const auto& dataChannelIds = table.dataChannelIds();

                for (std::size_t i = 0; i < dataSets.size(); ++i)
                {
                    const auto& dataset = dataSets[i];

                    // Validate each value against its data channel
                    for (std::size_t j = 0; j < dataChannelIds.size(); ++j)
                    {
                        const auto& dataChannelId = dataChannelIds[j];

                        // Find data channel in the list
                        std::optional<std::reference_wrapper<const datachannel::DataChannel>> dataChannelOpt;

                        dataChannelId.match(
                            [&](const LocalId& localId) {
                                dataChannelOpt = dcPackage.dataChannelList().from(localId);
                                if (!dataChannelOpt.has_value())
                                {
                                    errors.push_back("DataChannel with LocalId '" + localId.toString() + "' not found");
                                }
                            },
                            [&](std::string_view shortId) {
                                dataChannelOpt = dcPackage.dataChannelList().from(shortId);
                                if (!dataChannelOpt.has_value())
                                {
                                    errors.push_back(
                                        "DataChannel with ShortId '" + std::string{ shortId } + "' not found");
                                }
                            });

                        if (!dataChannelOpt.has_value())
                        {
                            continue;
                        }

                        const auto& dataChannel = dataChannelOpt->get();

                        // Validate value format
                        Value parsedValue{ Value::String{ "" } };
                        auto formatValidation =
                            dataChannel.property().format().validateValue(dataset.value()[j], parsedValue);

                        if (!formatValidation)
                        {
                            std::string errorMsg =
                                formatValidation.errors().empty() ? "Unknown error" : formatValidation.errors()[0];
                            errors.push_back(
                                "DataChannel " + dataChannelId.toString() + " has invalid value: " + errorMsg);
                            continue;
                        }

                        // Validate quality if present
                        std::optional<std::string> quality;
                        if (dataset.quality().has_value() && j < dataset.quality()->size())
                        {
                            quality = (*dataset.quality())[j];
                        }

                        // Custom validation callback
                        auto customValidation = onTabularData(dataset.timeStamp(), dataChannel, parsedValue, quality);
                        if (!customValidation)
                        {
                            std::string errorMsg =
                                customValidation.errors().empty() ? "Unknown error" : customValidation.errors()[0];
                            errors.push_back(
                                "DataChannel " + dataChannelId.toString() + " validation failed: " + errorMsg);
                        }
                    }
                }
            }
        }

        // Validate Event data
        if (hasEventData)
        {
            for (const auto& eventDataSet : *m_eventData->dataSet())
            {
                const auto& dataChannelId = eventDataSet.dataChannelId();

                // Find data channel in the list
                std::optional<std::reference_wrapper<const datachannel::DataChannel>> dataChannelOpt;

                dataChannelId.match(
                    [&](const LocalId& localId) {
                        dataChannelOpt = dcPackage.dataChannelList().from(localId);
                        if (!dataChannelOpt.has_value())
                        {
                            errors.push_back("DataChannel with LocalId '" + localId.toString() + "' not found");
                        }
                    },
                    [&](std::string_view shortId) {
                        dataChannelOpt = dcPackage.dataChannelList().from(shortId);
                        if (!dataChannelOpt.has_value())
                        {
                            errors.push_back("DataChannel with ShortId '" + std::string{ shortId } + "' not found");
                        }
                    });

                if (!dataChannelOpt.has_value())
                {
                    continue;
                }

                const auto& dataChannel = dataChannelOpt->get();

                // Validate value format
                Value parsedValue{ Value::String{ "" } };
                auto formatValidation =
                    dataChannel.property().format().validateValue(eventDataSet.value(), parsedValue);

                if (!formatValidation)
                {
                    std::string errorMsg =
                        formatValidation.errors().empty() ? "Unknown error" : formatValidation.errors()[0];
                    errors.push_back("DataChannel " + dataChannelId.toString() + " has invalid value: " + errorMsg);
                    continue;
                }

                // Custom validation callback
                auto customValidation =
                    onEventData(eventDataSet.timeStamp(), dataChannel, parsedValue, eventDataSet.quality());
                if (!customValidation)
                {
                    std::string errorMsg =
                        customValidation.errors().empty() ? "Unknown error" : customValidation.errors()[0];
                    errors.push_back("DataChannel " + dataChannelId.toString() + " validation failed: " + errorMsg);
                }
            }
        }

        if (!errors.empty())
        {
            return ValidateResult<>::invalid(errors);
        }

        return ValidateResult<>::ok();
    }
} // namespace dnv::vista::sdk::transport::timeseries
