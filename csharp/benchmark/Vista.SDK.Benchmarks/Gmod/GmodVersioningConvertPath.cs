using Vista.SDK;

namespace Vista.Benchmarks.SDK;

[MemoryDiagnoser]
[Orderer(SummaryOrderPolicy.FastestToSlowest, MethodOrderPolicy.Declared)]
public class GmodVersioningConvertPath
{
    private Gmod _gmod;
    private GmodPath _gmodPath;

    [GlobalSetup]
    public void Setup()
    {
        _gmod = VIS.Instance.GetGmod(VisVersion.v3_4a);
        _gmodPath = _gmod.ParsePath("411.1/C101.72/I101");
    }

    [Benchmark]
    public GmodPath ConvertPath() => VIS.Instance.ConvertPath(VisVersion.v3_4a, _gmodPath, VisVersion.v3_5a);
}
