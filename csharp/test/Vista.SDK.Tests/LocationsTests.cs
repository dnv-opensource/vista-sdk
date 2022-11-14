using System.Text.Json;

namespace Vista.SDK.Tests;

public class LocationsTests
{
    private sealed record class TestData(TestDataItem[] Locations);

    private sealed record class TestDataItem(string Value, bool Success, string? Output);

    [Fact]
    public void Test_Locations_Loads()
    {
        var (_, vis) = VISTests.GetVis();

        var locations = vis.GetLocations(VisVersion.v3_4a);
        Assert.NotNull(locations);
    }

    [Fact]
    public async void Test_Locations()
    {
        var text = await File.ReadAllTextAsync("testdata/Locations.json");

        var data = JsonSerializer.Deserialize<TestData>(
            text,
            new JsonSerializerOptions(JsonSerializerDefaults.Web)
        );

        var (_, vis) = VISTests.GetVis();

        var locations = vis.GetLocations(VisVersion.v3_4a);

        foreach (var (value, _, output) in data!.Locations)
        {
            var parsedLocation = locations.TryParse(value);
            Assert.Equal(output, parsedLocation?.ToString());
        }
    }

    [Fact]
    public void Test_Locations_Equality()
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(VisVersion.v3_4a);

        var node1 = gmod["C101.663"].WithLocation("FIPU");

        var node2 = gmod["C101.663"].WithLocation("FIPU");

        Assert.Equal(node1, node2);
        Assert.NotSame(node1, node2);
    }
}
