using System.Text;
using Vista.SDK.Internal;

namespace Vista.SDK.Mqtt;

public static class LocalIdBuilderExtensions
{
    public static MqttLocalId BuildMqtt(this LocalIdBuilder builder) => new MqttLocalId(builder);
}

public class MqttLocalId : LocalId
{
    private static readonly char _internal_separator = '_';

    public MqttLocalId(LocalIdBuilder builder)
        : base(builder) { }

    public override string ToString()
    {
        string namingRule = $"{NamingRule}/";
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
