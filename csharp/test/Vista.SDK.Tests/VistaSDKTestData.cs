using System.Text.Json;
using System.Text.Json.Serialization;

namespace Vista.SDK.Tests;

public class VistaSDKTestData
{
    public static IEnumerable<object[]> AddValidPositionData() =>
        AddCodebookData(CodebookTestData.ValidPosition);

    public static IEnumerable<object[]> AddStatesData() => AddCodebookData(CodebookTestData.States);

    public static IEnumerable<object[]> AddPositionsData() =>
        AddCodebookData(CodebookTestData.Positions);

    public static IEnumerable<object[]> AddTagData() => AddCodebookData(CodebookTestData.Tag);

    public static IEnumerable<object[]> AddDetailTagData() =>
        AddCodebookData(CodebookTestData.DetailTag);

    public static IEnumerable<object[]> AddInvalidLocalIdsData() =>
        AddInvalidLocalId(LocalIdTestData);

    private static CodebookTestData CodebookTestData => GetData<CodebookTestData>("Codebook");
    private static LocalIdTestData LocalIdTestData => GetData<LocalIdTestData>("InvalidLocalIds");

    private static T GetData<T>(string testName)
    {
        var path = $"testdata/{testName}.json";
        var testDataJson = File.ReadAllText(path);

        return JsonSerializer.Deserialize<T>(testDataJson)!;
    }

    public static IEnumerable<object[]> AddCodebookData(string[][] data)
    {
        foreach (var state in data)
        {
            yield return state;
        }
    }

    public static IEnumerable<object[]> AddInvalidLocalId(LocalIdTestData data)
    {
        foreach (var invalidLocalIdItem in data.InvalidLocalIds)
        {
            yield return new object[]
            {
                invalidLocalIdItem.localIdStr,
                invalidLocalIdItem.ExpectedErrormessages
            };
        }
    }
}

public record InvalidLocalIds(
    [property: JsonPropertyName("input")] string localIdStr,
    [property: JsonPropertyName("expectedErrorMessages")] string[] ExpectedErrormessages
);

public record LocalIdTestData(
    [property: JsonPropertyName("InvalidLocalIds")] InvalidLocalIds[] InvalidLocalIds
);

public record CodebookTestData(
    [property: JsonPropertyName("ValidPosition")] string[][] ValidPosition,
    [property: JsonPropertyName("Positions")] string[][] Positions,
    [property: JsonPropertyName("States")] string[][] States,
    [property: JsonPropertyName("Tag")] string[][] Tag,
    [property: JsonPropertyName("DetailTag")] string[][] DetailTag
);
