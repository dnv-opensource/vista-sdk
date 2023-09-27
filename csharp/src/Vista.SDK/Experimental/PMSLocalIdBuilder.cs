using System.Text;
using Vista.SDK.Internal;

namespace Vista.SDK.Experimental;

public sealed partial record class PMSLocalIdBuilder : ILocalIdBuilder<PMSLocalIdBuilder, PMSLocalId>
{
    public static readonly string NamingRule = "dnv-v2-experimental";
    public static readonly CodebookName[] UsedCodebooks = new[]
    {
        CodebookName.Quantity,
        CodebookName.Content,
        CodebookName.State,
        CodebookName.Command,
        CodebookName.FunctionalServices,
        CodebookName.MaintenanceCategory,
        CodebookName.ActivityType,
        CodebookName.Position,
        CodebookName.Detail,
    };
    public VisVersion? VisVersion { get; private init; }

    public bool VerboseMode { get; private init; } = false;

    internal LocalIdItems Items { get; private init; }

    public GmodPath? PrimaryItem => Items.PrimaryItem;

    public GmodPath? SecondaryItem => Items.SecondaryItem;

    public MetadataTag? Quantity { get; private init; }

    public MetadataTag? Content { get; private init; }

    public MetadataTag? State { get; private init; }

    public MetadataTag? Command { get; private init; }

    public MetadataTag? FunctionalServices { get; private init; }

    public MetadataTag? MaintenanceCategory { get; private init; }

    public MetadataTag? ActivityType { get; private init; }

    public MetadataTag? Position { get; private init; }

    public MetadataTag? Detail { get; private init; }

    public PMSLocalIdBuilder WithVisVersion(in string visVersion)
    {
        var localIdbuilder = TryWithVisVersion(visVersion, out var succeeded);
        if (!succeeded)
            throw new ArgumentException(nameof(visVersion));

        return localIdbuilder;
    }

    public PMSLocalIdBuilder WithVisVersion(in VisVersion version)
    {
        var localIdbuilder = TryWithVisVersion(version, out var succeeded);
        if (!succeeded)
            throw new ArgumentException(nameof(WithVisVersion));

        return localIdbuilder;
    }

    public PMSLocalIdBuilder TryWithVisVersion(in VisVersion? visVersion) => TryWithVisVersion(visVersion, out _);

    public PMSLocalIdBuilder TryWithVisVersion(in string? visVersionStr, out bool succeeded)
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

    public PMSLocalIdBuilder TryWithVisVersion(in VisVersion? visVersion, out bool succeeded)
    {
        succeeded = true;
        return this with { VisVersion = visVersion };
    }

    public PMSLocalIdBuilder WithoutVisVersion() => this with { VisVersion = null };

    public PMSLocalIdBuilder WithVerboseMode(in bool verboseMode) => this with { VerboseMode = verboseMode };

    public PMSLocalIdBuilder WithPrimaryItem(in GmodPath item)
    {
        var localIdbuilder = TryWithPrimaryItem(item, out var succeeded);
        if (!succeeded)
            throw new ArgumentException(nameof(WithPrimaryItem));

        return localIdbuilder;
    }

    public PMSLocalIdBuilder TryWithPrimaryItem(in GmodPath? item) => TryWithPrimaryItem(item, out _);

    public PMSLocalIdBuilder TryWithPrimaryItem(in GmodPath? item, out bool succeeded)
    {
        if (item is null)
        {
            succeeded = false;
            return this;
        }

        succeeded = true;
        return this with { Items = this.Items with { PrimaryItem = item } };
    }

    public PMSLocalIdBuilder WithoutPrimaryItem() => this with { Items = this.Items with { PrimaryItem = null } };

    public PMSLocalIdBuilder WithSecondaryItem(in GmodPath item)
    {
        var localIdBuilder = TryWithSecondaryItem(item, out var succeeded);
        if (!succeeded)
            throw new ArgumentException(nameof(WithSecondaryItem));

        return localIdBuilder;
    }

