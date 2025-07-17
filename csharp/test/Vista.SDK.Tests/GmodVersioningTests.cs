using Xunit.Abstractions;

namespace Vista.SDK.Tests;

public class GmodVersioningTests
{
    private readonly ITestOutputHelper testOutputHelper;

    public GmodVersioningTests(ITestOutputHelper testOutputHelper)
    {
        this.testOutputHelper = testOutputHelper;
    }

    public static IEnumerable<object[]> Valid_Test_Data_Path =>

        [
            ["411.1/C101.72/I101", "411.1/C101.72/I101"],
            ["323.51/H362.1", "323.61/H362.1"],
            ["321.38/C906", "321.39/C906"],
            ["511.331/C221", "511.31/C121.31/C221"],
            ["511.11/C101.663i/C663.5/CS6d", "511.11/C101.663i/C663.6/CS6d"],
            ["511.11-1/C101.663i/C663.5/CS6d", "511.11-1/C101.663i/C663.6/CS6d"],
            ["1012.21/C1147.221/C1051.7/C101.22", "1012.21/C1147.221/C1051.7/C101.93"],
            ["1012.21/C1147.221/C1051.7/C101.61/S203.6", "1012.21/C1147.221/C1051.7/C101.311/C467.5"],
            ["001", "001",],
            ["038.7/F101.2/F71", "038.7/F101.2/F71",],
            [
                "1012.21/C1147.221/C1051.7/C101.61/S203.6/S61",
                "1012.21/C1147.221/C1051.7/C101.311/C467.5/S61",
            ],
            ["000a", "000a",],
            [
                "1012.21/C1147.221/C1051.7/C101.61/S203.2/S101",
                "1012.21/C1147.221/C1051.7/C101.61/S203.3/S110.1/S101",
            ],
            [
                "1012.21/C1147.221/C1051.7/C101.661i/C624",
                "1012.21/C1147.221/C1051.7/C101.661i/C621",
            ],
            //new string[] // Parent code change and different depth, which introduced cycle
            //{
            //    "1012.22/S201.1/C151.2/S110.2/C101.61/S203.2/S101",
            //    "1012.22/S201.1/C151.2/S110.2/C101.61/S203.3/S110.1/S101",
            //},
            ["1012.22/S201.1/C151.2/S110.2/C101.64i", "1012.22/S201.1/C151.2/S110.2/C101.64",],
            ["632.32i/S110.2/C111.42/G203.31/S90.5/C401", "632.32i/S110.2/C111.42/G203.31/S90.5/C401",],
            [
                "864.11/G71.21/C101.64i/S201.1/C151.31/S110.2/C111.42/G204.41/S90.2/S51",
                "864.11/G71.21/C101.64/S201.1/C151.31/S110.2/C111.42/G204.41/S90.2/S51",
            ],
            [
                "864.11/G71.21/C101.64i/S201.1/C151.31/S110.2/C111.41/G240.1/G242.2/S90.5/C401",
                "864.11/G71.21/C101.64/S201.1/C151.31/S110.2/C111.41/G240.1/G242.2/S90.5/C401",
            ],
            ["221.31/C1141.41/C664.2/C471", "221.31/C1141.41/C664.2/C471",],
            //new string[] { "354.2/C1096", "354.2/C1096" }, // Was deleted, as path to root is gone
            ["514/E15", "514"],
            ["244.1i/H101.111/H401", "244.1i/H101.11/H407.1/H401", VisVersion.v3_7a, VisVersion.v3_8a],
            ["1346/S201.1/C151.31/S110.2/C111.1/C109.16/C509", "1346/S201.1/C151.31/S110.2/C111.1/C109.126/C509", VisVersion.v3_7a, VisVersion.v3_8a]
        // ["851/H231", "851", VisVersion.v3_6a, VisVersion.v3_7a], // Effect of SD - no idea how to version this
        ];
    public static IEnumerable<object[]> Valid_Test_Data_FullPath =>

