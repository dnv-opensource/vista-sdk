using Microsoft.Extensions.DependencyInjection;
using System.Reflection;
using Vista.SDK;

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
        var assembly = Assembly.GetExecutingAssembly();

        var resourceName = assembly.GetManifestResourceNames().Single(n => n.EndsWith("test.txt"));
        using var stream = (UnmanagedMemoryStream)assembly.GetManifestResourceStream(resourceName)!;

        var buffer = new byte[1024 * 8];

        Assert.Equal(512_000_000, stream.Length);

        var task = stream.ReadAsync(buffer, default);
        Assert.True(task.IsCompletedSuccessfully);
    }
}
