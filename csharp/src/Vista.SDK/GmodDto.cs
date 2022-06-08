using System.Text.Json.Serialization;

namespace Vista.SDK;

internal sealed record GmodNodeDto(
    [property: JsonPropertyName("category")] string Category,
    [property: JsonPropertyName("type")] string Type,
    [property: JsonPropertyName("code")] string Code,
    [property: JsonPropertyName("name")] string Name,
    [property: JsonPropertyName("commonName")] string? CommonName,
    [property: JsonPropertyName("definition")] string? Definition,
    [property: JsonPropertyName("commonDefinition")] string? CommonDefinition,
    [property: JsonPropertyName("installSubstructure")] bool? InstallSubstructure,
    [property: JsonPropertyName("normalAssignmentNames")]
        IReadOnlyDictionary<string, string>? NormalAssignmentNames
);

internal sealed record GmodDto(
    [property: JsonPropertyName("visRelease")] string VisVersion,
    [property: JsonPropertyName("items")] GmodNodeDto[] Items,
    [property: JsonPropertyName("relations")] string[][] Relations
);
