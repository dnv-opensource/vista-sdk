using System.Collections;
using System.Globalization;
using Microsoft.Extensions.Caching.Memory;

namespace Vista.SDK.Transport;

public enum ISO19848Version
{
    v2018,
    v2024
}

public interface IISO19848
{
    DataChannelTypeNames GetDataChannelTypeNames(ISO19848Version version);
    FormatDataTypes GetFormatDataTypes(ISO19848Version version);
}

public sealed class ISO19848 : IISO19848
{
    public static readonly ISO19848Version LatestVersion = ISO19848Version.v2024;
    public static readonly ISO19848 Instance = new ISO19848();
    private readonly MemoryCache _dataChannelTypeNamesDtoCache;
    private readonly MemoryCache _dataChannelTypeNamesCache;

    private readonly MemoryCache _formatDataTypesDtoCache;
    private readonly MemoryCache _formatDataTypesCache;

    private ISO19848()
    {
        _dataChannelTypeNamesDtoCache = new MemoryCache(
            new MemoryCacheOptions { SizeLimit = 10, ExpirationScanFrequency = TimeSpan.FromHours(1), }
        );
        _dataChannelTypeNamesCache = new MemoryCache(
            new MemoryCacheOptions { SizeLimit = 10, ExpirationScanFrequency = TimeSpan.FromHours(1), }
        );

        _formatDataTypesDtoCache = new MemoryCache(
            new MemoryCacheOptions { SizeLimit = 10, ExpirationScanFrequency = TimeSpan.FromHours(1), }
        );
        _formatDataTypesCache = new MemoryCache(
            new MemoryCacheOptions { SizeLimit = 10, ExpirationScanFrequency = TimeSpan.FromHours(1), }
        );
    }

    public DataChannelTypeNames GetDataChannelTypeNames(ISO19848Version version)
    {
        return _dataChannelTypeNamesCache.GetOrCreate(
            version,
            entry =>
            {
                entry.Size = 1;
                entry.SlidingExpiration = TimeSpan.FromHours(1);

                var dto = GetDataChannelTypeNamesDto(version);

                return new DataChannelTypeNames(
                    dto.Values.Select(x => new DataChannelTypeName(x.Type, x.Description)).ToList()
                );
            }
        )!;
    }

    internal DataChannelTypeNamesDto GetDataChannelTypeNamesDto(ISO19848Version version)
    {
        return _dataChannelTypeNamesDtoCache.GetOrCreate(
            version,
            entry =>
            {
                entry.Size = 1;
                entry.SlidingExpiration = TimeSpan.FromHours(1);

                var dto = LoadDataChannelTypeNamesDto(version);
                if (dto is null)
                    throw new Exception("Invalid state");

                return dto;
            }
        )!;
    }

    private static DataChannelTypeNamesDto? LoadDataChannelTypeNamesDto(ISO19848Version version) =>
        EmbeddedResource.GetDataChannelTypeNames(
            version switch
            {
                ISO19848Version.v2018 => "v2018",
                ISO19848Version.v2024 => "v2024",
                _ => throw new ArgumentOutOfRangeException(nameof(version), version, null),
            }
        );

    public FormatDataTypes GetFormatDataTypes(ISO19848Version version)
    {
        return _formatDataTypesCache.GetOrCreate(
            version,
            entry =>
            {
                entry.Size = 1;
                entry.SlidingExpiration = TimeSpan.FromHours(1);

                var dto = GetFormatDataTypesDto(version);

                return new FormatDataTypes(dto.Values.Select(x => new FormatDataType(x.Type, x.Description)).ToList());
            }
        )!;
    }

    internal FormatDataTypesDto GetFormatDataTypesDto(ISO19848Version version)
    {
        return _formatDataTypesDtoCache.GetOrCreate(
            version,
            entry =>
            {
                entry.Size = 1;
                entry.SlidingExpiration = TimeSpan.FromHours(1);

                var dto = LoadFormatDataTypesDto(version);
                if (dto is null)
                    throw new Exception("Invalid state");

                return dto;
            }
        )!;
    }

    private static FormatDataTypesDto? LoadFormatDataTypesDto(ISO19848Version version) =>
        EmbeddedResource.GetFormatDataTypes(
            version switch
            {
                ISO19848Version.v2018 => "v2018",
                ISO19848Version.v2024 => "v2024",
                _ => throw new ArgumentOutOfRangeException(nameof(version), version, null),
            }
        );
}

public sealed record DataChannelTypeName(string Type, string Description);

public sealed record DataChannelTypeNames : IEnumerable<DataChannelTypeName>
{
    private readonly IReadOnlyList<DataChannelTypeName> _values;

    public DataChannelTypeNames(IReadOnlyList<DataChannelTypeName> values) => _values = values;

