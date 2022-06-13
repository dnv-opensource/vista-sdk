namespace Vista.SDK;

public interface ILocalId
{
    VisVersion VisVersion { get; }

    bool VerboseMode { get; }

    GmodPath PrimaryItem { get; }

    GmodPath? SecondaryItem { get; }

    MetadataTag? Quantity { get; }

    MetadataTag? Content { get; }

    MetadataTag? Calculation { get; }

    MetadataTag? State { get; }

    MetadataTag? Command { get; }

    MetadataTag? Type { get; }

    MetadataTag? Position { get; }

    MetadataTag? Detail { get; }

    string ToString();
}
