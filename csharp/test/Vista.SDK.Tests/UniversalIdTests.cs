namespace Vista.SDK.Tests;

public class UniversalIdTests
{
    public static IEnumerable<object[]> Test_Data =>
        new object[][]
        {
            new object[]
            {
                "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/621.21/S90/sec/411.1/C101/meta/qty-mass/cnt-fuel.oil/pos-inlet"
            },
            new object[] { "data.dnv.com/IMO1234567/dnv-v2/vis-3-7a/612.21/C701.23/C633/meta/calc~accumulate" }
        };

    [Theory]
    [MemberData(nameof(Test_Data))]
    public void Test_TryParsing(string testCase)
    {
        var success = UniversalIdBuilder.TryParse(testCase, out var errors, out var uid);
        Assert.True(success);
    }

    [Theory]
    [MemberData(nameof(Test_Data))]
    public void Test_Parsing(string testCase)
    {
        var universalId = UniversalId.Parse(testCase);

        Assert.NotNull(universalId);
        Assert.True(universalId.ImoNumber.Equals(new ImoNumber(1234567)));
    }

    [Theory]
    [MemberData(nameof(Test_Data))]
    public void Test_ToString(string testCase)
    {
        var universalId = UniversalIdBuilder.Parse(testCase);

        var universalIdString = universalId.ToString();
        Assert.NotNull(universalId);
        Assert.True(testCase == universalIdString);
    }

    [Theory]
    [MemberData(nameof(Test_Data))]
    public void Test_UniversalBuilder_Add_And_RemoveAll(string testCase)
    {
        UniversalIdBuilder.TryParse(testCase, out var universalIdBuilder);

        Assert.NotNull(universalIdBuilder);
        Assert.NotNull(universalIdBuilder!.LocalId);
        Assert.NotNull(universalIdBuilder.ImoNumber);

        var id = universalIdBuilder.WithoutImoNumber().WithoutLocalId();

        Assert.Null(id.LocalId);
        Assert.Null(id.ImoNumber);
    }

    [Fact]
    public void Test_UniversalBuilder_TryWith()
    {
        var universalBuilder = new UniversalIdBuilder();
        universalBuilder.TryWithLocalId(null);
        universalBuilder.TryWithImoNumber(null);

        Assert.Null(universalBuilder.LocalId);
        Assert.Null(universalBuilder.ImoNumber);
    }
}
