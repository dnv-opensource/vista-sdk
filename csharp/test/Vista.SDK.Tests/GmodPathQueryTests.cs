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
}
