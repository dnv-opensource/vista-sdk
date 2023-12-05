namespace Vista.SDK.Benchmarks.Gmod;

[MemoryDiagnoser]
[Orderer(SummaryOrderPolicy.FastestToSlowest, MethodOrderPolicy.Declared)]
[GroupBenchmarksBy(BenchmarkLogicalGroupRule.ByCategory), CategoriesColumn]
// [DotTraceDiagnoser]
public class GmodPathParse
{
    private SDK.Gmod _gmod;
    private Locations _locations;

    [GlobalSetup]
    public void Setup()
    {
        var vis = VIS.Instance;
        // Load cache
        _gmod = vis.GetGmod(VisVersion.v3_4a);
        _locations = vis.GetLocations(VisVersion.v3_4a);
    }

    [Benchmark, BenchmarkCategory("No location")]
    public bool TryParse() => GmodPath.TryParse("411.1/C101.72/I101", _gmod, _locations, out _);

    [Benchmark, BenchmarkCategory("No location")]
    public bool TryParseFullPath() =>
        GmodPath.TryParseFullPath("VE/400a/410/411/411i/411.1/CS1/C101/C101.7/C101.72/I101", _gmod, _locations, out _);

    [Benchmark, BenchmarkCategory("With location")]
    public bool TryParseIndividualized() => GmodPath.TryParse("612.21-1/C701.13/S93", _gmod, _locations, out _);

    [Benchmark, BenchmarkCategory("With location")]
    public bool TryParseFullPathIndividualized() =>
        GmodPath.TryParseFullPath(
            "VE/600a/610/612/612.2/612.2i/612.21-1/CS10/C701/C701.1/C701.13/S93",
            _gmod,
            _locations,
            out _
        );
}
