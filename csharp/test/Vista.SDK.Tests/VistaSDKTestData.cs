using System.Text.Json;
using System.Text.Json.Serialization;

namespace Vista.SDK.Tests;

public class VistaSDKTestData
{
    public static IEnumerable<object[]> AddValidPositionData() =>
        AddData(CodebookTestData.ValidPosition);

    public static IEnumerable<object[]> AddStatesData() => AddData(CodebookTestData.States);

    public static IEnumerable<object[]> AddPositionsData() => AddData(CodebookTestData.Positions);

    public static IEnumerable<object[]> AddTagData() => AddData(CodebookTestData.Tag);

    public static IEnumerable<object[]> AddDetailTagData() => AddData(CodebookTestData.DetailTag);

    private static VistaTestData CodebookTestData => GetData("Codebook");

    private static VistaTestData GetData(string testName)
    {
        var path = $"testdata/{testName}.json";
        var testDataJson = File.ReadAllText(path);

        return JsonSerializer.Deserialize<VistaTestData>(testDataJson)!;
    }

    public static IEnumerable<object[]> AddData(string[][] data)
    {
        foreach (var state in data)
        {
            yield return state;
        }
    }
}

public record VistaTestData(
    [property: JsonPropertyName("ValidPosition")] string[][] ValidPosition,
    [property: JsonPropertyName("Positions")] string[][] Positions,
    [property: JsonPropertyName("States")] string[][] States,
    [property: JsonPropertyName("Tag")] string[][] Tag,
    [property: JsonPropertyName("DetailTag")] string[][] DetailTag
);
