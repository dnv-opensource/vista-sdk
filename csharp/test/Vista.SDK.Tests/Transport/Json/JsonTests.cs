using System.Text.Json;
using FluentAssertions;
using FluentAssertions.Equivalency;
using ICSharpCode.SharpZipLib.BZip2;
using Json.Schema;
using Json.Schema.Serialization;
using Vista.SDK.Transport;
using Vista.SDK.Transport.Json;
using Vista.SDK.Transport.Json.DataChannel;
using Vista.SDK.Transport.Json.TimeSeriesData;
using Domain = Vista.SDK.Transport.DataChannel;

namespace Vista.SDK.Tests.Transport.Json;

public class JsonTests
{
    [Fact]
    public void Test_ISO8601_DateTimeOffset()
    {
        var pattern = DateTimeConverter.Pattern;
        var provider = DateTimeConverter.Provider;
        var styles = DateTimeConverter.Style;

        var str = "2022-04-04T20:44:31Z";

        Assert.True(DateTimeOffset.TryParseExact(str, pattern, provider, styles, out var dto));
        Assert.Equal(str, dto.ToString(pattern, provider));
    }

    [Theory]
    [InlineData("schemas/json/DataChannelList.sample.json")]
    [InlineData("schemas/json/DataChannelList.sample.compact.json")]
    public async Task Test_DataChannelList_JSONSchema_Validation(string file)
    {
        var schemaStr = await File.ReadAllTextAsync("schemas/json/DataChannelList.schema.json");
        var schema = JsonSerializer.Deserialize<JsonSchema>(schemaStr);
        Assert.NotNull(schema);

        var jsonStr = await File.ReadAllTextAsync(file);
        ValidatingJsonConverter.MapType<DataChannelListPackage>(schema);
        var options = new JsonSerializerOptions { Converters = { new ValidatingJsonConverter() } };
        var package = JsonSerializer.Deserialize<DataChannelListPackage>(jsonStr, options);
        Assert.NotNull(package);

        var jsonDoc = JsonDocument.Parse(jsonStr);
        var results = schema.Evaluate(jsonDoc);
        Assert.Empty(results.Errors ?? new Dictionary<string, string>());
    }

    [Theory]
    [InlineData("schemas/json/TimeSeriesData.sample.json")]
    public async Task Test_TimeSeriesData_JSONSchema_Validation(string file)
    {
        var schemaStr = await File.ReadAllTextAsync("schemas/json/TimeSeriesData.schema.json");
        var schema = JsonSerializer.Deserialize<JsonSchema>(schemaStr);
        Assert.NotNull(schema);

        var jsonStr = await File.ReadAllTextAsync(file);
        ValidatingJsonConverter.MapType<TimeSeriesDataPackage>(schema);
        var options = new JsonSerializerOptions { Converters = { new ValidatingJsonConverter() } };
        var package = JsonSerializer.Deserialize<TimeSeriesDataPackage>(jsonStr, options);
        Assert.NotNull(package);

        var jsonDoc = JsonDocument.Parse(jsonStr);
        var results = schema.Evaluate(jsonDoc);
        Assert.Empty(results.Errors ?? new Dictionary<string, string>());
    }

    [Theory]
    [InlineData("Transport/Json/_files/DataChannelList.json")]
    [InlineData("schemas/json/DataChannelList.sample.json")]
    [InlineData("schemas/json/DataChannelList.sample.compact.json")]
    public async Task Test_DataChannelList_File_Serialization(string file)
    {
        await using var reader = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.Read);

        var package = await Serializer.DeserializeDataChannelListAsync(reader);
        Assert.NotNull(package);

