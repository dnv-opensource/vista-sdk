namespace Vista.SDK;

public delegate GmodPathQuery PathQueryConfiguration(GmodPathQueryBuilder.Path path);
public delegate GmodPathQuery NodesQueryConfiguration(GmodPathQueryBuilder.Nodes nodes);

public sealed record LocalIdQueryBuilder
{
    private GmodPathQuery? _primaryItem;
    private GmodPathQuery? _secondaryItem;
    private readonly Dictionary<CodebookName, MetadataTag> _tags = new();

    public IReadOnlyList<MetadataTag> Tags => _tags.Values.ToList();

    public LocalIdQuery Build() => new(this);

    public static LocalIdQueryBuilder Empty() => new();

    public static LocalIdQueryBuilder From(LocalId localId)
    {
        var builder = new LocalIdQueryBuilder().WithPrimaryItem(GmodPathQueryBuilder.From(localId.PrimaryItem).Build());
        if (localId.SecondaryItem != null)
            builder = builder.WithSecondaryItem(GmodPathQueryBuilder.From(localId.SecondaryItem).Build());
        foreach (var tag in localId.MetadataTags)
            builder = builder.WithTag(tag.Name, tag.Value);
        return builder;
    }

    /// <summary>Used to cofigure a path to query by a node.</summary>
    /// <returns>A new <see cref="LocalIdQueryBuilder"/> with the primary item configured.</returns>
    /// <example>
    /// This example configures the query to match all <see cref="LocalId"/> having a primary item with the node 500a in its path, with all possible individualizations.
    /// <code>
    /// var path = GmodPath.Parse("511.31-2/C101", VIS.LatestVisVersion);
    /// var builder = LocalIdQueryBuilder.Empty().WithPrimaryItem(configure => configure.WithNode(nodes => nodes["500a"], true));
    /// var query = builder.Build();
    /// Assert.True(query.Match(path));
    /// </code>
    /// </example>
    /// <param name="configure">A function that takes a <see cref="GmodPathQueryBuilder.Nodes" />.</param>
    public LocalIdQueryBuilder WithPrimaryItem(NodesQueryConfiguration configure)
    {
        return WithPrimaryItem(configure(GmodPathQueryBuilder.Empty()));
    }

    /// <summary>Used to cofigure a path to query by another path, which can the be configured.</summary>
    /// <remarks>This will be more specific the the overload using <see cref="NodesQueryConfiguration"/> config.</remarks>
    /// <returns>A new <see cref="LocalIdQueryBuilder"/> with the primary item configured.</returns>
    /// <example>
    /// <code>
    /// var visVersion = VIS.LatestVisVersion;
    /// var locations = VIS.Instance.GetLocations(visVersion);
    /// var path = GmodPath.Parse("511.31-2/C101", visVersion);
    /// var builder = LocalIdQueryBuilder.Empty().WithPrimaryItem(path, configure => configure.WithNode(nodes => nodes["511.31"], [locations.Parse("2")]), true));
    /// var query = builder.Build();
    /// Assert.True(query.Match("511.3/C101"));
    /// builder
    /// </code>
    /// </example>
    /// <param name="primaryItem">A function that takes a <see cref="GmodPath"/>.</param>
    /// <param name="configure">A function that takes a <see cref="GmodPathQueryBuilder.Nodes"/>.</param>
    public LocalIdQueryBuilder WithPrimaryItem(GmodPath primaryItem, PathQueryConfiguration configure)
    {
        var builder = GmodPathQueryBuilder.From(primaryItem);
        return WithPrimaryItem(configure(builder));
    }

    /// <summary>See documentation for <see cref="WithPrimaryItem(GmodPath, PathQueryConfiguration)"/></summary>
    public LocalIdQueryBuilder WithPrimaryItem(GmodPath primaryItem)
    {
        return WithPrimaryItem(GmodPathQueryBuilder.From(primaryItem).Build());
    }

    /// <summary>See documentation for <see cref="WithPrimaryItem(GmodPath, PathQueryConfiguration)"/></summary>
    public LocalIdQueryBuilder WithPrimaryItem(GmodPathQuery primaryItem)
    {
        return this with { _primaryItem = primaryItem };
    }

    /// <summary>See documentation for <see cref="WithPrimaryItem(NodesQueryConfiguration)"/></summary>
    public LocalIdQueryBuilder WithSecondaryItem(NodesQueryConfiguration configure)
    {
        return WithSecondaryItem(configure(GmodPathQueryBuilder.Empty()));
    }

    /// <summary>See documentation for <see cref="WithPrimaryItem(GmodPath, PathQueryConfiguration)"/></summary>
    public LocalIdQueryBuilder WithSecondaryItem(GmodPath secondaryItem, PathQueryConfiguration configure)
    {
        var builder = GmodPathQueryBuilder.From(secondaryItem);
        return WithSecondaryItem(configure(builder));
    }

    /// <summary>See documentation for <see cref="WithPrimaryItem(GmodPath, PathQueryConfiguration)"/></summary>
    public LocalIdQueryBuilder WithSecondaryItem(GmodPath secondaryItem)
    {
        return WithSecondaryItem(GmodPathQueryBuilder.From(secondaryItem).Build());
    }

    /// <summary>See documentation for <see cref="WithPrimaryItem(GmodPath, PathQueryConfiguration)"/></summary>
    public LocalIdQueryBuilder WithSecondaryItem(GmodPathQuery secondaryItem)
    {
        return this with { _secondaryItem = secondaryItem };
    }

    public LocalIdQueryBuilder WithTag(MetadataTag tag)
    {
        _tags.Add(tag.Name, tag);
        // Not sure if this is necessary
        return this with { };
    }

    public LocalIdQueryBuilder WithTag(CodebookName name, string value)
    {
        return WithTag(new MetadataTag(name, value));
    }

    internal bool Match(string other) => Match(LocalId.Parse(other));

    internal bool Match(LocalId other)
    {
        LocalId localId = other;
        if (other.VisVersion <= VIS.LatestVisVersion)
        {
            var converted = VIS.Instance.ConvertLocalId(other, VIS.LatestVisVersion);
            if (converted is null)
                throw new Exception("Failed to convert local id");
            localId = converted;
        }

        if (_primaryItem != null && _primaryItem.Match(localId.PrimaryItem) == false)
            return false;
        if (_secondaryItem != null && _secondaryItem.Match(localId.SecondaryItem) == false)
            return false;
        var metadataTags = other.MetadataTags.ToDictionary(t => t.Name);
        if (_tags.Count > 0)
        {
            foreach (var tag in _tags.Values)
            {
                if (!metadataTags.TryGetValue(tag.Name, out var otherTag))
                    return false;
                if (tag.Equals(otherTag) == false)
                    return false;
            }
        }

        return true;
    }
}

internal sealed record PathItem(GmodPath Path, bool Invidivualized);
