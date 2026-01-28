using System.Diagnostics;
using System.Globalization;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Text.RegularExpressions;
using Vista.SDK.Transport.Json.DataChannel;
using Vista.SDK.Transport.Json.TimeSeriesData;
using DcDomain = Vista.SDK.Transport.DataChannel;
using TsDomain = Vista.SDK.Transport.TimeSeries;

namespace Vista.SDK.Transport.Json;

public class DateTimeConverter : JsonConverter<DateTime>
{
    public static readonly IFormatProvider Provider = CultureInfo.InvariantCulture.DateTimeFormat;
    public static readonly DateTimeStyles Style = DateTimeStyles.None;

    public override DateTime Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
    {
        Debug.Assert(typeToConvert == typeof(DateTime));
        var value = reader.GetString() ?? string.Empty;

        if (!DatetimeOffsetConverter.Iso8601StrictRegex.IsMatch(value))
            throw new FormatException($"Invalid ISO 8601-1 format: '{value}'");

        return DateTimeOffset.Parse(value, CultureInfo.InvariantCulture, DateTimeStyles.RoundtripKind).DateTime;
    }

    public override void Write(Utf8JsonWriter writer, DateTime value, JsonSerializerOptions options)
    {
        writer.WriteStringValue(value.ToString("o", CultureInfo.InvariantCulture));
    }
}

public class DatetimeOffsetConverter : JsonConverter<DateTimeOffset>
{
    internal static readonly Regex Iso8601StrictRegex = new Regex(
        @"^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(\.\d+)?(Z|[+-]\d{2}:\d{2})$",
        RegexOptions.Compiled | RegexOptions.CultureInvariant
    );
    public static readonly IFormatProvider Provider = DateTimeConverter.Provider;
    public static readonly DateTimeStyles Style = DateTimeConverter.Style;

    public override DateTimeOffset Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
    {
        Debug.Assert(typeToConvert == typeof(DateTimeOffset));
        var value = reader.GetString() ?? string.Empty;

        if (!Iso8601StrictRegex.IsMatch(value))
            throw new FormatException($"Invalid ISO 8601-1 format: '{value}'");

        return DateTimeOffset.Parse(value, CultureInfo.InvariantCulture, DateTimeStyles.RoundtripKind);
    }

    public override void Write(Utf8JsonWriter writer, DateTimeOffset value, JsonSerializerOptions options)
    {
        writer.WriteStringValue(value.ToString("o", CultureInfo.InvariantCulture));
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

    public static string Serialize(this DcDomain.DataChannelListPackage package) => Serialize(package.ToJsonDto());

    public static void Serialize(this DataChannelListPackage package, Stream stream) =>
        JsonSerializer.Serialize(stream, package, Options);

    public static void Serialize(this DcDomain.DataChannelListPackage package, Stream stream) =>
        Serialize(package.ToJsonDto(), stream);

    public static Task SerializeAsync(
        this DataChannelListPackage package,
        Stream stream,
        CancellationToken cancellationToken = default
    ) => JsonSerializer.SerializeAsync(stream, package, Options, cancellationToken);

    public static Task SerializeAsync(
        this DcDomain.DataChannelListPackage package,
        Stream stream,
        CancellationToken cancellationToken = default
    ) => SerializeAsync(package.ToJsonDto(), stream, cancellationToken);

    public static string Serialize(this TimeSeriesDataPackage package) => JsonSerializer.Serialize(package, Options);

    public static string Serialize(this TsDomain.TimeSeriesDataPackage package) => Serialize(package.ToJsonDto());

    public static void Serialize(this TimeSeriesDataPackage package, Stream stream) =>
        JsonSerializer.Serialize(stream, package, Options);

    public static Task SerializeAsync(
        this TimeSeriesDataPackage package,
        Stream stream,
        CancellationToken cancellationToken = default
    ) => JsonSerializer.SerializeAsync(stream, package, Options, cancellationToken);

    public static Task SerializeAsync(
        this TsDomain.TimeSeriesDataPackage package,
        Stream stream,
        CancellationToken cancellationToken = default
    ) => SerializeAsync(package.ToJsonDto(), stream, cancellationToken);

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