    public PMSLocalIdBuilder TryWithSecondaryItem(in GmodPath? item) => TryWithSecondaryItem(in item, out _);

    public PMSLocalIdBuilder TryWithSecondaryItem(in GmodPath? item, out bool succeeded)
    {
        if (item is null)
        {
            succeeded = false;
            return this;
        }

        succeeded = true;
        return this with { Items = this.Items with { SecondaryItem = item } };
    }

    public PMSLocalIdBuilder WithoutSecondaryItem() => this with { Items = this.Items with { SecondaryItem = null } };

    public PMSLocalIdBuilder WithMetadataTag(in MetadataTag metadataTag)
    {
        var localIdBuilder = TryWithMetadataTag(metadataTag, out var succeeded);
        if (!succeeded)
            throw new ArgumentException("invalid metadata codebook name: " + metadataTag.Name);
        return localIdBuilder;
    }

    public PMSLocalIdBuilder TryWithMetadataTag(in MetadataTag? metadataTag) => TryWithMetadataTag(metadataTag, out _);

    public PMSLocalIdBuilder TryWithMetadataTag(in MetadataTag? metadataTag, out bool succeeded)
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

            case CodebookName.State:
                succeeded = true;
                return WithState(metadataTag.Value);
            case CodebookName.Command:
                succeeded = true;
                return WithCommand(metadataTag.Value);
            case CodebookName.MaintenanceCategory:
                succeeded = true;
                return WithMaintenanceCategory(metadataTag.Value);
            case CodebookName.FunctionalServices:
                succeeded = true;
                return WithFunctionalServices(metadataTag.Value);
            case CodebookName.ActivityType:
                succeeded = true;
                return WithActivityType(metadataTag.Value);
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

    public PMSLocalIdBuilder WithoutMetadataTag(in CodebookName name) =>
        name switch
        {
            CodebookName.Quantity => WithoutQuantity(),
            CodebookName.Content => WithoutContent(),
            CodebookName.State => WithoutState(),
            CodebookName.Command => WithoutCommand(),
            CodebookName.FunctionalServices => WithoutFunctionalServices(),
            CodebookName.MaintenanceCategory => WithoutMaintenanceCategory(),
            CodebookName.ActivityType => WithoutActivityType(),
            CodebookName.Position => WithoutPosition(),
            CodebookName.Detail => WithoutDetail(),
            _ => this
        };

    public PMSLocalIdBuilder WithoutQuantity() => this with { Quantity = null };

    public PMSLocalIdBuilder WithoutContent() => this with { Content = null };

    public PMSLocalIdBuilder WithoutState() => this with { State = null };

    public PMSLocalIdBuilder WithoutCommand() => this with { Command = null };

    public PMSLocalIdBuilder WithoutFunctionalServices() => this with { FunctionalServices = null };

    public PMSLocalIdBuilder WithoutMaintenanceCategory() => this with { MaintenanceCategory = null };

    public PMSLocalIdBuilder WithoutActivityType() => this with { ActivityType = null };

    public PMSLocalIdBuilder WithoutPosition() => this with { Position = null };

    public PMSLocalIdBuilder WithoutDetail() => this with { Detail = null };

    private PMSLocalIdBuilder WithQuantity(in MetadataTag quantity) => this with { Quantity = quantity, };

    private PMSLocalIdBuilder WithContent(in MetadataTag content) => this with { Content = content, };

    private PMSLocalIdBuilder WithState(in MetadataTag state) => this with { State = state, };

    private PMSLocalIdBuilder WithCommand(in MetadataTag command) => this with { Command = command, };

    private PMSLocalIdBuilder WithFunctionalServices(in MetadataTag functionalServices) =>
        this with
        {
            FunctionalServices = functionalServices,
        };

    private PMSLocalIdBuilder WithMaintenanceCategory(in MetadataTag maintenanceCategory) =>
        this with
        {
            MaintenanceCategory = maintenanceCategory,
        };

