using Vista.SDK;

namespace Vista.SDK.Tests;

public class GmodTests
{
    [Fact]
    public void Test_Gmod_Loads()
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(VisVersion.v3_4a);
        Assert.NotNull(gmod);

        Assert.True(gmod.TryGetNode("400a", out _));
    }

    [Fact]
    public void Test_Gmod_Node_Equality()
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(VisVersion.v3_4a);

        var node1 = gmod["400a"];

        var node2 = gmod["400a"];

        Assert.Equal(node1, node2);
        Assert.Same(node1, node2);

        var node3 = node2 with { Location = "1" };
        Assert.NotEqual(node1, node3);
        Assert.NotSame(node1, node3);

        var node4 = node2 with { };
        Assert.Equal(node1, node4);
        Assert.NotSame(node1, node4);
    }

    [Fact]
    public void Test_Gmod_RootNode_Children()
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(VisVersion.v3_4a);

        var node = gmod.RootNode;

        Assert.NotEmpty(node.Children);
    }

    [Fact]
    public void Test_Normal_Assignments()
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(VisVersion.v3_4a);

        var node = gmod["411.3"];
        Assert.NotNull(node.ProductType);
        Assert.Null(node.ProductSelection);

        node = gmod["H601"];
        Assert.Null(node.ProductType);
    }

    [Fact]
    public void Test_Node_With_Product_Selection()
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(VisVersion.v3_4a);

        var node = gmod["411.2"];
        Assert.NotNull(node.ProductSelection);
        Assert.Null(node.ProductType);

        node = gmod["H601"];
        Assert.Null(node.ProductSelection);
    }

    [Fact]
    public void Test_Product_Selection()
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(VisVersion.v3_4a);

        var node = gmod["CS1"];
        Assert.True(node.IsProductSelection);
    }

    [Theory]
    [InlineData("VE", false)]
    [InlineData("300a", false)]
    [InlineData("300", true)]
    [InlineData("411", true)]
    [InlineData("410", true)]
    [InlineData("651.21s", false)]
    [InlineData("924.2", true)]
    [InlineData("411.1", false)]
    [InlineData("C101", true)]
    [InlineData("CS1", false)]
    [InlineData("C101.663", true)]
    [InlineData("C101.4", true)]
    [InlineData("C101.21s", false)]
    [InlineData("F201.11", true)]
    [InlineData("C101.211", false)]
    public void Test_Mappability(string code, bool mappable)
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(VisVersion.v3_4a);

        var node = gmod[code];

        Assert.Equal(mappable, node.IsMappable);
    }

    [Fact]
    public void Test_Full_Traversal()
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(VisVersion.v3_4a);

        var paths = new List<GmodPath>();

        var completed = gmod.Traverse(
            (parents, node) =>
            {
                Assert.True(parents.Count == 0 || parents[0].Code == "VE");

                if (parents.Any(p => p.Code == "HG3") || node.Code == "HG3")
                {
                    paths.Add(new GmodPath(parents, node));
                }

                return TraversalHandlerResult.Continue;
            }
        );
        Assert.True(completed);
    }

    [Fact]
    public void Test_Partial_Traversal()
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(VisVersion.v3_4a);

        var state = new TraversalState(5) { NodeCount = 0 };

        var completed = gmod.Traverse(
            state,
            (state, parents, node) =>
            {
                Assert.True(parents.Count == 0 || parents[0].Code == "VE");
                if (++state.NodeCount == state.StopAfter)
                    return TraversalHandlerResult.Stop;
                return TraversalHandlerResult.Continue;
            }
        );

        Assert.Equal(state.StopAfter, state.NodeCount);
        Assert.False(completed);
    }

    [Fact]
    public void Test_Full_Traversal_From()
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(VisVersion.v3_4a);

        var state = new TraversalState(0) { NodeCount = 0 };

        var completed = gmod.Traverse(
            state,
            gmod["400a"],
            (state, parents, node) =>
            {
                Assert.True(parents.Count == 0 || parents[0].Code == "400a");
                ++state.NodeCount;
                return TraversalHandlerResult.Continue;
            }
        );
        Assert.True(completed);
    }

    private sealed record TraversalState(int StopAfter)
    {
        public int NodeCount { get; set; }
    }
}
