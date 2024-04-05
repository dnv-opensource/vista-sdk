using System.Collections;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Text;
using Vista.SDK.Internal;

namespace Vista.SDK;

public sealed record GmodIndividualizableSet
{
    private readonly List<int> _nodes;
    private GmodPath _path;

    public IReadOnlyList<GmodNode> Nodes => _nodes.Select(i => _path[i]).ToArray();

    public IReadOnlyList<int> NodeIndices => _nodes;

    internal GmodIndividualizableSet(List<int> nodes, GmodPath path)
    {
        if (nodes.Count == 0)
            throw new Exception("GmodIndividualizableSet cant be empty");
        if (nodes.Any(i => !path[i].IsIndividualizable(i == path.Length - 1, nodes.Count > 1)))
            throw new Exception("GmodIndividualizableSet nodes must be individualizable");
        if (nodes.Select(i => path[i].Location).Distinct().Count() != 1)
            throw new Exception("GmodIndividualizableSet nodes have different locations");
        if (!nodes.Any(i => path[i] == path.Node || path[i].IsLeafNode))
            throw new Exception("GmodIndividualizableSet has no nodes that are part of short path");

        _nodes = nodes;
        _path = path with { _parents = path._parents.ToList(), Node = path.Node with { } };
    }

    public Location? Location
    {
        get => _path[_nodes[0]].Location;
        set
        {
            for (int i = 0; i < _nodes.Count; i++)
            {
                if (value is null)
                    _path[_nodes[i]] = _path[_nodes[i]].WithoutLocation();
                else
                    _path[_nodes[i]] = _path[_nodes[i]].WithLocation(value.Value);
            }
        }
    }

    public GmodPath Build()
    {
        var path = _path ?? throw new Exception("Tried to build individualizable set twice");
        _path = null!;
        return path;
    }

    public override string ToString() =>
        string.Join(
            "/",
            _nodes.Where((i, j) => _path[i].IsLeafNode || j == _nodes.Count - 1).Select(i => _path[i].ToString())
        );
}

public sealed record GmodPath
{
    internal List<GmodNode> _parents { get; init; }
    public IReadOnlyList<GmodNode> Parents => _parents;

    public GmodNode Node { get; internal set; }

    public int Length => _parents.Count + 1;

    public bool IsMappable => Node.IsMappable;

    public GmodNode this[int depth]
    {
        get
        {
            if (depth < 0)
                throw new IndexOutOfRangeException("Index out of range for GmodPath indexer");
            if (depth > _parents.Count)
                throw new IndexOutOfRangeException("Index out of range for GmodPath indexer");

            return depth < _parents.Count ? _parents[depth] : Node;
        }
        internal set
        {
            if (depth < _parents.Count)
                _parents[depth] = value;
            else
                Node = value;
        }
    }

    public IReadOnlyList<GmodIndividualizableSet> IndividualizableSets
    {
        get
        {
            var result = new List<GmodIndividualizableSet>();
            var visitor = new LocationSetsVisitor();
            for (int i = 0; i < Length; i++)
            {
                var node = this[i];
                var set = visitor.Visit(node, i, _parents, Node);
                if (set is null)
                    continue;

                var (start, end, _) = set.Value;
                if (start == end)
                {
                    result.Add(new GmodIndividualizableSet(new List<int>() { start }, this));
                    continue;
                }

                var nodes = new List<int>(end - start);
                for (int j = start; j <= end; j++)
                    nodes.Add(j);

                result.Add(new GmodIndividualizableSet(nodes, this));
            }
            return result;
        }
    }

    public bool IsIndividualizable
    {
        get
        {
            var visitor = new LocationSetsVisitor();
            for (int i = 0; i < Length; i++)
            {
                var node = this[i];
                var set = visitor.Visit(node, i, _parents, Node);
                if (set is null)
                    continue;

                return true;
            }

            return false;
        }
    }

