using System.Diagnostics;
using FluentAssertions;
using Vista.SDK;
using Vista.SDK.Internal;

namespace Vista.SDK.Tests;

public class GmodTests
{
    public sealed record ExpectedValues(string Max, int Count);

    public static IEnumerable<object[]> Test_Vis_Versions => VisVersions.All.Select(x => new object[] { x }).ToArray();

    public static readonly Dictionary<VisVersion, ExpectedValues> ExpectedMaxes =
        new()
        {
            { VisVersion.v3_4a, new("C1053.3114", 6420) },
            { VisVersion.v3_5a, new("C1053.3114", 6557) },
            { VisVersion.v3_6a, new("C1053.3114", 6557) },
            { VisVersion.v3_7a, new("H346.11113", 6672) },
            { VisVersion.v3_8a, new("H346.11113", 6335) }
        };

    [Theory]
    [MemberData(nameof(Test_Vis_Versions))]
    public void Test_Gmod_Loads(VisVersion visVersion)
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(visVersion);
        Assert.NotNull(gmod);

        Assert.True(gmod.TryGetNode("400a", out _));
    }

    [Theory]
    [MemberData(nameof(Test_Vis_Versions))]
    public void Test_Gmod_Properties(VisVersion visVersion)
    {
        // This test ensures certain properties of the Gmod data
        // that we make some design desicisions based on,
        // i.e for hashing of the node code

        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(visVersion);
        Assert.NotNull(gmod);

        GmodNode? minLengthNode = null;
        int currentMinLength = int.MaxValue;

        GmodNode? maxLengthNode = null;
        int currentMaxLength = 0;
        int nodeCount = 0;

        foreach (var node in gmod)
        {
            nodeCount++;
            string code = node.Code;
            int len = code.Length;

            if (minLengthNode is null || len < currentMinLength)
            {
                currentMinLength = len;
                minLengthNode = node;
            }

            if (maxLengthNode is null || len > currentMaxLength)
            {
                currentMaxLength = len;
                maxLengthNode = node;
            }
            else if (len == currentMaxLength)
            {
                if (maxLengthNode is not null && string.CompareOrdinal(code, maxLengthNode.Code) > 0)
                {
                    maxLengthNode = node;
                }
            }
        }

        Assert.NotNull(minLengthNode);
        Assert.NotNull(maxLengthNode);

        Assert.Equal(2, minLengthNode.Code.Length);
        Assert.Equal("VE", minLengthNode.Code);

        Assert.Equal(10, maxLengthNode.Code.Length);
        Assert.True(ExpectedMaxes.TryGetValue(visVersion, out var expectedValues));

        Assert.Equal(expectedValues.Max, maxLengthNode.Code);

        Assert.Equal(expectedValues.Count, nodeCount);
    }

    [Theory]
    [MemberData(nameof(Test_Vis_Versions))]
    public void Test_Gmod_Lookup(VisVersion visVersion)
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(visVersion);
        Assert.NotNull(gmod);

        var gmodDto = VIS.Instance.GetGmodDto(visVersion);
        Assert.NotNull(gmodDto);

        {
            var seen = new HashSet<string>(StringComparer.Ordinal);
            var counter = 0;
            foreach (var node in gmodDto.Items)
            {
                Assert.NotNull(node?.Code);
                Assert.True(seen.Add(node.Code), $"Code: {node.Code}");

                Assert.NotNull(node.Code);
                Assert.True(gmod.TryGetNode(node.Code.AsSpan(), out var foundNode));
                Assert.NotNull(foundNode);
                Assert.Equal(node.Code, foundNode.Code);
                counter++;
            }
        }

        {
            var seen = new HashSet<string>(StringComparer.Ordinal);
            var counter = 0;
            foreach (var node in gmod)
            {
                Assert.NotNull(node?.Code);
                Assert.True(seen.Add(node.Code), $"Code: {node.Code}");

                Assert.NotNull(node.Code);
                Assert.True(gmod.TryGetNode(node.Code.AsSpan(), out var foundNode));
                Assert.NotNull(foundNode);
                Assert.Equal(node.Code, foundNode.Code);
                counter++;
            }

            Assert.Equal(gmodDto.Items.Length, counter);
        }

        Assert.False(gmod.TryGetNode("ABC", out _));
        Assert.False(gmod.TryGetNode(null!, out _));
        Assert.False(gmod.TryGetNode("", out _));
        Assert.False(gmod.TryGetNode("SDFASDFSDAFb", out _));
        Assert.False(gmod.TryGetNode("✅", out _));
        Assert.False(gmod.TryGetNode("a✅b", out _));
        Assert.False(gmod.TryGetNode("ac✅bc", out _));
        Assert.False(gmod.TryGetNode("✅bc", out _));
        Assert.False(gmod.TryGetNode("a✅", out _));
        Assert.False(gmod.TryGetNode("ag✅", out _));
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

        var node3 = node2.WithLocation("1");
        Assert.NotEqual(node1, node3);
        Assert.NotSame(node1, node3);

        var node4 = node2 with { };
        Assert.Equal(node1, node4);
        Assert.NotSame(node1, node4);
    }

    [Fact]
    public void Test_Gmod_Node_Types()
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(VisVersion.v3_4a);

        var set = new HashSet<string>();
        foreach (var node in gmod)
            set.Add($"{node.Metadata.Category} | {node.Metadata.Type}");

        Assert.NotEmpty(set);
    }

    [Theory]
    [MemberData(nameof(Test_Vis_Versions))]
    public void Test_Gmod_RootNode_Children(VisVersion visVersion)
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(visVersion);

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
        var maxExpected = TraversalOptions.DEFAULT_MAX_TRAVERSAL_OCCURRENCE;
        int maxOccurrence = 0;
        var completed = gmod.Traverse(
            (parents, node) =>
            {
                Assert.True(parents.Count == 0 || parents[0].IsRoot);

                if (parents.Any(p => p.Code == "HG3") || node.Code == "HG3")
                {
                    paths.Add(new GmodPath(parents.ToList(), node));
                }

                var skipOccurenceCheck = Gmod.IsProductSelectionAssignment(parents.LastOrDefault(), node);
                if (skipOccurenceCheck)
                    return TraversalHandlerResult.Continue;
                var occ = Occurrences(parents, node);
                if (occ > maxOccurrence)
                    maxOccurrence = occ;

                return TraversalHandlerResult.Continue;
            }
        );
        Assert.Equal(maxExpected, maxOccurrence);
        Assert.True(completed);
    }

    [Fact]
    public void Test_Full_Traversal_With_Options()
    {
        var (_, vis) = VISTests.GetVis();

        var gmod = vis.GetGmod(VisVersion.v3_4a);

        var maxExpected = 2;
        int maxOccurrence = 0;
        var completed = gmod.Traverse(
            (parents, node) =>
            {
                var skipOccurenceCheck = Gmod.IsProductSelectionAssignment(parents.LastOrDefault(), node);
                if (skipOccurenceCheck)
                    return TraversalHandlerResult.Continue;
                var occ = Occurrences(parents, node);
                if (occ > maxOccurrence)
                    maxOccurrence = occ;
                return TraversalHandlerResult.Continue;
            },
            new TraversalOptions { MaxTraversalOccurrence = maxExpected }
        );
        Assert.Equal(maxExpected, maxOccurrence);
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
                Assert.True(parents.Count == 0 || parents[0].IsRoot);
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

    private int Occurrences(IReadOnlyList<GmodNode> parents, GmodNode node) => parents.Count(p => p.Code == node.Code);
}
