using System.Xml.Serialization;
using Vista.SDK.Transport.Xml.DataChannel;
using Vista.SDK.Transport.Xml.TimeSeriesData;

namespace Vista.SDK.Tests.Transport.Xml;

public class XmlTests
{
    [Theory]
    [InlineData("Transport/Xml/_files/TimeSeriesData.xml")]
    public async Task Test_TimeSeriesData_Deserialization(string file)
    {
        var serializer = new XmlSerializer(typeof(TimeSeriesDataPackage));

        await using var reader = new FileStream(file, FileMode.Open);

        var package = serializer.Deserialize(reader) as TimeSeriesDataPackage;
        Assert.NotNull(package);
    }

    [Theory]
    [InlineData("Transport/Xml/_files/DataChannelList.xml")]
    public async Task Test_DataChannelList_Deserialization(string file)
    {
        var serializer = new XmlSerializer(typeof(DataChannelListPackage));

        await using var reader = new FileStream(file, FileMode.Open);

        var package = serializer.Deserialize(reader) as DataChannelListPackage;
        Assert.NotNull(package);
    }
}
