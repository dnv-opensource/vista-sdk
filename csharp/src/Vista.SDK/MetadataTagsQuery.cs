namespace Vista.SDK;

public sealed record MetadataTagsQuery
{
    internal MetadataTagsQueryBuilder Builder { get; }

    internal MetadataTagsQuery(MetadataTagsQueryBuilder builder) => Builder = builder;

    public bool Match(LocalId? localId) => Builder.Match(localId);
}

public sealed record MetadataTagsQueryBuilder
{
    private readonly Dictionary<CodebookName, MetadataTag> _tags = new();

    private bool _matchExact;

    private MetadataTagsQueryBuilder() { }

    public static MetadataTagsQueryBuilder Empty() => new();

    public static MetadataTagsQueryBuilder From(LocalId localId, bool allowOtherTags = true)
    {
        var builder = new MetadataTagsQueryBuilder();
        foreach (var tag in localId.MetadataTags)
            builder = builder.WithTag(tag);
        builder = builder.WithAllowOtherTags(allowOtherTags);
        return builder;
    }

    public MetadataTagsQuery Build() => new(this);

    public MetadataTagsQueryBuilder WithTag(CodebookName name, string value) => WithTag(new(name, value));

    public MetadataTagsQueryBuilder WithTag(MetadataTag tag)
    {
        _tags[tag.Name] = tag;
        return this;
    }

    public MetadataTagsQueryBuilder WithAllowOtherTags(bool allowOthers)
    {
        _matchExact = !allowOthers;
        return this;
    }

    internal bool Match(LocalId? localId)
    {
        if (localId is null)
            return false;

        var metadataTags = localId.MetadataTags.ToDictionary(t => t.Name);

        if (_tags.Count > 0)
        {
            if (_matchExact)
            {
                return _tags.Count == metadataTags.Count
                    && _tags.All(t => metadataTags.TryGetValue(t.Key, out var value) && t.Value.Equals(value));
            }
            else
            {
                foreach (var tag in _tags.Values)
                {
                    if (!metadataTags.TryGetValue(tag.Name, out var otherTag))
                        return false;
                    if (tag.Equals(otherTag) == false)
                        return false;
                }
                return true;
            }
        }
        else
        {
            return !_matchExact;
        }
    }
}
