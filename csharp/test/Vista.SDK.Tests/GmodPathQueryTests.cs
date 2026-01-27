namespace Vista.SDK.Tests;

public class GmodPathQueryTests
{
    public sealed record InputData(
        string Path,
        VisVersion VisVersion,
        (string Node, string[]? Locations)[] Parameters,
        bool ExpectedMatch
    );

    public static IEnumerable<object[]> Test_Path_Data =>

        [
            [new InputData("411.1-1/C101", VisVersion.v3_4a, [], true)],
            [new InputData("411.1-1/C101", VisVersion.v3_4a, [("411.1", ["1"])], true)],
            [new InputData("411.1-1/C101", VisVersion.v3_4a, [("411.1", ["A"])], false)],
            [new InputData("433.1-P/C322.31/C173", VisVersion.v3_4a, [("C322.31", null)], true)],
            [new InputData("433.1-P/C322.31-2/C173", VisVersion.v3_4a, [("433.1", ["P"]),("C322.31", null)], true)],
            [new InputData("433.1-P/C322.31-2/C173", VisVersion.v3_4a, [("433.1", ["A"]),("C322.31", null)], false)],
            [new InputData("433.1-P/C322.31-2/C173", VisVersion.v3_4a, [("433.1", ["P"]),("C322.31", ["1"])], false)],
            [new InputData("433.1-A/C322.31-2/C173", VisVersion.v3_4a, [("433.1", ["P"]),("C322.31", ["1"])], false)],
            [new InputData("433.1-A/C322.31-2/C173", VisVersion.v3_4a, [("433.1", null),("C322.31", null)], true)],
            [new InputData("433.1/C322.31-2/C173", VisVersion.v3_4a, [("433.1", ["A"])], false)],
            [new InputData("433.1/C322.31-2/C173", VisVersion.v3_4a, [("433.1", [])], true)],
        ];

    [Theory]
    [MemberData(nameof(Test_Path_Data))]
    public void Test_Path_Builder(InputData data)
    {
        var visVersion = data.VisVersion;
        var locations = VIS.Instance.GetLocations(visVersion);
        var gmod = VIS.Instance.GetGmod(visVersion);

        var pathStr = data.Path;
        var path = gmod.ParsePath(pathStr);

        var builder = GmodPathQueryBuilder.From(path);

        var query = builder.Build();
        // For consistency, the query should always match itself
        Assert.True(query.Match(path));

        foreach (var (node, locs) in data.Parameters)
        {
            var lcs = locs?.Select(locations.Parse).ToArray();
            if (lcs is null || lcs.Length == 0)
            {
                builder = builder.WithNode(nodes => nodes[node], true);
            }
            else
            {
                builder = builder.WithNode(nodes => nodes[node], lcs);
            }
        }
        query = builder.Build();
        var match = query.Match(path);
        Assert.Equal(data.ExpectedMatch, match);
    }

    public static IEnumerable<object[]> Test_Nodes_Data =>

        [
            [new InputData("411.1-1/C101", VisVersion.v3_4a, [("411.1", ["1"])], true)],
            [new InputData("411.1-1/C101.61/S203.3/S110.2/C101", VisVersion.v3_7a, [("411.1", ["1"])], true)],
            [new InputData("411.1/C101.61-1/S203.3/S110.2/C101", VisVersion.v3_7a, [("C101.61", ["1"])], true)],
            [new InputData("511.11/C101.61-1/S203.3/S110.2/C101", VisVersion.v3_7a, [("C101.61", ["1"])], true)],
            [new InputData("411.1/C101.61-1/S203.3/S110.2/C101", VisVersion.v3_7a, [("C101.61", null)], true)],
            [new InputData("511.11/C101.61-1/S203.3/S110.2/C101", VisVersion.v3_7a, [("C101.61", null)], true)],
            [new InputData("221.11/C1141.421/C1051.7/C101.61-2/S203", VisVersion.v3_7a, [("C101.61", null)], true)],
            [new InputData("411.1/C101.61-1/S203.3/S110.2/C101", VisVersion.v3_7a, [("411.1", null),("C101.61", null)], true)],
            [new InputData("511.11/C101.61-1/S203.3/S110.2/C101", VisVersion.v3_7a, [("411.1", null),("C101.61", null)], false)],
            [new InputData("411.1/C101.61/S203.3-1/S110.2/C101", VisVersion.v3_7a, [("S203.3", ["1"])], true)],
            [new InputData("411.1/C101.61/S203.3-1/S110.2/C101", VisVersion.v3_7a, [("S203.3", ["1"])], true)],
        ];

