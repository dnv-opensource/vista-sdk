namespace Vista.SDK;

public delegate TraversalHandlerResult TraverseHandler(IReadOnlyList<GmodNode> parents, GmodNode node);
public delegate TraversalHandlerResult TraverseHandlerWithState<TState>(
    TState state,
    IReadOnlyList<GmodNode> parents,
    GmodNode node
);

public enum TraversalHandlerResult
{
    Stop,
    SkipSubtree,
    Continue,
}

public record TraversalOptions()
{
    internal const int DEFAULT_MAX_TRAVERSAL_OCCURRENCE = 1;

    /// <summary>The maximum number of times a node can occur in a path. The traversal will stop and the first node to reach the limit, and include it as the end node. Increasing this will drastically reduce performance</summary>
    /// <remarks>The default value is 1.</remarks>
    /// <example>411.1/C101.63/S206.22/S110.2/C101 = MaxTraversalOccurrence 1. The travesal algorithm finds the second instance of C101 in the path, and stop, but include the node in the result.</example>
    public int MaxTraversalOccurrence { get; set; } = DEFAULT_MAX_TRAVERSAL_OCCURRENCE;
}

public sealed partial class Gmod
{
    public bool Traverse(TraverseHandler handler, TraversalOptions? options = null) =>
        Traverse(handler, _rootNode, (handler, parents, node) => handler(parents, node), options);

    public bool Traverse<TState>(
        TState state,
        TraverseHandlerWithState<TState> handler,
        TraversalOptions? options = null
    ) => Traverse(state, _rootNode, handler, options);

    public bool Traverse(GmodNode rootNode, TraverseHandler handler, TraversalOptions? options = null) =>
        Traverse(handler, rootNode, (handler, parents, node) => handler(parents, node), options);

    public bool Traverse<TState>(
        TState state,
        GmodNode rootNode,
        TraverseHandlerWithState<TState> handler,
        TraversalOptions? options = null
    )
    {
        var opts = options ?? new TraversalOptions();
        var context = new TraversalContext<TState>(new Parents(), handler, state, opts.MaxTraversalOccurrence);
        return TraverseNode(in context, rootNode) == TraversalHandlerResult.Continue;
    }

    private TraversalHandlerResult TraverseNode<TState>(in TraversalContext<TState> context, GmodNode node)
    {
        // note: installSubstructure doesn't work - martinothamar
        // if (node.Metadata.InstallSubstructure == false)
        //     return TraversalHandlerResult.Continue;
        var result = context.Handler(context.State, context.Parents.AsList, node);
        if (result is TraversalHandlerResult.Stop or TraversalHandlerResult.SkipSubtree)
            return result;

        var skipOccurenceCheck = IsProductSelectionAssignment(context.Parents.LastOrDefault(), node);
        // Skip the occurence check for "hidden" nodes such as selections, etc.
        if (!skipOccurenceCheck)
        {
            var occ = context.Parents.Occurrences(node);
            if (occ == context.MaxTraversalOccurrence)
                return TraversalHandlerResult.SkipSubtree;
            if (occ > context.MaxTraversalOccurrence)
                throw new Exception("Invalid state - node occured more than expected");
        }
        context.Parents.Push(node);

        for (int i = 0; i < node.Children.Count; i++)
        {
            var child = node.Children[i];
            result = TraverseNode(in context, child);
            if (result is TraversalHandlerResult.Stop)
                return result;
            else if (result is TraversalHandlerResult.SkipSubtree)
                continue;
        }

        context.Parents.Pop();
        return TraversalHandlerResult.Continue;
    }

    internal bool PathExistsBetween(
        IEnumerable<GmodNode> fromPath,
        GmodNode to,
        out IEnumerable<GmodNode> remainingParents
    )
    {
        var lastAssetFunction = fromPath.LastOrDefault(n => n.IsAssetFunctionNode);
        remainingParents =  [];

        var state = new PathExistsContext(to) { RemainingParents = remainingParents, FromPath =  [.. fromPath] };

        var reachedEnd = Traverse(
            state,
            lastAssetFunction ?? RootNode,
            (state, parents, node) =>
            {
                if (node.Code != state.To.Code)
                    return TraversalHandlerResult.Continue;
                List<GmodNode>? actualParents = null;
                while (!parents[0].IsRoot)
                {
                    if (actualParents is null)
                    {
                        actualParents =  [.. parents];
                        parents = actualParents;
                    }

                    var parent = parents[0];
                    if (parent.Parents.Count != 1)
                        throw new Exception("Invalid state - expected one parent");

                    actualParents.Insert(0, parent.Parents[0]);
                }

                // Validate parents
                if (parents.Count < state.FromPath.Count)
                    return TraversalHandlerResult.Continue;
                // Must have same start order
                var match = true;
                for (int i = 0; i < state.FromPath.Count; i++)
                {
                    if (parents[i].Code != state.FromPath[i].Code)
                    {
                        match = false;
                        break;
                    }
                }

                if (match)
                {
                    state.RemainingParents = parents.Where(p => !state.FromPath.Any(pp => pp.Code == p.Code)).ToArray();
                    return TraversalHandlerResult.Stop;
                }

                return TraversalHandlerResult.Continue;
            }
        );

        remainingParents = state.RemainingParents;

        return !reachedEnd;
    }

    record PathExistsContext(GmodNode To)
    {
        public IEnumerable<GmodNode> RemainingParents = Enumerable.Empty<GmodNode>();
        public required IReadOnlyList<GmodNode> FromPath { get; init; }
    }

    private readonly record struct TraversalContext<TState>(
        Parents Parents,
        TraverseHandlerWithState<TState> Handler,
        TState State,
        int MaxTraversalOccurrence
    );

    private readonly struct Parents
    {
        private readonly Dictionary<string, int> _occurrences = new(4);
        private readonly List<GmodNode> _parents = new(64);

        public Parents() { }

        public readonly void Push(GmodNode parent)
        {
            _parents.Add(parent);
            if (_occurrences.ContainsKey(parent.Code))
                _occurrences[parent.Code]++;
            else
                _occurrences.Add(parent.Code, 1);
        }

        public readonly void Pop()
        {
            var parent = _parents[_parents.Count - 1];
            _parents.RemoveAt(_parents.Count - 1);

            if (_occurrences.TryGetValue(parent.Code, out var occ))
            {
                if (occ == 1)
                    _occurrences.Remove(parent.Code);
                else
                    _occurrences[parent.Code]--;
            }
        }

        public readonly int Occurrences(GmodNode node) => _occurrences.TryGetValue(node.Code, out var occ) ? occ : 0;

        public readonly GmodNode? LastOrDefault() => _parents.Count > 0 ? _parents[_parents.Count - 1] : null;

        public readonly IReadOnlyList<GmodNode> ToList() => _parents.ToList();

        public readonly IReadOnlyList<GmodNode> AsList => _parents;
    }
}
