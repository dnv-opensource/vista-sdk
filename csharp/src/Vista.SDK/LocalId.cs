using System.Text;

using Vista.SDK.Internal;

namespace Vista.SDK;

public sealed partial record class LocalId
{
    internal const string NamingRule = "dnv-v2";

    public VisVersion? VisVersion { get; private init; }

    public bool VerboseMode { get; private init; } = false;

    internal LocalIdItems Items { get; private init; }

    public GmodPath? PrimaryItem => Items.PrimaryItem;

    public GmodPath? SecondaryItem => Items.SecondaryItem;

    public MetadataTag? Quantity { get; private init; }

    public MetadataTag? Content { get; private init; }

    public MetadataTag? Calculation { get; private init; }

    public MetadataTag? State { get; private init; }

    public MetadataTag? Command { get; private init; }

    public MetadataTag? Type { get; private init; }

    public MetadataTag? Position { get; private init; }

    public MetadataTag? Detail { get; private init; }

    public LocalId WithVisVersion(string visVersion) =>
        this with
        {
            VisVersion = VisVersions.Parse(visVersion)
        };

    public LocalId WithVisVersion(VisVersion version) => this with { VisVersion = version };

    public LocalId WithVerboseMode(bool verboseMode) => this with { VerboseMode = verboseMode };

    public LocalId WithPrimaryItem(GmodPath? item) =>
        this with
        {
            Items = this.Items with { PrimaryItem = item }
        };

    public LocalId WithSecondaryItem(GmodPath? item) =>
        this with
        {
            Items = this.Items with { SecondaryItem = item }
        };

    public LocalId WithMetadataTag(in MetadataTag metadataTag) =>
        metadataTag.Name switch
        {
            CodebookName.Quantity => WithQuantity(in metadataTag),
            CodebookName.Content => WithContent(in metadataTag),
            CodebookName.Calculation => WithCalculation(in metadataTag),
            CodebookName.State => WithState(in metadataTag),
            CodebookName.Command => WithCommand(in metadataTag),
            CodebookName.Type => WithType(in metadataTag),
            CodebookName.Position => WithPosition(in metadataTag),
            CodebookName.Detail => WithDetail(in metadataTag),
            _ => throw new ArgumentException("Invalid metadata tag: " + metadataTag),
        };

    public LocalId TryWithMetadataTag(in MetadataTag? metadataTag)
    {
        if (metadataTag is null)
            return this;
        return WithMetadataTag(metadataTag.Value);
    }

    private LocalId WithQuantity(in MetadataTag quantity) => this with { Quantity = quantity, };

    private LocalId WithContent(in MetadataTag content) => this with { Content = content, };

    private LocalId WithCalculation(in MetadataTag calculation) =>
        this with
        {
            Calculation = calculation,
        };

    private LocalId WithState(in MetadataTag state) => this with { State = state, };

    private LocalId WithCommand(in MetadataTag command) => this with { Command = command, };

    private LocalId WithType(in MetadataTag type) => this with { Type = type, };

    private LocalId WithPosition(in MetadataTag position) => this with { Position = position, };

    private LocalId WithDetail(in MetadataTag detail) => this with { Detail = detail, };

    public static LocalId Create(VisVersion version) => new LocalId().WithVisVersion(version);

    public bool IsValid =>
        Items.PrimaryItem is not null
        && (
            Quantity is not null
            || Calculation is not null
            || Content is not null
            || Position is not null
            || State is not null
            || Command is not null
            || Type is not null
            || Detail is not null
        );

    public bool IsEmpty =>
        Items.PrimaryItem is null
        && Items.SecondaryItem is null
        && (
            Quantity is null
            && Calculation is null
            && Content is null
            && Position is null
            && State is null
            && Command is null
            && Type is null
            && Detail is null
        );

    public bool Equals(LocalId? other)
    {
        if (other is null)
            return false;

        if (VisVersion != other.VisVersion)
            throw new InvalidOperationException(
                "Cant compare local IDs from different VIS versions"
            );

        return PrimaryItem == other.PrimaryItem
            && SecondaryItem == other.SecondaryItem
            && Quantity == other.Quantity
            && Calculation == other.Calculation
            && Content == other.Content
            && Position == other.Position
            && State == other.State
            && Command == other.Command
            && Type == other.Type
            && Detail == other.Detail;
    }

    public override int GetHashCode()
    {
        var hashCode = new HashCode();
        hashCode.Add(PrimaryItem);
        hashCode.Add(SecondaryItem);
        hashCode.Add(Quantity);
        hashCode.Add(Calculation);
        hashCode.Add(Content);
        hashCode.Add(Position);
        hashCode.Add(State);
        hashCode.Add(Command);
        hashCode.Add(Type);
        hashCode.Add(Detail);
        return hashCode.ToHashCode();
    }

    public override string ToString()
    {
        if (VisVersion is null)
            throw new InvalidOperationException("No VisVersion configured on LocalId");

        const string namingRule = $"/{NamingRule}/";
        using var lease = StringBuilderPool.Get();

        var builder = lease.Builder;

        builder.Append(namingRule);

        builder.Append("vis-");
        VisVersion.Value.ToVersionString(builder);
        builder.Append('/');

        Items.Append(builder, VerboseMode);

        builder.Append("meta/");

        builder.AppendMeta(Quantity);
        builder.AppendMeta(Content);
        builder.AppendMeta(Calculation);
        builder.AppendMeta(State);
        builder.AppendMeta(Command);
        builder.AppendMeta(Type);
        builder.AppendMeta(Position);
        builder.AppendMeta(Detail);

        if (builder[builder.Length - 1] == '/')
            builder.Remove(builder.Length - 1, 1);

        return lease.ToString();
    }
}

internal readonly record struct LocalIdItems
{
    public GmodPath? PrimaryItem { get; init; }

    public GmodPath? SecondaryItem { get; init; }

    internal void Append(StringBuilder builder, bool verboseMode)
    {
        if (PrimaryItem is null && SecondaryItem is null)
            return;

        if (PrimaryItem is not null)
        {
            PrimaryItem.ToString(builder);
            builder.Append('/');
        }

        if (SecondaryItem is not null)
        {
            builder.Append("sec/");
            SecondaryItem.ToString(builder);
            builder.Append('/');
        }

        if (verboseMode)
        {
            if (PrimaryItem is not null)
            {
                foreach (var (depth, name) in PrimaryItem.GetCommonNames())
                {
                    builder.Append('~');
                    var location = PrimaryItem[depth].Location;
                    AppendCommonName(builder, name, location);
                    builder.Append('/');
                }
            }

            if (SecondaryItem is not null)
            {
                var prefix = "~for.";
                foreach (var (depth, name) in SecondaryItem.GetCommonNames())
                {
                    builder.Append(prefix);
                    if (prefix != "~")
                        prefix = "~";

                    var location = SecondaryItem[depth].Location;
                    AppendCommonName(builder, name, location);
                    builder.Append('/');
                }
            }
        }

        static void AppendCommonName(StringBuilder builder, string commonName, string? location)
        {
            char? prev = null;
            foreach (ref readonly var ch in commonName.AsSpan())
            {
                if (ch == '/')
                    continue;
                if (prev == ' ' && ch == ' ')
                    continue;

                builder.Append(
                    ch switch
                    {
                        ' ' => '.',
                        _ => char.ToLowerInvariant(ch)
                    }
                );

                prev = ch;
            }

            if (location is { Length: > 0 })
            {
                builder.Append('.');
                builder.Append(location);
            }
        }
    }
}
