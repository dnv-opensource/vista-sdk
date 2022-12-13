using Vista.SDK.Internal;

namespace Vista.SDK;

public readonly record struct Location
{
    public readonly string Value { get; private init; }

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
        var location = TryParse(locationStr);
        if (location is null)
            throw new ArgumentException($"Invalid value for location: {locationStr}");

        return location.Value;
    }

    public Location Parse(string locationStr, out LocationParsingErrorBuilder errorBuilder)
    {
        var location = TryParse(locationStr, out errorBuilder);
        if (location is null)
            throw new ArgumentException($"Invalid value for location: {locationStr}");
        return location.Value;
    }

    public Location? TryParse(string? value)
    {
        return TryParse(value, out _);
    }

    public Location? TryParse(string? value, out LocationParsingErrorBuilder errorBuilder)
    {
        errorBuilder = LocationParsingErrorBuilder.Empty;
        return TryParseInternal(value, ref errorBuilder);
    }

    internal Location? TryParseInternal(
        string? locationStr,
        ref LocationParsingErrorBuilder errorBuilder
    )
    {
        if (locationStr is null || string.IsNullOrWhiteSpace(locationStr))
        {
            AddError(
                ref errorBuilder,
                LocationValidationResult.NullOrWhiteSpace,
                $"Invalid location: {locationStr} is null, empty or contains only whitespace"
            );
            return null;
        }

        if (locationStr.Trim().Length != locationStr.Length)
        {
            AddError(
                ref errorBuilder,
                LocationValidationResult.Invalid,
                $"Invalid location containing whitespace: {locationStr}"
            );
            return null;
        }

        var locationWithoutNumber = locationStr.Where(l => !char.IsDigit(l)).ToList();

        var invalidLocationCode = locationWithoutNumber.Any(
            l => !_locationCodes.Contains(l) || l == 'N'
        );
        if (invalidLocationCode)
        {
            AddError(
                ref errorBuilder,
                LocationValidationResult.InvalidCode,
                $"Invalid location code: {locationStr}"
            );
            return null;
        }
        var numberNotAtStart =
            locationStr.Any(l => char.IsDigit(l))
            && !int.TryParse(locationStr[0].ToString(), out _);
        if (numberNotAtStart)
        {
            AddError(
                ref errorBuilder,
                LocationValidationResult.Invalid,
                $"Invalid location: numbers starts before characters in location: {locationStr}"
            );
            return null;
        }

        var alphabeticallySorted = locationWithoutNumber.OrderBy(l => l).ToList();
        var notAlphabeticallySorted = !locationWithoutNumber.SequenceEqual(alphabeticallySorted);
        if (notAlphabeticallySorted)
        {
            AddError(
                ref errorBuilder,
                LocationValidationResult.InvalidOrder,
                $"Invalid location {locationStr}: not alphabetically sorted"
            );
            return null;
        }

        var notUpperCase = locationWithoutNumber.Any(l => !char.IsUpper(l));
        if (notUpperCase)
        {
            AddError(
                ref errorBuilder,
                LocationValidationResult.Invalid,
                $"Invalid location {locationStr}: characters can only be uppercase"
            );
            return null;
        }

        var locationNumbersFirst = locationStr.OrderBy(l => !char.IsDigit(l)).ToList();
        var notNumericalSorted = !locationStr.ToList().SequenceEqual(locationNumbersFirst);

        if (notNumericalSorted)
        {
            AddError(
                ref errorBuilder,
                LocationValidationResult.InvalidOrder,
                $"Invalid location {locationStr}: not numerically sorted"
            );
            return null;
        }

        return new Location(locationStr);
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

public sealed record RelativeLocation(char Code, string Name, string? Definition);

public enum LocationValidationResult
{
    Invalid,
    InvalidCode,
    InvalidOrder,
    NullOrWhiteSpace,
    Valid
}
