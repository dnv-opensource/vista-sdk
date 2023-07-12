using Vista.SDK.Experimental.Transport;

namespace Vista.SDK.Experimental.Tests.Transport;

public class DataIdTests
{
    [Theory]
    [InlineData("/dnv-v2/vis-3-4a/1031/meta/cnt-refrigerant/state-leaking")]
    [InlineData("/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo/pos~percentage")]
    [InlineData("/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened")]
    [InlineData("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet")]
    [InlineData(
        "/dnv-v2/vis-3-4a/411.1/C101.63/S206/~propulsion.engine/~cooling.system/meta/qty-temperature/cnt-exhaust.gas/pos-inlet"
    )]
    [InlineData(
        "/dnv-v2-experimental/vis-3-6a/632.32i-1/S110/meta/cnt-high.temperature.fresh.water/maint.cat-preventive/act.type-overhaul"
    )]
    public void Test_Equality(string dataIdStr)
    {
        var d1 = DataId.Parse(dataIdStr);
        var d2 = DataId.Parse(dataIdStr);

        Assert.Equal(d1, d2);

        var dict = new Dictionary<DataId, string>() { { d1, dataIdStr } };

        Assert.True(dict.TryGetValue(d2, out _));
    }

    [Fact]
    public void Test_Inequality()
    {
        var d1 = DataId.Parse(
            "/dnv-v2-experimental/vis-3-6a/632.32i-1/S110/meta/cnt-high.temperature.fresh.water/maint.cat-preventive/act.type-overhaul"
        );
        var d2 = DataId.Parse("/dnv-v2/vis-3-4a/1031/meta/cnt-refrigerant/state-leaking");
        var d3 = DataId.Parse("some short id");

        Assert.NotEqual(d1, d2);

        var dict = new Dictionary<DataId, string>() { { d1, "" }, { d3, "" } };

        Assert.True(dict.TryGetValue(d1, out _));
        Assert.False(dict.TryGetValue(d2, out _));
        Assert.True(dict.TryGetValue(d3, out _));
    }
}
