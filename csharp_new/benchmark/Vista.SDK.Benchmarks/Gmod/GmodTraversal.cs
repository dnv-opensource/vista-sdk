using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Order;

namespace Vista.SDK.Benchmarks.Gmod;

[MemoryDiagnoser]
[Orderer(SummaryOrderPolicy.FastestToSlowest, MethodOrderPolicy.Declared)]
// [InProcess]
public class GmodTraversal
{
    private Vista.SDK.Gmod? _gmod;

    [GlobalSetup]
    public void Setup()
    {
        var vis = VIS.Instance;
        _gmod = vis.GetGmod(VisVersion.V34a);
    }

    [Benchmark]
    public bool? FullTraversal() => _gmod?.Traverse((_, _) => TraversalHandlerResult.Continue);
}