        var newFile = $"{file}.out";
        await File.WriteAllTextAsync(newFile, Serializer.Serialize(package!));
    }

    [Theory]
    [InlineData("Transport/Json/_files/TimeSeriesData.json")]
    public async Task Test_TimeSeriesData_Deserialization(string file)
    {
        await using var reader = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.Read);

        var package = await Serializer.DeserializeTimeSeriesDataAsync(reader);
        Assert.NotNull(package);
    }

    [Theory]
    [InlineData("Transport/Json/_files/DataChannelList.json")]
    [InlineData("schemas/json/DataChannelList.sample.json")]
    [InlineData("schemas/json/DataChannelList.sample.compact.json")]
    public async Task Test_DataChannelList_Deserialization(string file)
    {
        await using var reader = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.Read);

        var package = await Serializer.DeserializeDataChannelListAsync(reader);
        Assert.NotNull(package);
    }

    [Theory]
    [InlineData("Transport/Json/_files/TimeSeriesData.json")]
    public async Task Test_TimeSeriesData_Serialization_Roundtrip(string file)
    {
        await using var reader = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.Read);

        var package = await Serializer.DeserializeTimeSeriesDataAsync(reader);
        Assert.NotNull(package);

        var serialized = package!.Serialize();
        var deserialized = Serializer.DeserializeTimeSeriesData(serialized);

        package.Should().BeEquivalentTo(deserialized, TimeSeriesDataEquivalency);
    }

    [Theory]
    [InlineData("Transport/Json/_files/DataChannelList.json")]
    [InlineData("schemas/json/DataChannelList.sample.json")]
    [InlineData("schemas/json/DataChannelList.sample.compact.json")]
    public async Task Test_DataChannelList_Serialization_Roundtrip(string file)
    {
        await using var reader = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.Read);

        var package = await Serializer.DeserializeDataChannelListAsync(reader);
        Assert.NotNull(package);

        var serialized = package!.Serialize();
        var deserialized = Serializer.DeserializeDataChannelList(serialized);

        package.Should().BeEquivalentTo(deserialized, DataChannelListEquivalency);
    }

    [Theory]
    [InlineData("Transport/Json/_files/DataChannelList.json")]
    [InlineData("schemas/json/DataChannelList.sample.json")]
    // [InlineData("schemas/json/DataChannelList.sample.compact.json")]
    public async Task Test_DataChannelList_Domain_Model_Roundtrip(string file)
    {
        await using var reader = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.Read);

        var package = await Serializer.DeserializeDataChannelListAsync(reader);
        Assert.NotNull(package);

        var domainPackage = package.ToDomainModel();
        var dto = domainPackage.ToJsonDto();

        dto.Should().BeEquivalentTo(package, DataChannelListEquivalency);
    }

    [Theory]
    [InlineData("Transport/Json/_files/DataChannelList.json")]
    [InlineData("schemas/json/DataChannelList.sample.json")]
    // [InlineData("schemas/json/DataChannelList.sample.compact.json")]
    public async Task Test_DataChannelListId_Date_Consistency(string file)
    {
        await using var reader = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.Read);

        var package = await Serializer.DeserializeDataChannelListAsync(reader);
        Assert.NotNull(package);

        var domainPackage = package.ToDomainModel();
        var dto = domainPackage.ToJsonDto();

        var serialized = dto.Serialize();
        var deserialized = Serializer.DeserializeDataChannelList(serialized);
        Assert.NotNull(deserialized);
        var utf8Serialized = JsonSerializer.SerializeToUtf8Bytes(package, Serializer.Options);
        var utf8Deserialized = JsonSerializer.Deserialize<DataChannelListPackage>(utf8Serialized, Serializer.Options);
        Assert.NotNull(utf8Deserialized);

        dto.Should().BeEquivalentTo(package, DataChannelListEquivalency);

        var dtoTimeStamp = dto.Package.Header.DataChannelListID.TimeStamp;
        var domainTimeStamp = domainPackage.Package.Header.DataChannelListId.TimeStamp;
        var packageTimestamp = package.Package.Header.DataChannelListID.TimeStamp;
        var deserializedTimeStamp = deserialized.Package.Header.DataChannelListID.TimeStamp;
        var utf8DeserializedTimeStamp = utf8Deserialized.Package.Header.DataChannelListID.TimeStamp;

        Assert.Equal(dtoTimeStamp, domainTimeStamp);
        Assert.Equal(dtoTimeStamp, packageTimestamp);
        Assert.Equal(dtoTimeStamp, deserializedTimeStamp);
        Assert.Equal(dtoTimeStamp, utf8DeserializedTimeStamp);
    }

    [Theory]
    [InlineData("Transport/Json/_files/DataChannelList.json")]
    [InlineData("schemas/json/DataChannelList.sample.json")]
    [InlineData("schemas/json/DataChannelList.sample.compact.json")]
    public async Task Test_DataChannelList_Compression(string file)
    {
        await using var reader = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.Read);

        var package = await Serializer.DeserializeDataChannelListAsync(reader);
        Assert.NotNull(package);

        using var serializedStream = new MemoryStream();
        using var compressedStream = new MemoryStream();

        package!.Serialize(serializedStream);
        serializedStream.Position = 0;
        BZip2.Compress(serializedStream, compressedStream, false, 5);

        Assert.True(serializedStream.Length > compressedStream.Length);

        compressedStream.Position = 0;
        using var decompressedStream = new MemoryStream();

        BZip2.Decompress(compressedStream, decompressedStream, false);

        Assert.Equal(serializedStream.Length, decompressedStream.Length);
    }

    [Theory]
    [InlineData("Transport/Json/_files/TimeSeriesData.json")]
    public async Task Test_TimeSeriesData_Domain_Model_Roundtrip(string file)
    {
        await using var reader = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.Read);

        var package = await Serializer.DeserializeTimeSeriesDataAsync(reader);
        Assert.NotNull(package);

        var domainPackage = package!.ToDomainModel();
        var dto = domainPackage.ToJsonDto();

        dto.Should().BeEquivalentTo(package, TimeSeriesDataEquivalency);
    }

    private sealed class JsonElementComparer : IEqualityComparer<JsonElement>
    {
        public bool Equals(JsonElement x, JsonElement y) =>
            EqualityComparer<string>.Default.Equals(x.GetString(), y.GetString());

        public int GetHashCode(JsonElement obj) => EqualityComparer<string>.Default.GetHashCode(obj.GetString()!);
    }

    private static readonly Func<
        EquivalencyAssertionOptions<DataChannelListPackage?>,
        EquivalencyAssertionOptions<DataChannelListPackage?>
    > DataChannelListEquivalency = opt => opt.Using<JsonElement, JsonElementComparer>();

    private static readonly Func<
        EquivalencyAssertionOptions<TimeSeriesDataPackage?>,
        EquivalencyAssertionOptions<TimeSeriesDataPackage?>
    > TimeSeriesDataEquivalency = opt => opt.Using<JsonElement, JsonElementComparer>();
}
