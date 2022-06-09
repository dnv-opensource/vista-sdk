namespace Vista.SDK;

public sealed record class Codebook
{
    public CodebookName Name { get; private init; }
    private readonly Dictionary<string, string> _groupMap;
    private readonly CodebookStandardValues _standardValues;
    private readonly CodebookGroups _groups;

    private const string TagAlphabet = "abcdefghijklmnopqrstuvwxyz0123456789.";
    private const string PositionTagAlphabet = TagAlphabet + "-";

    internal Codebook(CodebookDto dto)
    {
        Name = dto.Name switch
        {
            "positions" => CodebookName.Position,
            "calculations" => CodebookName.Calculation,
            "quantities" => CodebookName.Quantity,
            "states" => CodebookName.State,
            "contents" => CodebookName.Content,
            "commands" => CodebookName.Command,
            "types" => CodebookName.Type,
            "detail" => CodebookName.Detail,
            _ => throw new ArgumentException("Unknown metadata tag: " + dto.Name, nameof(dto.Name)),
        };

        _groupMap = new();

        var data = dto.Values
            .SelectMany(kvp => kvp.Value.Select(v => (Group: kvp.Key.Trim(), Value: v.Trim())))
            .Where(v => v.Value != "<number>")
            .ToArray();

        foreach (var t in data)
            _groupMap[t.Value] = t.Group;

        var valueSet = new HashSet<string>(data.Select(t => t.Value));
        var groupSet = new HashSet<string>(data.Select(t => t.Group));
        _standardValues = new CodebookStandardValues(Name, valueSet);
        _groups = new CodebookGroups(Name, groupSet);
    }

    public CodebookGroups Groups => _groups;

    public CodebookStandardValues StandardValues => _standardValues;

    public bool HasGroup(string group) => _groups.Contains(group);

    public bool HasStandardValue(string value) => _standardValues.Contains(value);

    public MetadataTag? TryCreateTag(string? value)
    {
        if (value is null || string.IsNullOrWhiteSpace(value))
            return null;

        var isCustom = false;

        if (Name.Equals(CodebookName.Position))
        {
            if ((int)ValidatePosition(value) < 100)
                return null;

            if (ValidatePosition(value).Equals(PositionValidationResult.Custom))
                isCustom = true;
        }
        else
        {
            if (!value.All(c => TagAlphabet.Contains(c)))
                return null;

            if (Name != CodebookName.Detail && !StandardValues.Contains(value))
                isCustom = true;
        }

        return new MetadataTag(Name, value, isCustom);
    }

    public MetadataTag CreateTag(string value)
    {
        var tag = TryCreateTag(value);
        if (tag is null)
            throw new ArgumentException(
                $"Invalid value for metadata tag: codebook={Name}, value={value}"
            );

        return tag.Value;
    }

    public PositionValidationResult ValidatePosition(string position)
    {
        if (string.IsNullOrWhiteSpace(position))
            return PositionValidationResult.Invalid;

        if (position.Trim().Length != position.Length)
            return PositionValidationResult.Invalid;

        if (!position.All(c => PositionTagAlphabet.Contains(c)))
            return PositionValidationResult.Invalid;

        if (StandardValues.Contains(position))
            return PositionValidationResult.Valid;

        if (int.TryParse(position, out _))
            return PositionValidationResult.Valid;

        if (!position.Contains('-'))
            return PositionValidationResult.Custom;

        var positions = position.Split('-');
        var validations = new List<PositionValidationResult>();
        foreach (var positionStr in positions)
        {
            validations.Add(ValidatePosition(positionStr));
        }

        if (validations.Any(v => (int)v < 100))
            return validations.Max();

        var numberNotAtEnd = positions
            .Where(
                (pValue, pIndex) => int.TryParse(pValue, out _) && pIndex < (positions.Length - 1)
            )
            .Any();

        var positionsWithoutNumber = positions.Where(p => !int.TryParse(p, out _)).ToList();
        var alphabeticallySorted = positionsWithoutNumber.OrderBy(p => p).ToList();
        var notAlphabeticallySorted = !positionsWithoutNumber.SequenceEqual(alphabeticallySorted);

        if (numberNotAtEnd || notAlphabeticallySorted)
            return PositionValidationResult.InvalidOrder;

        if (validations.All(v => (int)v == (int)PositionValidationResult.Valid))
        {
            var groups = positions
                .Select(p => int.TryParse(p, out _) ? "<number>" : _groupMap[p])
                .ToArray();

            var groupsSet = new HashSet<string>(groups);
            if (!groups.Contains("DEFAULT_GROUP") && groupsSet.Count != groups.Length)
                return PositionValidationResult.InvalidGrouping;
        }

        return validations.Max();
    }
}

public enum PositionValidationResult
{
    Invalid = 0,
    InvalidOrder = 1,
    InvalidGrouping = 2,

    Valid = 100,
    Custom = 101
}

public sealed class CodebookStandardValues
{
    private readonly CodebookName _name;
    private readonly HashSet<string> _standardValues;

    internal CodebookStandardValues(CodebookName name, HashSet<string> standardValues)
    {
        _name = name;
        _standardValues = standardValues;
    }

    public bool Contains(string tagValue)
    {
        if (_name == CodebookName.Position && int.TryParse(tagValue, out _))
            return true;

        return _standardValues.Contains(tagValue);
    }
}

public sealed class CodebookGroups
{
    private readonly CodebookName _name;
    private readonly HashSet<string> _groups;

    internal CodebookGroups(CodebookName name, HashSet<string> groups)
    {
        _name = name;
        _groups = groups;
    }

    public int Count => _groups.Count;

    public bool Contains(string group) => _groups.Contains(group);
}