    [Theory]
    [MemberData(nameof(Test_Nodes_Data))]
    public void Test_Nodes_Builder(InputData data)
    {
        var visVersion = data.VisVersion;
        var locations = VIS.Instance.GetLocations(visVersion);
        var gmod = VIS.Instance.GetGmod(visVersion);

        var pathStr = data.Path;
        var path = gmod.ParsePath(pathStr);

        var builder = GmodPathQueryBuilder.Empty();

        foreach (var (n, locs) in data.Parameters)
        {
            var node = gmod[n];
            var lcs = locs?.Select(locations.Parse).ToArray();
            if (lcs is null || lcs.Length == 0)
            {
                builder = builder.WithNode(node, true);
            }
            else
            {
                builder = builder.WithNode(node, lcs);
            }
        }
        var query = builder.Build();
        var match = query.Match(path);
        Assert.Equal(data.ExpectedMatch, match);
    }

    [Fact]
    public void Test_WithAnyNodeBefore()
    {
        // Test the WithAnyNodeBefore functionality
        var gmod = VIS.Instance.GetGmod(VisVersion.v3_9a);

        // Base path: 411.1/C101.31
        var basePath = gmod.ParsePath("411.1/C101.31");

        // Build query that ignores nodes before C101, but C101.31 must match
        var query = GmodPathQueryBuilder
            .From(basePath)
            .WithAnyNodeBefore(nodes => nodes["C101"])
            .WithoutLocations()
            .Build();

        // Should match the base path
        Assert.True(query.Match(basePath));

        // Should match path with same C101.31 but different parent (511.11 instead of 411.1)
        var pathDifferentParent = gmod.ParsePath("511.11/C101.31");
        Assert.True(query.Match(pathDifferentParent));

        // Should match path with location on C101.31
        var pathWithLocation = gmod.ParsePath("411.1/C101.31-2");
        Assert.True(query.Match(pathWithLocation));

        // Should NOT match path with different C-node (C102.31 instead of C101.31)
        var pathDifferentCNode = gmod.ParsePath("411.1/C102.31");
        Assert.False(query.Match(pathDifferentCNode));

        // Should NOT match path that doesn't contain C101.31
        var pathWithoutC101 = gmod.ParsePath("411.1/C101.61/S203");
        Assert.False(query.Match(pathWithoutC101));
    }

    [Fact]
    public void Test_WithAnyNodeBefore_WithLocations()
    {
        // Test WithAnyNodeBefore with locations
        var gmod = VIS.Instance.GetGmod(VisVersion.v3_4a);

        // Base path with location: 433.1-P/C322.31
        var basePath = gmod.ParsePath("433.1-P/C322.31");

        // Build query that ignores nodes before C322, keeps location requirement on C322.31
        var query = GmodPathQueryBuilder.From(basePath).WithAnyNodeBefore(nodes => nodes["C322"]).Build();

        // Should match the base path
        Assert.True(query.Match(basePath));

        // Should match path with different parent but same C322.31
        var pathDifferentParent = gmod.ParsePath("433.1-S/C322.31");
        Assert.True(query.Match(pathDifferentParent));

        // Should match path without location on parent since we ignore nodes before C322
        var pathNoParentLocation = gmod.ParsePath("433.1/C322.31");
        Assert.True(query.Match(pathNoParentLocation));
    }

