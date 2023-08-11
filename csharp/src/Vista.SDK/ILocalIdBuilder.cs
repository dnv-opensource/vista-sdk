using System.Diagnostics.CodeAnalysis;
using System.Text;
using Vista.SDK.Internal;

namespace Vista.SDK;

public interface ILocalIdBuilder
{
    VisVersion? VisVersion { get; }

    bool VerboseMode { get; }

    GmodPath? PrimaryItem { get; }

    GmodPath? SecondaryItem { get; }

    bool HasCustomTag { get; }

    IReadOnlyList<MetadataTag> MetadataTags { get; }

    bool IsValid { get; }

    bool IsEmpty { get; }

    string ToString();
}

public interface ILocalIdBuilder<TBuilder, TResult> : ILocalIdBuilder
    where TBuilder : ILocalIdBuilder<TBuilder, TResult>
    where TResult : ILocalId<TResult>
{
    TBuilder WithVisVersion(in string visVersion);
    TBuilder WithVisVersion(in VisVersion version);
    TBuilder TryWithVisVersion(in VisVersion? version);
    TBuilder TryWithVisVersion(in string? visVersionStr, out bool succeeded);
    TBuilder WithoutVisVersion();

    TBuilder WithVerboseMode(in bool verboseMode);

    TBuilder WithPrimaryItem(in GmodPath item);
    TBuilder TryWithPrimaryItem(in GmodPath? item);
    TBuilder TryWithPrimaryItem(in GmodPath? item, out bool succeeded);
    TBuilder WithoutPrimaryItem();

    TBuilder WithSecondaryItem(in GmodPath item);
    TBuilder TryWithSecondaryItem(in GmodPath? item);
    TBuilder TryWithSecondaryItem(in GmodPath? item, out bool succeeded);
    TBuilder WithoutSecondaryItem();

    TBuilder WithMetadataTag(in MetadataTag metadataTag);
    TBuilder TryWithMetadataTag(in MetadataTag? metadataTag);
    TBuilder TryWithMetadataTag(in MetadataTag? metadataTag, out bool succeeded);
    TBuilder WithoutMetadataTag(in CodebookName name);

    TResult Build();

#if NET7_0_OR_GREATER
    static abstract TBuilder Parse(string localIdStr);

    static abstract bool TryParse(string localIdStr, [MaybeNullWhen(false)] out TBuilder localId);

    static abstract bool TryParse(
        string localIdStr,
        out ParsingErrors errors,
        [MaybeNullWhen(false)] out TBuilder localId
    );
#endif
}
