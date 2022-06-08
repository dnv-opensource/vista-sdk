namespace Vista.SDK.Transport;

public readonly record struct ShortId
{
    private readonly string _id;

    public ShortId(string id)
    {
        if (string.IsNullOrWhiteSpace(id))
            throw new ArgumentNullException(nameof(id));
        _id = id;
    }

    public override string ToString() => _id.ToString();

    public static implicit operator ShortId(string id) => new ShortId(id);
}
