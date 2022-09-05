namespace Vista.SDK.Tests;

public class UniversalIdTests
{
    public static IEnumerable<object[]> Test_Data =>
        new object[][]
        {
            new object[]
            {
                "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/621.21/S90/sec/411.1/C101/meta/qty-mass/cnt-fuel.oil/pos-inlet"
            }
        };

    [Theory]
    [MemberData(nameof(Test_Data))]
    public void Test_TryParsing(string testCase)
    {
        Assert.True(UniversalIdBuilder.TryParse(testCase, out var _));
    }

    [Theory]
    [MemberData(nameof(Test_Data))]
    public void Test_Parsing(string testCase)
    {
        var universalId = UniversalId.Parse(testCase);

        Assert.NotNull(universalId);
        Assert.True(universalId.ImoNumber.Equals(new ImoNumber(1234567)));
    }
}
