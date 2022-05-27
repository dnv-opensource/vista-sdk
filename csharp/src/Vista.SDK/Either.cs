using System.Runtime.CompilerServices;

namespace Vista.SDK;

public sealed record Either<T1, T2>
    where T1 : class
    where T2 : class
{
    private readonly int _tag;
    private readonly object _value;

    private Either(object value, int tag)
    {
        _tag = tag;
        _value = value;
    }

    public Either(T1 value) : this(value, 1) { }

    public Either(T2 value) : this(value, 2) { }

    public T Match<T>(Func<T1, T> match1, Func<T2, T> match2) =>
        _tag switch
        {
            1 => match1(Unsafe.As<T1>(_value)),
            2 => match2(Unsafe.As<T2>(_value)),
            _ => throw new Exception("Invalid state in either"),
        };

    public static implicit operator Either<T1, T2>(T1 value) => new Either<T1, T2>(value);

    public static implicit operator Either<T1, T2>(T2 value) => new Either<T1, T2>(value);
}
