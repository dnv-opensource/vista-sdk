using Microsoft.Extensions.DependencyInjection;

namespace Vista.SDK.Tests;

public class CodebooksTests
{
    [Fact]
    public void Test_Codebooks_Loads()
    {
        var (_, vis) = VISTests.GetVis();

        var codebooks = vis.GetCodebooks(VisVersion.v3_4a);
        Assert.NotNull(codebooks);

        Assert.NotNull(codebooks.GetCodebook(CodebookName.Position));
    }

    [Fact]
    public void Test_Codebooks_Equality()
    {
        var (_, vis) = VISTests.GetVis();

        var codebooks = vis.GetCodebooks(VisVersion.v3_4a);

        Assert.True(codebooks[CodebookName.Position].HasStandardValue("centre"));
    }
}
