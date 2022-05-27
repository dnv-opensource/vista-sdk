using Vista.SDK;

namespace Vista.SDK.Tests;

public class GmodPathTests
{
    public static IEnumerable<string[]> Valid_Test_Data =>
        new string[][]
        {
            new string[] { "411.1/C101.72/I101" },
            new string[] { "1014.211/S110.1/S101" },
            new string[] { "441.1/E202" },
            new string[] { "621.21/S90" },
            new string[] { "940.1/F221.5" },
            new string[] { "411.1/C101.472/C444" },
            new string[] { "612.42" },
            new string[] { "411.1/C101.31-5" },
            new string[] { "411.1/E202.1/E31" },
            new string[] { "411.1/E31" },
            new string[] { "511.11-1/C101.45/CS6d" },
            new string[] { "652.31/S90.3/S61" }
        };

    [Theory]
    [MemberData(nameof(Valid_Test_Data))]
    public void Test_GmodPath_Parse(string inputPath)
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(VisVersion.v3_4a);

        var parsed = GmodPath.TryParse(inputPath, gmod, out var path);
        Assert.True(parsed);
        Assert.NotNull(path);
        Assert.Equal(inputPath, path?.ToString());
    }

    [Fact]
    public void Test_GetFullPath()
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(VisVersion.v3_4a);

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
        foreach (var (depth, node) in GmodPath.Parse(pathStr, gmod).GetFullPath())
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

        var gmod = vis.GetGmod(VisVersion.v3_4a);

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
        var path = GmodPath.Parse(pathStr, gmod);
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
