using System.Text.Json;
using System.Text.Json.Serialization;
using Vista.SDK.Transport.Json.DataChannel;
using Vista.SDK.Transport.Json.TimeSeriesData;

namespace Vista.SDK.Transport.Json;

public static class Serializer
{
    public static JsonSerializerOptions Options =
        new() { DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull, };

    public static string Serialize(this DataChannelListPackage package) =>
        JsonSerializer.Serialize(package, Options);

    public static void Serialize(this DataChannelListPackage package, Stream stream) =>
        JsonSerializer.Serialize(stream, package, Options);

    public static Task SerializeAsync(
        this DataChannelListPackage package,
        Stream stream,
        CancellationToken cancellationToken = default
    ) => JsonSerializer.SerializeAsync(stream, package, Options, cancellationToken);

    public static string Serialize(this TimeSeriesDataPackage package) =>
        JsonSerializer.Serialize(package, Options);

    public static void Serialize(this TimeSeriesDataPackage package, Stream stream) =>
        JsonSerializer.Serialize(stream, package, Options);

    public static Task SerializeAsync(
        this TimeSeriesDataPackage package,
        Stream stream,
        CancellationToken cancellationToken = default
    ) => JsonSerializer.SerializeAsync(stream, package, Options, cancellationToken);

    public static DataChannelListPackage? DeserializeDataChannelList(string packageJson) =>
        JsonSerializer.Deserialize<DataChannelListPackage>(packageJson, Options);

    public static DataChannelListPackage? DeserializeDataChannelList(
        ReadOnlySpan<char> packageJson
    ) => JsonSerializer.Deserialize<DataChannelListPackage>(packageJson, Options);

    public static ValueTask<DataChannelListPackage?> DeserializeDataChannelListAsync(
        Stream packageJsonStream,
        CancellationToken cancellationToken = default
    ) =>
        JsonSerializer.DeserializeAsync<DataChannelListPackage>(
            packageJsonStream,
            Options,
            cancellationToken
        );

    public static DataChannelListPackage? DeserializeDataChannelList(Stream packageJsonStream) =>
        JsonSerializer.Deserialize<DataChannelListPackage>(packageJsonStream, Options);

    public static TimeSeriesDataPackage? DeserializeTimeSeriesData(string packageJson) =>
        JsonSerializer.Deserialize<TimeSeriesDataPackage>(packageJson, Options);

    public static TimeSeriesDataPackage? DeserializeTimeSeriesData(
        ReadOnlySpan<char> packageJson
    ) => JsonSerializer.Deserialize<TimeSeriesDataPackage>(packageJson, Options);

    public static ValueTask<TimeSeriesDataPackage?> DeserializeTimeSeriesDataAsync(
        Stream packageJsonStream,
        CancellationToken cancellationToken = default
    ) =>
        JsonSerializer.DeserializeAsync<TimeSeriesDataPackage>(
            packageJsonStream,
            Options,
            cancellationToken
        );

    public static TimeSeriesDataPackage? DeserializeTimeSeriesData(Stream packageJsonStream) =>
        JsonSerializer.Deserialize<TimeSeriesDataPackage>(packageJsonStream, Options);
}