    [Fact]
    public void Test_WithAnyNodeBefore_S206Sensor()
    {
        // Test with_any_node_before correctly matches only paths containing the target node.
        // This tests the scenario where we want to match any path ending with S206 sensor,
        // ignoring the parent nodes (411.1 and C101.63) in the matching.
        var gmod = VIS.Instance.GetGmod(VisVersion.v3_4a);

        // Build query: Match paths containing S206, ignoring parent nodes
        var sensorPath = gmod.ParsePath("411.1/C101.63/S206");
        var query = GmodPathQueryBuilder
            .From(sensorPath)
            .WithoutLocations()
            .WithAnyNodeBefore(nodes => nodes["S206"])
            .Build();

        // Paths WITH S206 - should match
        var pathWithS206Loc1 = gmod.ParsePath("411.1-1/C101.63/S206");
        var pathWithS206Loc2 = gmod.ParsePath("411.1-2/C101.63/S206");
        Assert.True(query.Match(pathWithS206Loc1));
        Assert.True(query.Match(pathWithS206Loc2));

        // Paths WITHOUT S206 - should NOT match
        var pathC10131 = gmod.ParsePath("411.1/C101.31-2");
        var pathC10131Loc5 = gmod.ParsePath("411.1/C101.31-5");
        Assert.False(query.Match(pathC10131));
        Assert.False(query.Match(pathC10131Loc5));

        // Different sensor type - should NOT match
        var pathS203 = gmod.ParsePath("411.1/C101.61/S203");
        Assert.False(query.Match(pathS203));
    }

    [Fact]
    public void Test_WithAnyNodeAfter_PrefixMatching()
    {
        // Test with_any_node_after for prefix-style matching.
        // This tests the scenario where we want to match any path starting with
        // a specific prefix like "411.1/C101/*".
        var gmod = VIS.Instance.GetGmod(VisVersion.v3_4a);

        // Build query: Match paths starting with 411.1, ignoring children
        // This is like matching "411.1/*"
        var basePath = gmod.ParsePath("411.1/C101.31");
        var query = GmodPathQueryBuilder
            .From(basePath)
            .WithoutLocations()
            .WithAnyNodeAfter(nodes => nodes["411.1"])
            .Build();

        // Paths starting with 411.1 - should match
        var pathC10131 = gmod.ParsePath("411.1/C101.31-2");
        var pathC10163 = gmod.ParsePath("411.1/C101.63/S206");
        var pathC10161 = gmod.ParsePath("411.1-1/C101.61/S203");
        var pathC102 = gmod.ParsePath("411.1-1/C102");
        Assert.True(query.Match(pathC10131));
        Assert.True(query.Match(pathC10163));
        Assert.True(query.Match(pathC10161));
        Assert.True(query.Match(pathC102));

        // Paths NOT starting with 411.1 - should NOT match
        var path411i = gmod.ParsePath("411i");
        var path511 = gmod.ParsePath("511.11/C101.63/S206");
        var path652 = gmod.ParsePath("652.31/S90.3/S61");
        Assert.False(query.Match(path411i));
        Assert.False(query.Match(path511));
        Assert.False(query.Match(path652));
    }

    [Fact]
    public void Test_WithAnyNodeAfter_MidPath()
    {
        // Test with_any_node_after matching up to a mid-path node.
        var gmod = VIS.Instance.GetGmod(VisVersion.v3_4a);

        // Build query: Match paths containing 411.1/C101.6, ignoring what comes after
        var basePath = gmod.ParsePath("411.1/C101.63/S206");
        var query = GmodPathQueryBuilder
            .From(basePath)
            .WithoutLocations()
            .WithAnyNodeAfter(nodes => nodes["C101.6"])
            .Build();

        // Paths with 411.1/C101.6/* - should match
        var pathS206 = gmod.ParsePath("411.1/C101.63/S206");
        var pathS208 = gmod.ParsePath("411.1-1/C101.61/S203");
        Assert.True(query.Match(pathS206));
        Assert.True(query.Match(pathS208));

        // Paths with different C-node - should NOT match
        var pathC10131 = gmod.ParsePath("411.1/C101.31-2");
        var pathC101 = gmod.ParsePath("411.1/C101");
        Assert.False(query.Match(pathC10131));
        Assert.False(query.Match(pathC101));
    }
}
