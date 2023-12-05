using System.Collections.Frozen;

namespace Vista.SDK.Benchmarks.Gmod;

[Config(typeof(Config))]
public class GmodLookup
{
    private Dictionary<string, GmodNode> _dict;
    private FrozenDictionary<string, GmodNode> _frozenDict;
    private SDK.Gmod _gmod;

    [GlobalSetup]
    public void Setup()
    {
        var vis = VIS.Instance;
        // Load cache

        _gmod = vis.GetGmod(VisVersion.v3_7a);
        _dict = new Dictionary<string, GmodNode>(StringComparer.Ordinal);
        foreach (var node in _gmod)
            _dict[node.Code] = node;

        _frozenDict = _dict.ToFrozenDictionary(StringComparer.Ordinal);
    }

    [Benchmark(Baseline = true)]
    public bool Dict() =>
        _dict.TryGetValue("VE", out _)
        && _dict.TryGetValue("400a", out _)
        && _dict.TryGetValue("400", out _)
        && _dict.TryGetValue("H346.11112", out _);

    [Benchmark]
    public bool FrozenDict() =>
        _frozenDict.TryGetValue("VE", out _)
        && _frozenDict.TryGetValue("400a", out _)
        && _frozenDict.TryGetValue("400", out _)
        && _frozenDict.TryGetValue("H346.11112", out _);

    [Benchmark]
    public bool Gmod() =>
        _gmod.TryGetNode("VE", out _)
        && _gmod.TryGetNode("400a", out _)
        && _gmod.TryGetNode("400", out _)
        && _gmod.TryGetNode("H346.11112", out _);

    internal sealed class Config : ManualConfig
    {
        public Config()
        {
            this.SummaryStyle = SummaryStyle.Default.WithRatioStyle(RatioStyle.Trend);
            this.AddColumn(RankColumn.Arabic);
            this.Orderer = new DefaultOrderer(SummaryOrderPolicy.SlowestToFastest, MethodOrderPolicy.Declared);
            this.AddDiagnoser(MemoryDiagnoser.Default);
            // this.AddDiagnoser(new DotTraceDiagnoser());
        }
    }
}
