using System.Text;
using Vista.SDK.Internal;

namespace Vista.SDK.Mqtt;

public class MqttLocalId : LocalId
{
    public MqttLocalId(LocalIdBuilder builder) : base(builder)
    {
    }

    public override string ToString()
    {
        string namingRule = $"/{NamingRule}/";
        using var lease = StringBuilderPool.Get();

        var builder = lease.Builder;

        builder.Append(namingRule);

        builder.Append("vis-");
        VisVersion.ToVersionString(builder);
        builder.Append('/');

        var items = new LocalIdItems
        {
            PrimaryItem = PrimaryItem,
            SecondaryItem = SecondaryItem,
        };

        items.Append(builder, VerboseMode);

        builder.Append("meta/");

        builder.AppendMeta(Quantity);
        builder.AppendMeta(Content);
        builder.AppendMeta(Calculation);
        builder.AppendMeta(State);
        builder.AppendMeta(Command);
        builder.AppendMeta(Type);
        builder.AppendMeta(Position);
        builder.AppendMeta(Detail);

        if (builder[builder.Length - 1] == '/')
            builder.Remove(builder.Length - 1, 1);

        return lease.ToString();
    }

    void AppendPrimaryItem(StringBuilder builder)
    {
        for (int i = 0; i < PrimaryItem.Length; i++)
        {
            var node = PrimaryItem[i];
            if (node.IsLeafNode || i == PrimaryItem.Length - 1)
            {
                node.ToString(builder);
                builder.Append('~');
            }
        }

        builder.Append('/');
    }
}
