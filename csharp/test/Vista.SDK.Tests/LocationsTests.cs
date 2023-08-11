using FluentAssertions;
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

        var stringSuccess = locations.TryParse(
            value,
            out var stringParsedLocation,
            out var stringErrorBuilder
        );
        var spanSuccess = locations.TryParse(
            value.AsSpan(),
            out var spanParsedLocation,
            out var spanErrorBuilder
        );
        Verify(stringSuccess, stringErrorBuilder, stringParsedLocation);
        Verify(spanSuccess, spanErrorBuilder, spanParsedLocation);

        void Verify(bool succeeded, ParsingErrors errors, Location parsedLocation)
        {
            if (!success)
            {
                Assert.False(succeeded);
                Assert.Equal(default, parsedLocation);

                if (expectedErrorMessages.Length > 0)
                {
                    Assert.NotNull(errors);
                    Assert.True(errors.HasErrors);
                    var actualErrors = errors.Select(e => e.Message).ToArray();
                    actualErrors.Should().Equal(expectedErrorMessages);
                }
            }
            else
            {
                Assert.True(succeeded);
                Assert.False(errors.HasErrors);
                Assert.NotEqual(default, parsedLocation);
                Assert.Equal(output, parsedLocation.ToString());
            }
        }
    }

    [Fact]
    public void Test_Location_Parse_Throwing()
    {
        var locations = VIS.Instance.GetLocations(VisVersion.v3_4a);
        Assert.Throws<ArgumentException>(() => locations.Parse(null!));
        Assert.Throws<ArgumentException>(() => locations.Parse(ReadOnlySpan<char>.Empty));
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