        [
            ["VE/600a/630/632/632.3/632.32/632.32i-2/S110","VE/600a/630/632/632.3/632.32/632.32i-2/SS5/S110"]
        ];

    [Theory]
    [MemberData(nameof(Valid_Test_Data_Path))]
    public void Test_GmodVersioning_ConvertPath(
        string inputPath,
        string expectedPath,
        VisVersion sourceVersion = VisVersion.v3_4a,
        VisVersion targetVersion = VisVersion.v3_6a
    )
    {
        var targetGmod = VIS.Instance.GetGmod(targetVersion);

        var sourcePath = GmodPath.Parse(inputPath, sourceVersion);
        var parsedPath = targetGmod.TryParsePath(expectedPath, out var parsedTargetPath);

        var targetPath = VIS.Instance.ConvertPath(sourcePath.VisVersion, sourcePath, targetGmod.VisVersion);
        Assert.NotNull(targetPath);

        var nodesWithLocation = sourcePath
            .GetFullPath()
            .Where(n => n.Node.Location is not null)
            .Select(n => n.Node.Code)
            .ToArray();
        targetGmod.Traverse(
            (parents, node) =>
            {
                Assert.Null(node.Location);
                return TraversalHandlerResult.Continue;
            }
        );
        testOutputHelper.WriteLine(sourcePath.ToString());
        Assert.NotNull(sourcePath);
        Assert.Equal(inputPath, sourcePath?.ToString());

        Assert.True(parsedPath);
        Assert.Equal(expectedPath, parsedTargetPath?.ToString());

        Assert.NotNull(targetPath);
        Assert.Equal(expectedPath, targetPath?.ToString());
    }

    [Theory]
    [MemberData(nameof(Valid_Test_Data_FullPath))]
    public void Test_GmodVersioning_ConvertFullPath(string inputPath, string expectedPath)
    {
        var targetGmod = VIS.Instance.GetGmod(VisVersion.v3_6a);

        var sourcePath = GmodPath.ParseFullPath(inputPath, VisVersion.v3_4a);
        var parsedPath = targetGmod.TryParseFromFullPath(expectedPath, out var parsedTargetPath);

        var targetPath = VIS.Instance.ConvertPath(VisVersion.v3_4a, sourcePath, targetGmod.VisVersion);
        Assert.NotNull(targetPath);

        testOutputHelper.WriteLine(sourcePath.ToFullPathString());
        Assert.NotNull(sourcePath);
        Assert.Equal(inputPath, sourcePath?.ToFullPathString());

        Assert.True(parsedPath);
        Assert.Equal(expectedPath, parsedTargetPath?.ToFullPathString());

        Assert.NotNull(targetPath);
        Assert.Equal(expectedPath, targetPath?.ToFullPathString());
    }

    [Fact]
    public void Test_Finds_Path()
    {
        var gmod = VIS.Instance.GetGmod(VisVersion.v3_4a);
        var completed = gmod.Traverse(
            (parents, node) =>
            {
                if (parents.Count == 0)
                    return TraversalHandlerResult.Continue;

                var path = new GmodPath(parents.ToList(), node);
                if (path.ToString() == "1012.22/S201.1/C151.2/S110.2/C101.61/S203.2/S101")
                    return TraversalHandlerResult.Stop;

                return TraversalHandlerResult.Continue;
            }
        );

        Assert.False(completed);
    }

    [Fact]
    public void Test_One_Path_To_Root_For_Asset_Functions()
    {
        static bool OnePathToRoot(GmodNode node) =>
            node.IsRoot || (node.Parents.Count == 1 && OnePathToRoot(node.Parents[0]));

        foreach (var version in VisVersions.All)
        {
            var gmod = VIS.Instance.GetGmod(version);
            foreach (var node in gmod)
            {
                if (!node.IsAssetFunctionNode)
                    continue;

                Assert.True(OnePathToRoot(node));
            }
        }
    }

