using Microsoft.Extensions.DependencyInjection;

using Vista.SDK;

namespace Vista.SDK.Benchmarks.Gmod;

[MemoryDiagnoser]
[Orderer(SummaryOrderPolicy.FastestToSlowest, MethodOrderPolicy.Declared)]
public class GmodPathParse
{
    private Vista.SDK.Gmod _gmod;

    [GlobalSetup]
    public void Setup()
    {
        var vis = VIS.Instance;
        _gmod = vis.GetGmod(VisVersion.v3_4a);
    }

    [Benchmark]
    public bool TryParse() => GmodPath.TryParse("411.1/C101.72/I101", _gmod, out _);
}
