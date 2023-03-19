using System.Collections;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Text;
using Vista.SDK.Internal;

namespace Vista.SDK;

public sealed record GmodPath
{
    public IReadOnlyList<GmodNode> Parents { get; }

    public GmodNode Node { get; }

    public int Length => Parents.Count + 1;

    public bool IsMappable => Node.IsMappable;

    public GmodNode this[int depth]
    {
        get
        {
            if (depth < 0)
                throw new IndexOutOfRangeException("Index out of range for GmodPath indexer");
            if (depth > Parents.Count)
                throw new IndexOutOfRangeException("Index out of range for GmodPath indexer");

            return depth < Parents.Count ? Parents[depth] : Node;
        }
    }

    internal GmodPath(IReadOnlyList<GmodNode> parents, GmodNode node, bool skipVerify = true)
    {
        if (!skipVerify)
        {
            if (parents.Count == 0)
                throw new ArgumentException(
                    $"Invalid gmod path - no parents, and {node.Code} is not the root of gmod"
                );
            if (parents.Count > 0 && !parents[0].IsRoot)
                throw new ArgumentException(
                    $"Invalid gmod path - first parent should be root of gmod (VE), but was {parents[0].Code}"
                );

            var set = new HashSet<string>();

            set.Add("VE");
            for (int i = 0; i < parents.Count; i++)
            {
                var parent = parents[i];
                var nextIndex = i + 1;
                var child = nextIndex < parents.Count ? parents[nextIndex] : node;
                if (!parent.IsChild(child))
                    throw new ArgumentException(
                        $"Invalid gmod path - {child.Code} not child of {parent.Code}"
                    );

                if (!set.Add(child.Code))
                    throw new ArgumentException(
                        $"Recursion in gmod path argument for code: {child.Code}"
                    );
            }
        }

        Parents = parents;
        Node = node;
    }

    public static bool IsValid(IReadOnlyList<GmodNode> parents, GmodNode node) =>
        IsValid(parents, node, out _);

    internal static bool IsValid(
        IReadOnlyList<GmodNode> parents,
        GmodNode node,
        out int missingLinkAt
    )
    {
        missingLinkAt = -1;

        if (parents.Count == 0)
            return false;

        if (parents.Count > 0 && !parents[0].IsRoot)
            return false;

        var set = new HashSet<string> { "VE" };

        for (int i = 0; i < parents.Count; i++)
        {
            var parent = parents[i];
            var nextIndex = i + 1;
            var child = nextIndex < parents.Count ? parents[nextIndex] : node;
            if (!parent.IsChild(child))
            {
                missingLinkAt = i;
                return false;
            }

            if (!set.Add(child.Code))
                return false;
        }
        return true;
    }

    public GmodPath(IReadOnlyList<GmodNode> parents, GmodNode node) : this(parents, node, false) { }

    public GmodPath WithoutLocations() =>
        new GmodPath(Parents.Select(n => n.WithoutLocation()).ToList(), Node.WithoutLocation());

    public override string ToString()
    {
        using var lease = StringBuilderPool.Get();
        ToString(lease.Builder);
        return lease.ToString();
    }

    public void ToString(StringBuilder builder, char separator = '/')
    {
        foreach (var parent in Parents)
        {
            if (!Gmod.IsLeafNode(parent.Metadata))
                continue;

            parent.ToString(builder);
            builder.Append(separator);
        }

        Node.ToString(builder);
    }

    public string ToFullPathString()
    {
        using var lease = StringBuilderPool.Get();
        ToFullPathString(lease.Builder);
        return lease.ToString();
    }

    public void ToFullPathString(StringBuilder builder)
    {
        foreach (var (depth, pathNode) in GetFullPath())
        {
            pathNode.ToString(builder);

            if (depth != (Length - 1))
                builder.Append('/');
        }
    }

    public string ToStringDump()
    {
        using var lease = StringBuilderPool.Get();
        ToStringDump(lease.Builder);
        return lease.ToString();
    }

    public void ToStringDump(StringBuilder builder)
    {
        foreach (var (depth, pathNode) in GetFullPath())
        {
            if (depth == 0)
                continue;

            if (depth != 1)
                builder.Append(" | ");

            builder.Append(pathNode.Code);

            if (!string.IsNullOrWhiteSpace(pathNode.Metadata.Name))
            {
                builder.Append("/N:");
                builder.Append(pathNode.Metadata.Name);
            }

            if (!string.IsNullOrWhiteSpace(pathNode.Metadata.CommonName))
            {
                builder.Append("/CN:");
                builder.Append(pathNode.Metadata.CommonName);
            }

            var normalAssignmentName = GetNormalAssignmentName(depth);
            if (!string.IsNullOrWhiteSpace(normalAssignmentName))
            {
                builder.Append("/NAN:");
                builder.Append(normalAssignmentName);
            }
        }
    }

    public bool Equals(GmodPath? other)
    {
        if (other is null)
            return false;

        if (Parents.Count != other.Parents.Count)
            return false;

        for (int i = 0; i < Parents.Count; i++)
        {
            if (Parents[i] != other.Parents[i])
                return false;
        }

        return Node == other.Node;
    }

