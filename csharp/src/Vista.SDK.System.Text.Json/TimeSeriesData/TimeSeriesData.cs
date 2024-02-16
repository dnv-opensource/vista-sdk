using System.Text.Json.Serialization;

namespace Vista.SDK.Transport.Json.TimeSeriesData;

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
        string? @author,
        DateTimeOffset? @dateCreated,
        DateTimeOffset? @dateModified,
        string @shipID,
        IReadOnlyList<ConfigurationReference>? @systemConfiguration,
        TimeSpan? @timeSpan
    )
    {
        this.ShipID = @shipID;

        this.TimeSpan = @timeSpan;

        this.DateCreated = @dateCreated;

        this.DateModified = @dateModified;

        this.Author = @author;

        this.SystemConfiguration = @systemConfiguration;
    }

    [JsonPropertyName("ShipID")]
    public string ShipID { get; }

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

public sealed class TabularData
{
    [JsonConstructor]
    public TabularData(
        IReadOnlyList<string>? @dataChannelID,
        IReadOnlyList<DataSet_Tabular>? @dataSet,
        int? @numberOfDataChannel,
        int? @numberOfDataSet
    )
    {
        this.NumberOfDataSet = @numberOfDataSet;

        this.NumberOfDataChannel = @numberOfDataChannel;

        this.DataChannelID = @dataChannelID;

        this.DataSet = @dataSet;
    }

    [JsonPropertyName("NumberOfDataSet")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public int? NumberOfDataSet { get; }

    [JsonPropertyName("NumberOfDataChannel")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public int? NumberOfDataChannel { get; }

    [JsonPropertyName("DataChannelID")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public IReadOnlyList<string>? DataChannelID { get; }

    [JsonPropertyName("DataSet")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public IReadOnlyList<DataSet_Tabular>? DataSet { get; }
}

public sealed class EventData
{
    [JsonConstructor]
    public EventData(IReadOnlyList<DataSet_Event>? @dataSet, int? @numberOfDataSet)
    {
        this.NumberOfDataSet = @numberOfDataSet;

        this.DataSet = @dataSet;
    }

    [JsonPropertyName("NumberOfDataSet")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public int? NumberOfDataSet { get; }

    [JsonPropertyName("DataSet")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public IReadOnlyList<DataSet_Event>? DataSet { get; }
}

public sealed class DataSet_Tabular
{
    [JsonConstructor]
    public DataSet_Tabular(IReadOnlyList<string>? @quality, DateTimeOffset @timeStamp, IReadOnlyList<string> @value)
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

public sealed class DataSet_Event
{
    [JsonConstructor]
    public DataSet_Event(string @dataChannelID, string? @quality, DateTimeOffset @timeStamp, string @value)
    {
        this.TimeStamp = @timeStamp;

        this.DataChannelID = @dataChannelID;

        this.Value = @value;

        this.Quality = @quality;
    }

    [JsonPropertyName("TimeStamp")]
    public DateTimeOffset TimeStamp { get; }

    [JsonPropertyName("DataChannelID")]
    public string DataChannelID { get; }

    [JsonPropertyName("Value")]
    public string Value { get; }

    [JsonPropertyName("Quality")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public string? Quality { get; }
}

/// <summary>
/// A TimeSeriesData package for ISO19848
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
