using System.Text.Json.Serialization;

namespace Vista.SDK;

internal sealed record DataChannelTypeNamesDto([property: JsonPropertyName("values")] DataChannelTypeNameDto[] Values);

internal sealed record DataChannelTypeNameDto(
    [property: JsonPropertyName("type")] string Type,
    [property: JsonPropertyName("description")] string Description
);

internal sealed record FormatDataTypesDto([property: JsonPropertyName("values")] FormatDataTypeDto[] Values);

internal sealed record FormatDataTypeDto(
    [property: JsonPropertyName("type")] string Type,
    [property: JsonPropertyName("description")] string Description
);
