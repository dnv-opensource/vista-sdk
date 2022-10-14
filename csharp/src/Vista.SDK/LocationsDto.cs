using System.Text.Json.Serialization;

namespace Vista.SDK;

internal sealed record RelativeLocationsDto(
    [property: JsonPropertyName("code")] char Code,
    [property: JsonPropertyName("name")] string Name,
    [property: JsonPropertyName("definition")] string? Definition
);

internal sealed record LocationsDto(
    [property: JsonPropertyName("visRelease")] string VisVersion,
    [property: JsonPropertyName("items")] RelativeLocationsDto[] Items
);
