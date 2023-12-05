namespace Vista.SDK.Benchmarks.Gmod;

[Config(typeof(Config))]
public class GmodLoad
{
    [Benchmark]
    public SDK.Gmod Load()
    {
        var dto = VIS.LoadGmodDto(VisVersion.v3_7a);
        return new SDK.Gmod(VisVersion.v3_7a, dto);
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
