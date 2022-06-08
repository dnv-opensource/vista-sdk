using System.Text.Json.Serialization;

namespace Vista.SDK;

internal sealed record CodebookDto(
    [property: JsonPropertyName("name")] string Name,
    [property: JsonPropertyName("values")] IReadOnlyDictionary<string, string[]> Values
);

internal sealed record CodebooksDto(
    [property: JsonPropertyName("visRelease")] string VisVersion,
    [property: JsonPropertyName("items")] CodebookDto[] Items
);
