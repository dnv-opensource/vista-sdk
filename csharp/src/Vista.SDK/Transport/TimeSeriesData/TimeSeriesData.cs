using Vista.SDK.Transport.DataChannel;

namespace Vista.SDK.Transport.TimeSeries;

public sealed record TimeSeriesDataPackage
{
    public required Package Package { get; set; }
}

public sealed record Package
{
    public required Header? Header { get; set; }
    public required List<TimeSeriesData> TimeSeriesData { get; set; }
}

public sealed record Header
{
    public required ShipId ShipId { get; set; }
    public required TimeSpan? TimeSpan { get; set; }
    public DateTimeOffset? DateCreated { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? DateModified { get; set; }
    public required string? Author { get; set; }
    public List<ConfigurationReference>? SystemConfiguration { get; set; }
    public Dictionary<string, object>? CustomHeaders { get; set; }
}

public sealed record TimeSpan
{
    public required DateTimeOffset Start { get; set; }
    public required DateTimeOffset End { get; set; }
}

public sealed record ConfigurationReference
{
    public required string Id { get; set; }
    public required DateTimeOffset TimeStamp { get; set; }
}

public sealed record TimeSeriesData
{
    public required ConfigurationReference? DataConfiguration { get; set; }
    public required List<TabularData>? TabularData { get; set; }
    public required EventData? EventData { get; set; }
    public Dictionary<string, object>? CustomDataKinds { get; set; }

    public ValidateResult Validate(
        DataChannelListPackage dcPackage,
        ValidateData onTabularData,
        ValidateData onEventData
    )
    {
        if (DataConfiguration is not null)
        {
            if (dcPackage.Package.Header.DataChannelListId.Id != DataConfiguration.Id)
            {
                return new ValidateResult.Invalid(["DataConfiguration Id does not match DataChannelList Id"]);
            }
        }
        if ((TabularData is null || TabularData.Count == 0) && (EventData is null || EventData.DataSet?.Count == 0))
            return new ValidateResult.Invalid(["Can't ingest timeseries data without data"]);

        var errorneousDataChannels = new List<(DataChannelId DataChannelId, string Cause)>();
        // Validate Tabluar data
        foreach (var table in TabularData ?? [])
        {
            if (table is null || table.DataSets is null || table.DataChannelIds is null)
                continue;
            if (table.DataSets.Count != table.NumberOfDataSets)
                return new ValidateResult.Invalid(

                    [
                        $"Tabular data expects {table.NumberOfDataSets} datasets, but {table.DataSets.Count} sets are provided"
                    ]
                );
            var result = TimeSeries.TabularData.Validate(table);
            if (result is not ValidateResult.Ok)
                return result;

            for (var i = 0; i < table.DataSets.Count; i++)
            {
                var dataset = table.DataSets[i];
                if (dataset.Value.Count != table.NumberOfDataChannels)
                    return new ValidateResult.Invalid(

                        [
                            $"Tabular data set {i} expects {table.DataChannelIds.Count} values, but {dataset.Value.Count} values are provided"
                        ]
                    );

                for (var j = 0; j < table.NumberOfDataChannels; j++)
                {
                    var dataChannelId = table.DataChannelIds[j];
                    DataChannel.DataChannel? dataChannel = dataChannelId.Match(
                        onLocalId: id =>
                        {
                            if (!dcPackage.Package.DataChannelList.TryGetByLocalId(id, out var dc) || dc is null)
                            {
                                errorneousDataChannels.Add(
                                    (dataChannelId, $"Data channel with localId '{id}' not found")
                                );
                                return null;
                            }
                            return dc;
                        },
                        onShortId: shortId =>
                        {
                            if (!dcPackage.Package.DataChannelList.TryGetByShortId(shortId, out var dc) || dc is null)
                            {
                                errorneousDataChannels.Add(
                                    (dataChannelId, $"Data channel with short id '{shortId}' not found")
                                );
                                return null;
                            }
                            return dc;
                        }
                    );
                    if (dataChannel is null)
                        continue;

                    var typeValidation = dataChannel
                        .Property
                        .Format
                        .ValidateValue(dataset.Value[j], out var parsedValue);

                    if (typeValidation is ValidateResult.Invalid invalid)
                    {
                        errorneousDataChannels.Add((dataChannelId, string.Join(", ", invalid.Messages)));
                        continue;
                    }

                    result = onTabularData(dataset.TimeStamp, dataChannel, parsedValue, dataset.Quality?[j]);

                    if (result is not ValidateResult.Ok)
                    {
                        errorneousDataChannels.Add((dataChannelId, result.ToString()));
                        continue;
                    }
                }
            }

            // Validate event data
            if (EventData is not null)
            {
                foreach (var eventData in EventData.DataSet ?? [])
                {
                    DataChannel.DataChannel? dataChannel = eventData
                        .DataChannelId
                        .Match(
                            onLocalId: id =>
                            {
                                if (!dcPackage.Package.DataChannelList.TryGetByLocalId(id, out var dc) || dc is null)
                                {
                                    errorneousDataChannels.Add(
                                        (eventData.DataChannelId, $"Data channel with localId '{id}' not found")
                                    );
                                    return null;
                                }
                                return dc;
                            },
                            onShortId: shortId =>
                            {
                                if (
                                    !dcPackage.Package.DataChannelList.TryGetByShortId(shortId, out var dc)
                                    || dc is null
                                )
                                {
                                    errorneousDataChannels.Add(
                                        (eventData.DataChannelId, $"Data channel with short id '{shortId}' not found")
                                    );
                                    return null;
                                }
                                return dc;
                            }
                        );
                    if (dataChannel is null)
                        continue;

                    var typeValidation = dataChannel
                        .Property
                        .Format
                        .ValidateValue(eventData.Value, out var parsedValue);
                    if (typeValidation is ValidateResult.Invalid invalid)
                    {
                        errorneousDataChannels.Add((eventData.DataChannelId, string.Join(", ", invalid.Messages)));
                        continue;
                    }

                    result = onEventData(eventData.TimeStamp, dataChannel, parsedValue, eventData.Quality);

                    if (result is not ValidateResult.Ok)
                    {
                        errorneousDataChannels.Add((eventData.DataChannelId, result.ToString()));
                        continue;
                    }
                }
            }
        }

        if (errorneousDataChannels.Count > 0)
        {
            return new ValidateResult.Invalid(
                errorneousDataChannels.Select(x => $"DataChannel {x.DataChannelId} is invalid: {x.Cause}").ToArray()
            );
        }

        return new ValidateResult.Ok();
    }
}

public sealed record TabularData(int dataChannels = 0, int dataSets = 0)
{
    public required List<DataChannelId>? DataChannelIds { get; set; } = new(dataChannels);
    public required List<TabularDataSet>? DataSets { get; set; } = new(dataSets);

