namespace Vista.SDK;

public delegate TraversalHandlerResult TraverseHandler(
    IReadOnlyList<GmodNode> parents,
    GmodNode node
);
public delegate TraversalHandlerResult TraverseHandlerWithState<TState>(
    TState state,
    IReadOnlyList<GmodNode> parents,
    GmodNode node
) where TState : class;

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

    public bool Traverse<TState>(TState state, TraverseHandlerWithState<TState> handler)
        where TState : class => Traverse(state, _rootNode, handler);

    public bool Traverse(GmodNode rootNode, TraverseHandler handler) =>
        Traverse(handler, rootNode, (handler, parents, node) => handler(parents, node));

    public bool PathExistsBetween(GmodNode from, GmodNode to)
    {
        if (from.Code == to.Code)
            return true;

        var reachedEnd = this.Traverse(
            to,
            from,
            (to, parents, node) =>
            {
                if (node.Code == to.Code)
                    return TraversalHandlerResult.Stop;

                return TraversalHandlerResult.Continue;
            }
        );

        return !reachedEnd;
    }

    public bool PathExistsBetween(IEnumerable<GmodNode> fromPath, GmodNode to)
    {
        var lastNode = fromPath.Last();

        var reachedEnd = this.Traverse(
            to,
            (to, parents, node) =>
            {
                if (node.Code != to.Code)
                    return TraversalHandlerResult.Continue;

                if (fromPath.All(qn => parents.Any(p => p.Code == qn.Code)))
                    return TraversalHandlerResult.Stop;

                return TraversalHandlerResult.Continue;
            }
        );

        return !reachedEnd;
    }

    public bool Traverse<TState>(
        TState state,
        GmodNode rootNode,
        TraverseHandlerWithState<TState> handler
    ) where TState : class
    {
        var context = new TraversalContext<TState>(new Parents(), handler, state);
        return TraverseNode(in context, rootNode) == TraversalHandlerResult.Continue;
    }

    private TraversalHandlerResult TraverseNode<TState>(
        in TraversalContext<TState> context,
        GmodNode node
    ) where TState : class
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
    ) where TState : class;

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
