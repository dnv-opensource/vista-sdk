namespace Vista.SDK.Experimental.Transport;

public readonly record struct AssetIdentifier
{
    private readonly int _tag;
    private readonly ImoNumber _imoNumber;
    private readonly string? _otherId;

    public readonly bool IsImoNumber => _tag == 1;
    public readonly bool IsOtherId => _tag == 2;

    public readonly ImoNumber? ImoNumber => _tag == 1 ? _imoNumber : null;

    public readonly string? OtherId => _tag == 2 ? _otherId : null;

    private AssetIdentifier(ImoNumber value)
    {
        _tag = 1;
        _imoNumber = value;
        _otherId = null;
    }

    private AssetIdentifier(string value)
    {
        _tag = 2;
        _imoNumber = default;
        _otherId = value;
    }

    public readonly T Match<T>(Func<ImoNumber, T> onImoNumber, Func<string, T> onOtherId)
    {
        return _tag switch
        {
            1 => onImoNumber(_imoNumber),
            2 => onOtherId(_otherId!),
            _ => throw new InvalidOperationException("Tried to match on invalid ShipId"),
        };
    }

    public readonly void Switch(Action<ImoNumber> onImoNumber, Action<string> onOtherId)
    {
        if (_tag == 1)
            onImoNumber(_imoNumber);
        else if (_tag == 2)
            onOtherId(_otherId!);
        else
            throw new InvalidOperationException("Tried to switch on invalid ShipId");
    }

    public override readonly string ToString() =>
        _tag switch
        {
            // In ISO-19848, IMO number as ShipID should be prefixed with "IMO"
            1 => _imoNumber.ToString(),
            2 => _otherId!,
            _ => throw new Exception("Invalid state exception"),
        };

    public static AssetIdentifier Parse(ReadOnlySpan<char> value)
    {
        if (value.IsEmpty)
            throw new ArgumentNullException(nameof(value));

        // In ISO-19848, IMO number as ShipID should be prefixed with "IMO"
        if (
            value.StartsWith("IMO".AsSpan(), StringComparison.OrdinalIgnoreCase)
            && SDK.ImoNumber.TryParse(value, out var imo)
        )
            return new AssetIdentifier(imo);
        else
            return new AssetIdentifier(value.ToString());
    }

    public static AssetIdentifier Parse(string value)
    {
        if (value is null)
            throw new ArgumentNullException(nameof(value));

        var span = value.AsSpan();
        // In ISO-19848, IMO number as ShipID should be prefixed with "IMO"
        if (
            span.StartsWith("IMO".AsSpan(), StringComparison.OrdinalIgnoreCase)
            && SDK.ImoNumber.TryParse(span, out var imo)
        )
            return new AssetIdentifier(imo);
        else
            return new AssetIdentifier(value);
    }

    public static implicit operator AssetIdentifier(ImoNumber id) => new AssetIdentifier(id);
}
