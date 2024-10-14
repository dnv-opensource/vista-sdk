using Vista.SDK;

/// <summary>
/// Only used for test purposes.
/// </summary>
internal sealed record GmodBuilder(VisVersion VisVersion)
{
    private readonly Dictionary<string, GmodNode> _nodeMap = new();

    public void AddNode(GmodNode node)
    {
        _nodeMap[node.Code] = node;
    }

    public Gmod Build() => new(VisVersion, _nodeMap);
}
