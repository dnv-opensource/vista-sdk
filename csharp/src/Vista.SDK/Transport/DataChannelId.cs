namespace Vista.SDK.Transport;

public readonly record struct DataChannelId
{
    private readonly int _tag;
    private readonly LocalIdBuilder? _localId;
    private readonly ShortId _shortId;

    public DataChannelId(LocalIdBuilder value)
    {
        _tag = 1;
        _localId = value;
        _shortId = default;
    }

    public DataChannelId(ShortId value)
    {
        _tag = 2;
        _localId = default;
        _shortId = value;
    }

    public override string ToString() =>
        _tag switch
        {
            1 => _localId!.ToString(),
            2 => _shortId.ToString(),
            _ => throw new Exception("Invalid state exception"),
        };
}