    internal GmodPath(List<GmodNode> parents, GmodNode node, bool skipVerify = true)
    {
        if (!skipVerify)
        {
            if (parents.Count == 0)
                throw new ArgumentException($"Invalid gmod path - no parents, and {node.Code} is not the root of gmod");
            if (parents.Count > 0 && !parents[0].IsRoot)
                throw new ArgumentException(
                    $"Invalid gmod path - first parent should be root of gmod (VE), but was {parents[0].Code}"
                );

#if NETCOREAPP3_1_OR_GREATER
            var set = new HashSet<string>(parents.Count);
#else
            var set = new HashSet<string>();
#endif
            set.Add("VE");

            for (int i = 0; i < parents.Count; i++)
            {
                var parent = parents[i];
                var nextIndex = i + 1;
                var child = nextIndex < parents.Count ? parents[nextIndex] : node;
                if (!parent.IsChild(child))
                    throw new ArgumentException($"Invalid gmod path - {child.Code} not child of {parent.Code}");

                // if (!set.Add(child.Code))
                //     throw new ArgumentException($"Recursion in gmod path argument for code: {child.Code}");
            }

            var visitor = new LocationSetsVisitor();
            for (int i = 0; i < parents.Count + 1; i++)
            {
                var n = i < parents.Count ? parents[i] : node;
                var _ = visitor.Visit(n, i, parents, node);
            }
        }

        _parents = parents;
        Node = node;
    }

    public static bool IsValid(IReadOnlyList<GmodNode> parents, GmodNode node) => IsValid(parents, node, out _);

