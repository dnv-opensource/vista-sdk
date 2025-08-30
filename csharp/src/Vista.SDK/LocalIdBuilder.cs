using System.Text;
using Vista.SDK.Internal;

namespace Vista.SDK;

public sealed partial record class LocalIdBuilder : ILocalIdBuilder<LocalIdBuilder, LocalId>
{
    public static readonly string NamingRule = "dnv-v2";
    public static readonly CodebookName[] UsedCodebooks =
    [
        CodebookName.Quantity,
        CodebookName.Content,
        CodebookName.State,
        CodebookName.Command,
        CodebookName.FunctionalServices,
        CodebookName.MaintenanceCategory,
        CodebookName.ActivityType,
        CodebookName.Position,
        CodebookName.Detail,
    ];
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

    public LocalIdBuilder WithVisVersion(in string visVersion)
    {
        var localIdbuilder = TryWithVisVersion(visVersion, out var succeeded);
        if (!succeeded)
            throw new ArgumentException("Failed to parse VIS version", nameof(visVersion));

        return localIdbuilder;
    }

    public LocalIdBuilder WithVisVersion(in VisVersion version)
    {
        var localIdbuilder = TryWithVisVersion(version, out var succeeded);
        if (!succeeded)
            throw new ArgumentException(nameof(WithVisVersion));

        return localIdbuilder;
    }

    public LocalIdBuilder TryWithVisVersion(in VisVersion? visVersion) => TryWithVisVersion(visVersion, out _);

    public LocalIdBuilder TryWithVisVersion(in string? visVersionStr, out bool succeeded)
    {
        if (VisVersions.TryParse(visVersionStr, out VisVersion v) == true)
        {
            var localIdbuilder = TryWithVisVersion(v, out var succeededInner);
            succeeded = succeededInner;
            return localIdbuilder;
        }
        succeeded = false;
        return this;
    }

    public LocalIdBuilder TryWithVisVersion(in VisVersion? visVersion, out bool succeeded)
    {
        succeeded = true;
        return this with { VisVersion = visVersion };
    }

    public LocalIdBuilder WithoutVisVersion() => this with { VisVersion = null };

    public LocalIdBuilder WithVerboseMode(in bool verboseMode) => this with { VerboseMode = verboseMode };

    public LocalIdBuilder WithPrimaryItem(in GmodPath item)
    {
        var localIdbuilder = TryWithPrimaryItem(item, out var succeeded);
        if (!succeeded)
            throw new ArgumentException(nameof(WithPrimaryItem));

        return localIdbuilder;
    }

    public LocalIdBuilder TryWithPrimaryItem(in GmodPath? item) => TryWithPrimaryItem(item, out _);

    public LocalIdBuilder TryWithPrimaryItem(in GmodPath? item, out bool succeeded)
    {
        if (item is null)
        {
            succeeded = false;
            return this;
        }

        succeeded = true;
        return this with { Items = this.Items with { PrimaryItem = item } };
    }

    public LocalIdBuilder WithoutPrimaryItem() => this with { Items = this.Items with { PrimaryItem = null } };

    public LocalIdBuilder WithSecondaryItem(in GmodPath item)
    {
        var localIdBuilder = TryWithSecondaryItem(item, out var succeeded);
        if (!succeeded)
            throw new ArgumentException(nameof(WithSecondaryItem));

        return localIdBuilder;
    }

    public LocalIdBuilder TryWithSecondaryItem(in GmodPath? item) => TryWithSecondaryItem(in item, out _);

    public LocalIdBuilder TryWithSecondaryItem(in GmodPath? item, out bool succeeded)
    {
        if (item is null)
        {
            succeeded = false;
            return this;
        }

        succeeded = true;
        return this with { Items = this.Items with { SecondaryItem = item } };
    }

    public LocalIdBuilder WithoutSecondaryItem() => this with { Items = this.Items with { SecondaryItem = null } };

    public LocalIdBuilder WithMetadataTag(in MetadataTag metadataTag)
    {
        var localIdBuilder = TryWithMetadataTag(metadataTag, out var succeeded);
        if (!succeeded)
            throw new ArgumentException("invalid metadata codebook name: " + metadataTag.Name);
        return localIdBuilder;
    }

    public LocalIdBuilder TryWithMetadataTag(in MetadataTag? metadataTag) => TryWithMetadataTag(metadataTag, out _);