    public override int GetHashCode()
    {
        var hashCode = new HashCode();
        for (int i = 0; i < Parents.Count; i++)
            hashCode.Add(Parents[i]);

        hashCode.Add(Node);
        return hashCode.ToHashCode();
    }

    public Enumerator GetFullPath() => new Enumerator(this);

    public Enumerator GetFullPathFrom(int fromDepth) => new Enumerator(this, fromDepth);

    public struct Enumerator
        : IEnumerable<(int Depth, GmodNode Node)>,
          IEnumerator<(int Depth, GmodNode Node)>
    {
        private readonly GmodPath _path;

        public void Reset() { }

        object IEnumerator.Current => Current;

        public (int Depth, GmodNode Node) Current { get; private set; }

        public Enumerator(GmodPath path, int? fromDepth = null)
        {
            _path = path;
            Current = (-1, null!);
            if (fromDepth is not null)
            {
                if (fromDepth < 0 || fromDepth > _path.Parents.Count)
                    throw new ArgumentOutOfRangeException(nameof(fromDepth));

                Current = (
                    fromDepth.Value - 1,
                    fromDepth == 0 ? null! : _path[fromDepth.Value - 1]
                );
            }
        }

        public bool MoveNext()
        {
            if (Current.Depth < _path.Parents.Count)
            {
                Current =
                    Current.Depth == _path.Parents.Count - 1
                        ? (Current.Depth + 1, _path.Node)
                        : (Current.Depth + 1, _path.Parents[Current.Depth + 1]);
                return true;
            }

            return false;
        }

        IEnumerator IEnumerable.GetEnumerator() => this;

        public IEnumerator<(int Depth, GmodNode Node)> GetEnumerator() => this;

        public void Dispose() { }
    }

    public string? GetNormalAssignmentName(int nodeDepth)
    {
        var node = this[nodeDepth];
        var normalAssignmentNames = node.Metadata.NormalAssignmentNames;
        if (normalAssignmentNames is null || normalAssignmentNames.Count == 0)
            return null;

        for (int i = Length - 1; i >= 0; i--)
        {
            var child = this[i];
            if (normalAssignmentNames.TryGetValue(child.Code, out var name))
                return name;
        }

        return null;
    }

    public IEnumerable<(int Depth, string Name)> GetCommonNames()
    {
        foreach (var (depth, node) in this.GetFullPath())
        {
            var isTarget = depth == Parents.Count;
            if (!(node.IsLeafNode || isTarget) || !node.IsFunctionNode)
                continue;

            var name = node.Metadata.CommonName ?? node.Metadata.Name;
            var normalAssignmentNames = node.Metadata.NormalAssignmentNames;

            if (normalAssignmentNames is not null)
            {
                {
                    if (normalAssignmentNames.TryGetValue(Node.Code, out var assignment))
                        name = assignment;
                }
                for (int i = Parents.Count - 1; i >= depth; i--)
                {
                    if (!normalAssignmentNames.TryGetValue(Parents[i].Code, out var assignment))
                        continue;

                    name = assignment;
                }
            }

            yield return (depth, name);
        }
    }

    private readonly record struct PathNode(string Code, Location? Location = null);

    private sealed record ParseContext(Queue<PathNode> Parts)
    {
        public PathNode ToFind;
        public Dictionary<string, Location>? Locations;
        public GmodPath? Path;
    }

    public static GmodPath Parse(string item, VisVersion visVersion)
    {
        if (!TryParse(item, visVersion, out var path))
            throw new ArgumentException("Couldnt parse path");

        return path;
    }

    public static bool TryParse(
        string? item,
        VisVersion visVersion,
        [NotNullWhen(true)] out GmodPath? path
    )
    {
        var gmod = VIS.Instance.GetGmod(visVersion);
        var locations = VIS.Instance.GetLocations(visVersion);
        return TryParse(item, gmod, locations, out path);
    }

    public static GmodPath Parse(string item, Gmod gmod, Locations locations)
    {
        if (!TryParse(item, gmod, locations, out var path))
            throw new ArgumentException("Couldnt parse path");

        return path;
    }

