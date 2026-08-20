using System.Text;
using Vista.SDK.Internal;

namespace Vista.SDK.Mqtt;

public static class LocalIdBuilderExtensions
{
    public static MqttLocalId BuildMqtt(this LocalIdBuilder builder) => new MqttLocalId(builder);
}

public class MqttLocalId : IEquatable<MqttLocalId>
{
    private static readonly char _internal_separator = '_';

    private readonly LocalIdBuilder _builder;

    public MqttLocalId(LocalIdBuilder builder)
    {
        if (builder.IsEmpty)
            throw new ArgumentException("LocalId cannot be constructed from empty LocalIdBuilder");
        if (!builder.IsValid)
            throw new ArgumentException("LocalId cannot be constructed from invalid LocalIdBuilder");
        _builder = builder;
    }

    public LocalIdBuilder Builder => _builder;

    public VisVersion VisVersion => _builder.VisVersion!.Value;

    public GmodPath PrimaryItem => _builder.PrimaryItem!;

    public GmodPath? SecondaryItem => _builder.SecondaryItem;

    public MetadataTag? Quantity => _builder.Quantity;

    public MetadataTag? Content => _builder.Content;

    public MetadataTag? Calculation => _builder.Calculation;

    public MetadataTag? State => _builder.State;

    public MetadataTag? Command => _builder.Command;

    public MetadataTag? Type => _builder.Type;

    public MetadataTag? Position => _builder.Position;

    public MetadataTag? Detail => _builder.Detail;

    public sealed override bool Equals(object? obj) => Equals(obj as MqttLocalId);

    public bool Equals(MqttLocalId? other)
    {
        if (other is null)
            return false;

        return _builder.Equals(other._builder);
    }

    public static bool operator !=(MqttLocalId? left, MqttLocalId? right) => !(left == right);

    public static bool operator ==(MqttLocalId? left, MqttLocalId? right)
    {
        if (!ReferenceEquals(left, right))
        {
            if (left is not null)
                return left.Equals(right);
            return false;
        }
        return true;
    }

    public sealed override int GetHashCode() => _builder.GetHashCode();

    public override string ToString()
    {
        string namingRule = $"{LocalId.NamingRule}/";
        using var lease = StringBuilderPool.Get();

        var builder = lease.Builder;

        builder.Append(namingRule);

        builder.Append("vis-");
        VisVersion.ToVersionString(builder);
        builder.Append('/');

        AppendPrimaryItem(builder);
        AppendSecondaryItem(builder);

        AppendMeta(builder, Quantity);
        AppendMeta(builder, Content);
        AppendMeta(builder, Calculation);
        AppendMeta(builder, State);
        AppendMeta(builder, Command);
        AppendMeta(builder, Type);
        AppendMeta(builder, Position);
        AppendMeta(builder, Detail);

        if (builder[builder.Length - 1] == '/')
            builder.Remove(builder.Length - 1, 1);

        return lease.ToString();
    }

    void AppendPath(StringBuilder builder, GmodPath path)
    {
        path.ToString(builder, separator: _internal_separator);
        builder.Append('/');
    }

    void AppendPrimaryItem(StringBuilder builder) => AppendPath(builder, PrimaryItem);

    void AppendSecondaryItem(StringBuilder builder)
    {
        if (SecondaryItem is null)
            builder.Append("_/");
        else
            AppendPath(builder, SecondaryItem);
    }

    void AppendMeta(StringBuilder builder, in MetadataTag? tag)
    {
        if (tag is null)
            builder.Append("_/");
        else
            tag.Value.ToString(builder);
    }
}
