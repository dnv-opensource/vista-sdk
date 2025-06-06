namespace Vista.SDK;

public delegate GmodPathQuery PathQueryConfiguration(GmodPathQueryBuilder.Path path);
public delegate GmodPathQuery NodesQueryConfiguration(GmodPathQueryBuilder.Nodes nodes);

public sealed record LocalIdQueryBuilder
{
    private GmodPathQuery? _primaryItem;
    private GmodPathQuery? _secondaryItem;
    private MetadataTagsQuery? _tags;

    public LocalIdQuery Build() => new(this);

    public static LocalIdQueryBuilder Empty() => new();

    public GmodPath? PrimaryItem => _primaryItem?.Builder is GmodPathQueryBuilder.Path p ? p.GmodPath : null;
    public GmodPath? SecondaryItem => _secondaryItem?.Builder is GmodPathQueryBuilder.Path p ? p.GmodPath : null;

    public static LocalIdQueryBuilder From(string localId) => From(LocalId.Parse(localId));

    public static LocalIdQueryBuilder From(LocalId localId)
    {
        var builder = new LocalIdQueryBuilder().WithPrimaryItem(GmodPathQueryBuilder.From(localId.PrimaryItem).Build());
        if (localId.SecondaryItem != null)
            builder = builder.WithSecondaryItem(GmodPathQueryBuilder.From(localId.SecondaryItem).Build());
        builder = builder.WithTags(MetadataTagsQueryBuilder.From(localId).Build());
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
    /// <remarks>Should only be used in combination with the <see cref="From(LocalId)"/> overload, or previously configured Path item/></remarks>
    public LocalIdQueryBuilder WithPrimaryItem(PathQueryConfiguration configure)
    {
        if (PrimaryItem is null)
            throw new InvalidOperationException("Primary item is null");
        return WithPrimaryItem(PrimaryItem, configure);
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

    public LocalIdQueryBuilder WithSecondaryItem(PathQueryConfiguration configure)
    {
        if (SecondaryItem is null)
            throw new InvalidOperationException("Secondary item is null");
        return WithSecondaryItem(SecondaryItem, configure);
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

    public LocalIdQueryBuilder WithTags(Func<MetadataTagsQueryBuilder, MetadataTagsQuery> configure)
    {
        MetadataTagsQueryBuilder builder = _tags?.Builder ?? MetadataTagsQueryBuilder.Empty();
        return WithTags(configure(builder));
    }

    public LocalIdQueryBuilder WithTags(MetadataTagsQuery tags)
    {
        return this with { _tags = tags };
    }

    internal bool Match(string other) => Match(LocalId.Parse(other));

    internal bool Match(LocalId other)
    {
        LocalId localId = other;
        if (other.VisVersion < VIS.LatestVisVersion)
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
        if (_tags != null && _tags.Match(localId) == false)
            return false;

        return true;
    }
}

internal sealed record PathItem(GmodPath Path, bool Invidivualized);