    public static bool TryParse(
        string? item,
        Gmod gmod,
        Locations locations,
        [NotNullWhen(true)] out GmodPath? path
    )
    {
        if (gmod.VisVersion != locations.VisVersion)
            throw new ArgumentException(
                "Got different VIS versions for Gmod and Locations arguments"
            );

        path = null;
        if (string.IsNullOrWhiteSpace(item))
            return false;

        item = item!.Trim().TrimStart('/');

        var parts = new Queue<PathNode>();
        foreach (var partStr in item.Split('/'))
        {
            if (partStr.Contains('-'))
            {
                var split = partStr.Split('-');
                if (!locations.TryParse(split[1], out var location))
                    return false;
                parts.Enqueue(new PathNode(split[0], location));
            }
            else
            {
                parts.Enqueue(new PathNode(partStr));
            }
        }

        if (parts.Count == 0)
            return false;
        if (parts.Any(p => string.IsNullOrWhiteSpace(p.Code)))
            return false;

        var toFind = parts.Dequeue();
        if (!gmod.TryGetNode(toFind.Code, out var baseNode))
            return false;

        var context = new ParseContext(parts) { ToFind = toFind };

        gmod.Traverse(
            context,
            baseNode,
            (context, parents, current) =>
            {
                ref var toFind = ref context.ToFind;
                var found = current.Code == toFind.Code;

                if (!found && Gmod.IsLeafNode(current.Metadata))
                    return TraversalHandlerResult.SkipSubtree;

                if (!found)
                    return TraversalHandlerResult.Continue;

                if (toFind.Location is not null)
                {
                    context.Locations ??= new();
                    context.Locations.Add(toFind.Code, toFind.Location.Value);
                }

                if (context.Parts.Count > 0)
                {
                    toFind = context.Parts.Dequeue();
                    return TraversalHandlerResult.Continue;
                }

                var pathParents = new List<GmodNode>(parents.Count + 1);
                foreach (var parent in parents)
                {
                    if (context.Locations?.TryGetValue(parent.Code, out var location) ?? false)
                        pathParents.Add(parent.WithLocation(location));
                    else
                        pathParents.Add(parent);
                }
                var endNode = toFind.Location is not null
                    ? current.WithLocation(toFind.Location)
                    : current;

                var startNode =
                    pathParents.Count > 0 && pathParents[0].Parents.Count == 1
                        ? pathParents[0].Parents[0]
                        : endNode.Parents.Count == 1
                            ? endNode.Parents[0]
                            : null;

                if (startNode is null || startNode.Parents.Count > 1)
                    return TraversalHandlerResult.Stop;

                while (startNode.Parents.Count == 1)
                {
                    pathParents.Insert(0, startNode);
                    startNode = startNode.Parents[0];
                    if (startNode.Parents.Count > 1)
                        return TraversalHandlerResult.Stop;
                }

                pathParents.Insert(0, gmod.RootNode);

                context.Path = new GmodPath(pathParents, endNode);
                return TraversalHandlerResult.Stop;
            }
        );

        if (context.Path is null)
            return false;

        path = context.Path;
        return true;
    }

    public static bool TryParseFullPath(
        string? pathStr,
        VisVersion visVersion,
        [MaybeNullWhen(false)] out GmodPath path
    )
    {
        var vis = VIS.Instance;
        var gmod = vis.GetGmod(visVersion);
        var locations = vis.GetLocations(visVersion);
        return TryParseFullPathInternal(pathStr.AsSpan(), gmod, locations, out path);
    }

    public static bool TryParseFullPath(
        ReadOnlySpan<char> pathStr,
        VisVersion visVersion,
        [MaybeNullWhen(false)] out GmodPath path
    )
    {
        var vis = VIS.Instance;
        var gmod = vis.GetGmod(visVersion);
        var locations = vis.GetLocations(visVersion);
        return TryParseFullPathInternal(pathStr, gmod, locations, out path);
    }

    public static bool TryParseFullPath(
        string? pathStr,
        Gmod gmod,
        Locations locations,
        [MaybeNullWhen(false)] out GmodPath path
    ) => TryParseFullPathInternal(pathStr.AsSpan(), gmod, locations, out path);

    public static bool TryParseFullPath(
        ReadOnlySpan<char> pathStr,
        Gmod gmod,
        Locations locations,
        [MaybeNullWhen(false)] out GmodPath path
    ) => TryParseFullPathInternal(pathStr, gmod, locations, out path);

    private static bool TryParseFullPathInternal(
        ReadOnlySpan<char> span,
        Gmod gmod,
        Locations locations,
        [MaybeNullWhen(false)] out GmodPath path
    )
    {
        Debug.Assert(gmod.VisVersion == locations.VisVersion);

        path = default;
        if (span.IsEmpty || span.IsWhiteSpace())
            return false;

        var nodes = new List<GmodNode>(1);
        foreach (ReadOnlySpan<char> nodeStr in span.Split('/'))
        {
            var dashIndex = nodeStr.IndexOf('-');
            if (dashIndex == -1)
            {
                if (!gmod.TryGetNode(nodeStr, out var node))
                    return false;

                nodes.Add(node);
            }
            else
            {
                if (!gmod.TryGetNode(nodeStr.Slice(0, dashIndex), out var node))
                    return false;

                var locationStr = nodeStr.Slice(dashIndex + 1);
                if (!locations.TryParse(locationStr, out var location))
                    return false;

                nodes.Add(node.WithLocation(location));
            }
        }

        if (nodes.Count == 0)
            return false;

        var endNode = nodes[nodes.Count - 1];
        nodes.RemoveAt(nodes.Count - 1);
        if (!IsValid(nodes, endNode))
            return false;

        path = new GmodPath(nodes, endNode, skipVerify: true);
        return true;
    }
}
