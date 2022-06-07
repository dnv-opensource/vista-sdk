using Microsoft.Extensions.DependencyInjection;

using Vista.SDK;

namespace Vista.Benchmarks.SDK;

[MemoryDiagnoser]
[Orderer(SummaryOrderPolicy.FastestToSlowest, MethodOrderPolicy.Declared)]
public class GmodVersioningConvertPath
{
    private ServiceProvider _serviceProvider;
    private Vista.SDK.Gmod _gmod;
    private GmodPath _gmodPath;
    private GmodVersioning _gmodVersioning;

    [GlobalSetup]
    public void Setup()
    {
        var services = new ServiceCollection();

        services.AddVIS();

        _serviceProvider = services.BuildServiceProvider();

        var vis = _serviceProvider.GetRequiredService<IVIS>();
        _gmod = vis.GetGmod(VisVersion.v3_4a);
        _gmodPath = _gmod.ParsePath("411.1/C101.72/I101");
        _gmodVersioning = vis.GetGmodVersioning();
    }

    [Benchmark]
    public GmodPath ConvertPath() =>
        _gmodVersioning.ConvertPath(VisVersion.v3_4a, _gmodPath, VisVersion.v3_5a);

    [GlobalCleanup]
    public void Cleanup()
    {
        _serviceProvider.Dispose();
    }
}
