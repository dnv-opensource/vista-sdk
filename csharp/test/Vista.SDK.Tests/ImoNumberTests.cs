namespace Vista.SDK.Tests;

public class ImoNumberTests
{
    [Theory]
    [InlineData(-1)]
    [InlineData(0)]
    [InlineData(1)]
    [InlineData(123412034)]
    [InlineData(1234507)]
    public void Test_Invalid_Numbers(int imoNumber)
    {
        Assert.False(ImoNumber.IsValid(imoNumber));
        Assert.Throws<ArgumentException>(() => new ImoNumber(imoNumber));
    }

    [Theory]
    [InlineData(9074729)]
    [InlineData(9785811)]
    [InlineData(9704611)]
    [InlineData(9368302)]
    [InlineData(9387762)]
    [InlineData(9370537)]
    [InlineData(9680102)]
    [InlineData(9735048)]
    [InlineData(9785823)]
    [InlineData(1234567)]
    public void Test_Valid_Number(int imoNumber)
    {
        Assert.True(ImoNumber.IsValid(imoNumber));
        var _ = new ImoNumber(imoNumber);
    }
}
