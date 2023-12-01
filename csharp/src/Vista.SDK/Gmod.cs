using System.Collections;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
#if NET8_0_OR_GREATER
using System.Collections.Frozen;
using System.Collections.Immutable;
#endif

namespace Vista.SDK;

public sealed partial class Gmod : IEnumerable<GmodNode>
{
    public VisVersion VisVersion { get; }

    private readonly GmodNode _rootNode;

#if NET8_0_OR_GREATER
    private readonly FrozenDictionary<string, GmodNode> _nodeMap;
#else
    private readonly Dictionary<string, GmodNode> _nodeMap;
#endif

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

#if NET8_0_OR_GREATER
        _nodeMap = nodeMap.ToFrozenDictionary(StringComparer.Ordinal);
#else
        _nodeMap = nodeMap;
#endif
    }

    public GmodNode this[string key] => _nodeMap[key];

    public bool TryGetNode(string code, [MaybeNullWhen(false)] out GmodNode node) =>
        _nodeMap.TryGetValue(code, out node);

    public bool TryGetNode(ReadOnlySpan<char> code, [MaybeNullWhen(false)] out GmodNode node) =>
        _nodeMap.TryGetValue(code.ToString(), out node);

    public GmodPath ParsePath(string item) => GmodPath.Parse(item, VisVersion);

    public bool TryParsePath(string item, [NotNullWhen(true)] out GmodPath? path) =>
        GmodPath.TryParse(item, VisVersion, out path);

    public GmodPath ParseFromFullPath(string item) => GmodPath.ParseFullPath(item, VisVersion);

    public bool TryParseFromFullPath(string item, [NotNullWhen(true)] out GmodPath? path) =>
        GmodPath.TryParseFullPath(item, VisVersion, out path);

    public Enumerator GetEnumerator()
    {
        var enumerator = new Enumerator { Inner = _nodeMap.Values.GetEnumerator() };
        return enumerator;
    }

    IEnumerator<GmodNode> IEnumerable<GmodNode>.GetEnumerator() => GetEnumerator();

    IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

    public struct Enumerator : IEnumerator<GmodNode>
    {
#if NET8_0_OR_GREATER
        internal ImmutableArray<GmodNode>.Enumerator Inner;
#else
        internal Dictionary<string, GmodNode>.ValueCollection.Enumerator Inner;
#endif

        public GmodNode Current => Inner.Current;

        object IEnumerator.Current => Inner.Current;

        public void Dispose()
        {
#if !NET8_0_OR_GREATER
            Inner.Dispose();
#endif
        }

        public bool MoveNext() => Inner.MoveNext();

        public void Reset() { }
    }
}
