using System.Text.Json.Serialization;

namespace Vista.SDK.Experimental.Transport.Json.DataList;

public sealed class Package
{
    [JsonConstructor]
    public Package(DataList @dataList, Header @header)
    {
        this.Header = @header;

        this.DataList = @dataList;
    }

    [JsonPropertyName("Header")]
    public Header Header { get; }

    [JsonPropertyName("DataList")]
    public DataList DataList { get; }
}

public sealed class Header
{
    [JsonConstructor]
    public Header(
        string @assetId,
        string? @author,
        ConfigurationReference @dataListID,
        DateTimeOffset? @dateCreated,
        VersionInformation? @versionInformation
    )
    {
        this.AssetId = @assetId;

        this.DataListID = @dataListID;

        this.VersionInformation = @versionInformation;

        this.Author = @author;

        this.DateCreated = @dateCreated;
    }

    [JsonPropertyName("AssetId")]
    public string AssetId { get; }

    [JsonPropertyName("DataListID")]
    public ConfigurationReference DataListID { get; }

    [JsonPropertyName("VersionInformation")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public VersionInformation? VersionInformation { get; }

    [JsonPropertyName("Author")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public string? Author { get; }

    [JsonPropertyName("DateCreated")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public DateTimeOffset? DateCreated { get; }

    private IDictionary<string, object>? _customHeaders;

    [JsonExtensionData]
    public IDictionary<string, object> CustomHeaders
    {
        get { return _customHeaders ?? (_customHeaders = new Dictionary<string, object>()); }
        set { _customHeaders = value; }
    }
}

public sealed class VersionInformation
{
    [JsonConstructor]
    public VersionInformation(string @namingRule, string @namingSchemeVersion, string? @referenceURL)
    {
        this.NamingRule = @namingRule;

        this.NamingSchemeVersion = @namingSchemeVersion;

        this.ReferenceURL = @referenceURL;
    }

    [JsonPropertyName("NamingRule")]
    public string NamingRule { get; }

    [JsonPropertyName("NamingSchemeVersion")]
    public string NamingSchemeVersion { get; }

    [JsonPropertyName("ReferenceURL")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public string? ReferenceURL { get; }
}

public sealed class DataList
{
    [JsonConstructor]
    public DataList(IReadOnlyList<Data> @data)
    {
        this.Data = @data;
    }

    [JsonPropertyName("Data")]
    public IReadOnlyList<Data> Data { get; }
}

public sealed class Data
{
    [JsonConstructor]
    public Data(DataID @dataID, Property @property)
    {
        this.DataID = @dataID;

        this.Property = @property;
    }

    [JsonPropertyName("DataID")]
    public DataID DataID { get; }

    [JsonPropertyName("Property")]
    public Property Property { get; }
}

public sealed class DataID
{
    [JsonConstructor]
    public DataID(string @localID, NameObject? @nameObject, string? @shortID)
    {
        this.LocalID = @localID;

        this.ShortID = @shortID;

        this.NameObject = @nameObject;
    }

    [JsonPropertyName("LocalID")]
    public string LocalID { get; }

    [JsonPropertyName("ShortID")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public string? ShortID { get; }

    [JsonPropertyName("NameObject")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public NameObject? NameObject { get; }
}

public sealed class NameObject
{
    [JsonConstructor]
    public NameObject(string @namingRule)
    {
        this.NamingRule = @namingRule;
    }

    [JsonPropertyName("NamingRule")]
    public string NamingRule { get; }

    private IDictionary<string, object>? _customProperties;

    [JsonExtensionData]
    public IDictionary<string, object> CustomProperties
    {
        get { return _customProperties ?? (_customProperties = new Dictionary<string, object>()); }
        set { _customProperties = value; }
    }
}

public sealed class Property
{
    [JsonConstructor]
    public Property(
        string? @alertPriority,
        DataType @dataType,
        Format @format,
        string? @name,
        string? @qualityCoding,
        Range? @range,
        string? @remarks,
        Unit? @unit
    )
    {
        this.DataType = @dataType;

        this.Format = @format;

        this.Range = @range;

        this.Unit = @unit;

        this.QualityCoding = @qualityCoding;

        this.AlertPriority = @alertPriority;

        this.Name = @name;

        this.Remarks = @remarks;
    }

    [JsonPropertyName("DataType")]
    public DataType DataType { get; }

    [JsonPropertyName("Format")]
    public Format Format { get; }

    [JsonPropertyName("Range")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public Range? Range { get; }

    [JsonPropertyName("Unit")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public Unit? Unit { get; }

    [JsonPropertyName("QualityCoding")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public string? QualityCoding { get; }

    [JsonPropertyName("AlertPriority")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public string? AlertPriority { get; }

    [JsonPropertyName("Name")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public string? Name { get; }

    [JsonPropertyName("Remarks")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public string? Remarks { get; }

    private IDictionary<string, object>? _customProperties;

    [JsonExtensionData]
    public IDictionary<string, object> CustomProperties
    {
        get { return _customProperties ?? (_customProperties = new Dictionary<string, object>()); }
        set { _customProperties = value; }
    }
}

public sealed class DataType
{
    [JsonConstructor]
    public DataType(double? @calculationPeriod, string @type, double? @updateCycle)
    {
        this.Type = @type;

        this.UpdateCycle = @updateCycle;

        this.CalculationPeriod = @calculationPeriod;
    }

    [JsonPropertyName("Type")]
    public string Type { get; }

    [JsonPropertyName("UpdateCycle")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public double? UpdateCycle { get; }

    [JsonPropertyName("CalculationPeriod")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public double? CalculationPeriod { get; }
}

public sealed class Format
{
    [JsonConstructor]
    public Format(Restriction? @restriction, string @type)
    {
        this.Type = @type;

        this.Restriction = @restriction;
    }

    [JsonPropertyName("Type")]
    public string Type { get; }

    [JsonPropertyName("Restriction")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public Restriction? Restriction { get; }
}

public sealed class Range
{
    [JsonConstructor]
    public Range(double @high, double @low)
    {
        this.High = @high;

        this.Low = @low;
    }

    [JsonPropertyName("High")]
    public double High { get; }

    [JsonPropertyName("Low")]
    public double Low { get; }
}

public sealed class Unit
{
    [JsonConstructor]
    public Unit(string? @quantityName, string @unitSymbol)
    {
        this.UnitSymbol = @unitSymbol;

        this.QuantityName = @quantityName;
    }

    [JsonPropertyName("UnitSymbol")]
    public string UnitSymbol { get; }

    [JsonPropertyName("QuantityName")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public string? QuantityName { get; }

    private IDictionary<string, object>? _customProperties;

    [JsonExtensionData]
    public IDictionary<string, object> CustomProperties
    {
        get { return _customProperties ?? (_customProperties = new Dictionary<string, object>()); }
        set { _customProperties = value; }
    }
}

public sealed class Restriction
{
    [JsonConstructor]
    public Restriction(
        IReadOnlyList<string>? @enumeration,
        int? @fractionDigits,
        int? @length,
        double? @maxExclusive,
        double? @maxInclusive,
        int? @maxLength,
        double? @minExclusive,
        double? @minInclusive,
        int? @minLength,
        string? @pattern,
        int? @totalDigits,
        RestrictionWhiteSpace? @whiteSpace
    )
    {
        this.Enumeration = @enumeration;

        this.FractionDigits = @fractionDigits;

        this.Length = @length;

        this.MaxExclusive = @maxExclusive;

        this.MaxInclusive = @maxInclusive;

        this.MaxLength = @maxLength;

        this.MinExclusive = @minExclusive;

        this.MinInclusive = @minInclusive;

        this.MinLength = @minLength;

        this.Pattern = @pattern;

        this.TotalDigits = @totalDigits;

        this.WhiteSpace = @whiteSpace;
    }

    [JsonPropertyName("Enumeration")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public IReadOnlyList<string>? Enumeration { get; }

    [JsonPropertyName("FractionDigits")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public int? FractionDigits { get; }

    [JsonPropertyName("Length")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public int? Length { get; }

    [JsonPropertyName("MaxExclusive")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public double? MaxExclusive { get; }

    [JsonPropertyName("MaxInclusive")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public double? MaxInclusive { get; }

    [JsonPropertyName("MaxLength")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public int? MaxLength { get; }

    [JsonPropertyName("MinExclusive")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public double? MinExclusive { get; }

    [JsonPropertyName("MinInclusive")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public double? MinInclusive { get; }

    [JsonPropertyName("MinLength")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public int? MinLength { get; }

    [JsonPropertyName("Pattern")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public string? Pattern { get; }

    [JsonPropertyName("TotalDigits")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public int? TotalDigits { get; }

    [JsonPropertyName("WhiteSpace")]
#if NET8_0_OR_GREATER
    [JsonConverter(typeof(JsonStringEnumConverter<RestrictionWhiteSpace>))]
#else
    [JsonConverter(typeof(JsonStringEnumConverter))]
#endif
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public RestrictionWhiteSpace? WhiteSpace { get; }
}

public sealed class ConfigurationReference
{
    [JsonConstructor]
    public ConfigurationReference(string @iD, DateTimeOffset @timeStamp, string? @version)
    {
        this.ID = @iD;

        this.Version = @version;

        this.TimeStamp = @timeStamp;
    }

    [JsonPropertyName("ID")]
    public string ID { get; }

    [JsonPropertyName("Version")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public string? Version { get; }

    [JsonPropertyName("TimeStamp")]
    public DateTimeOffset TimeStamp { get; }
}

/// <summary>
/// An experimental generalized DataList package based on ISO19848
/// </summary>

public sealed class DataListPackage
{
    [JsonConstructor]
    public DataListPackage(Package @package)
    {
        this.Package = @package;
    }

    [JsonPropertyName("Package")]
    public Package Package { get; }
}

public enum RestrictionWhiteSpace
{
    [System.Runtime.Serialization.EnumMember(Value = @"Preserve")]
    Preserve = 0,

    [System.Runtime.Serialization.EnumMember(Value = @"Replace")]
    Replace = 1,

    [System.Runtime.Serialization.EnumMember(Value = @"Collapse")]
    Collapse = 2,
}
