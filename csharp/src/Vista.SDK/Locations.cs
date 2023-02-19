using System.Globalization;
using Vista.SDK.Internal;

namespace Vista.SDK;

public readonly record struct Location
{
    public readonly string Value { get; }

    internal Location(string value)
    {
        Value = value;
    }

    public readonly override string ToString() => Value;

    public static implicit operator string(Location n) => n.Value;
}

public sealed class Locations
{
    private readonly char[] _locationCodes;
    private readonly List<RelativeLocation> _relativeLocations;

    public VisVersion VisVersion { get; }

    // This is if we need Code, Name, Definition in Frontend UI
    public IReadOnlyList<RelativeLocation> RelativeLocations => _relativeLocations;

    internal Locations(VisVersion version, LocationsDto dto)
    {
        VisVersion = version;

        _locationCodes = dto.Items.Select(d => d.Code).ToArray();

        _relativeLocations = new List<RelativeLocation>(dto.Items.Length);
        foreach (var relativeLocationsDto in dto.Items)
        {
            _relativeLocations.Add(
                new RelativeLocation(
                    relativeLocationsDto.Code,
                    relativeLocationsDto.Name,
                    relativeLocationsDto.Definition
                )
            );
        }
    }

    public Location Parse(string locationStr)
    {
        if (!TryParse(locationStr, out var location))
            throw new ArgumentException($"Invalid value for location: {locationStr}");

        return location;
    }

    public Location Parse(string locationStr, out LocationParsingErrorBuilder errorBuilder)
    {
        if (!TryParse(locationStr, out var location, out errorBuilder))
            throw new ArgumentException($"Invalid value for location: {locationStr}");

        return location;
    }

    public bool TryParse(string? value, out Location location)
    {
        return TryParse(value, out location, out _);
    }

    public bool TryParse(
        string? value,
        out Location location,
        out LocationParsingErrorBuilder errorBuilder
    )
    {
        errorBuilder = LocationParsingErrorBuilder.Empty;
        return TryParseInternal(value, out location, ref errorBuilder);
    }

    private bool TryParseInternal(
        string? locationStr,
        out Location location,
        ref LocationParsingErrorBuilder errorBuilder
    )
    {
        location = default;

        if (locationStr is null)
            return false;

        if (string.IsNullOrWhiteSpace(locationStr))
        {
            AddError(
                ref errorBuilder,
                LocationValidationResult.NullOrWhiteSpace,
                "Invalid location: contains only whitespace"
            );
            return false;
        }

        var span = locationStr.AsSpan();

        int? digitStartIndex = null;
        int? lastLetterIndex = null;
        int? charsStartIndex = null;
        int? n = null;

        for (int i = 0; i < span.Length; i++)
        {
            ref readonly var ch = ref span[i];

            if (char.IsDigit(ch))
            {
                if (digitStartIndex is null)
                {
                    digitStartIndex = i;
                    if (lastLetterIndex is not null)
                    {
                        AddError(
                            ref errorBuilder,
                            LocationValidationResult.Invalid,
                            $"Invalid location: numeric location should start before location code(s) in location: '{locationStr}'"
                        );
                        return false;
                    }
                }
                else
                {
                    if (lastLetterIndex is not null)
                    {
                        if (lastLetterIndex < digitStartIndex)
                        {
                            AddError(
                                ref errorBuilder,
                                LocationValidationResult.Invalid,
                                $"Invalid location: numeric location should start before location code(s) in location: '{locationStr}'"
                            );
                            return false;
                        }
                        else if (lastLetterIndex > digitStartIndex && lastLetterIndex < i)
                        {
                            AddError(
                                ref errorBuilder,
                                LocationValidationResult.Invalid,
                                $"Invalid location: cannot have multiple separated digits in location: '{locationStr}'"
                            );
                            return false;
                        }
                    }

#if NETCOREAPP3_1_OR_GREATER
                    n = int.Parse(
                        span.Slice(digitStartIndex.Value, i - digitStartIndex.Value),
                        NumberStyles.None,
                        CultureInfo.InvariantCulture
                    );
#else
                    n = int.Parse(
                        span.Slice(digitStartIndex.Value, i - digitStartIndex.Value).ToString(),
                        NumberStyles.None,
                        CultureInfo.InvariantCulture
                    );
#endif

                    if (n.Value < 0)
                    {
                        AddError(
                            ref errorBuilder,
                            LocationValidationResult.Invalid,
                            $"Invalid location: negative numeric location is not allowed: '{locationStr}'"
                        );
                        return false;
                    }
                }
            }
            else
            {
                if (ch == 'N' || !_locationCodes.Contains(ch))
                {
                    var invalidChars = string.Join(
                        ",",
                        locationStr
                            .Where(
                                c => !char.IsDigit(c) && (c == 'N' || !_locationCodes.Contains(c))
                            )
                            .Select(c => $"'{c}'")
                    );
                    AddError(
                        ref errorBuilder,
                        LocationValidationResult.InvalidCode,
                        $"Invalid location code: '{locationStr}' with invalid location code(s): {invalidChars}"
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
                            $"Invalid location: '{locationStr}' not alphabetically sorted"
                        );
                        return false;
                    }
                }

                lastLetterIndex = i;
            }
        }

        location = new Location(locationStr);
        return true;
    }

    static void AddError(
        ref LocationParsingErrorBuilder errorBuilder,
        LocationValidationResult name,
        string message
    )
    {
        if (!errorBuilder.HasError)
            errorBuilder = LocationParsingErrorBuilder.Create();
        errorBuilder.AddError(name, message);
    }
}

public sealed record RelativeLocation
{
    public char Code { get; }
    public string Name { get; }
    public string? Definition { get; }

    internal RelativeLocation(char code, string name, string? definition)
    {
        Code = code;
        Name = name;
        Definition = definition;
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
