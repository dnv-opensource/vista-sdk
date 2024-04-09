using System.Collections;
using System.Diagnostics.CodeAnalysis;
using uniffi.bindings;
using Vista.SDK.Internal;

namespace Vista.SDK;

public delegate TraversalHandlerResult TraverseHandler(IReadOnlyList<GmodNode> parents, GmodNode node);

public sealed class Gmod
{
    private class TraverseCallbackImpl : TraversalCallback
    {
        private readonly TraverseHandler _handler;

        public TraverseCallbackImpl(TraverseHandler handler) => _handler = handler;

        public TraversalHandlerResult Handler(List<uniffi.bindings.GmodNode> parents, uniffi.bindings.GmodNode node) =>
            _handler(parents.Select(GmodNode.FromBindings).ToList(), node.ToGmodNode());
    }

    private readonly uniffi.bindings.Gmod _inner;
    public VisVersion VisVersion => _inner.Version();

    private Gmod(uniffi.bindings.Gmod inner) => _inner = inner;

    internal static Gmod FromBindings(uniffi.bindings.Gmod inner) => new Gmod(inner);

    public bool Traverse(TraverseHandler handler) => _inner.Traverse(new TraverseCallbackImpl(handler));

    public GmodNode this[string key] => _inner.GetNode(key).ToGmodNode();

    public bool TryGetNode(string code, [MaybeNullWhen(false)] out GmodNode node)
    {
        node = default;
        var result = _inner.TryGetNode(code);
        if (result is null)
            return false;
        node = result.ToGmodNode();
        return true;
    }
}
