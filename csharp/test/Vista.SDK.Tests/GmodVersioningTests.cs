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
        new string[][]
        {
            new string[] { "411.1/C101.72/I101", "411.1/C101.72/I101" },
            new string[] { "323.51/H362.1", "323.61/H362.1" },
            new string[] { "321.38/C906", "321.39/C906" },
            new string[] { "511.331/C221", "511.31/C121.31/C221" },
            new string[] { "511.11/C101.663i/C663.5/CS6d", "511.11/C101.663i/C663.6/CS6d" },
            new string[] { "511.11-1/C101.663i/C663.5/CS6d", "511.11-1/C101.663i/C663.6/CS6d" },
            new string[]
            {
                "1012.21/C1147.221/C1051.7/C101.22",
                "1012.21/C1147.221/C1051.7/C101.93"
            },
            new string[]
            {
                "1012.21/C1147.221/C1051.7/C101.61/S203.6",
                "1012.21/C1147.221/C1051.7/C101.311/C467.5"
            },
            new string[] { "001", "001", },
            new string[] { "038.7/F101.2/F71", "038.7/F101.2/F71", },
            new string[]
            {
                "1012.21/C1147.221/C1051.7/C101.61/S203.6/S61",
                "1012.21/C1147.221/C1051.7/C101.311/C467.5/S61",
            },
            new string[] { "000a", "000a", },
            new string[]
            {
                "1012.21/C1147.221/C1051.7/C101.61/S203.2/S101",
                "1012.21/C1147.221/C1051.7/C101.61/S203.3/S110.1/S101",
            },
            new string[] // Normal assignment change
            {
                "1012.21/C1147.221/C1051.7/C101.661i/C624",
                "1012.21/C1147.221/C1051.7/C101.661i/C621",
            },
            //new string[] // Parent code change and different depth, which introduced cycle
            //{
            //    "1012.22/S201.1/C151.2/S110.2/C101.61/S203.2/S101",
            //    "1012.22/S201.1/C151.2/S110.2/C101.61/S203.3/S110.1/S101",
            //},
            new string[]
            {
                "1012.22/S201.1/C151.2/S110.2/C101.64i",
                "1012.22/S201.1/C151.2/S110.2/C101.64",
            },
            new string[]
            {
                "632.32i/S110.2/C111.42/G203.31/S90.5/C401",
                "632.32i/S110.2/C111.42/G203.31/S90.5/C401",
            },
            new string[]
            {
                "864.11/G71.21/C101.64i/S201.1/C151.31/S110.2/C111.42/G204.41/S90.2/S51",
                "864.11/G71.21/C101.64/S201.1/C151.31/S110.2/C111.42/G204.41/S90.2/S51",
            },
            new string[]
            {
                "864.11/G71.21/C101.64i/S201.1/C151.31/S110.2/C111.41/G240.1/G242.2/S90.5/C401",
                "864.11/G71.21/C101.64/S201.1/C151.31/S110.2/C111.41/G240.1/G242.2/S90.5/C401",
            },
            new string[] { "221.31/C1141.41/C664.2/C471", "221.31/C1141.41/C664.2/C471", },
            //new string[] { "354.2/C1096", "354.2/C1096" }, // Was deleted, as path to root is gone
            new string[] { "514/E15", "514" },
        };

    [Theory]
    [MemberData(nameof(Valid_Test_Data_Path))]
    public void Test_GmodVersioning_ConvertPath(string inputPath, string expectedPath)
    {
        var gmod = VIS.Instance.GetGmod(VisVersion.v3_4a);
        var targetGmod = VIS.Instance.GetGmod(VisVersion.v3_5a);

        var sourcePath = GmodPath.Parse(inputPath, gmod);
        var parsedPath = targetGmod.TryParsePath(expectedPath, out var parsedTargetPath);
        var targetPath = VIS.Instance.ConvertPath(VisVersion.v3_4a, sourcePath, VisVersion.v3_5a);
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

    [Fact]
    public void Test_Finds_Path()
    {
        var gmod = VIS.Instance.GetGmod(VisVersion.v3_4a);
        var completed = gmod.Traverse(
            (parents, node) =>
            {
                if (parents.Count == 0)
                    return TraversalHandlerResult.Continue;

                var path = new GmodPath(parents.ToArray(), node);
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

    public static IEnumerable<string?[]> Valid_Test_Data_Node =>
        new string?[][]
        {
            new string?[] { "1014.211", null, "1014.211" },
            new string?[] { "323.5", null, "323.6" },
            new string?[] { "412.72", null, "412.7i" },
            new string?[] { "323.4", null, "323.5" },
            new string?[] { "323.51", null, "323.61" },
            new string?[] { "323.6", null, "323.7" },
            new string?[] { "C101.212", null, "C101.22" },
            new string?[] { "C101.22", null, "C101.93" },
            new string?[] { "511.31", null, "C121.1" },
            new string?[] { "C101.31", "5", "C101.31" }
        };

    [Theory]
    [MemberData(nameof(Valid_Test_Data_Node))]
    public void Test_GmodVersioning_ConvertNode(
        string inputCode,
        string? location,
        string expectedCode
    )
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(VisVersion.v3_4a);
        var targetGmod = vis.GetGmod(VisVersion.v3_5a);

        var sourceNode = gmod[inputCode] with { Location = location };
        var expectedNode = targetGmod[expectedCode] with { Location = location };

        var targetNode = vis.ConvertNode(VisVersion.v3_4a, sourceNode, VisVersion.v3_5a);

        Assert.Equal(expectedNode.Code, targetNode?.Code);
        Assert.Equal(expectedNode.Location, targetNode?.Location);
        Assert.Equal(expectedNode, targetNode);
    }
}
