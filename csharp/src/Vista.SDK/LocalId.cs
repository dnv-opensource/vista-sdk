using Vista.SDK.Internal;

namespace Vista.SDK;

public class LocalId : ILocalId<LocalId>, IEquatable<LocalId>
{
    public static readonly string NamingRule = "dnv-v2";

    private readonly LocalIdBuilder _builder;

    public LocalIdBuilder Builder => _builder;

    internal LocalId(LocalIdBuilder builder)
    {
        if (builder.IsEmpty)
            throw new ArgumentException("LocalId cannot be constructed from empty LocalIdBuilder");
        if (!builder.IsValid)
            throw new ArgumentException("LocalId cannot be constructed from invalid LocalIdBuilder");
        _builder = builder;
    }

    public VisVersion VisVersion => _builder.VisVersion!.Value;

    public bool VerboseMode => _builder.VerboseMode;

    public GmodPath PrimaryItem => _builder.PrimaryItem!;

    public GmodPath? SecondaryItem => _builder.SecondaryItem;

    public MetadataTag? Quantity => _builder.Quantity;

    public MetadataTag? Content => _builder.Content;

    public MetadataTag? Calculation => _builder.Calculation;

    public MetadataTag? State => _builder.State;

    public MetadataTag? Command => _builder.Command;

    public MetadataTag? Type => _builder.Type;

    public MetadataTag? Position => _builder.Position;

    public MetadataTag? Detail => _builder.Detail;

    public bool HasCustomTag => _builder.HasCustomTag;

    public IReadOnlyList<MetadataTag> MetadataTags => _builder.MetadataTags;

    public sealed override bool Equals(object? obj) => Equals(obj as LocalId);

    public bool Equals(LocalId? other)
    {
        if (other is null)
            return false;

        return _builder.Equals(other._builder);
    }

    public static bool operator !=(LocalId? left, LocalId? right) => !(left == right);

    public static bool operator ==(LocalId? left, LocalId? right)
    {
        if (!ReferenceEquals(left, right))
        {
            if (left is not null)
                return left.Equals(right);
            return false;
        }
        return true;
    }

    public sealed override int GetHashCode() => _builder.GetHashCode();

    public override string ToString() => _builder.ToString();

    public static LocalId Parse(string localIdStr) => LocalIdBuilder.Parse(localIdStr).Build();

    public static bool TryParse(string localIdStr, out ParsingErrors errors, out LocalId? localId)
    {
        if (!LocalIdBuilder.TryParse(localIdStr, out errors, out var localIdBuilder))
        {
            localId = null;
            return false;
        }

        localId = localIdBuilder.Build();
        return true;
    }
}
