using System.Text.Json.Serialization;

namespace Vista.SDK;

internal sealed record GmodVersioningDto(
    [property: JsonPropertyName("visRelease")] string VisVersion,
    [property: JsonPropertyName("items")] IReadOnlyDictionary<string, GmodNodeConversionDto> Items
);

internal sealed record GmodVersioningAssignmentChangeDto(
    [property: JsonPropertyName("oldAssignment")] string OldAssignment,
    [property: JsonPropertyName("currentAssignment")] string CurrentAssignment
);

internal sealed record GmodNodeConversionDto(
    [property: JsonPropertyName("operations")] HashSet<string> Operations,
    [property: JsonPropertyName("source")] string Source,
    [property: JsonPropertyName("target")] string Target,
    [property: JsonPropertyName("oldAssignment")] string OldAssignment,
    [property: JsonPropertyName("newAssignment")] string NewAssignment,
    [property: JsonPropertyName("deleteAssignment")] bool DeleteAssignment
);
