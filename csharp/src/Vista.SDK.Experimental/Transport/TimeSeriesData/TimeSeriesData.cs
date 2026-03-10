namespace Vista.SDK.Experimental.Transport.TimeSeries;

public sealed record TimeSeriesDataPackage(Package Package);

public sealed record Package(Header? Header, IReadOnlyList<TimeSeriesData> TimeSeriesData);

public sealed record Header(
    AssetIdentifier AssetId,
    TimeSpan? TimeSpan,
    DateTimeOffset? DateCreated,
    DateTimeOffset? DateModified,
    string? Author,
    IReadOnlyList<ConfigurationReference>? SystemConfiguration,
    IReadOnlyDictionary<string, object> CustomHeaders
);

public sealed record ConfigurationReference(string Id, DateTimeOffset TimeStamp);

public sealed record TimeSpan(DateTimeOffset Start, DateTimeOffset End);

public sealed record TimeSeriesData(
    ConfigurationReference? DataConfiguration,
    IReadOnlyList<TabularData>? TabularData,
    EventData? EventData,
    IReadOnlyDictionary<string, object> CustomProperties
);

public sealed record TabularData(
    int? NumberOfDataSet,
    int? NumberOfDataPoints,
    IReadOnlyList<DataId>? DataId,
    IReadOnlyList<TabularDataSet>? DataSet
);

public sealed record TabularDataSet(
    DateTimeOffset TimeStamp,
    IReadOnlyList<string> Value,
    IReadOnlyList<string>? Quality
);

public sealed record EventData(int? NumberOfDataSet, IReadOnlyList<EventDataSet>? DataSet);

public sealed record EventDataSet(DateTimeOffset TimeStamp, DataId DataId, string Value, string? Quality);
