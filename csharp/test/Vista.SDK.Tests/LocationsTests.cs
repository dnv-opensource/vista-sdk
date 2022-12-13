using System.Text.Json;
using System.Text.Json.Serialization;
using Vista.SDK.Internal;

namespace Vista.SDK.Tests;

public class LocationsTests
{
    private sealed record TestData(
        [property: JsonPropertyName("Locations")] TestDataItem[] Locations
    );

    private sealed record class TestDataItem(
        string Value,
        bool Success,
        string? Output,
        string[]? ExpectedErrorMessages
    );

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

        foreach (var (value, success, output, expectedErrorMessages) in data!.Locations)
        {
            var parsedLocation = locations.TryParse(
                value,
                out LocationParsingErrorBuilder errorBuilder
            );
            if (!success && expectedErrorMessages is not null)
            {
                foreach (var error in errorBuilder.ErrorMessages)
                {
                    Assert.Contains(error.message, expectedErrorMessages);
                }

                Assert.NotNull(errorBuilder);
                Assert.Equal(expectedErrorMessages!.Count(), errorBuilder.ErrorMessages.Count);
            }
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
