using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Globalization;
using Vista.SDK.Internal;

namespace Vista.SDK;

public enum LocationGroup
{
    Number,
    Side,
    Vertical,
    Transverse,
    Longitudinal
}

public readonly record struct Location
{
    public readonly string Value { get; }

    internal Location(string value)
    {
        Value = value;
    }

    public override readonly string ToString() => Value;

    public static implicit operator string(Location n) => n.Value;
}

public sealed class Locations
{
    private readonly char[] _locationCodes;
    private readonly List<RelativeLocation> _relativeLocations;
    internal Dictionary<char, LocationGroup> _reversedGroups;

    public VisVersion VisVersion { get; }

    // This is if we need Code, Name, Definition in Frontend UI
    public IReadOnlyList<RelativeLocation> RelativeLocations => _relativeLocations;

    public IReadOnlyDictionary<LocationGroup, IReadOnlyList<RelativeLocation>> Groups;

    internal Locations(VisVersion version, LocationsDto dto)
    {
        VisVersion = version;

        _locationCodes = dto.Items.Select(d => d.Code).ToArray();
        _relativeLocations = new List<RelativeLocation>(dto.Items.Length);

        var groups = new Dictionary<LocationGroup, List<RelativeLocation>>(5);

        _reversedGroups = new Dictionary<char, LocationGroup>();

        foreach (var relativeLocationsDto in dto.Items)
        {
            var relativeLocation = new RelativeLocation(
                relativeLocationsDto.Code,
                relativeLocationsDto.Name,
                new Location(relativeLocationsDto.Code.ToString()),
                relativeLocationsDto.Definition
            );
            _relativeLocations.Add(relativeLocation);

            // Not interested in horizontal and vertical codes
            if (relativeLocationsDto.Code is 'H' or 'V')
                continue;

            var key = relativeLocationsDto.Code switch
            {
                'N' => LocationGroup.Number,
                'P' or 'C' or 'S' => LocationGroup.Side,
                'U' or 'M' or 'L' => LocationGroup.Vertical,
                'I' or 'O' => LocationGroup.Transverse,
                'F' or 'A' => LocationGroup.Longitudinal,
                _ => throw new Exception($"Unsupported code: {relativeLocationsDto.Code}")
            };
            if (!groups.ContainsKey(key))
                groups.Add(key, new());
            if (key == LocationGroup.Number)
                continue;
            _reversedGroups.Add(relativeLocationsDto.Code, key);
            groups.GetValueOrDefault(key)?.Add(relativeLocation);
        }

        Groups = groups.ToDictionary(g => g.Key, g => g.Value.ToArray() as IReadOnlyList<RelativeLocation>);
    }

    public Location Parse(string locationStr)
    {
        var errorBuilder = LocationParsingErrorBuilder.Empty;

        var span = locationStr is null ? ReadOnlySpan<char>.Empty : locationStr.AsSpan();
        if (!TryParseInternal(span, locationStr, out var location, ref errorBuilder))
            throw new ArgumentException($"Invalid value for location: {locationStr}, errors: {errorBuilder.Build()}");

        return location;
    }

    public Location Parse(ReadOnlySpan<char> locationStr)
    {
        var errorBuilder = LocationParsingErrorBuilder.Empty;

        if (!TryParseInternal(locationStr, null, out var location, ref errorBuilder))
            throw new ArgumentException(
                $"Invalid value for location: {locationStr.ToString()}, errors: {errorBuilder.Build()}"
            );

        return location;
    }

    public bool TryParse(string? value, out Location location)
    {
        var span = value is null ? ReadOnlySpan<char>.Empty : value.AsSpan();
        var errorBuilder = LocationParsingErrorBuilder.Empty;
        return TryParseInternal(span, value, out location, ref errorBuilder);
    }

    public bool TryParse(string? value, out Location location, out ParsingErrors errors)
    {
        var span = value is null ? ReadOnlySpan<char>.Empty : value.AsSpan();
        var errorBuilder = LocationParsingErrorBuilder.Empty;
        var result = TryParseInternal(span, value, out location, ref errorBuilder);
        errors = errorBuilder.Build();
        return result;
    }

    public bool TryParse(ReadOnlySpan<char> value, out Location location)
    {
        var errorBuilder = LocationParsingErrorBuilder.Empty;
        return TryParseInternal(value, null, out location, ref errorBuilder);
    }

    public bool TryParse(ReadOnlySpan<char> value, out Location location, out ParsingErrors errors)
    {
        var errorBuilder = LocationParsingErrorBuilder.Empty;
        var result = TryParseInternal(value, null, out location, ref errorBuilder);
        errors = errorBuilder.Build();
        return result;
    }

