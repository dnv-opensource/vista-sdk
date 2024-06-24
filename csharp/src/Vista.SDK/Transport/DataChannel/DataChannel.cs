using System.Collections;
using System.Diagnostics.CodeAnalysis;
using System.Text.RegularExpressions;

namespace Vista.SDK.Transport.DataChannel;

public sealed record DataChannelListPackage
{
    public required Package Package { get; set; }

    public DataChannelList DataChannelList => Package.DataChannelList;
}

// Pacakge
public sealed record Package
{
    public required Header Header { get; set; }
    public required DataChannelList DataChannelList { get; set; }
}

// Header
public sealed record Header
{
    public required ShipId ShipId { get; set; }
    public required ConfigurationReference DataChannelListId { get; set; }
    public VersionInformation? VersionInformation { get; set; } = new();
    public required string? Author { get; set; }
    public DateTimeOffset? DateCreated { get; set; } = DateTimeOffset.UtcNow;
    public Dictionary<string, object>? CustomHeaders { get; set; }
}

public sealed record ConfigurationReference
{
    public required string Id { get; set; }
    public string? Version { get; set; }
    public required DateTimeOffset TimeStamp { get; set; }
}

public sealed record VersionInformation
{
    public static readonly string AnnexCNamingRule = "dnv";
    public static readonly string AnnexCNamingSchemeVersion = "v2";

    // Properties
    public string NamingRule { get; set; } = AnnexCNamingRule;
    public string NamingSchemeVersion { get; set; } = AnnexCNamingSchemeVersion;
    public string? ReferenceUrl { get; set; } = "https://docs.vista.dnv.com";
}

// DataChannelList
public sealed record DataChannelList() : ICollection<DataChannel>
{
    private List<DataChannel> dataChannels = new();
    private Dictionary<string, DataChannel> shortIdMap = new();
    private Dictionary<LocalId, DataChannel> localIdMap = new();

    public IReadOnlyList<DataChannel> DataChannels => dataChannels.AsReadOnly();

    public int Count => dataChannels.Count;

    public bool IsReadOnly => false;

    public bool TryGetByShortId(string shortId, [MaybeNullWhen(false)] out DataChannel dataChannel) =>
        shortIdMap.TryGetValue(shortId, out dataChannel);

    public bool TryGetByLocalId(LocalId localId, [MaybeNullWhen(false)] out DataChannel dataChannel) =>
        localIdMap.TryGetValue(localId, out dataChannel);

    public void Add(DataChannel dataChannel)
    {
        Add([dataChannel]);
    }

    public void Add(IEnumerable<DataChannel> dcs)
    {
        foreach (var dataChannel in dcs)
        {
            if (localIdMap.ContainsKey(dataChannel.DataChannelId.LocalId))
                throw new ArgumentException(
                    $"DataChannel with LocalId {dataChannel.DataChannelId.LocalId} already exists"
                );
            if (dataChannel.DataChannelId.ShortId is not null)
            {
                if (shortIdMap.ContainsKey(dataChannel.DataChannelId.ShortId))
                    throw new ArgumentException(
                        $"DataChannel with ShortId {dataChannel.DataChannelId.ShortId} already exists"
                    );
                shortIdMap.Add(dataChannel.DataChannelId.ShortId, dataChannel);
            }
            dataChannels.Add(dataChannel);
            localIdMap.Add(dataChannel.DataChannelId.LocalId, dataChannel);
        }
    }

    public void Clear()
    {
        dataChannels.Clear();
        shortIdMap.Clear();
        localIdMap.Clear();
    }

    public bool Contains(DataChannel item) => dataChannels.Contains(item);

    public void CopyTo(DataChannel[] array, int arrayIndex) => dataChannels.CopyTo(array, arrayIndex);

    public IEnumerator<DataChannel> GetEnumerator() => dataChannels.GetEnumerator();

    public bool Remove(DataChannel item)
    {
        if (!localIdMap.Remove(item.DataChannelId.LocalId))
            return false;
        if (item.DataChannelId.ShortId is not null && !shortIdMap.Remove(item.DataChannelId.ShortId))
            return false;
        return dataChannels.Remove(item);
    }

    IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

    public DataChannel this[string shortId] => shortIdMap[shortId];
    public DataChannel this[int index] => dataChannels[index];
    public DataChannel this[LocalId localId] => localIdMap[localId];
}

// DataChannel
public sealed record DataChannel
{
    private Property? property;

    public required DataChannelId DataChannelId { get; set; }
    public required Property Property
    {
        get => property ?? throw new InvalidOperationException("Property property not set");
        set
        {
            if (value.Validate() is ValidateResult.Invalid invalid)
                throw new ArgumentException($"Invalid property - Messages='[{string.Join(", ", invalid.Messages)}]'");
            property = value;
        }
    }
}

// DataChannelId
public sealed record DataChannelId
{
    public required LocalId LocalId { get; set; }
    public required string? ShortId { get; set; }
    public NameObject? NameObject { get; set; } = new();
}

