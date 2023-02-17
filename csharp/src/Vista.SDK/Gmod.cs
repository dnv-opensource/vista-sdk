using System.Collections;
using System.Diagnostics.CodeAnalysis;

namespace Vista.SDK;

public sealed partial class Gmod : IEnumerable<GmodNode>
{
    public VisVersion VisVersion { get; }

    private readonly GmodNode _rootNode;
    private readonly Dictionary<string, GmodNode> _nodeMap;

    public GmodNode RootNode => _rootNode;

    private static readonly (string Category, string Type)[] LeafTypes = new[]
    {
        ("ASSET FUNCTION", "LEAF"),
        ("PRODUCT FUNCTION", "LEAF"),
    };

    private static bool IsLeafNode(string category, string type)
    {
        foreach (var leafType in LeafTypes)
        {
            if (leafType.Category == category && leafType.Type == type)
                return true;
        }

        return false;
    }

    public static bool IsLeafNode(GmodNodeMetadata metadata) =>
        IsLeafNode(metadata.Category, metadata.Type);

    private static bool IsFunctionNode(string category) =>
        category != "PRODUCT" && category != "ASSET";

    public static bool IsFunctionNode(GmodNodeMetadata metadata) =>
        IsFunctionNode(metadata.Category);

    public static bool IsProductSelection(GmodNodeMetadata metadata) =>
        metadata.Category == "PRODUCT" && metadata.Type == "SELECTION";

    public static bool IsAsset(GmodNodeMetadata metadata) => metadata.Category == "ASSET";

    public static bool IsAssetFunctionNode(GmodNodeMetadata metadata) =>
        metadata.Category == "ASSET FUNCTION";

    internal Gmod(VisVersion version, GmodDto dto)
    {
        VisVersion = version;

        _nodeMap = new Dictionary<string, GmodNode>(dto.Items.Length);

        foreach (var nodeDto in dto.Items)
        {
            var node = new GmodNode(VisVersion, nodeDto);
            _nodeMap.Add(nodeDto.Code, node);
        }

        foreach (var relation in dto.Relations)
        {
            var parentCode = relation[0];
            var childCode = relation[1];

            var parentNode = _nodeMap[parentCode];
            var childNode = _nodeMap[childCode];

            parentNode.AddChild(childNode);
            childNode.AddParent(parentNode);
        }

        foreach (var node in _nodeMap.Values)
            node.Trim();

        _rootNode = _nodeMap["VE"];
    }

    public GmodNode this[string key] => _nodeMap[key];

    public bool TryGetNode(string code, [MaybeNullWhen(false)] out GmodNode node) =>
        _nodeMap.TryGetValue(code, out node);

    public bool TryGetNode(ReadOnlySpan<char> code, [MaybeNullWhen(false)] out GmodNode node) =>
        _nodeMap.TryGetValue(code.ToString(), out node);

    public GmodPath ParsePath(string item) => GmodPath.Parse(item, VisVersion);

    public bool TryParsePath(string item, [NotNullWhen(true)] out GmodPath? path) =>
        GmodPath.TryParse(item, VisVersion, out path);

    public Dictionary<string, GmodNode>.ValueCollection.Enumerator GetEnumerator() =>
        _nodeMap.Values.GetEnumerator();

    IEnumerator<GmodNode> IEnumerable<GmodNode>.GetEnumerator() => GetEnumerator();

    IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
}