    public int? NumberOfDataSets => DataSets?.Count;
    public int? NumberOfDataChannels => DataChannelIds?.Count;

    public static ValidateResult Validate(TabularData table)
    {
        // Ensure data channels are provided
        if (table.DataChannelIds is null || table.DataChannelIds.Count == 0)
            return new ValidateResult.Invalid(["Tabular data has no data channels"]);
        if (table.NumberOfDataChannels is not null && table.NumberOfDataChannels != table.DataChannelIds?.Count)
            return new ValidateResult.Invalid(

                [
                    $"Tabular data has {table.NumberOfDataChannels} data channels, but {table.DataChannelIds?.Count} data channels are provided"
                ]
            );
        // Ensure data sets are provided
        if (table.DataSets is null || table.DataSets.Count == 0)
            return new ValidateResult.Invalid(["Tabular data has no data"]);

        if (table.NumberOfDataSets is not null && table.NumberOfDataSets != table.DataSets?.Count)
            return new ValidateResult.Invalid(

                [
                    $"Tabular data has {table.NumberOfDataSets} data sets, but {table.DataSets?.Count} data sets are provided"
                ]
            );

        return new ValidateResult.Ok();
    }
}

public sealed record EventData(int dataSet = 0)
{
    public required List<EventDataSet>? DataSet { get; set; } = new(dataSet);

    public int? NumberOfDataSet => DataSet?.Count;
}

public sealed record TabularDataSet(int capacity = 0)
{
    public required DateTimeOffset TimeStamp { get; set; }
    public required List<string> Value { get; set; } = new(capacity);
    public required List<string>? Quality { get; set; } = null;
}

public sealed record EventDataSet
{
    public required DateTimeOffset TimeStamp { get; set; }
    public required DataChannelId DataChannelId { get; set; }
    public required string Value { get; set; }
    public required string? Quality { get; set; }
}

// Custom definitions
public delegate ValidateResult ValidateData(
    DateTimeOffset timeStamp,
    DataChannel.DataChannel dataChannel,
    Value value,
    string? quality
);
