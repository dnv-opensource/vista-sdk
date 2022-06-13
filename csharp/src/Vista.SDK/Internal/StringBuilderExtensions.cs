using System.Text;

namespace Vista.SDK.Internal;

internal static class StringBuilderExtensions
{
    public static StringBuilder AppendMeta(this StringBuilder builder, in MetadataTag? tag)
    {
        if (tag is null)
            return builder;

        tag.Value.ToString(builder);

        return builder;
    }
}
