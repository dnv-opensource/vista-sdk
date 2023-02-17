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

        if (locationStr.Trim().Length != locationStr.Length)
        {
            AddError(
                ref errorBuilder,
                LocationValidationResult.Invalid,
                $"Invalid location with leading and/or trailing whitespace: {locationStr}"
            );
            return false;
        }
        if (locationStr.Any(char.IsWhiteSpace))
        {
            AddError(
                ref errorBuilder,
                LocationValidationResult.Invalid,
                $"Invalid location containing whitespace: {locationStr}"
            );
            return false;
        }

        var locationWithoutNumber = locationStr.Where(l => !char.IsDigit(l)).ToList();
        var invalidLocationCodes = locationWithoutNumber
            .Where(l => !_locationCodes.Contains(l) || l == 'N')
            .ToArray();
        if (invalidLocationCodes.Length > 0)
        {
            var invalidChars = string.Join(",", invalidLocationCodes);
            AddError(
                ref errorBuilder,
                LocationValidationResult.InvalidCode,
                $"Invalid location code: {locationStr} with invalid character(s): {invalidChars}"
            );
        }
        var numberNotAtStart =
            locationStr.Any(l => char.IsDigit(l))
            && !int.TryParse(locationStr[0].ToString(), out _);
        if (numberNotAtStart)
            AddError(
                ref errorBuilder,
                LocationValidationResult.Invalid,
                $"Invalid location: numbers should start before characters in location: {locationStr}"
            );

        var alphabeticallySorted = locationWithoutNumber
            .OrderBy(l => char.ToUpperInvariant(l))
            .ToList();
        var notAlphabeticallySorted = !locationWithoutNumber.SequenceEqual(alphabeticallySorted);
        if (notAlphabeticallySorted)
            AddError(
                ref errorBuilder,
                LocationValidationResult.InvalidOrder,
                $"Invalid location {locationStr}: not alphabetically sorted"
            );

        var notUpperCase = locationWithoutNumber.Any(l => !char.IsUpper(l));
        if (notUpperCase)
            AddError(
                ref errorBuilder,
                LocationValidationResult.Invalid,
                $"Invalid location {locationStr}: characters can only be uppercase"
            );

        var locationNumbersFirst = locationStr.OrderBy(l => !char.IsDigit(l)).ToList();
        var notNumericalSorted = !locationStr.ToList().SequenceEqual(locationNumbersFirst);

        if (notNumericalSorted)
            AddError(
                ref errorBuilder,
                LocationValidationResult.InvalidOrder,
                $"Invalid location {locationStr}: not numerically sorted"
            );

        if (errorBuilder.HasError)
            return false;

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
