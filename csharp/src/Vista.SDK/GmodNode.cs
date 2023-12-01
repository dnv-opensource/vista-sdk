using System.Text;
#if NET8_0_OR_GREATER
using System.Collections.Frozen;
#endif

namespace Vista.SDK;

public record class GmodNode
{
    public string Code { get; init; }
    public Location? Location { get; internal init; }

    public VisVersion VisVersion { get; }

    public GmodNodeMetadata Metadata { get; }

    internal readonly List<GmodNode> _children;
#if NET8_0_OR_GREATER
    internal FrozenSet<string> _childrenSet;
#endif
    internal readonly List<GmodNode> _parents;

    public IReadOnlyList<GmodNode> Children => _children;

    public IReadOnlyList<GmodNode> Parents => _parents;

    internal GmodNode(VisVersion version, GmodNodeDto dto)
    {
        VisVersion = version;
        Code = dto.Code;
        Metadata = new(
            dto.Category,
            dto.Type,
            dto.Name,
            dto.CommonName,
            dto.Definition,
            dto.CommonDefinition,
            dto.InstallSubstructure,
            dto.NormalAssignmentNames ?? new Dictionary<string, string>(0)
        );
        _children = new List<GmodNode>();
#if NET8_0_OR_GREATER
        _childrenSet = FrozenSet<string>.Empty;
#endif
        _parents = new List<GmodNode>();
    }

    internal GmodNode WithoutLocation() => Location is null ? this : this with { Location = null };

    internal GmodNode WithLocation(string location)
    {
        var locations = VIS.Instance.GetLocations(VisVersion);

        return this with
        {
            Location = locations.Parse(location)
        };
    }

    internal GmodNode TryWithLocation(string? locationStr)
    {
        var locations = VIS.Instance.GetLocations(VisVersion);
        if (!locations.TryParse(locationStr, out var location))
            return this;

        return WithLocation(location);
    }

    internal GmodNode TryWithLocation(string? locationStr, out ParsingErrors errors)
    {
        var locations = VIS.Instance.GetLocations(VisVersion);

        if (!locations.TryParse(locationStr, out var location, out errors))
            return this;

        return WithLocation(location);
    }

    internal GmodNode WithLocation(in Location location) => this with { Location = location };

    internal GmodNode TryWithLocation(in Location? location)
    {
        if (location is null)
            return this;

        return WithLocation(location.Value);
    }

    internal bool IsIndividualizable(bool isTargetNode = false, bool isInSet = false)
    {
        if (Metadata.Type == "GROUP")
            return false;
        if (Metadata.Type == "SELECTION")
            return false;
        if (IsProductType)
            return false;
        if (Metadata.Category == "ASSET" && Metadata.Type == "TYPE")
            return false;
        if (IsFunctionComposition)
            return Code[Code.Length - 1] == 'i' || isInSet || isTargetNode;
        return true;
    }

    public bool IsFunctionComposition =>
        (Metadata.Category == "ASSET FUNCTION" || Metadata.Category == "PRODUCT FUNCTION")
        && Metadata.Type == "COMPOSITION";

    public bool IsMappable
    {
        get
        {
            if (ProductType is not null)
                return false;
            if (ProductSelection is not null)
                return false;
            if (IsProductSelection)
                return false;
            if (IsAsset)
                return false;

            var lastChar = Code[Code.Length - 1];
            return lastChar != 'a' && lastChar != 's';
        }
    }

    public bool IsProductSelection => Gmod.IsProductSelection(Metadata);

    public bool IsProductType => Gmod.IsProductType(Metadata);

    public bool IsAsset => Gmod.IsAsset(Metadata);

    public GmodNode? ProductType
    {
        get
        {
            if (_children.Count != 1)
                return null;

            if (!Metadata.Category.Contains("FUNCTION"))
                return null;

            var child = _children[0];
            if (child.Metadata.Category != "PRODUCT")
                return null;

            if (child.Metadata.Type != "TYPE")
                return null;

            return child;
        }
    }

    public GmodNode? ProductSelection
    {
        get
        {
            if (_children.Count != 1)
                return null;

            if (!Metadata.Category.Contains("FUNCTION"))
                return null;

            var child = _children[0];
            if (!child.Metadata.Category.Contains("PRODUCT"))
                return null;

            if (child.Metadata.Type != "SELECTION")
                return null;

            return child;
        }
    }

    public bool IsChild(GmodNode node) => IsChild(node.Code);

    public bool IsChild(string code)
    {
#if NET8_0_OR_GREATER
        return _childrenSet.Contains(code);
#else
        for (int i = 0; i < _children.Count; i++)
        {
            if (_children[i].Code == code)
                return true;
        }

        return false;
#endif
    }

    public virtual bool Equals(GmodNode? other) => Code == other?.Code && Location == other?.Location;

    public override int GetHashCode() => HashCode.Combine(Code, Location);

    public sealed override string ToString() => Location is null ? Code : $"{Code}-{Location}";

    public void ToString(StringBuilder builder)
    {
        if (Location is null)
        {
            builder.Append(Code);
        }
        else
        {
            builder.Append(Code);
            builder.Append('-');
            builder.Append(Location);
        }
    }

    internal void AddChild(GmodNode child) => _children.Add(child);

    internal void AddParent(GmodNode parent) => _parents.Add(parent);

    internal void Trim()
    {
        _children.TrimExcess();
        _parents.TrimExcess();
#if NET8_0_OR_GREATER
        _childrenSet = _children.Select(c => c.Code).ToFrozenSet(StringComparer.Ordinal);
#endif
    }

    public bool IsLeafNode => Gmod.IsLeafNode(Metadata);

    public bool IsFunctionNode => Gmod.IsFunctionNode(Metadata);

    public bool IsAssetFunctionNode => Gmod.IsAssetFunctionNode(Metadata);

    public bool IsRoot => Code == "VE";
}

public sealed record class GmodNodeMetadata(
    string Category,
    string Type,
    string Name,
    string? CommonName,
    string? Definition,
    string? CommonDefinition,
    bool? InstallSubstructure,
    IReadOnlyDictionary<string, string> NormalAssignmentNames
)
{
    public string FullType { get; } = $"{Category} {Type}";
}
