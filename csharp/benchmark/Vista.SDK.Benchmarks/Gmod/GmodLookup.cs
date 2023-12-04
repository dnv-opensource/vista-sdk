using System.Collections.Frozen;

namespace Vista.SDK.Benchmarks.Gmod;

[Config(typeof(Config))]
public class GmodLookup
{
    private Dictionary<string, GmodNode> _dict;
    private FrozenDictionary<string, GmodNode> _frozenDict;
    private SDK.Internal.NodeMap _nodeMap;

    [GlobalSetup]
    public void Setup()
    {
        var vis = VIS.Instance;
        // Load cache
        var gmod = vis.GetGmod(VisVersion.v3_7a);

        _dict = new Dictionary<string, GmodNode>(StringComparer.Ordinal);
        _nodeMap = new SDK.Internal.NodeMap(gmod.VisVersion, vis.GetGmodDto(gmod.VisVersion));
        foreach (var node in gmod)
            _dict[node.Code] = node;

        _frozenDict = _dict.ToFrozenDictionary(StringComparer.Ordinal);
    }

    // [Benchmark]
    // public bool Dict() => _dict.TryGetValue("400a", out _);

    [Benchmark]
    public bool FrozenDict() =>
        _frozenDict.TryGetValue("VE", out _)
        && _frozenDict.TryGetValue("400a", out _)
        && _frozenDict.TryGetValue("400", out _)
        && _frozenDict.TryGetValue("H346.11112", out _);

    [Benchmark]
    public bool NodeMap() =>
        _nodeMap.TryGetValue("VE", out _)
        && _nodeMap.TryGetValue("400a", out _)
        && _nodeMap.TryGetValue("400", out _)
        && _nodeMap.TryGetValue("H346.11112", out _);

    internal sealed class Config : ManualConfig
    {
        public Config()
        {
            this.SummaryStyle = SummaryStyle.Default.WithRatioStyle(RatioStyle.Trend);
            this.AddColumn(RankColumn.Arabic);
            this.Orderer = new DefaultOrderer(SummaryOrderPolicy.SlowestToFastest, MethodOrderPolicy.Declared);
            this.AddDiagnoser(MemoryDiagnoser.Default);
        }
    }
}
