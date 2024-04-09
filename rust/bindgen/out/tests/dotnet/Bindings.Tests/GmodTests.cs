using uniffi.bindings;

namespace Bindings.Tests;

public class GmodTests
{
    private static readonly VisVersion _version = VisVersion.V37a;

    [Fact]
    public void Test_New_Gmod()
    {
        var gmod = Vis.Instance().GetGmod(_version);
        var rootNode = gmod.RootNode();
        var rootCode = rootNode.Code();

        Assert.Equal(_version, gmod.Version());
        Assert.Equal("VE", rootCode);
        var code = "C101";
        var node = gmod.GetNode(code);
        Assert.Equal(code, node.Code());
    }
}
