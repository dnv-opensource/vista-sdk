using Microsoft.Extensions.DependencyInjection;

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
    [InlineData("C101.663", "FIPU", true)]
    [InlineData("C101.663", "1FIPU", true)]
    [InlineData("C101.663", "F1IPU", false)]
    [InlineData("C101.663", "1IFPU", false)]
    [InlineData("C101.663", "I1FPU", false)]
    [InlineData("C101.663", "1aFPU", false)]
    [InlineData("C101.663", "1BFPU", false)]
    public void Test_Locations(string code, string location, bool isValid)
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(VisVersion.v3_4a);
        var locations = vis.GetLocations(VisVersion.v3_4a);

        var node = gmod[code] with { Location = location };

        var validLocation = locations.IsValid(node);

        Assert.Equal(isValid, validLocation);
    }
}
