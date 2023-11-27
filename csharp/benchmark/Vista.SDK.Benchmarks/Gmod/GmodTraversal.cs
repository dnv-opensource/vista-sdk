using Microsoft.Extensions.DependencyInjection;
using Vista.SDK;

namespace Vista.SDK.Benchmarks.Gmod;

[MemoryDiagnoser]
[Orderer(SummaryOrderPolicy.FastestToSlowest, MethodOrderPolicy.Declared)]
// [InProcess]
public class GmodTraversal
{
    private Vista.SDK.Gmod _gmod;

    [GlobalSetup]
    public void Setup()
    {
        var vis = VIS.Instance;
        _gmod = vis.GetGmod(VisVersion.v3_4a);
    }

    [Benchmark]
    public bool FullTraversal() => _gmod.Traverse((_, _) => TraversalHandlerResult.Continue);
}
