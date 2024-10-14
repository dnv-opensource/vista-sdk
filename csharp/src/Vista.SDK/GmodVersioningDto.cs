using System.Text.Json.Serialization;

namespace Vista.SDK;

internal sealed record GmodVersioningNodeChangesDto(
    [property: JsonPropertyName("nextVisVersion")] string? NextVisVersion,
    [property: JsonPropertyName("nextCode")] string? NextCode,
    [property: JsonPropertyName("nextAssignmentChange")] GmodVersioningAssignmentChangeDto? NextAssignment,
    [property: JsonPropertyName("previousVisVersion")] string? PreviousVisVersion,
    [property: JsonPropertyName("previousCode")] string? PreviousCode,
    [property: JsonPropertyName("previousAssignmentChange")] GmodVersioningAssignmentChangeDto? PreviousAssignment
);

internal sealed record GmodVersioningDto(
    [property: JsonPropertyName("visRelease")] string VisVersion,
    [property: JsonPropertyName("items")] IReadOnlyDictionary<string, GmodVersioningNodeChangesDto> Items
);

internal sealed record GmodVersioningAssignmentChangeDto(
    [property: JsonPropertyName("oldAssignment")] string OldAssignment,
    [property: JsonPropertyName("currentAssignment")] string CurrentAssignment
);
