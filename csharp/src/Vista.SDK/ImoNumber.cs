using System.Diagnostics.CodeAnalysis;

namespace Vista.SDK;

public readonly record struct ImoNumber
{
    private readonly int _value;

    public ImoNumber(int value)
    {
        if (!IsValid(value))
            throw new ArgumentException("Invalid IMO number: " + value);
        _value = value;
    }

    public ImoNumber(ReadOnlySpan<char> value)
    {
        if (!TryParse(value, out this))
            throw new ArgumentException("Invalid IMO number: " + value.ToString());
    }

    private ImoNumber(int value, bool skipValidate)
    {
        if (!skipValidate && !IsValid(value))
            throw new ArgumentException("Invalid IMO number: " + value);
        _value = value;
    }

    public static ImoNumber Parse(ReadOnlySpan<char> value)
    {
        if (!TryParse(value, out var imo))
            throw new ArgumentException("Failed to parse ImoNumber: " + value.ToString());
        return imo;
    }

    public static bool TryParse(ReadOnlySpan<char> value, out ImoNumber imoNumber)
    {
        imoNumber = default;
        var startsWithImo = value.StartsWith("IMO".AsSpan(), StringComparison.OrdinalIgnoreCase);

#if NETCOREAPP3_1_OR_GREATER
        int num = int.TryParse(startsWithImo ? value.Slice(3) : value, out var n) ? n : 0;
#else
        int num = int.TryParse(
            startsWithImo ? value.Slice(3).ToString() : value.ToString(),
            out var n
        )
          ? n
          : 0;
#endif

        if (num == 0 || !IsValid(num))
            return false;

        imoNumber = new ImoNumber(num, true);

        return true;
    }

    public static bool IsValid(int imoNumber)
    {
        // https://en.wikipedia.org/wiki/IMO_number
        // An IMO number is made of the three letters "IMO" followed by a seven-digit number.
        // This consists of a six-digit sequential unique number followed by a check digit.
        // The integrity of an IMO number can be verified using its check digit.
        // This is done by multiplying each of the first six digits by a factor
        // of 2 to 7 corresponding to their position from right to left.
        // The rightmost digit of this sum is the check digit.
        // For example, for IMO 9074729: (9×7) + (0×6) + (7×5) + (4×4) + (7×3) + (2×2) = 139
        if (imoNumber < 1000000 || imoNumber > 9999999)
            return false;

        Span<byte> digits = stackalloc byte[7];
        GetDigits(imoNumber, digits);

        var checkDigit = 0;
        for (int i = 1; i < digits.Length; i++)
            checkDigit += (i + 1) * digits[i];

        return imoNumber % 10 == checkDigit % 10;
    }

    private static void GetDigits(int number, Span<byte> digits)
    {
        var current = number;
        var index = 0;
        while (current > 0)
        {
            if (current < 10)
            {
                digits[index++] = (byte)current;
                break;
            }
            var next = current / 10;
            var digit = current - next * 10;
            digits[index++] = (byte)digit;
            current = next;
        }
    }

    public override string ToString() => $"IMO{_value}";
}
