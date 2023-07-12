namespace Vista.SDK.Experimental.Transport;

public readonly record struct DataId : IEquatable<DataId>
{
    private readonly int _tag;
    private readonly LocalId? _localId;
    private readonly PMSLocalId? _pmsLocalId;
    private readonly string? _shortId;

    public readonly bool IsLocalId => _tag == 1;
    public readonly bool IsPmsLocalId => _tag == 2;
    public readonly bool IsShortId => _tag == 3;

    public readonly LocalId? LocalId => _tag == 1 ? _localId : null;
    public readonly PMSLocalId? PMSLocalId => _tag == 2 ? _pmsLocalId : null;

    public readonly string? ShortId => _tag == 3 ? _shortId : null;

    public DataId(LocalId value)
    {
        _tag = 1;
        _localId = value;
        _pmsLocalId = null;
        _shortId = null;
    }

    public DataId(PMSLocalId value)
    {
        _tag = 2;
        _localId = null;
        _pmsLocalId = value;
        _shortId = null;
    }

    public DataId(string value)
    {
        _tag = 3;
        _localId = null;
        _pmsLocalId = null;
        _shortId = value;
    }

    public readonly T Match<T>(
        Func<LocalId, T> onLocalId,
        Func<PMSLocalId, T> onPmsLocalId,
        Func<string, T> onShortId
    )
    {
        return _tag switch
        {
            1 => onLocalId(_localId!),
            2 => onPmsLocalId(_pmsLocalId!),
            3 => onShortId(_shortId!),
            _ => throw new InvalidOperationException("Tried to match on invalid DataChannelId"),
        };
    }

    public readonly void Switch(
        Action<LocalId> onLocalId,
        Action<PMSLocalId> onPmsLocalId,
        Action<string> onShortId
    )
    {
        if (_tag == 1)
            onLocalId(_localId!);
        else if (_tag == 2)
            onPmsLocalId(_pmsLocalId!);
        else if (_tag == 3)
            onShortId(_shortId!);
        else
            throw new InvalidOperationException("Tried to switch on invalid DataChannelId");
    }

    public override readonly string ToString() =>
        _tag switch
        {
            1 => _localId!.ToString(),
            2 => _pmsLocalId!.ToString(),
            3 => _shortId!,
            _ => throw new Exception("Invalid state exception"),
        };

    public static DataId Parse(string value)
    {
        if (value is null)
            throw new ArgumentNullException(nameof(value));
        if (LocalIdBuilder.TryParse(value, out var localIdBuilder))
            return new DataId(localIdBuilder.Build());
        else if (PMSLocalIdBuilder.TryParse(value, out var pmsLocalIdBuilder))
            return new DataId(pmsLocalIdBuilder.Build());
        else
            return new DataId(value);
    }

    public override int GetHashCode()
    {
        var hash = default(HashCode);

        Switch(
            l => hash.Add(l.GetHashCode()),
            pl => hash.Add(pl.GetHashCode()),
            s => hash.Add(s.GetHashCode())
        );

        return hash.ToHashCode();
    }

    public bool Equals(DataId other) =>
        _tag switch
        {
            1 => other.IsLocalId && other.LocalId == _localId,
            2 => other.IsPmsLocalId && other.PMSLocalId == _pmsLocalId,
            3 => other.IsShortId && other.ShortId == _shortId,
            _ => throw new Exception("Invalid state exception"),
        };
}