public sealed record NameObject
{
    public static readonly string AnnexCNamingRule =
        $"/{VersionInformation.AnnexCNamingRule}-{VersionInformation.AnnexCNamingSchemeVersion}";
    public string NamingRule { get; set; } = AnnexCNamingRule;
    public Dictionary<string, object>? CustomNameObjects { get; set; }
}

// Property
public sealed class Property
{
    public required DataChannelType DataChannelType { get; set; }
    public required Format Format { get; set; }
    public required Range? Range { get; set; }
    public required Unit? Unit { get; set; }

    // TODO : Validate Quality coding 'In the case of “IEC 61162-STATUS”, “A” (Data valid) and “V” (Data invalid) are used for the data quality.'
    public string? QualityCoding { get; set; }

    // TODO : Validate - Priority level and criteria for classification shall be in accordance with IEC 62923-1:2018, 6.2.2.1
    public required string? AlertPriority { get; set; }
    public string? Name { get; set; }
    public string? Remarks { get; set; }

    public Dictionary<string, object>? CustomProperties { get; set; }

    public ValidateResult Validate()
    {
        var messages = new List<string>();

        if (Format.IsDecimal && Range is null)
            messages.Add("Range is required for Decimal format type");
        if (Format.IsDecimal && Unit is null)
            messages.Add("Unit is required for Decimal format type");
        if (DataChannelType.IsAlert && AlertPriority is null)
            messages.Add("AlertPriority is required for Alert DataChannelType");
        if (messages.Count > 0)
            return new ValidateResult.Invalid(messages.ToArray());
        return new ValidateResult.Ok();
    }
}

public sealed record DataChannelType
{
    private string? type;
    private double? updateCycle;
    private double? calculationPeriod;

    public required string Type
    {
        get => type ?? throw new InvalidOperationException("DataChannelType not set");
        set
        {
            var names = ISO19848.Instance.GetDataChannelTypeNames(ISO19848.LatestVersion);
            var result = names.Parse(value);
            if (result is not DataChannelTypeNames.ParseResult.Ok ok)
                throw new ArgumentException($"Invalid data channel type {type}");
            type = ok.TypeName.Type;
        }
    }
    public double? UpdateCycle
    {
        get => updateCycle;
        set
        {
            if (value < 0)
                throw new ArgumentException($"Invalid update cycle {value}. Should be positive");
            updateCycle = value;
        }
    }
    public double? CalculationPeriod
    {
        get => calculationPeriod;
        set
        {
            if (value < 0)
                throw new ArgumentException($"Invalid calculation period {value}. Should be positive");
            calculationPeriod = value;
        }
    }

    internal bool IsAlert => Type == "Alert";
}

public sealed record Format
{
    private string? type;
    private FormatDataType? dataType;

    public required string Type
    {
        get => type ?? throw new InvalidOperationException("Format type not set");
        set
        {
            var names = ISO19848.Instance.GetFormatDataTypes(ISO19848.LatestVersion);
            var result = names.Parse(value);
            if (result is not FormatDataTypes.ParseResult.Ok ok)
                throw new ArgumentException($"Invalid format type {type}");
            type = ok.TypeName.Type;
            dataType = ok.TypeName;
        }
    }
    public required Restriction? Restriction { get; set; } = null;

    internal FormatDataType DataType => dataType ?? throw new InvalidOperationException("Format type not set");
    internal bool IsDecimal => Type == "Decimal";

    public ValidateResult ValidateValue(string value)
    {
        if (DataType.Validate(value, out _) is ValidateResult.Invalid invalid)
            return invalid;
        if (Restriction is not null)
            return Restriction.ValidateValue(value, this);
        return new ValidateResult.Ok();
    }
}

public sealed record Restriction
{
    private uint? totalDigits;

    /// <summary>Defines a list of acceptable values.</summary>
    public IReadOnlyList<string>? Enumeration { get; set; }

    /// <summary>Specifies the maximum number of decimal places allowed. Shall be equal to or greater than zero.</summary>
    public uint? FractionDigits { get; set; }

    /// <summary>Specifies the exact number of characters or list items allowed. Shall be equal to or greater than zero.</summary>
    public uint? Length { get; set; }

    /// <summary>Specifies the upper bounds for numeric values (the value shall be less than this value).</summary>
    public double? MaxExclusive { get; set; }

    /// <summary>Specifies the upper bounds for numeric values (the value shall be less than or equal to this value).</summary>
    public double? MaxInclusive { get; set; }

    /// <summary>Specifies the maximum number of characters or list items allowed. Shall be equal to or greater than zero.</summary>
    public uint? MaxLength { get; set; }

    /// <summary>Specifies the lower bounds for numeric values (the value shall be greater than this value).</summary>
    public double? MinExclusive { get; set; }

    /// <summary>Specifies the lower bounds for numeric values (the value shall be greater than or equal to this value).</summary>
    public double? MinInclusive { get; set; }