    public LocalIdBuilder TryWithMetadataTag(in MetadataTag? metadataTag, out bool succeeded)
    {
        if (metadataTag is null)
        {
            succeeded = false;
            return this;
        }

        switch (metadataTag.Value.Name)
        {
            case CodebookName.Quantity:
                succeeded = true;
                return WithQuantity(metadataTag.Value);
            case CodebookName.Content:
                succeeded = true;
                return WithContent(metadataTag.Value);
            case CodebookName.Calculation:
                succeeded = true;
                return WithCalculation(metadataTag.Value);
            case CodebookName.State:
                succeeded = true;
                return WithState(metadataTag.Value);
            case CodebookName.Command:
                succeeded = true;
                return WithCommand(metadataTag.Value);
            case CodebookName.Type:
                succeeded = true;
                return WithType(metadataTag.Value);
            case CodebookName.Position:
                succeeded = true;
                return WithPosition(metadataTag.Value);
            case CodebookName.Detail:
                succeeded = true;
                return WithDetail(metadataTag.Value);
            default:
                succeeded = false;
                return this;
        }
    }

    public LocalIdBuilder WithoutMetadataTag(in CodebookName name) =>
        name switch
        {
            CodebookName.Quantity => WithoutQuantity(),
            CodebookName.Content => WithoutContent(),
            CodebookName.Calculation => WithoutCalculation(),
            CodebookName.State => WithoutState(),
            CodebookName.Command => WithoutCommand(),
            CodebookName.Type => WithoutType(),
            CodebookName.Position => WithoutPosition(),
            CodebookName.Detail => WithoutDetail(),
            _ => this
        };

    public LocalIdBuilder WithoutQuantity() => this with { Quantity = null };

    public LocalIdBuilder WithoutContent() => this with { Content = null };

    public LocalIdBuilder WithoutCalculation() => this with { Calculation = null };

    public LocalIdBuilder WithoutState() => this with { State = null };

    public LocalIdBuilder WithoutCommand() => this with { Command = null };

    public LocalIdBuilder WithoutType() => this with { Type = null };

    public LocalIdBuilder WithoutPosition() => this with { Position = null };

    public LocalIdBuilder WithoutDetail() => this with { Detail = null };

    private LocalIdBuilder WithQuantity(in MetadataTag quantity) => this with { Quantity = quantity, };

    private LocalIdBuilder WithContent(in MetadataTag content) => this with { Content = content, };

    private LocalIdBuilder WithCalculation(in MetadataTag calculation) => this with { Calculation = calculation, };

    private LocalIdBuilder WithState(in MetadataTag state) => this with { State = state, };

    private LocalIdBuilder WithCommand(in MetadataTag command) => this with { Command = command, };

    private LocalIdBuilder WithType(in MetadataTag type) => this with { Type = type, };

    private LocalIdBuilder WithPosition(in MetadataTag position) => this with { Position = position, };

    private LocalIdBuilder WithDetail(in MetadataTag detail) => this with { Detail = detail, };

    public static LocalIdBuilder Create(VisVersion version) => new LocalIdBuilder().WithVisVersion(version);

    public LocalId Build()
    {
        if (IsEmpty)
            throw new InvalidOperationException("Cant build to LocalId from empty LocalIdBuilder");
        if (!IsValid)
            throw new InvalidOperationException("Cant build to LocalId from invalid LocalIdBuilder");

        return new LocalId(this);
    }

    public bool HasCustomTag =>
        (Quantity?.IsCustom ?? false)
        || (Calculation?.IsCustom ?? false)
        || (Content?.IsCustom ?? false)
        || (Position?.IsCustom ?? false)
        || (State?.IsCustom ?? false)
        || (Command?.IsCustom ?? false)
        || (Type?.IsCustom ?? false)
        || (Detail?.IsCustom ?? false);

    public bool IsValid =>
        VisVersion is not null
        && Items.PrimaryItem is not null
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

    public bool IsEmptyMetadata =>
        Quantity is null
        && Calculation is null
        && Content is null
        && Position is null
        && State is null
        && Command is null
        && Type is null
        && Detail is null;

    public IReadOnlyList<MetadataTag> MetadataTags =>
        new List<MetadataTag?>() { Quantity, Calculation, Content, Position, State, Command, Type, Detail }
            .Where(m => m is not null)
            .Cast<MetadataTag>()
            .ToArray();

    public bool Equals(LocalIdBuilder? other)
    {
        if (other is null)
            return false;

        if (VisVersion != other.VisVersion)
            throw new InvalidOperationException("Cant compare local IDs from different VIS versions");

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

    public sealed override int GetHashCode()
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

    public void ToString(StringBuilder builder)
    {
        if (VisVersion is null)
            throw new InvalidOperationException("No VisVersion configured on LocalId");

        string namingRule = $"/{NamingRule}/";

        builder.Append(namingRule);

        builder.Append("vis-");
        VisVersion.Value.ToVersionString(builder);
        builder.Append('/');

        Items.Append(builder, VerboseMode);

        builder.Append("meta/");

        // NOTE: order of metadatatags matter,
        // should not be changed unless changed in the naming rule/standard
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
    }

    public override string ToString()
    {
        using var lease = StringBuilderPool.Get();
        var builder = lease.Builder;

        ToString(builder);

        return lease.ToString();
    }
}
