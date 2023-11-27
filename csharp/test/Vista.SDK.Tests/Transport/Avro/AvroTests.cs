using FluentAssertions;
using Vista.SDK.Transport.Apache.Avro;
using Vista.SDK.Transport.Avro.DataChannel;
using Vista.SDK.Transport.Avro.TimeSeriesData;
using JsonSerializer = Vista.SDK.Transport.Json.Serializer;

namespace Vista.SDK.Tests.Transport.Avro;

public class AvroTests
{
    [Fact]
    public void Test_Create()
    {
        var domainPackage = IsoMessageTests.TestDataChannelListPackage;
        var dto = domainPackage.ToAvroDto();
    }

    [Theory]
    [InlineData("Transport/Json/_files/DataChannelList.json")]
    public async Task DataChannelList_Json_Dto_To_Domain_And_Avro_Dto(string file)
    {
        await using var reader = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.Read);

        var package = await JsonSerializer.DeserializeDataChannelListAsync(reader);
        Assert.NotNull(package);

        var domainModel = Vista.SDK.Transport.Json.DataChannel.Extensions.ToDomainModel(package!);
        Assert.NotNull(domainModel);
        var dto = domainModel.ToAvroDto();

        await using var writer = new FileStream("DataChannelList.avro", FileMode.OpenOrCreate);
        dto!.Serialize(writer);
    }

    [Theory]
    [InlineData("Transport/Json/_files/TimeSeriesData.json")]
    public async Task TimeSeriesData_Json_Dto_To_Domain_And_Avro_Dto(string file)
    {
        await using var reader = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.Read);

        var package = await JsonSerializer.DeserializeTimeSeriesDataAsync(reader);
        Assert.NotNull(package);

        var domainModel = Vista.SDK.Transport.Json.TimeSeriesData.Extensions.ToDomainModel(package!);
        Assert.NotNull(domainModel);
        var dto = domainModel.ToAvroDto();

        await using var writer = new FileStream("TimeSeriesData.avro", FileMode.OpenOrCreate);
        dto.Serialize(writer);
    }

    [Theory]
    [InlineData("Transport/Avro/_files/TimeSeriesData.avro")]
    public async Task Test_TimeSeriesData_Deserialization(string file)
    {
        await using var reader = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.Read);

        var package = Serializer.DeserializeTimeSeriesData(reader);
        Assert.NotNull(package);
    }

    [Theory]
    [InlineData("Transport/Avro/_files/DataChannelList.avro")]
    public async Task Test_DataChannelList_Deserialization(string file)
    {
        await using var reader = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.Read);

        var package = Serializer.DeserializeDataChannelList(reader);
        Assert.NotNull(package);
    }

    [Theory]
    [InlineData("Transport/Avro/_files/TimeSeriesData.avro")]
    public async Task Test_TimeSeriesData_Serialization_Roundtrip(string file)
    {
        await using var reader = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.Read);

        var package = Serializer.DeserializeTimeSeriesData(reader);
        Assert.NotNull(package);

        using var serialized = new MemoryStream();
        package.Serialize(serialized);
        serialized.Position = 0;

        var deserialized = Serializer.DeserializeTimeSeriesData(serialized);

        package.Should().BeEquivalentTo(deserialized);
    }

    [Theory]
    [InlineData("Transport/Avro/_files/DataChannelList.avro")]
    public async Task Test_DataChannelList_Serialization_Roundtrip(string file)
    {
        await using var reader = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.Read);

        var package = Serializer.DeserializeDataChannelList(reader);
        Assert.NotNull(package);

        using var serialized = new MemoryStream();
        package.Serialize(serialized);
        serialized.Position = 0;

        var deserialized = Serializer.DeserializeDataChannelList(serialized);

        package.Should().BeEquivalentTo(deserialized);
    }

    [Theory]
    [InlineData("Transport/Avro/_files/DataChannelList.avro")]
    public async Task Test_DataChannelList_Domain_Model_Roundtrip(string file)
    {
        await using var reader = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.Read);

        var package = Serializer.DeserializeDataChannelList(reader);
        Assert.NotNull(package);

        var domainPackage = package!.ToDomainModel();
        var dto = domainPackage.ToAvroDto();

        dto.Should().BeEquivalentTo(package);
    }

    [Theory]
    [InlineData("Transport/Avro/_files/TimeSeriesData.avro")]
    public async Task Test_TimeSeriesData_Domain_Model_Roundtrip(string file)
    {
        await using var reader = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.Read);

        var package = Serializer.DeserializeTimeSeriesData(reader);
        Assert.NotNull(package);

        var domainPackage = package!.ToDomainModel();
        var dto = domainPackage.ToAvroDto();

        dto.Should().BeEquivalentTo(package);
    }
}
