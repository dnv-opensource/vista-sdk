using Vista.SDK.Internal;

namespace Vista.SDK.Experimental;

public class PMSLocalId : ILocalId<PMSLocalId>, IEquatable<PMSLocalId>
{
    public static readonly string NamingRule = "dnv-v2";

    private readonly PMSLocalIdBuilder _builder;

    internal PMSLocalId(PMSLocalIdBuilder builder)
    {
        if (builder.IsEmpty)
            throw new ArgumentException(
                "PMSLocalId cannot be constructed from empty PMSLocalIdBuilder"
            );
        if (!builder.IsValid)
            throw new ArgumentException(
                "PMSLocalId cannot be constructed from invalid PMSLocalIdBuilder"
            );
        _builder = builder;
    }

    public VisVersion VisVersion => _builder.VisVersion!.Value;

    public bool VerboseMode => _builder.VerboseMode;

    public GmodPath PrimaryItem => _builder.PrimaryItem!;

    public GmodPath? SecondaryItem => _builder.SecondaryItem;

    public MetadataTag? Quantity => _builder.Quantity;

    public MetadataTag? Content => _builder.Content;

    public MetadataTag? State => _builder.State;

    public MetadataTag? Command => _builder.Command;

    public MetadataTag? FunctionalServices => _builder.FunctionalServices;

    public MetadataTag? MaintenanceCategory => _builder.MaintenanceCategory;

    public MetadataTag? ActivityType => _builder.ActivityType;

    public MetadataTag? Position => _builder.Position;

    public MetadataTag? Detail => _builder.Detail;

    public bool HasCustomTag => _builder.HasCustomTag;

    public IReadOnlyList<MetadataTag> MetadataTags => _builder.MetadataTags;

    public override string ToString() => _builder.ToString();

    public static PMSLocalId Parse(string localIdStr, out ParsingErrors errors) =>
        PMSLocalIdBuilder.Parse(localIdStr, out errors).Build();

    public static PMSLocalId Parse(string localIdStr) =>
        PMSLocalIdBuilder.Parse(localIdStr).Build();

    public sealed override bool Equals(object? obj) => Equals(obj as PMSLocalId);

    public bool Equals(PMSLocalId? other)
    {
        if (other is null)
            return false;

        return _builder.Equals(other._builder);
    }

    public static bool operator !=(PMSLocalId? left, PMSLocalId? right) => !(left == right);

    public static bool operator ==(PMSLocalId? left, PMSLocalId? right)
    {
        if ((object?)left != right)
        {
            if ((object?)left != null)
            {
                return left.Equals(right);
            }
            return false;
        }
        return true;
    }

    public sealed override int GetHashCode() => _builder.GetHashCode();
}