    public static IEnumerable<object?[]> Valid_Test_Data_Node =>

        [
            ["1014.211", null, "1014.211"],
            ["323.5", null, "323.6"],
            ["412.72", null, "412.7i"],
            ["323.4", null, "323.5"],
            ["323.51", null, "323.61"],
            ["323.6", null, "323.7"],
            ["C101.212", null, "C101.22"],
            ["C101.22", null, "C101.93"],
            ["511.31", null, "C121.1"],
            ["C101.31", "5", "C101.31"]
        ];

    [Theory]
    [MemberData(nameof(Valid_Test_Data_Node))]
    public void Test_GmodVersioning_ConvertNode(string inputCode, string? location, string expectedCode)
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(VisVersion.v3_4a);
        var targetGmod = vis.GetGmod(VisVersion.v3_6a);

        var sourceNode = gmod[inputCode].TryWithLocation(location);
        var expectedNode = targetGmod[expectedCode].TryWithLocation(location);

        var targetNode = vis.ConvertNode(VisVersion.v3_4a, sourceNode, targetGmod.VisVersion);

        Assert.Equal(expectedNode.Code, targetNode?.Code);
        Assert.Equal(expectedNode.Location, targetNode?.Location);
        Assert.Equal(expectedNode, targetNode);
    }

    [Theory]
    [InlineData(
        "/dnv-v2/vis-3-4a/411.1/C101/sec/411.1/C101.64i/S201/meta/cnt-condensate",
        "/dnv-v2/vis-3-5a/411.1/C101/sec/411.1/C101.64/S201/meta/cnt-condensate"
    )]
    public void ConvertLocalId(string sourceLocalIdStr, string targetLocalIdStr)
    {
        var sourceLocalId = LocalIdBuilder.Parse(sourceLocalIdStr);
        var targetLocalId = LocalIdBuilder.Parse(targetLocalIdStr);

        var convertedLocalId = VIS.Instance.ConvertLocalId(sourceLocalId, targetLocalId.VisVersion!.Value);
        Assert.Equal(targetLocalId, convertedLocalId);
        Assert.Equal(targetLocalIdStr, convertedLocalId!.ToString());
    }

    [Theory]
    [MemberData(nameof(VistaSDKTestData.AddValidGmodPathsData), MemberType = typeof(VistaSDKTestData))]
    public void Test_Valid_GmodPath_To_Latest(GmodPathTestItem item)
    {
        var (_, vis) = VISTests.GetVis();
        Assert.True(VisVersions.TryParse(item.VisVersion, out var sourceVersion));
        Assert.True(GmodPath.TryParse(item.Path, sourceVersion, out var sourcePath));
        var targetPath = vis.ConvertPath(sourceVersion, sourcePath, VIS.LatestVisVersion);
        Assert.NotNull(targetPath);
    }

    [Fact(Skip = "3-8 S204 is not in 3-8a")]
    public void ConvertEveryNodeToLatest()
    {
        var (_, vis) = VISTests.GetVis();
        VisVersion[] visVersions = [VisVersion.v3_7a];
        var gmodMap = vis.GetGmodsMap(visVersions);
        var errored = new Dictionary<VisVersion, List<string>>();
        foreach (var visVersion in visVersions)
        {
            var gmod = gmodMap[visVersion];
            testOutputHelper.WriteLine($"Converting {visVersion} to {VIS.LatestVisVersion}");
            if (!errored.TryGetValue(visVersion, out var errs))
                errored.Add(visVersion, errs =  []);

            gmod.Traverse(
                (parents, node) =>
                {
                    var parent = parents.LastOrDefault();
                    var targetNode = vis.ConvertNode(visVersion, node, VIS.LatestVisVersion);
                    if (targetNode is null)
                        errs.Add(node.Code);
                    return TraversalHandlerResult.Continue;
                }
            );
        }

        Assert.All(errored, kvp => Assert.Empty(kvp.Value));
    }
}
