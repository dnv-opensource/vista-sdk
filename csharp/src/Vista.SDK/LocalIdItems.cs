using System.Text;

namespace Vista.SDK;

internal readonly record struct LocalIdItems
{
    public GmodPath? PrimaryItem { get; init; }

    public GmodPath? SecondaryItem { get; init; }

    internal void Append(StringBuilder builder, bool verboseMode)
    {
        if (PrimaryItem is null && SecondaryItem is null)
            return;

        if (PrimaryItem is not null)
        {
            PrimaryItem.ToString(builder);
            builder.Append('/');
        }

        if (SecondaryItem is not null)
        {
            builder.Append("sec/");
            SecondaryItem.ToString(builder);
            builder.Append('/');
        }

        if (verboseMode)
        {
            if (PrimaryItem is not null)
            {
                foreach (var (depth, name) in PrimaryItem.GetCommonNames())
                {
                    builder.Append('~');
                    var location = PrimaryItem[depth].Location;
                    AppendCommonName(builder, name, location?.ToString());
                    builder.Append('/');
                }
            }

            if (SecondaryItem is not null)
            {
                var prefix = "~for.";
                foreach (var (depth, name) in SecondaryItem.GetCommonNames())
                {
                    builder.Append(prefix);
                    if (prefix != "~")
                        prefix = "~";

                    var location = SecondaryItem[depth].Location;
                    AppendCommonName(builder, name, location?.ToString());
                    builder.Append('/');
                }
            }
        }

        static void AppendCommonName(StringBuilder builder, string commonName, string? location)
        {
            char? prev = null;
            foreach (ref readonly var ch in commonName.AsSpan())
            {
                if (ch == '/')
                    continue;
                if (prev == ' ' && ch == ' ')
                    continue;

                var current = ch;
                switch (ch)
                {
                    case ' ':
                        current = '.';
                        break;
                    default:
                        var match = VIS.IsISOString(ch);
                        if (!match)
                        {
                            current = '.';
                            break;
                        }
                        current = char.ToLowerInvariant(ch);
                        break;
                }
                if (current == '.' && prev == '.')
                    continue;
                builder.Append(current);
                prev = current;
            }

            if (location is { Length: > 0 })
            {
                builder.Append('.');
                builder.Append(location);
            }
        }
    }
}
