using System.Text.RegularExpressions;

namespace Vista.SDK;

public sealed class Locations
{
    public VisVersion VisVersion { get; }

    private readonly List<RelativeLocation> _relativeLocations;
    public IReadOnlyList<RelativeLocation> RelativeLocations => _relativeLocations;

    internal Locations(VisVersion version, LocationsDto dto)
    {
        VisVersion = version;

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

    public bool IsValid(GmodNode node) => IsValid(node.Location);

    public bool IsValid(string? location)
    {
        if (location is null)
            return true;

        if (location.Trim().Length != location.Length)
            return false;

        var numberNotAtStart = location
            .Where((lValue, lIndex) => int.TryParse(lValue.ToString(), out _) && lIndex > 0)
            .Any();

        // Use Span because of memory?
        //ReadOnlySpan<char> span = location.AsSpan();

        var locationWithoutNumber = location.Where(l => !char.IsDigit(l)).ToList();
        var alphabeticallySorted = locationWithoutNumber.OrderBy(l => l).ToList();
        var notAlphabeticallySorted = !locationWithoutNumber.SequenceEqual(alphabeticallySorted);
        var notUpperCase = locationWithoutNumber.Any(l => !char.IsUpper(l));

        if (numberNotAtStart || notAlphabeticallySorted || notUpperCase)
            return false;

        var locationCodes = _relativeLocations.Select(r => r.Code).ToList();
        var invalidLocationCode = locationWithoutNumber.Any(
            l => !locationCodes.Contains(l.ToString()) || l == 'N'
        );
        if (invalidLocationCode)
            return false;

        return true;
    }
}

public sealed record RelativeLocation(string Code, string Name, string? Definition);
