namespace Vista.SDK;

public interface ILocalIdBuilder
{
    VisVersion? VisVersion { get; }

    bool VerboseMode { get; }

    GmodPath? PrimaryItem { get; }

    GmodPath? SecondaryItem { get; }

    MetadataTag? Quantity { get; }

    MetadataTag? Content { get; }

    MetadataTag? Calculation { get; }

    MetadataTag? State { get; }

    MetadataTag? Command { get; }

    MetadataTag? Type { get; }

    MetadataTag? Position { get; }

    MetadataTag? Detail { get; }

    LocalIdBuilder WithVisVersion(string visVersion);

    LocalIdBuilder WithVisVersion(VisVersion version);

    LocalIdBuilder WithVerboseMode(bool verboseMode);

    LocalIdBuilder WithPrimaryItem(GmodPath? item);

    LocalIdBuilder WithSecondaryItem(GmodPath? item);

    LocalIdBuilder WithMetadataTag(in MetadataTag metadataTag);

    LocalIdBuilder TryWithMetadataTag(in MetadataTag? metadataTag);

    LocalId Build();

    bool IsValid { get; }

    bool IsEmpty { get; }

    string ToString();
}
