﻿using System.Text.Json;

using FluentAssertions;
using FluentAssertions.Equivalency;
using ICSharpCode.SharpZipLib.BZip2;
using Vista.SDK.Transport.Json.DataChannel;
using Vista.SDK.Transport.Json.TimeSeriesData;

namespace Vista.SDK.Tests.Transport.Json;

public class JsonTests
{
    [Theory]
    [InlineData("Transport/Json/_files/TimeSeriesData.json")]
    public async Task Test_TimeSeriesData_Deserialization(string file)
    {
        await using var reader = new FileStream(
            file,
            FileMode.Open,
            FileAccess.Read,
            FileShare.Read
        );

        var package = await JsonSerializer.DeserializeAsync<TimeSeriesDataPackage>(reader);
        Assert.NotNull(package);
    }

    [Theory]
    [InlineData("Transport/Json/_files/DataChannelList.json")]
    public async Task Test_DataChannelList_Deserialization(string file)
    {
        await using var reader = new FileStream(
            file,
            FileMode.Open,
            FileAccess.Read,
            FileShare.Read
        );

        var package = await JsonSerializer.DeserializeAsync<DataChannelListPackage>(reader);
        Assert.NotNull(package);
    }

    [Theory]
    [InlineData("Transport/Json/_files/TimeSeriesData.json")]
    public async Task Test_TimeSeriesData_Serialization_Roundtrip(string file)
    {
        await using var reader = new FileStream(
            file,
            FileMode.Open,
            FileAccess.Read,
            FileShare.Read
        );

        var package = await JsonSerializer.DeserializeAsync<TimeSeriesDataPackage>(reader);
        Assert.NotNull(package);

        var serialized = JsonSerializer.Serialize(package);
        var deserialized = JsonSerializer.Deserialize<TimeSeriesDataPackage>(serialized);

        package.Should().BeEquivalentTo(deserialized, TimeSeriesDataEquivalency);
    }

    [Theory]
    [InlineData("Transport/Json/_files/DataChannelList.json")]
    public async Task Test_DataChannelList_Serialization_Roundtrip(string file)
    {
        await using var reader = new FileStream(
            file,
            FileMode.Open,
            FileAccess.Read,
            FileShare.Read
        );

        var package = await JsonSerializer.DeserializeAsync<DataChannelListPackage>(reader);
        Assert.NotNull(package);

        var serialized = JsonSerializer.Serialize(package);
        var deserialized = JsonSerializer.Deserialize<DataChannelListPackage>(serialized);

        package.Should().BeEquivalentTo(deserialized, DataChannelListEquivalency);
    }

    [Theory]
    [InlineData("Transport/Json/_files/DataChannelList.json")]
    public async Task Test_DataChannelList_Domain_Model_Roundtrip(string file)
    {
        await using var reader = new FileStream(
            file,
            FileMode.Open,
            FileAccess.Read,
            FileShare.Read
        );

        var package = await JsonSerializer.DeserializeAsync<DataChannelListPackage>(reader);
        Assert.NotNull(package);

        var domainPackage = package!.ToDomainModel();
        var dto = domainPackage.ToJsonDto();

        dto.Should().BeEquivalentTo(package, DataChannelListEquivalency);
    }

    [Theory]
    [InlineData("Transport/Json/_files/DataChannelList.json")]
    public async Task Test_DataChannelList_Compression(string file)
    {
        await using var reader = new FileStream(
            file,
            FileMode.Open,
            FileAccess.Read,
            FileShare.Read
        );

        var package = await JsonSerializer.DeserializeAsync<DataChannelListPackage>(reader);
        Assert.NotNull(package);

        using var serializedStream = new MemoryStream();
        using var compressedStream = new MemoryStream();

        JsonSerializer.Serialize(serializedStream, package);
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
        await using var reader = new FileStream(
            file,
            FileMode.Open,
            FileAccess.Read,
            FileShare.Read
        );

        var package = await JsonSerializer.DeserializeAsync<TimeSeriesDataPackage>(reader);
        Assert.NotNull(package);

        var domainPackage = package!.ToDomainModel();
        var dto = domainPackage.ToJsonDto();

        dto.Should().BeEquivalentTo(package, TimeSeriesDataEquivalency);
    }

    private sealed class JsonElementComparer : IEqualityComparer<JsonElement>
    {
        public bool Equals(JsonElement x, JsonElement y) =>
            EqualityComparer<string>.Default.Equals(x.GetString(), y.GetString());

        public int GetHashCode(JsonElement obj) =>
            EqualityComparer<string>.Default.GetHashCode(obj.GetString()!);
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
