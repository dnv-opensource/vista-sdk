using System.ComponentModel.DataAnnotations;
using FluentAssertions;

namespace Vista.SDK.Tests;

public class LocationsTests
{
    public static IEnumerable<object[]> Test_Vis_Versions => GmodTests.Test_Vis_Versions;

    [Theory]
    [MemberData(nameof(Test_Vis_Versions))]
    public void Test_Locations_Loads(VisVersion visVersion)
    {
        var (_, vis) = VISTests.GetVis();

        var locations = vis.GetLocations(visVersion);
        Assert.NotNull(locations);
        Assert.NotNull(locations.Groups);
    }

    [Fact]
    public void Test_LocationGroups_Properties()
    {
        var values = Enum.GetValues(typeof(LocationGroup)).Cast<int>().ToArray();
        Assert.Equal(values.Length, new HashSet<int>(values).Count);
        Assert.Equal(5, values.Length); // If this changes, we need to change the way LocationCharDict is used
        Assert.Equal(0, (int)LocationGroup.Number);
        for (int i = 0; i < values.Length - 1; i++) // Start at one
        {
            Assert.Equal(i, values[i + 1] - 1);
        }
    }

    [Theory]
    [MemberData(nameof(VistaSDKTestData.AddLocationsData), MemberType = typeof(VistaSDKTestData))]
    public void Test_Locations(string value, bool success, string? output, string[] expectedErrorMessages)
    {
        var locations = VIS.Instance.GetLocations(VisVersion.v3_4a);

        var stringSuccess = locations.TryParse(value, out var stringParsedLocation, out var stringErrorBuilder);
        var spanSuccess = locations.TryParse(value.AsSpan(), out var spanParsedLocation, out var spanErrorBuilder);
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
    public void Test_Location_Builder()
    {
        var locations = VIS.Instance.GetLocations(VisVersion.v3_4a);

        var locationStr = "11FIPU";
        var location = locations.Parse(locationStr);

        var builder = LocationBuilder.Create(locations);

        builder = builder.WithNumber(11).WithSide('P').WithTransverse('I').WithLongitudinal('F').WithValue('U');

        Assert.Equal("11FIPU", builder.ToString());
        Assert.Equal(11, builder.Number);
        Assert.Equal('P', builder.Side);
        Assert.Equal('U', builder.Vertical);
        Assert.Equal('I', builder.Transverse);
        Assert.Equal('F', builder.Longitudinal);

        Assert.Throws<ValidationException>(() => builder = builder.WithValue('X'));
        Assert.Throws<ValidationException>(() => builder = builder.WithNumber(-1));
        Assert.Throws<ValidationException>(() => builder = builder.WithNumber(0));
        Assert.Throws<ValidationException>(() => builder = builder.WithSide('A'));
        Assert.Throws<ValidationException>(() => builder = builder.WithValue('a'));

        Assert.Equal(location, builder.Build());

        builder = LocationBuilder.Create(locations).WithLocation(builder.Build());

        Assert.Equal("11FIPU", builder.ToString());
        Assert.Equal(11, builder.Number);
        Assert.Equal('P', builder.Side);
        Assert.Equal('U', builder.Vertical);
        Assert.Equal('I', builder.Transverse);
        Assert.Equal('F', builder.Longitudinal);

        builder = builder.WithValue('S').WithValue(2);

        Assert.Equal("2FISU", builder.ToString());
        Assert.Equal(2, builder.Number);
        Assert.Equal('S', builder.Side);
        Assert.Equal('U', builder.Vertical);
        Assert.Equal('I', builder.Transverse);
        Assert.Equal('F', builder.Longitudinal);
    }

    [Fact]
    public void Test_LocationBuilder_WithLocation_SingleDigit()
    {
        var locations = VIS.Instance.GetLocations(VisVersion.v3_4a);

        // Test single digit location parsing
        var builder1 = LocationBuilder.Create(locations).WithLocation(new Location("1"));
        Assert.Equal(1, builder1.Number);
        Assert.Equal("1", builder1.ToString());

        var builder5 = LocationBuilder.Create(locations).WithLocation(new Location("5"));
        Assert.Equal(5, builder5.Number);
        Assert.Equal("5", builder5.ToString());

        var builder9 = LocationBuilder.Create(locations).WithLocation(new Location("9"));
        Assert.Equal(9, builder9.Number);
        Assert.Equal("9", builder9.ToString());

        // Test single digit with characters
        var builderMixed = LocationBuilder.Create(locations).WithLocation(new Location("1FIPU"));
        Assert.Equal(1, builderMixed.Number);
        Assert.Equal('P', builderMixed.Side);
        Assert.Equal('U', builderMixed.Vertical);
        Assert.Equal('I', builderMixed.Transverse);
        Assert.Equal('F', builderMixed.Longitudinal);
        Assert.Equal("1FIPU", builderMixed.ToString());
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
