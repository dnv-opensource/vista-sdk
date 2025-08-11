using System.Runtime.CompilerServices;
using System.Text;
using Vista.SDK;

public sealed record LocationBuilder
{
    public int? Number { get; private init; }
    public char? Side { get; private init; }
    public char? Vertical { get; private init; }
    public char? Transverse { get; private init; }
    public char? Longitudinal { get; private init; }

    public VisVersion VisVersion { get; }

    private IReadOnlyDictionary<char, LocationGroup> _reversedGroups;

    private LocationBuilder(VisVersion visVersion, IReadOnlyDictionary<char, LocationGroup> reversedGroups)
    {
        VisVersion = visVersion;

        _reversedGroups = reversedGroups;
    }

    public static LocationBuilder Create(Locations locations) =>
        new LocationBuilder(locations.VisVersion, locations._reversedGroups);

    public LocationBuilder WithLocation(Location value)
    {
        var builder = this with { };

        var span = value.ToString().AsSpan();

        int? n = null;

        for (var i = 0; i < span.Length; i++)
        {
            ref readonly var ch = ref span[i];
            if (char.IsDigit(ch))
            {
                if (n is null)
                    n = ch - '0';
                else
                {
                    if (!Locations.TryParseInt(span, 0, i + 1, out var num))
                        throw new ArgumentException("Should include a valid number");
                    n = num;
                }
                continue;
            }

            builder = builder.WithValue(ch);
        }

        if (n is not null)
            builder = builder.WithNumber(n.Value);

        return builder;
    }

    public LocationBuilder WithNumber(int number) => WithValueInternal(LocationGroup.Number, number);

    public LocationBuilder WithSide(char side) => WithValueInternal(LocationGroup.Side, side);

    public LocationBuilder WithVertical(char vertical) => WithValueInternal(LocationGroup.Vertical, vertical);

    public LocationBuilder WithTransverse(char transverse) => WithValueInternal(LocationGroup.Transverse, transverse);

    public LocationBuilder WithLongitudinal(char longitudinal) =>
        WithValueInternal(LocationGroup.Longitudinal, longitudinal);

    public LocationBuilder WithValue(int value)
    {
        return WithValueInternal(LocationGroup.Number, value);
    }

    public LocationBuilder WithValue(char value)
    {
        if (!_reversedGroups.TryGetValue(value, out var key))
            throw new ValidationException($"The value {value} is an invalid Locations value");
        return WithValueInternal(key, value);
    }

    private LocationBuilder WithValueInternal<T>(LocationGroup group, T value)
        where T : struct
    {
        if (group == LocationGroup.Number)
        {
            if (typeof(T) != typeof(int))
                throw new ValidationException("Value should be number");
            ref var n = ref Unsafe.As<T, int>(ref value);
            if (n < 1)
                throw new ValidationException("Value should be greater than 0");
            return this with { Number = n };
        }

        if (typeof(T) != typeof(char))
            throw new ValidationException("Value should be a character");
        ref var val = ref Unsafe.As<T, char>(ref value);

        if (!_reversedGroups.TryGetValue(val, out var key) || key != group)
            throw new ValidationException(
                $"The value {value} is an invalid {Enum.GetName(typeof(LocationGroup), group)} value"
            );

        return group switch
        {
            LocationGroup.Side => this with { Side = val },
            LocationGroup.Vertical => this with { Vertical = val },
            LocationGroup.Transverse => this with { Transverse = val },
            LocationGroup.Longitudinal => this with { Longitudinal = val },
            _ => throw new Exception($"Unsupported code: {group}"),
        };
    }

    public LocationBuilder WithoutValue(LocationGroup group)
    {
        return group switch
        {
            LocationGroup.Number => this with { Number = null },
            LocationGroup.Side => this with { Side = null },
            LocationGroup.Vertical => this with { Vertical = null },
            LocationGroup.Transverse => this with { Transverse = null },
            LocationGroup.Longitudinal => this with { Longitudinal = null },
            _ => throw new Exception($"Unsupported code: {group}"),
        };
    }

    public LocationBuilder WithoutNumber() => WithoutValue(LocationGroup.Number);

    public LocationBuilder WithoutSide() => WithoutValue(LocationGroup.Side);

    public LocationBuilder WithoutVertical() => WithoutValue(LocationGroup.Vertical);

    public LocationBuilder WithoutTransverse() => WithoutValue(LocationGroup.Transverse);

    public LocationBuilder WithoutLongitudinal() => WithoutValue(LocationGroup.Longitudinal);

    public Location Build()
    {
        return new Location(ToString());
    }

    public override string ToString()
    {
        var items = new char?[] { Side, Vertical, Transverse, Longitudinal };

        var str = new StringBuilder();

        if (Number is not null)
            str.Append(Number.ToString());

        foreach (var item in items)
        {
            if (item is null)
                continue;

            str.Append(item);
        }

        var parts = str.ToString().OrderBy(c => c).Select(s => s.ToString());

        return string.Concat(parts);
    }
}
