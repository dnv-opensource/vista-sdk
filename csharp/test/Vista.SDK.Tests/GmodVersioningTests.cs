using System.Diagnostics;
using Xunit.Abstractions;

namespace Vista.SDK.Tests;

public class GmodVersioningTests
{
    private readonly ITestOutputHelper testOutputHelper;

    public GmodVersioningTests(ITestOutputHelper testOutputHelper)
    {
        this.testOutputHelper = testOutputHelper;
    }

    [Fact]
    public void Test_GmodVersioning_Loads()
    {
        // Arrange
        var (_, vis) = VISTests.GetVis();

        // Act
        var gmodVersioning = vis.GetGmodVersioning();

        // Assert
        Assert.NotNull(gmodVersioning);
        Assert.True(gmodVersioning.TryGetVersioningNode("3-5a", out _));
        Assert.True(gmodVersioning.TryGetVersioningNode("3-4a", out _));
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
        };

    [Theory]
    [MemberData(nameof(Valid_Test_Data_Path))]
    public void Test_GmodVersioning_ConvertPath(string inputPath, string expectedPath)
    {
        var (_, vis) = VISTests.GetVis();
        var gmod = vis.GetGmod(VisVersion.v3_4a);
        var targetGmod = vis.GetGmod(VisVersion.v3_5a);
        var gmodVersioning = vis.GetGmodVersioning();

        var sourcePath = GmodPath.Parse(inputPath, gmod);
        var parsedPath = targetGmod.TryParsePath(expectedPath, out var parsedTargetPath);
        var targetPath = gmodVersioning.ConvertPath(VisVersion.v3_4a, sourcePath, VisVersion.v3_5a);

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
    public void SmokeTest_GmodVersioning_ConvertPath()
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(VisVersion.v3_4a);
        var targetGmod = vis.GetGmod(VisVersion.v3_5a);
        var gmodVersioning = vis.GetGmodVersioning();

        var counter = 0;
        GmodPath? targetPath;
        var failedPaths = new List<Exception>();

        var completed = gmod.Traverse(
            (parents, node) =>
            {
                counter++;

                if (!GmodPath.IsValid(parents, node))
                    return TraversalHandlerResult.Continue;
                try
                {
                    var path = new GmodPath(parents, node);

                    targetPath = gmodVersioning.ConvertPath(
                        VisVersion.v3_4a,
                        path,
                        VisVersion.v3_5a
                    );
                    Assert.NotNull(targetPath);
                    var parsedPath = targetGmod.TryParsePath(
                        targetPath.ToString(),
                        out var parsedTargetPath
                    );
                    Assert.True(parsedPath);
                    Assert.Equal(parsedTargetPath?.ToString(), targetPath.ToString());
                }
                catch (Exception e)
                {
                    testOutputHelper.WriteLine(e.ToString());
                    failedPaths.Add(e);
                }
                return TraversalHandlerResult.Continue;
            }
        );
        Assert.True(completed);
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

        var gmodVersioning = vis.GetGmodVersioning();

        var targetNode = gmodVersioning.ConvertNode(VisVersion.v3_4a, sourceNode, VisVersion.v3_5a);

        Assert.Equal(expectedNode.Code, targetNode.Code);
        Assert.Equal(expectedNode.Location, targetNode.Location);
        Assert.Equal(expectedNode, targetNode);
    }
}
