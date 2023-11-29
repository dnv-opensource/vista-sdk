using System.Reflection.Metadata;
using Vista.SDK;

namespace Vista.SDK.Tests;

public class GmodPathTests
{
    [Theory]
    [MemberData(nameof(VistaSDKTestData.AddValidGmodPathsData), MemberType = typeof(VistaSDKTestData))]
    public void Test_GmodPath_Parse(string inputPath)
    {
        var (_, vis) = VISTests.GetVis();

        var parsed = GmodPath.TryParse(inputPath, VisVersion.v3_4a, out var path);
        Assert.True(parsed);
        Assert.NotNull(path);
        Assert.Equal(inputPath, path?.ToString());
    }

    [Theory]
    [MemberData(nameof(VistaSDKTestData.AddInvalidGmodPathsData), MemberType = typeof(VistaSDKTestData))]
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

    [Theory]
    [InlineData("411.1/C101.72/I101", "VE/400a/410/411/411i/411.1/CS1/C101/C101.7/C101.72/I101")]
    [InlineData("612.21-1/C701.13/S93", "VE/600a/610/612/612.2/612.2i-1/612.21-1/CS10/C701/C701.1/C701.13/S93")]
    public void Test_FullPathParsing(string shortPathStr, string expectedFullPathStr)
    {
        var version = VisVersion.v3_4a;

        var path = GmodPath.Parse(shortPathStr, version);
        var fullString = path.ToFullPathString();
        Assert.Equal(expectedFullPathStr, fullString);

        var parsed = GmodPath.TryParseFullPath(fullString, version, out var parsedPath);
        Assert.NotNull(parsedPath);
        Assert.True(parsed);
        Assert.StrictEqual(path, parsedPath);
        Assert.Equal(fullString, path.ToFullPathString());
        Assert.Equal(fullString, parsedPath.ToFullPathString());
        Assert.Equal(shortPathStr, path.ToString());
        Assert.Equal(shortPathStr, parsedPath.ToString());

        parsedPath = GmodPath.ParseFullPath(fullString, version);
        Assert.NotNull(parsedPath);
        Assert.True(parsed);
        Assert.StrictEqual(path, parsedPath);
        Assert.Equal(fullString, path.ToFullPathString());
        Assert.Equal(fullString, parsedPath.ToFullPathString());
        Assert.Equal(shortPathStr, path.ToString());
        Assert.Equal(shortPathStr, parsedPath.ToString());
    }

    [Theory]
    [MemberData(nameof(VistaSDKTestData.AddIndividualizableSetsData), MemberType = typeof(VistaSDKTestData))]
    public void Test_IndividualizableSets(bool isFullPath, string visVersion, string inputPath, string[][]? expected)
    {
        var version = VisVersions.Parse(visVersion);
        var gmod = VIS.Instance.GetGmod(version);

        if (expected is null)
        {
            GmodPath? parsed;
            Assert.False(
                isFullPath ? gmod.TryParseFromFullPath(inputPath, out parsed) : gmod.TryParsePath(inputPath, out parsed)
            );
            Assert.Null(parsed);
            return;
        }

        var path = isFullPath ? gmod.ParseFromFullPath(inputPath) : gmod.ParsePath(inputPath);
        var sets = path.IndividualizableSets;
        Assert.Equal(expected.Length, sets.Count);
        for (int i = 0; i < expected.Length; i++)
        {
            Assert.Equal(expected[i], sets[i].Nodes.Select(n => n.Code));
        }
    }

    [Theory]
    [MemberData(nameof(VistaSDKTestData.AddIndividualizableSetsData), MemberType = typeof(VistaSDKTestData))]
    public void Test_IndividualizableSets_FullPath(
        bool isFullPath,
        string visVersion,
        string inputPath,
        string[][]? expected
    )
    {
        var version = VisVersions.Parse(visVersion);
        var gmod = VIS.Instance.GetGmod(version);

        if (isFullPath)
            return; // Already is full path

        if (expected is null)
        {
            Assert.False(gmod.TryParsePath(inputPath, out var parsed));
            Assert.Null(parsed);
            return;
        }

        var path = GmodPath.ParseFullPath(gmod.ParsePath(inputPath).ToFullPathString(), version);
        var sets = path.IndividualizableSets;
        Assert.Equal(expected.Length, sets.Count);
        for (int i = 0; i < expected.Length; i++)
        {
            Assert.Equal(expected[i], sets[i].Nodes.Select(n => n.Code));
        }
    }

    // [Fact]
    // public void Test_GmodPath_Individualizes3()
    // {
    //     var version = VisVersion.v3_7a;
    //     var gmod = VIS.Instance.GetGmod(version);
    //     {
    //         var path = gmod.ParseFromFullPath("VE/600a/690/691/691.5");
    //         var shortPath = path.ToString();
    //         var sets = path.IndividualizableSets;
    //         Assert.Equal(1, sets.Count);
    //     }
    //     {
    //         var path = gmod.ParseFromFullPath("VE/600a/690/691/691.5/691.51/691.51s");
    //         var shortPath = path.ToString();
    //         var sets = path.IndividualizableSets;
    //         Assert.Equal(0, sets.Count);
    //     }
    // }

    [Fact]
    public void Test_GmodPath_Does_Not_Individualize()
    {
        var version = VisVersion.v3_7a;
        var gmod = VIS.Instance.GetGmod(version);
        var parsed = gmod.TryParsePath("500a-1", out var path);
        Assert.False(parsed);
        Assert.Null(path);
    }

    [Fact]
    public void Test_ToFullPathString()
    {
        var version = VisVersion.v3_7a;
        var gmod = VIS.Instance.GetGmod(version);

        var path = gmod.ParsePath("511.11-1/C101.663i-1/C663");
        Assert.Equal(
            "VE/500a/510/511/511.1/511.1i-1/511.11-1/CS1/C101/C101.6/C101.66/C101.663/C101.663i-1/C663",
            path.ToFullPathString()
        );

        path = gmod.ParsePath("846/G203.32-2/S110.2-1/E31");
        Assert.Equal("VE/800a/840/846/G203/G203.3-2/G203.32-2/S110/S110.2-1/CS1/E31", path.ToFullPathString());
    }

    [Theory]
    [MemberData(nameof(VistaSDKTestData.AddValidGmodPathsData), MemberType = typeof(VistaSDKTestData))]
    public void Test_Valid_GmodPath_IndividualizableSets(string inputPath)
    {
        var version = VisVersion.v3_4a;
        var gmod = VIS.Instance.GetGmod(version);

        var path = gmod.ParsePath(inputPath);
        var sets = path.IndividualizableSets;

        var uniqueCodes = new HashSet<string>();
        foreach (var set in sets)
        {
            foreach (var node in set.Nodes)
                Assert.True(uniqueCodes.Add(node.Code));
        }
    }

    [Theory]
    [MemberData(nameof(VistaSDKTestData.AddValidGmodPathsData), MemberType = typeof(VistaSDKTestData))]
    public void Test_Valid_GmodPath_IndividualizableSets_FullPath(string inputPath)
    {
        var version = VisVersion.v3_4a;
        var gmod = VIS.Instance.GetGmod(version);

        var path = GmodPath.ParseFullPath(gmod.ParsePath(inputPath).ToFullPathString(), version);
        var sets = path.IndividualizableSets;

        var uniqueCodes = new HashSet<string>();
        foreach (var set in sets)
        {
            foreach (var node in set.Nodes)
                Assert.True(uniqueCodes.Add(node.Code));
        }
    }
}
