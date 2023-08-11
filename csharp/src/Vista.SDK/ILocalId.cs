using Vista.SDK.Internal;

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

public interface ILocalId<T> : ILocalId
    where T : ILocalId<T>
{
#if NET7_0_OR_GREATER
    static abstract T Parse(string localIdStr);
    static abstract T Parse(string localIdStr, out ParsingErrors errors);
#endif
}
