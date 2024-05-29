namespace Vista.SDK;

public sealed record LocalIdQuery
{
    private readonly LocalIdQueryBuilder _builder;

    public IReadOnlyList<MetadataTag> Tags => _builder.Tags;

    internal LocalIdQuery(LocalIdQueryBuilder builder) => _builder = builder;

    public bool Match(LocalId other) => _builder.Match(other);

    public bool Match(string other) => _builder.Match(other);
}
