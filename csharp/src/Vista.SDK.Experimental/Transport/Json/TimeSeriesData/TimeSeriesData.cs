using System.Text.Json.Serialization;

namespace Vista.SDK.Experimental.Transport.Json.TimeSeriesData;

public sealed class Package
{
    [JsonConstructor]
    public Package(Header? @header, IReadOnlyList<TimeSeriesData> @timeSeriesData)
    {
        this.Header = @header;

        this.TimeSeriesData = @timeSeriesData;
    }

    [JsonPropertyName("Header")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public Header? Header { get; }

    [JsonPropertyName("TimeSeriesData")]
    public IReadOnlyList<TimeSeriesData> TimeSeriesData { get; }
}

public sealed class Header
{
    [JsonConstructor]
    public Header(
        string @assetId,
        string? @author,
        DateTimeOffset? @dateCreated,
        DateTimeOffset? @dateModified,
        IReadOnlyList<ConfigurationReference>? @systemConfiguration,
        TimeSpan? @timeSpan
    )
    {
        this.AssetId = @assetId;

        this.TimeSpan = @timeSpan;

        this.DateCreated = @dateCreated;

        this.DateModified = @dateModified;

        this.Author = @author;

        this.SystemConfiguration = @systemConfiguration;
    }

    [JsonPropertyName("AssetId")]
    public string AssetId { get; }

    [JsonPropertyName("TimeSpan")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public TimeSpan? TimeSpan { get; }

    [JsonPropertyName("DateCreated")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public DateTimeOffset? DateCreated { get; }

    [JsonPropertyName("DateModified")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public DateTimeOffset? DateModified { get; }

    [JsonPropertyName("Author")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public string? Author { get; }

    [JsonPropertyName("SystemConfiguration")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public IReadOnlyList<ConfigurationReference>? SystemConfiguration { get; }

    private IDictionary<string, object>? _customHeaders;

    [JsonExtensionData]
    public IDictionary<string, object> CustomHeaders
    {
        get { return _customHeaders ?? (_customHeaders = new Dictionary<string, object>()); }
        set { _customHeaders = value; }
    }
}

public sealed class TimeSeriesData
{
    [JsonConstructor]
    public TimeSeriesData(
        ConfigurationReference? @dataConfiguration,
        EventData? @eventData,
        IReadOnlyList<TabularData>? @tabularData
    )
    {
        this.DataConfiguration = @dataConfiguration;

        this.TabularData = @tabularData;

        this.EventData = @eventData;
    }

    [JsonPropertyName("DataConfiguration")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public ConfigurationReference? DataConfiguration { get; }

    [JsonPropertyName("TabularData")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public IReadOnlyList<TabularData>? TabularData { get; }

    [JsonPropertyName("EventData")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public EventData? EventData { get; }

    private IDictionary<string, object>? _customData;

    [JsonExtensionData]
    public IDictionary<string, object> CustomData
    {
        get { return _customData ?? (_customData = new Dictionary<string, object>()); }
        set { _customData = value; }
    }
}

public sealed class TimeSpan
{
    [JsonConstructor]
    public TimeSpan(DateTimeOffset @end, DateTimeOffset @start)
    {
        this.Start = @start;

        this.End = @end;
    }

    [JsonPropertyName("Start")]
    public DateTimeOffset Start { get; }

    [JsonPropertyName("End")]
    public DateTimeOffset End { get; }
}

public sealed class ConfigurationReference
{
    [JsonConstructor]
    public ConfigurationReference(string @iD, DateTimeOffset @timeStamp)
    {
        this.ID = @iD;

        this.TimeStamp = @timeStamp;
    }

    [JsonPropertyName("ID")]
    public string ID { get; }

    [JsonPropertyName("TimeStamp")]
    public DateTimeOffset TimeStamp { get; }
}

public sealed class TabularData
{
    [JsonConstructor]
    public TabularData(
        IReadOnlyList<string>? @dataId,
        IReadOnlyList<TabularDataSet>? @dataSet,
        int? @numberOfDataPoints,
        int? @numberOfDataSet
    )
    {
        this.NumberOfDataSet = @numberOfDataSet;

        this.NumberOfDataPoints = @numberOfDataPoints;

        this.DataId = @dataId;

        this.DataSet = @dataSet;
    }

    [JsonPropertyName("NumberOfDataSet")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public int? NumberOfDataSet { get; }

    [JsonPropertyName("NumberOfDataPoints")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public int? NumberOfDataPoints { get; }

    [JsonPropertyName("DataId")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public IReadOnlyList<string>? DataId { get; }

    [JsonPropertyName("DataSet")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public IReadOnlyList<TabularDataSet>? DataSet { get; }
}

public sealed class TabularDataSet
{
    [JsonConstructor]
    public TabularDataSet(IReadOnlyList<string>? @quality, DateTimeOffset @timeStamp, IReadOnlyList<string> @value)
    {
        this.TimeStamp = @timeStamp;

        this.Value = @value;

        this.Quality = @quality;
    }

    [JsonPropertyName("TimeStamp")]
    public DateTimeOffset TimeStamp { get; }

    [JsonPropertyName("Value")]
    public IReadOnlyList<string> Value { get; }

    [JsonPropertyName("Quality")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public IReadOnlyList<string>? Quality { get; }
}

public sealed class EventData
{
    [JsonConstructor]
    public EventData(IReadOnlyList<EventDataSet>? @dataSet, int? @numberOfDataSet)
    {
        this.NumberOfDataSet = @numberOfDataSet;

        this.DataSet = @dataSet;
    }

    [JsonPropertyName("NumberOfDataSet")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public int? NumberOfDataSet { get; }

    [JsonPropertyName("DataSet")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public IReadOnlyList<EventDataSet>? DataSet { get; }
}

public sealed class EventDataSet
{
    [JsonConstructor]
    public EventDataSet(string @dataId, string? @quality, DateTimeOffset @timeStamp, string @value)
    {
        this.TimeStamp = @timeStamp;

        this.DataId = @dataId;

        this.Value = @value;

        this.Quality = @quality;
    }

    [JsonPropertyName("TimeStamp")]
    public DateTimeOffset TimeStamp { get; }

    [JsonPropertyName("DataId")]
    public string DataId { get; }

    [JsonPropertyName("Value")]
    public string Value { get; }

    [JsonPropertyName("Quality")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public string? Quality { get; }
}

/// <summary>
/// An experimental generalized TimeSeriesData package based on ISO19848
/// </summary>
public sealed class TimeSeriesDataPackage
{
    [JsonConstructor]
    public TimeSeriesDataPackage(Package @package)
    {
        this.Package = @package;
    }

    [JsonPropertyName("Package")]
    public Package Package { get; }
}
