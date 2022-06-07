using System.Diagnostics.CodeAnalysis;

namespace Vista.SDK;

public sealed partial class Gmod
{
    public VisVersion VisVersion { get; }

    private readonly GmodNode _rootNode;
    private readonly Dictionary<string, GmodNode> _nodeMap;

    public GmodNode RootNode => _rootNode;

    public static readonly (string Category, string Type)[] LeafTypes = new[]
    {
        ("ASSET FUNCTION", "LEAF"),
        ("PRODUCT FUNCTION", "LEAF"),
        ("PRODUCT", "GROUP LEVEL 2"),
    };

    public static bool IsLeafNode(string category, string type)
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

    public static bool IsFunctionNode(string category) =>
        category != "PRODUCT" && category != "ASSET";

    public static bool IsFunctionNode(GmodNodeMetadata metadata) =>
        IsFunctionNode(metadata.Category);

    public static bool IsProductSelection(GmodNodeMetadata metadata) =>
        metadata.Category == "PRODUCT" && metadata.Type == "SELECTION";

    public static bool IsAsset(GmodNodeMetadata metadata) => metadata.Category == "ASSET";

    public static bool IsAssetFunctionNode(GmodNodeMetadata metadata) =>
        metadata.Category.Contains("FUNCTION") && !metadata.Category.Contains("PRODUCT");

    public static bool IsProductGroupLevel(GmodNodeMetadata metadata) =>
        metadata.Category == "PRODUCT" && metadata.Type == "GROUP LEVEL 1"
        || metadata.Type == "GROUP LEVEL 2";

    internal Gmod(VisVersion version, GmodDto dto)
    {
        VisVersion = version;

        _nodeMap = new Dictionary<string, GmodNode>(dto.Items.Length);

        foreach (var nodeDto in dto.Items)
        {
            var node = new GmodNode(nodeDto);
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

    public GmodPath ParsePath(string item) => GmodPath.Parse(item, this);

    public bool TryParsePath(string item, [NotNullWhen(true)] out GmodPath? path) =>
        GmodPath.TryParse(item, this, out path);

    public Dictionary<string, GmodNode>.ValueCollection.Enumerator GetEnumerator() =>
        _nodeMap.Values.GetEnumerator();
}
