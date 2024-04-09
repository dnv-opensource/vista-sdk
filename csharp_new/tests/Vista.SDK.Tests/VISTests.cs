using Microsoft.Extensions.DependencyInjection;

namespace Vista.SDK.Tests;

public class VISTests
{
    public static (IServiceProvider ServiceProvider, IVIS Vis) GetVis()
    {
        var services = new ServiceCollection();
        services.AddVIS();
        var sp = services.BuildServiceProvider();

        var vis = sp.GetRequiredService<IVIS>();
        return (sp, vis);
    }

    [Fact]
    public void Test_DI()
    {
        var services = new ServiceCollection();

        services.AddVIS();

        var sp = services.BuildServiceProvider();

        var vis = sp.GetService<IVIS>();

        Assert.NotNull(vis);
    }

    [Fact]
    public void Test_VersionString()
    {
        var version = VisVersion.V34a;
        var versionStr = version.ToVersionString();

        Assert.Equal("3-4a", versionStr);
        Assert.Equal(version, VisVersions.Parse(versionStr));
    }
}
