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

public sealed record DataType(string Type, double? UpdateCycle, double? CalculationPeriod);

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
