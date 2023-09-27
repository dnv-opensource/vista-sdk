using System.Text.Json;

namespace Vista.SDK.Tests;

public class ImoNumberTests
{
    private sealed record class TestData(TestDataItem[] ImoNumbers);

    private sealed record class TestDataItem(string Value, bool Success, string? Output);

    [Fact]
    public async Task Test_Validation()
    {
        var text = await File.ReadAllTextAsync("testdata/ImoNumbers.json");

        var data = JsonSerializer.Deserialize<TestData>(text, new JsonSerializerOptions(JsonSerializerDefaults.Web));

        foreach (var (value, success, output) in data!.ImoNumbers)
        {
            var parsedOk = ImoNumber.TryParse(value, out var parsedImo);
            if (success)
            {
                Assert.True(parsedOk);
                Assert.NotEqual(default, parsedImo);
            }
            else
            {
                Assert.False(parsedOk);
                Assert.Equal(default, parsedImo);
            }

            if (output is not null)
                Assert.Equal(parsedImo.ToString(), output);
        }
    }
}
