using System.Collections;

namespace Vista.SDK;

public readonly record struct Location
{
    private readonly string _value;

    internal Location(string value)
    {
        _value = value;
    }
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
        if (location is null)
            return true;

        if (location.Trim().Length != location.Length)
            return false;

        var locationWithoutNumber = location.Where(l => !char.IsDigit(l)).ToList();
        var invalidLocationCode = locationWithoutNumber.Any(
            l => !_locationCodes.Contains(l) || l == 'N'
        );
        if (invalidLocationCode)
            return false;

        var numberNotAtStart = location
            .Where((lValue, lIndex) => int.TryParse(lValue.ToString(), out _) && lIndex > 0)
            .Any();

        var alphabeticallySorted = locationWithoutNumber.OrderBy(l => l).ToList();
        var notAlphabeticallySorted = !locationWithoutNumber.SequenceEqual(alphabeticallySorted);
        var notUpperCase = locationWithoutNumber.Any(l => !char.IsUpper(l));

        if (numberNotAtStart || notAlphabeticallySorted || notUpperCase)
            return false;

        return true;
    }

    public bool TryParse(string value, out Location location)
    {
        location = default;
        if (!IsValid(value))
            return false;
        location = new Location(value); 
        return true;
    }
}

public sealed record RelativeLocation(char Code, string Name, string? Definition);
