namespace Vista.SDK;

public class UniversalId : IUniversalId, IEquatable<UniversalId>
{
    private readonly IUniversalIdBuilder _builder;

    internal UniversalId(IUniversalIdBuilder builder)
    {
        if (!builder.IsValid)
            throw new ArgumentException("Invalid UniversalId state");
        _builder = builder;
    }

    public ImoNumber? ImoNumber => _builder.ImoNumber;

    public sealed override bool Equals(object? obj) => Equals(obj as UniversalId);

    public bool Equals(UniversalId? other)
    {
        if (other is null)
            return false;

        return _builder.Equals(other._builder);
    }

    public override string ToString() => _builder.ToString();

    public override int GetHashCode() => _builder.GetHashCode();
}
