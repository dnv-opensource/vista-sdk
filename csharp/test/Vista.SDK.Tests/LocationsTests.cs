namespace Vista.SDK.Tests;

public class LocationsTests
{
    [Fact]
    public void Test_Locations_Loads()
    {
        var (_, vis) = VISTests.GetVis();

        var locations = vis.GetLocations(VisVersion.v3_4a);
        Assert.NotNull(locations);
    }

    [Theory]
    [InlineData("FIPU", "FIPU")]
    [InlineData("1FIPU", "1FIPU")]
    [InlineData("F1IPU", null)]
    [InlineData("1IFPU", null)]
    [InlineData("I1FPU", null)]
    [InlineData("1aFPU", null)]
    [InlineData("1BFPU", null)]
    public void Test_Locations(string input, string? output)
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(VisVersion.v3_4a);
        var locations = vis.GetLocations(VisVersion.v3_4a);

        var createdLocation = locations.TryCreateLocation(input);
        Assert.Equal(output, createdLocation?.ToString());
    }

    [Fact]
    public void Test_Locations_Equality()
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(VisVersion.v3_4a);
        var locations = vis.GetLocations(VisVersion.v3_5a);

        var node1 = gmod["C101.663"].WithLocation(locations.CreateLocation("FIPU"));

        var node2 = gmod["C101.663"].WithLocation(locations.CreateLocation("FIPU"));

        Assert.Equal(node1, node2);
        Assert.NotSame(node1, node2);
    }
}
