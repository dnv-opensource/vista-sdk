public sealed class GmodNode
{
    private readonly uniffi.bindings.GmodNode _inner;

    private GmodNode(uniffi.bindings.GmodNode inner) => _inner = inner;

    public string Code => _inner.Code();

    internal static GmodNode FromBindings(uniffi.bindings.GmodNode inner) => new GmodNode(inner);
}

public static class GmodNodeBindingsExtensions
{
    public static GmodNode ToGmodNode(this uniffi.bindings.GmodNode node) => GmodNode.FromBindings(node);
}
