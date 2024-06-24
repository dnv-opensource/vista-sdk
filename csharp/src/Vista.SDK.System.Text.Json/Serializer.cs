using System.Diagnostics;
using System.Globalization;
using System.Text.Json;
using System.Text.Json.Serialization;
using Vista.SDK.Transport.Json.DataChannel;
using Vista.SDK.Transport.Json.TimeSeriesData;

namespace Vista.SDK.Transport.Json;

public class DateTimeConverter : JsonConverter<DateTime>
{
    public static readonly string Pattern = "yyyy-MM-ddTHH:mm:ssZ";
    public static readonly IFormatProvider Provider = CultureInfo.InvariantCulture.DateTimeFormat;
    public static readonly DateTimeStyles Style = DateTimeStyles.None;

    public override DateTime Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
    {
        Debug.Assert(typeToConvert == typeof(DateTime));
        return DateTime.ParseExact(reader.GetString() ?? string.Empty, Pattern, Provider, Style);
    }

    public override void Write(Utf8JsonWriter writer, DateTime value, JsonSerializerOptions options)
    {
        writer.WriteStringValue(value.ToString(Pattern, CultureInfo.InvariantCulture));
    }
}

public class DatetimeOffsetConverter : JsonConverter<DateTimeOffset>
{
    public static readonly string Pattern = DateTimeConverter.Pattern;
    public static readonly IFormatProvider Provider = DateTimeConverter.Provider;
    public static readonly DateTimeStyles Style = DateTimeConverter.Style;

    public override DateTimeOffset Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
    {
        Debug.Assert(typeToConvert == typeof(DateTimeOffset));
        return DateTimeOffset.ParseExact(reader.GetString() ?? string.Empty, Pattern, Provider, Style);
    }

    public override void Write(Utf8JsonWriter writer, DateTimeOffset value, JsonSerializerOptions options)
    {
        writer.WriteStringValue(value.ToString(Pattern, CultureInfo.InvariantCulture));
    }
}

public static class Serializer
{
    public static readonly JsonSerializerOptions Options =
        new()
        {
            DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull,
            Converters = { new DateTimeConverter(), new DatetimeOffsetConverter() }
        };

    public static string Serialize(this DataChannelListPackage package) => JsonSerializer.Serialize(package, Options);

    public static void Serialize(this DataChannelListPackage package, Stream stream) =>
        JsonSerializer.Serialize(stream, package, Options);

    public static Task SerializeAsync(
        this DataChannelListPackage package,
        Stream stream,
        CancellationToken cancellationToken = default
    ) => JsonSerializer.SerializeAsync(stream, package, Options, cancellationToken);

    public static string Serialize(this TimeSeriesDataPackage package) => JsonSerializer.Serialize(package, Options);

    public static void Serialize(this TimeSeriesDataPackage package, Stream stream) =>
        JsonSerializer.Serialize(stream, package, Options);

    public static Task SerializeAsync(
        this TimeSeriesDataPackage package,
        Stream stream,
        CancellationToken cancellationToken = default
    ) => JsonSerializer.SerializeAsync(stream, package, Options, cancellationToken);

    public static DataChannelListPackage? DeserializeDataChannelList(string packageJson) =>
        JsonSerializer.Deserialize<DataChannelListPackage>(packageJson, Options);

    public static DataChannelListPackage? DeserializeDataChannelList(ReadOnlySpan<char> packageJson) =>
        JsonSerializer.Deserialize<DataChannelListPackage>(packageJson, Options);

    public static ValueTask<DataChannelListPackage?> DeserializeDataChannelListAsync(
        Stream packageJsonStream,
        CancellationToken cancellationToken = default
    ) => JsonSerializer.DeserializeAsync<DataChannelListPackage>(packageJsonStream, Options, cancellationToken);

    public static DataChannelListPackage? DeserializeDataChannelList(Stream packageJsonStream) =>
        JsonSerializer.Deserialize<DataChannelListPackage>(packageJsonStream, Options);

    public static TimeSeriesDataPackage? DeserializeTimeSeriesData(string packageJson) =>
        JsonSerializer.Deserialize<TimeSeriesDataPackage>(packageJson, Options);

    public static TimeSeriesDataPackage? DeserializeTimeSeriesData(ReadOnlySpan<char> packageJson) =>
        JsonSerializer.Deserialize<TimeSeriesDataPackage>(packageJson, Options);

    public static ValueTask<TimeSeriesDataPackage?> DeserializeTimeSeriesDataAsync(
        Stream packageJsonStream,
        CancellationToken cancellationToken = default
    ) => JsonSerializer.DeserializeAsync<TimeSeriesDataPackage>(packageJsonStream, Options, cancellationToken);

    public static TimeSeriesDataPackage? DeserializeTimeSeriesData(Stream packageJsonStream) =>
        JsonSerializer.Deserialize<TimeSeriesDataPackage>(packageJsonStream, Options);
}
