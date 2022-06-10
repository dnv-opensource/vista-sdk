using System.Text.Json;
using System.Text.Json.Serialization;

namespace Vista.SDK.Tests;

public class VistaSDKTestData
{
    public static IEnumerable<object[]> AddValidPositionData()
    {
        foreach (var vp in CodebookTestData.ValidPosition)
        {
            yield return vp;
        }
    }

    public static IEnumerable<object[]> AddStatesData()
    {
        foreach (var state in CodebookTestData.States)
        {
            yield return state;
        }
    }

    public static IEnumerable<object[]> AddPositionsData()
    {
        foreach (var position in CodebookTestData.Positions)
        {
            yield return position;
        }
    }

    public static IEnumerable<object[]> AddTagData()
    {
        foreach (var tag in CodebookTestData.Tag)
        {
            yield return tag;
        }
    }

    public static IEnumerable<object[]> AddDetailTagData()
    {
        foreach (var tag in CodebookTestData.DetailTag)
        {
            yield return tag;
        }
    }

    private static VistaTestData CodebookTestData => GetData("Codebook");

    private static VistaTestData GetData(string testName)
    {
        var resourcePath = $@"..\..\..\testdata\{testName}.json";
        var path = Path.GetFullPath(
            Path.Combine(resourcePath, $@"..\..\..\..\..\testdata\{testName}.json")
        );

        var testDataJson = File.ReadAllText(path);

        return JsonSerializer.Deserialize<VistaTestData>(testDataJson)!;
    }
}

public record VistaTestData(
    [property: JsonPropertyName("ValidPosition")] string[][] ValidPosition,
    [property: JsonPropertyName("Positions")] string[][] Positions,
    [property: JsonPropertyName("States")] string[][] States,
    [property: JsonPropertyName("Tag")] string[][] Tag,
    [property: JsonPropertyName("DetailTag")] string[][] DetailTag
);
