using Vista.SDK;

namespace Vista.SDK.Tests;

public class GmodPathTests
{
    [Theory]
    [MemberData(
        nameof(VistaSDKTestData.AddValidGmodPathsData),
        MemberType = typeof(VistaSDKTestData)
    )]
    public void Test_GmodPath_Parse(string inputPath)
    {
        var (_, vis) = VISTests.GetVis();

        var parsed = GmodPath.TryParse(inputPath, VisVersion.v3_4a, out var path);
        Assert.True(parsed);
        Assert.NotNull(path);
        Assert.Equal(inputPath, path?.ToString());
    }

    [Theory]
    [MemberData(
        nameof(VistaSDKTestData.AddInvalidGmodPathsData),
        MemberType = typeof(VistaSDKTestData)
    )]
    public void Test_GmodPath_Parse_Invalid(string inputPath)
    {
        var (_, vis) = VISTests.GetVis();

        var parsed = GmodPath.TryParse(inputPath, VisVersion.v3_4a, out var path);
        Assert.False(parsed);
        Assert.Null(path);
    }

    [Fact]
    public void Test_GetFullPath()
    {
        var (_, vis) = VISTests.GetVis();

        var pathStr = "411.1/C101.72/I101";
        var expectation = new Dictionary<int, string>
        {
            [0] = "VE",
            [1] = "400a",
            [2] = "410",
            [3] = "411",
            [4] = "411i",
            [5] = "411.1",
            [6] = "CS1",
            [7] = "C101",
            [8] = "C101.7",
            [9] = "C101.72",
            [10] = "I101",
        };

        var seen = new HashSet<int>();
        foreach (var (depth, node) in GmodPath.Parse(pathStr, VisVersion.v3_4a).GetFullPath())
        {
            if (!seen.Add(depth))
                Assert.True(false, "Got same depth twice");
            if (seen.Count == 1)
                Assert.Equal(0, depth);
            Assert.Equal(expectation[depth], node.Code);
        }

        Assert.True(expectation.Keys.OrderBy(x => x).SequenceEqual(seen.OrderBy(x => x)));
    }

    [Fact]
    public void Test_GetFullPathFrom()
    {
        var (_, vis) = VISTests.GetVis();

        var pathStr = "411.1/C101.72/I101";
        var expectation = new Dictionary<int, string>
        {
            [4] = "411i",
            [5] = "411.1",
            [6] = "CS1",
            [7] = "C101",
            [8] = "C101.7",
            [9] = "C101.72",
            [10] = "I101",
        };

        var seen = new HashSet<int>();
        var path = GmodPath.Parse(pathStr, VisVersion.v3_4a);
        foreach (var (depth, node) in path.GetFullPathFrom(4))
        {
            if (!seen.Add(depth))
                Assert.True(false, "Got same depth twice");
            if (seen.Count == 1)
                Assert.Equal(4, depth);
            Assert.Equal(expectation[depth], node.Code);
        }

        Assert.True(expectation.Keys.OrderBy(x => x).SequenceEqual(seen.OrderBy(x => x)));
    }
}