    private bool TryParseInternal(
        ReadOnlySpan<char> span,
        string? originalStr,
        out Location location,
        ref LocationParsingErrorBuilder errorBuilder
    )
    {
        location = default;

        if (span.IsEmpty)
        {
            AddError(
                ref errorBuilder,
                LocationValidationResult.NullOrWhiteSpace,
                "Invalid location: contains only whitespace"
            );
            return false;
        }

        if (span.IsWhiteSpace())
        {
            AddError(
                ref errorBuilder,
                LocationValidationResult.NullOrWhiteSpace,
                "Invalid location: contains only whitespace"
            );
            return false;
        }

        var originalSpan = span;

        int? prevDigitIndex = null;
        int? digitStartIndex = null;
        int? charsStartIndex = null;
        int? number = null;

        Debug.Assert(Enum.GetValues(typeof(LocationGroup)).Length == 5);
        var charDict = new LocationCharDict(stackalloc char?[4]);

        for (int i = 0; i < span.Length; i++)
        {
            ref readonly var ch = ref span[i];

            if (char.IsDigit(ch))
            {
                // First digit should be at index 0
                if (digitStartIndex is null && i != 0)
                {
                    AddError(
                        ref errorBuilder,
                        LocationValidationResult.Invalid,
                        $"Invalid location: numeric location should start before location code(s) in location: '{originalStr ?? originalSpan.ToString()}'"
                    );
                    return false;
                }
                // Other digits should neighbor the first
                if (prevDigitIndex is not null && prevDigitIndex != (i - 1))
                {
                    AddError(
                        ref errorBuilder,
                        LocationValidationResult.Invalid,
                        $"Invalid location: cannot have multiple separated digits in location: '{originalStr ?? originalSpan.ToString()}'"
                    );
                    return false;
                }

                if (digitStartIndex is null)
                {
                    number = ch - '0';
                    digitStartIndex = i;
                }
                else
                {
                    if (!TryParseInt(span, digitStartIndex.Value, i - digitStartIndex.Value + 1, out var num))
                    {
                        AddError(
                            ref errorBuilder,
                            LocationValidationResult.Invalid,
                            $"Invalid location: failed to parse numeric location: '{originalStr ?? originalSpan.ToString()}'"
                        );
                        return false;
                    }
                    number = num;
                }

                prevDigitIndex = i;
            }
            else
            {
                if (!_reversedGroups.TryGetValue(ch, out var group))
                {
                    var invalidChars = string.Join(
                        ",",
                        (originalStr ?? originalSpan.ToString())
                            .Where(c => !char.IsDigit(c) && (c == 'N' || !_locationCodes.Contains(c)))
                            .Select(c => $"'{c}'")
                    );
                    AddError(
                        ref errorBuilder,
                        LocationValidationResult.InvalidCode,
                        $"Invalid location code: '{originalStr ?? originalSpan.ToString()}' with invalid location code(s): {invalidChars}"
                    );
                    return false;
                }

                if (!charDict.TryAdd(group, ch, out var existingCh))
                {
                    AddError(
                        ref errorBuilder,
                        LocationValidationResult.Invalid,
                        $"Invalid location: Multiple '{Enum.GetName(typeof(LocationGroup), group)}' values. Got both '{existingCh}' and '{ch}' in '{originalStr ?? originalSpan.ToString()}'"
                    );
                    return false;
                }

                if (charsStartIndex is null)
                {
                    charsStartIndex = i;
                }
                else if (i > 0)
                {
                    ref readonly var prevCh = ref span[i - 1];
                    if (ch.CompareTo(prevCh) < 0)
                    {
                        AddError(
                            ref errorBuilder,
                            LocationValidationResult.InvalidOrder,
                            $"Invalid location: '{originalStr ?? originalSpan.ToString()}' not alphabetically sorted"
                        );
                        return false;
                    }
                }
            }
        }

        location = new Location(originalStr ?? originalSpan.ToString());
        return true;
    }

    static void AddError(ref LocationParsingErrorBuilder errorBuilder, LocationValidationResult name, string message)
    {
        if (!errorBuilder.HasError)
            errorBuilder = LocationParsingErrorBuilder.Create();
        errorBuilder.AddError(name, message);
    }

    internal static bool TryParseInt(ReadOnlySpan<char> span, int start, int length, out int number)
    {
#if NETCOREAPP3_1_OR_GREATER
        return int.TryParse(span.Slice(start, length), NumberStyles.None, CultureInfo.InvariantCulture, out number);
#else
        return int.TryParse(
            span.Slice(start, length).ToString(),
            NumberStyles.None,
            CultureInfo.InvariantCulture,
            out number
        );
#endif
    }
}

internal ref struct LocationCharDict
{
    private readonly Span<char?> _table;

    public LocationCharDict(Span<char?> table)
    {
        Debug.Assert(table.Length == Enum.GetValues(typeof(LocationGroup)).Length - 1);
        _table = table;
    }

    private ref char? this[LocationGroup key]
    {
        get
        {
            var index = (int)key - 1;
            if (index >= _table.Length)
                throw new Exception($"Unsupported code: {key}");

            return ref _table[index];
        }
    }

    public bool TryAdd(LocationGroup key, char value, [MaybeNullWhen(true)] out char? existingValue)
    {
        ref var v = ref this[key];
        if (v is not null)
        {
            existingValue = v;
            return false;
        }

        existingValue = null;
        v = value;
        return true;
    }
}

public sealed record RelativeLocation
{
    public char Code { get; }
    public string Name { get; }
    public string? Definition { get; }

    public Location Location { get; }

    internal RelativeLocation(char code, string name, Location location, string? definition)
    {
        Code = code;
        Name = name;
        Definition = definition;
        Location = location;
    }

    public override int GetHashCode() => Code.GetHashCode();

    public bool Equals(RelativeLocation? other) => Code == other?.Code;
}

public enum LocationValidationResult
{
    Invalid,
    InvalidCode,
    InvalidOrder,
    NullOrWhiteSpace,
    Valid
}