    private PMSLocalIdBuilder WithActivityType(in MetadataTag activityType) =>
        this with
        {
            ActivityType = activityType,
        };

    private PMSLocalIdBuilder WithPosition(in MetadataTag position) => this with { Position = position, };

    private PMSLocalIdBuilder WithDetail(in MetadataTag detail) => this with { Detail = detail, };

    public static PMSLocalIdBuilder Create(VisVersion version) => new PMSLocalIdBuilder().WithVisVersion(version);

    public bool HasCustomTag =>
        (Quantity?.IsCustom ?? false)
        || (FunctionalServices?.IsCustom ?? false)
        || (Content?.IsCustom ?? false)
        || (Position?.IsCustom ?? false)
        || (State?.IsCustom ?? false)
        || (Command?.IsCustom ?? false)
        || (MaintenanceCategory?.IsCustom ?? false)
        || (ActivityType?.IsCustom ?? false)
        || (Detail?.IsCustom ?? false);

    public bool IsValid =>
        VisVersion is not null
        && Items.PrimaryItem is not null
        && ActivityType is not null
        && (
            Quantity is not null
            || FunctionalServices is not null
            || Content is not null
            || Position is not null
            || State is not null
            || Command is not null
            || MaintenanceCategory is not null
            || ActivityType is not null
            || Detail is not null
        );

    public bool IsEmpty =>
        Items.PrimaryItem is null
        && Items.SecondaryItem is null
        && (
            Quantity is null
            && FunctionalServices is null
            && Content is null
            && Position is null
            && State is null
            && Command is null
            && MaintenanceCategory is null
            && ActivityType is null
            && Detail is null
        );

    public bool IsEmptyMetadata =>
        Quantity is null
        && FunctionalServices is null
        && Content is null
        && Position is null
        && State is null
        && Command is null
        && MaintenanceCategory is null
        && ActivityType is null
        && Detail is null;

    public IReadOnlyList<MetadataTag> MetadataTags =>
        new List<MetadataTag?>()
        {
            Quantity,
            Content,
            Position,
            State,
            Command,
            FunctionalServices,
            MaintenanceCategory,
            ActivityType,
            Detail
        }
            .Where(m => m is not null)
            .Cast<MetadataTag>()
            .ToArray();

    public bool Equals(PMSLocalIdBuilder? other)
    {
        if (other is null)
            return false;

        if (VisVersion != other.VisVersion)
            throw new InvalidOperationException("Cant compare local IDs from different VIS versions");

        return PrimaryItem == other.PrimaryItem
            && SecondaryItem == other.SecondaryItem
            && Quantity == other.Quantity
            && FunctionalServices == other.FunctionalServices
            && Content == other.Content
            && Position == other.Position
            && State == other.State
            && Command == other.Command
            && MaintenanceCategory == other.MaintenanceCategory
            && ActivityType == other.ActivityType
            && Detail == other.Detail;
    }

    public sealed override int GetHashCode()
    {
        var hashCode = new HashCode();
        hashCode.Add(PrimaryItem);
        hashCode.Add(SecondaryItem);
        hashCode.Add(Quantity);
        hashCode.Add(FunctionalServices);
        hashCode.Add(Content);
        hashCode.Add(Position);
        hashCode.Add(State);
        hashCode.Add(Command);
        hashCode.Add(MaintenanceCategory);
        hashCode.Add(ActivityType);
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
        builder.AppendMeta(State);
        builder.AppendMeta(Command);
        builder.AppendMeta(FunctionalServices);
        builder.AppendMeta(MaintenanceCategory);
        builder.AppendMeta(ActivityType);
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

    public PMSLocalId Build()
    {
        if (IsEmpty)
            throw new InvalidOperationException("Cant build to PMSLocalId from empty PMSLocalIdBuilder");
        if (!IsValid)
            throw new InvalidOperationException("Cant build to PMSLocalId from invalid PMSLocalIdBuilder");

        return new PMSLocalId(this);
    }
}