    public ParseResult Parse(string type)
    {
        var typeName = _values.FirstOrDefault(x => x.Type == type);
        if (typeName is null)
            return new ParseResult.Invalid();
        return new ParseResult.Ok(typeName);
    }

    public IEnumerator<DataChannelTypeName> GetEnumerator() => _values.GetEnumerator();

    IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

    public abstract record ParseResult
    {
        private ParseResult() { }

        public sealed record Ok(DataChannelTypeName TypeName) : ParseResult;

        public sealed record Invalid : ParseResult;
    }
}

public sealed record FormatDataType(string Type, string Description)
{
    public void Switch(
        string value,
        Action<decimal> onDecimal,
        Action<int> onInteger,
        Action<bool> onBoolean,
        Action<string> onString,
        Action<DateTimeOffset> onDateTime
    )
    {
        var result = Validate(value, out var obj);

        switch (result)
        {
            case ValidateResult.Ok:
                switch (obj)
                {
                    case Value.Decimal d:
                        onDecimal(d.Value);
                        break;
                    case Value.Integer i:
                        onInteger(i.Value);
                        break;
                    case Value.Boolean b:
                        onBoolean(b.Value);
                        break;
                    case Value.String s:
                        onString(s.Value);
                        break;
                    case Value.DateTime dt:
                        onDateTime(dt.Value);
                        break;
                    default:
                        throw new Exception("Invalid type");
                }
                break;
            case ValidateResult.Invalid:
                throw new ValidationException("Invalid value");
            default:
                throw new Exception("Invalid result");
        }
    }

    public T Match<T>(
        string value,
        Func<decimal, T> onDecimal,
        Func<int, T> onInteger,
        Func<bool, T> onBoolean,
        Func<string, T> onString,
        Func<DateTimeOffset, T> onDateTime
    )
    {
        T? result = default;
        Switch(
            value,
            d =>
            {
                result = onDecimal(d);
            },
            i =>
            {
                result = onInteger(i);
            },
            b =>
            {
                result = onBoolean(b);
            },
            s =>
            {
                result = onString(s);
            },
            dt =>
            {
                result = onDateTime(dt);
            }
        );
        if (result is null)
            throw new ValidationException("Invalid value");
        return result;
    }

    public ValidateResult Validate(string value, out Value outValue)
    {
        outValue = new Value.String(value);
        switch (Type)
        {
            case "Decimal":
                if (!decimal.TryParse(value, out var d))
                    return new ValidateResult.Invalid([$"Invalid decimal value - Value='{value}'"]);
                outValue = new Value.Decimal(d);
                return new ValidateResult.Ok();
            case "Integer":
                if (!int.TryParse(value, out var i))
                    return new ValidateResult.Invalid([$"Invalid integer value - Value='{value}'"]);
                outValue = new Value.Integer(i);
                return new ValidateResult.Ok();
            case "Boolean":
                if (!bool.TryParse(value, out var b))
                    return new ValidateResult.Invalid([$"Invalid boolean value - Value='{value}'"]);
                outValue = new Value.Boolean(b);
                return new ValidateResult.Ok();
            case "String":
                return new ValidateResult.Ok();
            case "DateTime":
                var pattern = "yyyy-MM-ddTHH:mm:ssZ";
                if (
                    !DateTimeOffset.TryParseExact(
                        value,
                        pattern,
                        CultureInfo.InvariantCulture,
                        DateTimeStyles.None,
                        out var dt
                    )
                )
                    return new ValidateResult.Invalid([$"Invalid datetime value - Value='{value}'"]);
                outValue = new Value.DateTime(dt);
                return new ValidateResult.Ok();
            default:
                throw new Exception($"Invalid format type {Type}");
        }
    }
}

public sealed record FormatDataTypes : IEnumerable<FormatDataType>
{
    private readonly IReadOnlyList<FormatDataType> _values;

    public FormatDataTypes(IReadOnlyList<FormatDataType> values) => _values = values;

    public ParseResult Parse(string type)
    {
        var typeName = _values.FirstOrDefault(x => x.Type == type);
        if (typeName is null)
            return new ParseResult.Invalid();
        return new ParseResult.Ok(typeName);
    }

    public IEnumerator<FormatDataType> GetEnumerator() => _values.GetEnumerator();

    IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

    public abstract record ParseResult
    {
        private ParseResult() { }

        public sealed record Ok(FormatDataType TypeName) : ParseResult;

        public sealed record Invalid : ParseResult;
    }
}

// Custom definitions
public abstract record Value
{
    private Value() { }

    public sealed record Decimal(decimal Value) : Value;

    public sealed record Integer(int Value) : Value;

    public sealed record Boolean(bool Value) : Value;

    public sealed record String(string Value) : Value;

    public sealed record DateTime(DateTimeOffset Value) : Value;
}
