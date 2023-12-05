using System.Collections.Frozen;

namespace Vista.SDK.Benchmarks.Codebooks;

[Config(typeof(Config))]
public class CodebooksLookup
{
    private Dictionary<CodebookName, Codebook> _dict;
    private FrozenDictionary<CodebookName, Codebook> _frozenDict;
    private SDK.Codebooks _codebooks;

    [GlobalSetup]
    public void Setup()
    {
        var vis = VIS.Instance;
        // Load cache

        _codebooks = vis.GetCodebooks(VisVersion.v3_7a);
        _dict = new Dictionary<CodebookName, Codebook>();
        foreach (var codebook in _codebooks)
            _dict[codebook.Name] = codebook.Codebook;

        _frozenDict = _dict.ToFrozenDictionary();
    }

    [Benchmark(Baseline = true)]
    public bool Dict() =>
        _dict.TryGetValue(CodebookName.Quantity, out _)
        && _dict.TryGetValue(CodebookName.Type, out _)
        && _dict.TryGetValue(CodebookName.Detail, out _);

    [Benchmark]
    public bool FrozenDict() =>
        _frozenDict.TryGetValue(CodebookName.Quantity, out _)
        && _frozenDict.TryGetValue(CodebookName.Type, out _)
        && _frozenDict.TryGetValue(CodebookName.Detail, out _);

    [Benchmark]
    public bool Codebooks()
    {
        var a = _codebooks.GetCodebook(CodebookName.Quantity);
        var b = _codebooks.GetCodebook(CodebookName.Type);
        var c = _codebooks.GetCodebook(CodebookName.Detail);
        return a is not null && b is not null && c is not null;
    }

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
