namespace Vista.SDK.Experimental.Transport.DataList;

public sealed record DataListPackage(Package Package);

public sealed record Package(Header Header, DataList DataList);

public sealed record Header(
    AssetIdentifier AssetId,
    ConfigurationReference DataListId,
    VersionInformation? VersionInformation,
    string? Author,
    DateTimeOffset? DateCreated,
    IReadOnlyDictionary<string, object> CustomHeaders
);

public sealed record ConfigurationReference(string Id, string? Version, DateTimeOffset TimeStamp);

public sealed record VersionInformation(string NamingRule, string NamingSchemeVersion, string? ReferenceUrl);

public sealed record DataList(IReadOnlyList<Data> Data);

public sealed record Data(DataId DataId, Property Property);

public sealed record DataId(ILocalId LocalId, string? ShortId, NameObject? NameObject);

public sealed record NameObject(string NamingRule, IReadOnlyDictionary<string, object> CustomProperties);

public sealed record DataType(string Type, string? UpdateCycle, string? CalculationPeriod);

public sealed record Restriction(
    IReadOnlyList<string>? Enumeration,
    string? FractionDigits,
    string? Length,
    string? MaxExclusive,
    string? MaxInclusive,
    string? MaxLength,
    string? MinExclusive,
    string? MinInclusive,
    string? MinLength,
    string? Pattern,
    string? TotalDigits,
    WhiteSpace? WhiteSpace
);

public enum WhiteSpace
{
    Preserve = 0,
    Replace = 1,
    Collapse = 2,
}

public sealed record Format(string Type, Restriction? Restriction);

public sealed record Range(string High, string Low);

public sealed record Unit(
    string UnitSymbol,
    string? QuantityName,
    IReadOnlyDictionary<string, object> CustomProperties
);

public sealed record Property(
    DataType DataType,
    Format Format,
    Range? Range,
    Unit? Unit,
    string? QualityCoding,
    string? AlertPriority,
    string? Name,
    string? Remarks,
    IReadOnlyDictionary<string, object> CustomProperties
);