    /// <summary>Specifies the minimum number of characters or list items allowed. Shall be equal to or greater than zero.</summary>
    public uint? MinLength { get; set; }

    /// <summary>Defines the exact sequence of characters that are acceptable.</summary>
    public string? Pattern { get; set; }

    /// <summary>Specifies the exact number of digits allowed. Shall be greater than zero.</summary>
    public uint? TotalDigits
    {
        get => totalDigits;
        set
        {
            if (value is not null && value <= 0)
                throw new ArgumentException("TotalDigits should be greater than zero");
            totalDigits = value;
        }
    }

    /// <summary>Specifies how white space (line feeds, tabs, spaces, and carriage returns) is handled.</summary>
    public WhiteSpace? WhiteSpace { get; set; }

    public ValidateResult ValidateValue(string value, Format format)
    {
        if (Enumeration is not null && !Enumeration.Contains(value, StringComparer.Ordinal))
            return new ValidateResult.Invalid([$"Value {value} is not in the enumeration"]);
        ValidateResult result = format
            .DataType
            .Match<ValidateResult>(
                value,
                dec =>
                {
                    if (FractionDigits is not null && CountDecimalPlaces(dec) > FractionDigits)
                        return new ValidateResult.Invalid(["Value has more decimal places than allowed"]);
                    if (ValidateNumber((double)dec) is ValidateResult.Invalid invalid)
                        return invalid;
                    return new ValidateResult.Ok();
                },
                i =>
                {
                    if (ValidateNumber(i) is ValidateResult.Invalid invalid)
                        return invalid;
                    if (TotalDigits is not null)
                    {
                        var numDigist = Math.Floor(Math.Log10(Math.Abs(i)) + 1) != TotalDigits;
                        return new ValidateResult.Invalid(
                            [$"Value {i} has {numDigist} digits but should be {TotalDigits}"]
                        );
                    }
                    return new ValidateResult.Ok();
                },
                b => new ValidateResult.Ok(),
                str =>
                {
                    var length = str.Length;
                    // check length
                    if (Length is not null && length != Length)
                        return new ValidateResult.Invalid([$"Value {str} has length {length} but should be {Length}"]);
                    // check max length
                    if (MaxLength is not null && length >= MaxLength)
                        return new ValidateResult.Invalid(
                            [$"Value {str} has length {length} but should be less than {MaxLength}"]
                        );
                    // check min length
                    if (MinLength is not null && length <= MinLength)
                        return new ValidateResult.Invalid(
                            [$"Value {str} has length {length} but should be greater than {MinLength}"]
                        );
                    // check pattern
                    if (Pattern is not null && !Regex.IsMatch(str, Pattern))
                        return new ValidateResult.Invalid([$"Value {str} does not match pattern {Pattern}"]);

                    return new ValidateResult.Ok();
                },
                date => new ValidateResult.Ok()
            );

        return result;
    }

    private ValidateResult ValidateNumber(double number)
    {
        // check max exclusive
        if (MaxExclusive is not null && number >= MaxExclusive)
            return new ValidateResult.Invalid([$"Value {number} is greater than or equal to {MaxExclusive}"]);
        // check max inclusive
        if (MaxInclusive is not null && number > MaxInclusive)
            return new ValidateResult.Invalid([$"Value {number} is greater than {MaxInclusive}"]);
        // check min exclusive
        if (MinExclusive is not null && number <= MinExclusive)
            return new ValidateResult.Invalid([$"Value {number} is less than or equal to {MinExclusive}"]);
        // check min inclusive
        if (MinInclusive is not null && number < MinInclusive)
            return new ValidateResult.Invalid([$"Value {number} is less than {MinInclusive}"]);
        return new ValidateResult.Ok();
    }

    private static decimal CountDecimalPlaces(decimal dec)
    {
        int[] bits = decimal.GetBits(dec);
        ulong lowInt = (uint)bits[0];
        ulong midInt = (uint)bits[1];
        int exponent = (bits[3] & 0x00FF0000) >> 16;
        int result = exponent;
        ulong lowDecimal = lowInt | (midInt << 32);
        while (result > 0 && (lowDecimal % 10) == 0)
        {
            result--;
            lowDecimal /= 10;
        }

        return result;
    }
}

public enum WhiteSpace
{
    Preserve = 0,
    Replace = 1,
    Collapse = 2,
}

public sealed record Range
{
    private double low = double.MinValue;
    private double high = double.MaxValue;

    public required double Low
    {
        get => low;
        set
        {
            if (value > High)
                throw new ArgumentException($"Low value {value} should be less than high value {High}");
            low = value;
        }
    }
    public required double High
    {
        get => high;
        set
        {
            if (value < Low)
                throw new ArgumentException($"High value {value} should be greater than low value {Low}");
            high = value;
        }
    }
}

// TODO : Validate Unit according to ISO 80000, IEC 80000 of Table 4
public sealed record Unit
{
    public required string UnitSymbol { get; set; }
    public string? QuantityName { get; set; }

    public Dictionary<string, object>? CustomElements { get; set; }
}
