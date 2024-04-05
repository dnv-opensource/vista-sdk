using System.Collections;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using Vista.SDK.Internal;
#if NET8_0_OR_GREATER
using System.Collections.Frozen;
using System.Collections.Immutable;
#endif

namespace Vista.SDK;

public sealed partial class Gmod : IEnumerable<GmodNode>
{
    public VisVersion VisVersion { get; }

    private readonly GmodNode _rootNode;

    private readonly ChdDictionary<GmodNode> _nodeMap;

    public GmodNode RootNode => _rootNode;

    private static readonly string[] PotentialParentScopeTypes = ["SELECTION", "GROUP", "LEAF"];
#if NET8_0_OR_GREATER
    private static readonly FrozenSet<string> PotentialParentScopeTypesSet = PotentialParentScopeTypes.ToFrozenSet(
        StringComparer.Ordinal
    );
#endif

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    internal static bool IsPotentialParent(string type)
    {
#if NET8_0_OR_GREATER
        return PotentialParentScopeTypesSet.Contains(type);
#else
        return PotentialParentScopeTypes.Contains(type);
#endif
    }

    private static readonly string[] LeafTypes = ["ASSET FUNCTION LEAF", "PRODUCT FUNCTION LEAF",];
#if NET8_0_OR_GREATER
    internal static readonly FrozenSet<string> LeafTypesSet = LeafTypes.ToFrozenSet(StringComparer.Ordinal);
#endif

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private static bool IsLeafNode(string fullType)
    {
#if NET8_0_OR_GREATER
        return LeafTypesSet.Contains(fullType);
#else
        return LeafTypes.Contains(fullType);
#endif
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool IsLeafNode(GmodNodeMetadata metadata) => IsLeafNode(metadata.FullType);

    private static bool IsFunctionNode(string category) => category != "PRODUCT" && category != "ASSET";

    public static bool IsFunctionNode(GmodNodeMetadata metadata) => IsFunctionNode(metadata.Category);

    public static bool IsProductSelection(GmodNodeMetadata metadata) =>
        metadata.Category == "PRODUCT" && metadata.Type == "SELECTION";

    public static bool IsProductType(GmodNodeMetadata metadata) =>
        metadata.Category == "PRODUCT" && metadata.Type == "TYPE";

    public static bool IsAsset(GmodNodeMetadata metadata) => metadata.Category == "ASSET";

    public static bool IsAssetFunctionNode(GmodNodeMetadata metadata) => metadata.Category == "ASSET FUNCTION";

    public static bool IsProductTypeAssignment(GmodNode? parent, GmodNode? child)
    {
        if (parent is null || child is null)
            return false;
        if (!parent.Metadata.Category.Contains("FUNCTION"))
            return false;
        if (child.Metadata.Category != "PRODUCT" || child.Metadata.Type != "TYPE")
            return false;
        return true;
    }

    public static bool IsProductSelectionAssignment(GmodNode? parent, GmodNode? child)
    {
        if (parent is null || child is null)
            return false;
        if (!parent.Metadata.Category.Contains("FUNCTION"))
            return false;
        if (!child.Metadata.Category.Contains("PRODUCT") || child.Metadata.Type != "SELECTION")
            return false;
        return true;
    }

    internal Gmod(VisVersion version, GmodDto dto)
    {
        VisVersion = version;

        var nodeMap = new Dictionary<string, GmodNode>(dto.Items.Length);

        foreach (var nodeDto in dto.Items)
        {
            var node = new GmodNode(VisVersion, nodeDto);
            nodeMap.Add(nodeDto.Code, node);
        }

        foreach (var relation in dto.Relations)
        {
            var parentCode = relation[0];
            var childCode = relation[1];

            var parentNode = nodeMap[parentCode];
            var childNode = nodeMap[childCode];

            parentNode.AddChild(childNode);
            childNode.AddParent(parentNode);
        }

        foreach (var node in nodeMap.Values)
            node.Trim();

        _rootNode = nodeMap["VE"];

        _nodeMap = new ChdDictionary<GmodNode>(nodeMap.Select(kvp => (kvp.Key, kvp.Value)).ToArray());
    }

    public GmodNode this[string key] => _nodeMap[key.AsSpan()];

    public bool TryGetNode(string code, [MaybeNullWhen(false)] out GmodNode node) =>
        _nodeMap.TryGetValue(code.AsSpan(), out node);

    public bool TryGetNode(ReadOnlySpan<char> code, [MaybeNullWhen(false)] out GmodNode node) =>
        _nodeMap.TryGetValue(code, out node);

    public GmodPath ParsePath(string item) => GmodPath.Parse(item, VisVersion);

    public bool TryParsePath(string item, [NotNullWhen(true)] out GmodPath? path) =>
        GmodPath.TryParse(item, VisVersion, out path);

    public GmodPath ParseFromFullPath(string item) => GmodPath.ParseFullPath(item, VisVersion);

    public bool TryParseFromFullPath(string item, [NotNullWhen(true)] out GmodPath? path) =>
        GmodPath.TryParseFullPath(item, VisVersion, out path);

    public Enumerator GetEnumerator()
    {
        var enumerator = new Enumerator { Inner = _nodeMap.GetEnumerator() };
        return enumerator;
    }

    IEnumerator<GmodNode> IEnumerable<GmodNode>.GetEnumerator() => GetEnumerator();

    IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

    public struct Enumerator : IEnumerator<GmodNode>
    {
        internal ChdDictionary<GmodNode>.Enumerator Inner;

        public GmodNode Current => Inner.Current.Value;

        object IEnumerator.Current => Inner.Current.Value;

        public void Dispose() => Inner.Dispose();

        public bool MoveNext() => Inner.MoveNext();

        public void Reset() => Inner.Reset();
    }
}
