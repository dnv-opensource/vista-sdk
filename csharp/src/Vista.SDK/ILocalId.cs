namespace Vista.SDK;

public interface ILocalId
{
    VisVersion VisVersion { get; }

    bool VerboseMode { get; }

    GmodPath PrimaryItem { get; }

    GmodPath? SecondaryItem { get; }

    bool HasCustomTag { get; }

    IReadOnlyList<MetadataTag> MetadataTags { get; }

    string ToString();
}

public interface ILocalId<T> : ILocalId, IEquatable<T>
    where T : ILocalId<T>
{
#if NET7_0_OR_GREATER
    static abstract T Parse(string localIdStr);
    static abstract bool TryParse(string localIdStr, out ParsingErrors errors, out T? localId);
#endif
}