    internal static bool IsValid(IReadOnlyList<GmodNode> parents, GmodNode node, out int missingLinkAt)
    {
        missingLinkAt = -1;

        if (parents.Count == 0)
            return false;

        if (parents.Count > 0 && !parents[0].IsRoot)
            return false;

#if NETCOREAPP3_1_OR_GREATER
        var set = new HashSet<string>(parents.Count + 1, StringComparer.Ordinal) { "VE" };
#else
        var set = new HashSet<string>();
        set.Add("VE");
#endif

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

            // if (!set.Add(child.Code))
            //     return false;
        }
        return true;
    }

    public GmodPath(List<GmodNode> parents, GmodNode node)
        : this(parents, node, false) { }

    public GmodPath WithoutLocations() =>
        new GmodPath(_parents.Select(n => n.WithoutLocation()).ToList(), Node.WithoutLocation());

    public override string ToString()
    {
        using var lease = StringBuilderPool.Get();
        ToString(lease.Builder);
        return lease.ToString();
    }

    public void ToString(StringBuilder builder, char separator = '/')
    {
        foreach (var parent in _parents)
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

        if (_parents.Count != other._parents.Count)
            return false;

        for (int i = 0; i < _parents.Count; i++)
        {
            if (_parents[i] != other._parents[i])
                return false;
        }

        return Node == other.Node;
    }

    public override int GetHashCode()
    {
        var hashCode = new HashCode();
        for (int i = 0; i < _parents.Count; i++)
            hashCode.Add(_parents[i]);

        hashCode.Add(Node);
        return hashCode.ToHashCode();
    }

    public Enumerator GetFullPath() => new Enumerator(this);

    public Enumerator GetFullPathFrom(int fromDepth) => new Enumerator(this, fromDepth);

    public struct Enumerator : IEnumerable<(int Depth, GmodNode Node)>, IEnumerator<(int Depth, GmodNode Node)>
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
                if (fromDepth < 0 || fromDepth > _path._parents.Count)
                    throw new ArgumentOutOfRangeException(nameof(fromDepth));

                Current = (fromDepth.Value - 1, fromDepth == 0 ? null! : _path[fromDepth.Value - 1]);
            }
        }

        public bool MoveNext()
        {
            if (Current.Depth < _path._parents.Count)
            {
                Current =
                    Current.Depth == _path._parents.Count - 1
                        ? (Current.Depth + 1, _path.Node)
                        : (Current.Depth + 1, _path._parents[Current.Depth + 1]);
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
            var isTarget = depth == _parents.Count;
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
                for (int i = _parents.Count - 1; i >= depth; i--)
                {
                    if (!normalAssignmentNames.TryGetValue(_parents[i].Code, out var assignment))
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

    public static bool TryParse(string? item, VisVersion visVersion, [NotNullWhen(true)] out GmodPath? path)
    {
        var gmod = VIS.Instance.GetGmod(visVersion);
        var locations = VIS.Instance.GetLocations(visVersion);
        return TryParse(item, gmod, locations, out path);
    }

    public static GmodPath Parse(string item, Gmod gmod, Locations locations)
    {
        var result = ParseInternal(item, gmod, locations);

        return result switch
        {
            GmodParsePathResult.Ok r => r.Path,
            GmodParsePathResult.Err e => throw new ArgumentException(e.Error),
            _ => throw new Exception("Unexpected result")
        };
    }

    private record struct LocationSetsVisitor
    {
        public int currentParentStart;

        public LocationSetsVisitor() => currentParentStart = -1;

        public (int Start, int End, Location? Location)? Visit(
            GmodNode node,
            int i,
            IReadOnlyList<GmodNode> parents,
            GmodNode target
        )
        {
            var isParent = Gmod.IsPotentialParent(node.Metadata.Type);
            var isTargetNode = i == parents.Count;
            if (currentParentStart == -1)
            {
                if (isParent)
                    currentParentStart = i;
                if (node.IsIndividualizable(isTargetNode))
                    return (i, i, node.Location); // TODO - is this correct?
            }
            else
            {
                if (isParent || isTargetNode)
                {
                    (int Start, int End, Location? Location)? nodes = null;
                    if (currentParentStart + 1 == i)
                    {
                        if (node.IsIndividualizable(isTargetNode))
                            nodes = (i, i, node.Location);
                    }
                    else
                    {
                        var skippedOne = -1;
                        var hasComposition = false;
                        for (var j = currentParentStart + 1; j <= i; j++)
                        {
                            var setNode = j < parents.Count ? parents[j] : target;
                            if (!setNode.IsIndividualizable(j == parents.Count, isInSet: true))
                            {
                                if (nodes is not null)
                                    skippedOne = j;
                                continue;
                            }

                            if (
                                nodes?.Location is not null
                                && setNode.Location is not null
                                && nodes.Value.Location != setNode.Location
                            )
                                throw new Exception(
                                    $"Mapping error: different locations in the same nodeset: {nodes.Value.Location}, {setNode.Location}"
                                );

                            if (skippedOne != -1)
                                throw new Exception("Can't skip in the middle of individualizable set");

                            if (setNode.IsFunctionComposition)
                                hasComposition = true;

                            var location = nodes?.Location is null ? setNode.Location : nodes?.Location;
                            var start = nodes?.Start ?? j;
                            var end = j;
                            nodes = (start, end, location);
                        }

                        if (nodes is not null && nodes.Value.Start == nodes.Value.End && hasComposition)
                            nodes = null;
                    }

                    currentParentStart = i;
                    if (nodes is not null)
                    {
                        var hasLeafNode = false;
                        for (int j = nodes.Value.Start; j <= nodes.Value.End; j++)
                        {
                            var setNode = j < parents.Count ? parents[j] : target;
                            if (setNode.IsLeafNode || j == parents.Count)
                            {
                                hasLeafNode = true;
                                break;
                            }
                        }
                        if (hasLeafNode)
                            return nodes;
                    }
                }

                if (isTargetNode && node.IsIndividualizable(isTargetNode))
                    return (i, i, node.Location);
            }

            return null;
        }
    }

    public static bool TryParse(string? item, Gmod gmod, Locations locations, [NotNullWhen(true)] out GmodPath? path)
    {
        var result = ParseInternal(item, gmod, locations);
        path = null;
        if (result is GmodParsePathResult.Ok r)
        {
            path = r.Path;
            return true;
        }
        return false;
    }

    private static GmodParsePathResult ParseInternal(string? item, Gmod gmod, Locations locations)
    {
        if (gmod.VisVersion != locations.VisVersion)
            throw new ArgumentException("Got different VIS versions for Gmod and Locations arguments");

        if (string.IsNullOrWhiteSpace(item))
            return new GmodParsePathResult.Err("Item is empty");

        item = item!.Trim().TrimStart('/');

        var parts = new Queue<PathNode>();
        foreach (var partStr in item.Split('/'))
        {
            if (partStr.Contains('-'))
            {
                var split = partStr.Split('-');
                if (!gmod.TryGetNode(split[0], out _))
                    return new GmodParsePathResult.Err($"Failed to get GmodNode for {partStr}");
                if (!locations.TryParse(split[1], out var location))
                    return new GmodParsePathResult.Err($"Failed to parse location {split[1]}");
                parts.Enqueue(new PathNode(split[0], location));
            }
            else
            {
                if (!gmod.TryGetNode(partStr, out _))
                    return new GmodParsePathResult.Err($"Failed to get GmodNode for {partStr}");
                parts.Enqueue(new PathNode(partStr));
            }
        }

        if (parts.Count == 0)
            return new GmodParsePathResult.Err("Failed find any parts");
        if (parts.Any(p => string.IsNullOrWhiteSpace(p.Code)))
            return new GmodParsePathResult.Err("Failed find any parts");

        var toFind = parts.Dequeue();
        if (!gmod.TryGetNode(toFind.Code, out var baseNode))
            return new GmodParsePathResult.Err("Failed to find base node");

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
                var endNode = toFind.Location is not null ? current.WithLocation(toFind.Location) : current;

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

                var visitor = new LocationSetsVisitor();
                for (var i = 0; i < pathParents.Count + 1; i++)
                {
                    var n = i < pathParents.Count ? pathParents[i] : endNode;
                    var set = visitor.Visit(n, i, pathParents, endNode);
                    if (set is null)
                    {
                        if (n.Location is not null)
                            return TraversalHandlerResult.Stop;
                        continue;
                    }

                    var (start, end, location) = set.Value;
                    if (start == end)
                        continue;

                    for (int j = start; j <= end; j++)
                    {
                        if (j < pathParents.Count)
                            pathParents[j] = pathParents[j] with { Location = location };
                        else
                            endNode = endNode with { Location = location };
                    }
                }

                context.Path = new GmodPath(pathParents, endNode);
                return TraversalHandlerResult.Stop;
            }
        );

        if (context.Path is null)
            return new GmodParsePathResult.Err("Failed to find path after travesal");

        return new GmodParsePathResult.Ok(context.Path);
    }

    public static GmodPath ParseFullPath(string pathStr, VisVersion visVersion)
    {
        var vis = VIS.Instance;
        var gmod = vis.GetGmod(visVersion);
        var locations = vis.GetLocations(visVersion);
        var result = ParseFullPathInternal(pathStr.AsSpan(), gmod, locations);

        return result switch
        {
            GmodParsePathResult.Ok r => r.Path,
            GmodParsePathResult.Err e => throw new ArgumentException(e.Error),
            _ => throw new Exception("Unexpected result")
        };
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
        return TryParseFullPath(pathStr.AsSpan(), gmod, locations, out path);
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
        return TryParseFullPath(pathStr, gmod, locations, out path);
    }

    public static bool TryParseFullPath(
        string? pathStr,
        Gmod gmod,
        Locations locations,
        [MaybeNullWhen(false)] out GmodPath path
    ) => TryParseFullPath(pathStr.AsSpan(), gmod, locations, out path);

    private static bool TryParseFullPath(
        ReadOnlySpan<char> span,
        Gmod gmod,
        Locations locations,
        [MaybeNullWhen(false)] out GmodPath path
    )
    {
        var result = ParseFullPathInternal(span, gmod, locations);
        if (result is GmodParsePathResult.Ok r)
        {
            path = r.Path;
            return true;
        }
        path = null;
        return false;
    }

    private static GmodParsePathResult ParseFullPathInternal(ReadOnlySpan<char> span, Gmod gmod, Locations locations)
    {
        Debug.Assert(gmod.VisVersion == locations.VisVersion);

        if (span.IsEmpty || span.IsWhiteSpace())
            return new GmodParsePathResult.Err("Item is empty");

        if (!span.StartsWith(gmod.RootNode.Code.AsSpan(), StringComparison.Ordinal))
            return new GmodParsePathResult.Err($"Path must start with {gmod.RootNode.Code}");

        var nodes = new List<GmodNode>(span.Length / 3);
        foreach (ReadOnlySpan<char> nodeStr in span.Split('/'))
        {
            var dashIndex = nodeStr.IndexOf('-');

            GmodNode? node;
            if (dashIndex == -1)
            {
                if (!gmod.TryGetNode(nodeStr, out node))
                    return new GmodParsePathResult.Err($"Failed to get GmodNode for {nodeStr.ToString()}");
            }
            else
            {
                var slice = nodeStr.Slice(0, dashIndex);
                if (!gmod.TryGetNode(slice, out node))
                    return new GmodParsePathResult.Err($"Failed to get GmodNode for {slice.ToString()}");

                var locationStr = nodeStr.Slice(dashIndex + 1);
                if (!locations.TryParse(locationStr, out var location))
                    return new GmodParsePathResult.Err($"Failed to parse location - {locationStr.ToString()}");

                node = node.WithLocation(location);
            }

            nodes.Add(node);
        }

        if (nodes.Count == 0)
            return new GmodParsePathResult.Err("Failed to find any nodes");

        var endNode = nodes[nodes.Count - 1];
        nodes.RemoveAt(nodes.Count - 1);
        if (!IsValid(nodes, endNode))
            return new GmodParsePathResult.Err("Sequence of nodes are invalid");

        var visitor = new LocationSetsVisitor();
        int? prevNonNullLocation = null;
        Span<(int Start, int End)> sets = stackalloc (int, int)[16];
        var setCounter = 0;
        for (var i = 0; i < nodes.Count + 1; i++)
        {
            var n = i < nodes.Count ? nodes[i] : endNode;
            var set = visitor.Visit(n, i, nodes, endNode);
            if (set is null)
            {
                if (prevNonNullLocation is null && n.Location is not null)
                    prevNonNullLocation = i;
                continue;
            }
            var (start, end, location) = set.Value;

            if (prevNonNullLocation is int sj)
            {
                for (int j = sj; j < start; j++)
                {
                    var pn = j < nodes.Count ? nodes[j] : endNode;
                    if (pn.Location is not null)
                        return new GmodParsePathResult.Err(
                            $"Expected all nodes in the set to be without individualization. Found {pn}"
                        ); // This one is between
                }
            }
            prevNonNullLocation = null;

            sets[setCounter++] = (start, end);
            if (start == end)
                continue;

            for (int j = start; j <= end; j++)
            {
                if (j < nodes.Count)
                    nodes[j] = nodes[j] with { Location = location };
                else
                    endNode = endNode with { Location = location };
            }
        }

        (int Start, int End) currentSet = (-1, -1);
        var currentSetIndex = 0;
        for (var i = 0; i < nodes.Count + 1; i++)
        {
            while (currentSetIndex < setCounter && currentSet.End < i)
                currentSet = sets[currentSetIndex++];

            var insideSet = i >= currentSet.Start && i <= currentSet.End;

            var n = i < nodes.Count ? nodes[i] : endNode;
            var expectedLocationNode =
                currentSet.End != -1 && currentSet.End < nodes.Count ? nodes[currentSet.End] : endNode;

            if (insideSet)
            {
                if (n.Location != expectedLocationNode.Location)
                    return new GmodParsePathResult.Err(
                        $"Expected all nodes in the set to be individualized the same. Found {n.Code} with location {n.Location}"
                    );
            }
            else
            {
                if (n.Location is not null)
                    return new GmodParsePathResult.Err(
                        $"Expected all nodes in the set to be without individualization. Found {n}"
                    );
            }
        }

        return new GmodParsePathResult.Ok(new GmodPath(nodes, endNode, skipVerify: true));
    }
}

public abstract record GmodParsePathResult
{
    private GmodParsePathResult() { }

    public record Ok(GmodPath Path) : GmodParsePathResult;

    public record Err(string Error) : GmodParsePathResult;
}
