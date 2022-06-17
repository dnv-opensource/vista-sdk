namespace Vista.SDK;

public delegate TraversalHandlerResult TraverseHandler(
    IReadOnlyList<GmodNode> parents,
    GmodNode node
);
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

public sealed partial class Gmod
{
    public bool Traverse(TraverseHandler handler) =>
        Traverse(handler, _rootNode, (handler, parents, node) => handler(parents, node));

    public bool Traverse<TState>(TState state, TraverseHandlerWithState<TState> handler) =>
        Traverse(state, _rootNode, handler);

    public bool Traverse(GmodNode rootNode, TraverseHandler handler) =>
        Traverse(handler, rootNode, (handler, parents, node) => handler(parents, node));

    internal bool PathExistsBetween(
        IEnumerable<GmodNode> fromPath,
        GmodNode to,
        out IEnumerable<GmodNode> remainingParents
    )
    {
        var lastAssetFunction = fromPath.LastOrDefault(n => n.IsAssetFunctionNode);
        remainingParents = Enumerable.Empty<GmodNode>();

        var state = new PathExistsContext(to) { RemainingParents = remainingParents, };

        var reachedEnd = this.Traverse(
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
                        actualParents = new(parents);
                        parents = actualParents;
                    }

                    var parent = parents[0];
                    if (parent.Parents.Count != 1)
                        throw new Exception("Invalid state - expected one parent");

                    actualParents.Insert(0, parent.Parents[0]);
                }

                if (fromPath.All(qn => parents.Any(p => p.Code == qn.Code)))
                {
                    state.RemainingParents = parents
                        .Where(p => !fromPath.Any(pp => pp.Code == p.Code))
                        .ToArray();
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
    }

    public bool Traverse<TState>(
        TState state,
        GmodNode rootNode,
        TraverseHandlerWithState<TState> handler
    )
    {
        var context = new TraversalContext<TState>(new Parents(), handler, state);
        return TraverseNode(in context, rootNode) == TraversalHandlerResult.Continue;
    }

    private TraversalHandlerResult TraverseNode<TState>(
        in TraversalContext<TState> context,
        GmodNode node
    )
    {
        if (context.Parents.Has(node))
            // Avoid cycles
            // note: installSubstructure doesn't work - martinothamar
            return TraversalHandlerResult.Continue;

        var result = context.Handler(context.State, context.Parents.AsList, node);
        if (result is TraversalHandlerResult.Stop or TraversalHandlerResult.SkipSubtree)
            return result;

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

    private readonly record struct TraversalContext<TState>(
        Parents Parents,
        TraverseHandlerWithState<TState> Handler,
        TState State
    );

    private readonly struct Parents
    {
#if NETCOREAPP3_0_OR_GREATER
        private readonly HashSet<string> _codes = new(64);
#else
        private readonly HashSet<string> _codes = new();
#endif
        private readonly List<GmodNode> _parents = new(64);

        public Parents() { }

        public readonly void Push(GmodNode parent)
        {
            _codes.Add(parent.Code);
            _parents.Add(parent);
        }

        public readonly void Pop()
        {
            var parent = _parents[_parents.Count - 1];
            _parents.RemoveAt(_parents.Count - 1);
            _codes.Remove(parent.Code);
        }

        public readonly bool Has(GmodNode parent) => _codes.Contains(parent.Code);

        public readonly IReadOnlyList<GmodNode> ToList() => _parents.ToList();

        public readonly IReadOnlyList<GmodNode> AsList => _parents;
    }
}
