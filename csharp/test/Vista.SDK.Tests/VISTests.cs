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
        var version = VisVersion.v3_4a;
        var versionStr = version.ToVersionString();

        Assert.Equal("3-4a", versionStr);
        Assert.Equal(version, VisVersions.Parse(versionStr));
    }

    [Fact]
    public void Test_EmbeddedResource()
    {
        var assembly = typeof(EmbeddedResource).Assembly;
        var resourceName = EmbeddedResource
            .GetResourceNames(assembly)
            .FirstOrDefault(n => n.Contains("gmod", StringComparison.Ordinal));
        Assert.NotNull(resourceName);

        using var stream = EmbeddedResource.GetStream(assembly, resourceName!);

        var buffer = new byte[1024 * 8];

        Assert.True(stream.Length > 1024);

        var task = stream.ReadAsync(buffer, default);
        Assert.True(task.IsCompletedSuccessfully);
    }
}
