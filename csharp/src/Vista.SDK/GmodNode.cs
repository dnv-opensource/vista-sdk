using System.Text;

namespace Vista.SDK;

public record class GmodNode
{
    public string Code { get; init; }

    public string? Location { get; init; }

    public GmodNodeMetadata Metadata { get; }

    private readonly List<GmodNode> _children;
    private readonly List<GmodNode> _parents;

    public IReadOnlyList<GmodNode> Children => _children;

    public IReadOnlyList<GmodNode> Parents => _parents;

    internal GmodNode(GmodNodeDto dto)
    {
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
        _parents = new List<GmodNode>();
    }

    public GmodNode WithoutLocation() => Location is null ? this : this with { Location = null };

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

    public bool IsAsset => Gmod.IsAsset(Metadata);

    public bool IsProductGroupLevel => Gmod.IsProductGroupLevel(Metadata);

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
        for (int i = 0; i < _children.Count; i++)
        {
            if (_children[i].Code == code)
                return true;
        }

        return false;
    }

    public virtual bool Equals(GmodNode? other) =>
        Code == other?.Code && Location == other?.Location;

    public override int GetHashCode() => HashCode.Combine(Code, Location);

    public override sealed string ToString() => Location is null ? Code : $"{Code}-{Location}";

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
);
