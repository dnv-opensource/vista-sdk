using Vista.SDK.Transport;

namespace Vista.SDK.Tests.Transport;

public class ISO19848Tests
{
    public static IEnumerable<object[]> Test_Iso_Versions =>
        Enum.GetValues<ISO19848Version>().Select(x => new object[] { x }).ToArray();

    [Fact]
    public void Test_Instance()
    {
        var iso = ISO19848.Instance;
        Assert.NotNull(iso);
    }

    [Fact]
    public void Test_EmbeddedResource()
    {
        var assembly = typeof(EmbeddedResource).Assembly;
        var resourceName = EmbeddedResource
            .GetResourceNames(assembly)
            .FirstOrDefault(n => n.Contains("iso19848", StringComparison.Ordinal));
        Assert.NotNull(resourceName);

        using var stream = EmbeddedResource.GetStream(assembly, resourceName!);

        var buffer = new byte[1024 * 8];

        var task = stream.ReadAsync(buffer, default);
        Assert.True(task.IsCompletedSuccessfully);
    }

    [Theory]
    [MemberData(nameof(Test_Iso_Versions))]
    public void Test_DataChannelTypeNames_Load(ISO19848Version version)
    {
        var iso = ISO19848.Instance;
        var dataChannelTypeNames = iso.GetDataChannelTypeNames(version);
        Assert.NotNull(dataChannelTypeNames);
    }

    [Fact]
    public void Test_DataChannelTypeNames_Parse_Self()
    {
        var iso = ISO19848.Instance;
        var dataChannelTypeNames = iso.GetDataChannelTypeNames(ISO19848Version.v2024);
        Assert.NotNull(dataChannelTypeNames);
        foreach (var typeName in dataChannelTypeNames)
        {
            var result = dataChannelTypeNames.Parse(typeName.Type);
            var ok = Assert.IsType<DataChannelTypeNames.ParseResult.Ok>(result);
            Assert.Equal(typeName.Type, ok.TypeName.Type);
        }
    }

    [Theory]
    [InlineData("Inst", true)]
    [InlineData("Average", true)]
    [InlineData("Max", true)]
    [InlineData("Min", true)]
    [InlineData("Median", true)]
    [InlineData("Mode", true)]
    [InlineData("StandardDeviation", true)]
    [InlineData("Calculated", true)]
    [InlineData("SetPoint", true)]
    [InlineData("Command", true)]
    [InlineData("Alert", true)]
    [InlineData("Status", true)]
    [InlineData("ManualInput", true)]
    [InlineData("manualInput", false)]
    [InlineData("asd", false)]
    [InlineData("some-random", false)]
    [InlineData("InputManual", false)]
    public void Test_DataChannelTypeNames_Parse(string value, bool expectedResult)
    {
        var iso = ISO19848.Instance;
        var dataChannelTypeNames = iso.GetDataChannelTypeNames(ISO19848Version.v2024);
        Assert.NotNull(dataChannelTypeNames);
        var result = dataChannelTypeNames.Parse(value);

        if (expectedResult)
        {
            var ok = Assert.IsType<DataChannelTypeNames.ParseResult.Ok>(result);
            Assert.Equal(expectedResult, ok.TypeName.Type == value);
        }
        else
            Assert.IsType<DataChannelTypeNames.ParseResult.Invalid>(result);
    }

    [Theory]
    [MemberData(nameof(Test_Iso_Versions))]
    public void Test_FormatDataTypes_Load(ISO19848Version version)
    {
        var iso = ISO19848.Instance;
        var types = iso.GetFormatDataTypes(version);
        Assert.NotNull(types);
    }

    [Fact]
    public void Test_FormatDataType_Parse_Self()
    {
        var iso = ISO19848.Instance;
        var types = iso.GetFormatDataTypes(ISO19848Version.v2024);
        Assert.NotNull(types);
        foreach (var typeName in types)
        {
            var result = types.Parse(typeName.Type);
            var ok = Assert.IsType<FormatDataTypes.ParseResult.Ok>(result);
            Assert.Equal(typeName.Type, ok.TypeName.Type);
        }
    }

    [Theory]
    [InlineData("Decimal", true)]
    [InlineData("Integer", true)]
    [InlineData("Boolean", true)]
    [InlineData("String", true)]
    [InlineData("DateTime", true)]
    [InlineData("decimal", false)]
    [InlineData("string", false)]
    [InlineData("asd", false)]
    [InlineData("some-random", false)]
    [InlineData("TimeDate", false)]
    public void Test_FormatDataType_Parse(string value, bool expectedResult)
    {
        var iso = ISO19848.Instance;
        var types = iso.GetFormatDataTypes(ISO19848Version.v2024);
        Assert.NotNull(types);
        var result = types.Parse(value);
        if (expectedResult)
        {
            var ok = Assert.IsType<FormatDataTypes.ParseResult.Ok>(result);
            Assert.Equal(expectedResult, ok.TypeName.Type == value);
        }
        else
            Assert.IsType<FormatDataTypes.ParseResult.Invalid>(result);
    }
}
