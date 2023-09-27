namespace Vista.SDK.Transport;

public readonly record struct DataChannelId
{
    private readonly int _tag;
    private readonly LocalId? _localId;
    private readonly string? _shortId;

    public readonly bool IsLocalId => _tag == 1;
    public readonly bool IsShortId => _tag == 2;

    public readonly LocalId? LocalId => _tag == 1 ? _localId : null;

    public readonly string? ShortId => _tag == 2 ? _shortId : null;

    private DataChannelId(LocalId value)
    {
        _tag = 1;
        _localId = value;
        _shortId = null;
    }

    private DataChannelId(string value)
    {
        _tag = 2;
        _localId = null;
        _shortId = value;
    }

    public readonly T Match<T>(Func<LocalId, T> onLocalId, Func<string, T> onShortId)
    {
        return _tag switch
        {
            1 => onLocalId(_localId!),
            2 => onShortId(_shortId!),
            _ => throw new InvalidOperationException("Tried to match on invalid DataChannelId"),
        };
    }

    public readonly void Switch(Action<LocalId> onLocalId, Action<string> onShortId)
    {
        if (_tag == 1)
            onLocalId(_localId!);
        else if (_tag == 2)
            onShortId(_shortId!);
        else
            throw new InvalidOperationException("Tried to switch on invalid DataChannelId");
    }

    public override readonly string ToString() =>
        _tag switch
        {
            1 => _localId!.ToString(),
            2 => _shortId!,
            _ => throw new Exception("Invalid state exception"),
        };

    public static DataChannelId Parse(string value)
    {
        if (value is null)
            throw new ArgumentNullException(nameof(value));

        if (LocalIdBuilder.TryParse(value, out var localIdBuilder))
            return new DataChannelId(localIdBuilder.Build());
        else
            return new DataChannelId(value);
    }

    public static implicit operator DataChannelId(LocalId id) => new DataChannelId(id);
}
