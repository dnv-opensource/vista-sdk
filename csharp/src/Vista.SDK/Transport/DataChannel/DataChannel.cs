using System.Collections;
using System.Diagnostics.CodeAnalysis;
using Vista.SDK.Internal;

namespace Vista.SDK.Transport.DataChannel;

public sealed record DataChannelListPackage(Package Package);

public sealed record Package(Header Header, DataChannelList DataChannelList);

public sealed record Header(
    ShipId ShipId,
    ConfigurationReference DataChannelListId,
    VersionInformation? VersionInformation,
    string? Author,
    DateTimeOffset? DateCreated,
    IReadOnlyDictionary<string, object> CustomHeaders
);

public sealed record ConfigurationReference(string Id, string? Version, DateTimeOffset TimeStamp);

public sealed record VersionInformation(string NamingRule, string NamingSchemeVersion, string? ReferenceUrl);

public sealed record DataChannelList : IEnumerable<DataChannel>
{
    private ChdDictionary<DataChannel> _shortIdMap;
    private Dictionary<LocalId, DataChannel> _localIdMap;
    public IReadOnlyList<DataChannel> DataChannel;

    public DataChannelList(IReadOnlyList<DataChannel> dataChannel)
    {
        DataChannel = dataChannel;

        var shortIdMap = new Dictionary<string, DataChannel>();
        _localIdMap = new Dictionary<LocalId, DataChannel>();
        foreach (var dc in dataChannel)
        {
            if (dc.DataChannelId.ShortId is null)
                continue;
            shortIdMap.Add(dc.DataChannelId.ShortId, dc);
            _localIdMap.Add(dc.DataChannelId.LocalId, dc);
        }
        _shortIdMap = new ChdDictionary<DataChannel>(shortIdMap.Select((kvp) => (kvp.Key, kvp.Value)).ToArray());
    }

    public bool TryGetByShortId(string shortId, [MaybeNullWhen(false)] out DataChannel dataChannel) =>
        _shortIdMap.TryGetValue(shortId.AsSpan(), out dataChannel);

    public bool TryGetByLocalId(LocalId localId, [MaybeNullWhen(false)] out DataChannel dataChannel) =>
        _localIdMap.TryGetValue(localId, out dataChannel);

    public DataChannel this[string shortId] => _shortIdMap[shortId.AsSpan()];
    public DataChannel this[int index] => DataChannel[index];
    public DataChannel this[LocalId localId] => _localIdMap[localId];

    public IEnumerator<DataChannel> GetEnumerator()
    {
        return DataChannel.GetEnumerator();
    }

    IEnumerator IEnumerable.GetEnumerator()
    {
        return DataChannel.GetEnumerator();
    }
}

public sealed record DataChannel(DataChannelId DataChannelId, Property Property);

public sealed record DataChannelId(LocalId LocalId, string? ShortId, NameObject? NameObject);

public sealed record NameObject(string NamingRule, IReadOnlyDictionary<string, object> CustomProperties);

public sealed record DataChannelType(string Type, double? UpdateCycle, double? CalculationPeriod);

public sealed record Restriction(
    IReadOnlyList<string>? Enumeration,
    int? FractionDigits,
    int? Length,
    double? MaxExclusive,
    double? MaxInclusive,
    int? MaxLength,
    double? MinExclusive,
    double? MinInclusive,
    int? MinLength,
    string? Pattern,
    int? TotalDigits,
    WhiteSpace? WhiteSpace
);

public enum WhiteSpace
{
    Preserve = 0,
    Replace = 1,
    Collapse = 2,
}

public sealed record Format(string Type, Restriction? Restriction);

public sealed record Range(double High, double Low);

public sealed record Unit(
    string UnitSymbol,
    string? QuantityName,
    IReadOnlyDictionary<string, object> CustomProperties
);

public sealed record Property(
    DataChannelType DataChannelType,
    Format Format,
    Range? Range,
    Unit? Unit,
    string? QualityCoding,
    string? AlertPriority,
    string? Name,
    string? Remarks,
    IReadOnlyDictionary<string, object> CustomProperties
);
