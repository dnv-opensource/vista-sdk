using Vista.SDK.Internal;

namespace Vista.SDK;

public interface ILocalId<T> where T : ILocalId<T>
{
    VisVersion VisVersion { get; }

    bool VerboseMode { get; }

    GmodPath PrimaryItem { get; }

    GmodPath? SecondaryItem { get; }

    bool HasCustomTag { get; }
    IReadOnlyList<MetadataTag> MetadataTags { get; }

#if NET7_0_OR_GREATER

    static abstract T Parse(string localIdStr);
    static abstract T Parse(string localIdStr, out LocalIdParsingErrorBuilder errorBuilder);
#endif
    string ToString();
}
