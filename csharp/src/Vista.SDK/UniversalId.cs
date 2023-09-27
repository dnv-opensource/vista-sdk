namespace Vista.SDK;

public class UniversalId : IUniversalId, IEquatable<UniversalId>
{
    private readonly IUniversalIdBuilder _builder;
    private readonly LocalId _localId;

    internal UniversalId(IUniversalIdBuilder builder)
    {
        if (!builder.IsValid)
            throw new ArgumentException("Invalid UniversalId state");
        _builder = builder;
        _localId = builder.LocalId!.Build();
    }

    public ImoNumber ImoNumber =>
        _builder.ImoNumber is not null ? _builder.ImoNumber.Value : throw new Exception("Invalid ImoNumber");
    public LocalId LocalId => _localId;

    public sealed override bool Equals(object? obj) => Equals(obj as UniversalId);

    public bool Equals(UniversalId? other)
    {
        if (other is null)
            return false;

        return _builder.Equals(other._builder);
    }

    public static UniversalId Parse(string universalIdStr) => UniversalIdBuilder.Parse(universalIdStr).Build();

    public static bool TryParse(string universalIdStr, out ParsingErrors errors, out UniversalId? universalId)
    {
        if (!UniversalIdBuilder.TryParse(universalIdStr, out errors, out var universalIdBuilder))
        {
            universalId = null;
            return false;
        }

        universalId = universalIdBuilder.Build();
        return true;
    }

    public override string ToString() => _builder.ToString();

    public override int GetHashCode() => _builder.GetHashCode();
}
