using FluentAssertions;
using System.Text.Json;
using System.Text.Json.Serialization;
using Vista.SDK.Internal;

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
    [MemberData(nameof(VistaSDKTestData.AddLocationsData), MemberType = typeof(VistaSDKTestData))]
    public void Test_Locations(
        string value,
        bool success,
        string? output,
        string[] expectedErrorMessages
    )
    {
        var locations = VIS.Instance.GetLocations(VisVersion.v3_4a);

        var succeeded = locations.TryParse(
            value,
            out var parsedLocation,
            out LocationParsingErrorBuilder errorBuilder
        );
        if (!success)
        {
            Assert.False(succeeded);
            Assert.Equal(default, parsedLocation);

            if (expectedErrorMessages.Length > 0)
            {
                Assert.NotNull(errorBuilder);
                Assert.True(errorBuilder.HasError);
                var actualErrors = errorBuilder.ErrorMessages.Select(e => e.message).ToArray();
                actualErrors.Should().Equal(expectedErrorMessages);
            }
        }
        else
        {
            Assert.True(succeeded);
            Assert.False(errorBuilder.HasError);
            Assert.NotEqual(default, parsedLocation);
            Assert.Equal(output, parsedLocation.ToString());
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
