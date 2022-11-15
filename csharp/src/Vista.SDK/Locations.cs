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

    public bool IsValid(string? location)
    {
        if (location is null || string.IsNullOrWhiteSpace(location))
            return false;

        if (location.Trim().Length != location.Length)
            return false;

        var locationWithoutNumber = location.Where(l => !char.IsDigit(l)).ToList();

        var invalidLocationCode = locationWithoutNumber.Any(
            l => !_locationCodes.Contains(l) || l == 'N'
        );
        if (invalidLocationCode)
            return false;

        var numberNotAtStart =
            location.Any(l => char.IsDigit(l)) && !int.TryParse(location[0].ToString(), out _);

        var alphabeticallySorted = locationWithoutNumber.OrderBy(l => l).ToList();
        var notAlphabeticallySorted = !locationWithoutNumber.SequenceEqual(alphabeticallySorted);
        var notUpperCase = locationWithoutNumber.Any(l => !char.IsUpper(l));

        var locationNumbersFirst = location.OrderBy(l => !char.IsDigit(l)).ToList();
        var notNumericalSorted = !location.ToList().SequenceEqual(locationNumbersFirst);

        if (numberNotAtStart || notAlphabeticallySorted || notUpperCase || notNumericalSorted)
            return false;

        return true;
    }

    public Location? TryParse(string? value)
    {
        if (!IsValid(value))
            return null;

        return new Location(value!);
    }

    public Location Parse(string value)
    {
        var location = TryParse(value);
        if (location is null)
            throw new ArgumentException($"Invalid value for location: {value}");

        return location.Value;
    }
}

public sealed record RelativeLocation(char Code, string Name, string? Definition);
