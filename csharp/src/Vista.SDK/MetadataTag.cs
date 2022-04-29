﻿using System.Text;

namespace Vista.SDK;

public readonly record struct MetadataTag
{
    public readonly CodebookName Name { get; private init; }

    public readonly string Value { get; private init; }

    public readonly bool IsCustom { get; private init; }

    public readonly char Prefix => IsCustom ? '~' : '-';

    internal MetadataTag(CodebookName name, string value, bool isCustom = false)
    {
        Name = name;
        Value = value;
        IsCustom = isCustom;
    }

    public static implicit operator string(MetadataTag n) => n.Value;

    public readonly bool Equals(MetadataTag other)
    {
        if (Name != other.Name)
            throw new InvalidOperationException($"Cant compare {this} with {other}");

        return other.Value.Equals(Value);
    }

    public readonly override int GetHashCode() => Value.GetHashCode();

    public readonly override string ToString() => Value;

    public readonly void ToString(StringBuilder builder)
    {
        var prefix = Name switch
        {
            CodebookName.Position => "pos",
            CodebookName.Quantity => "qty",
            CodebookName.Calculation => "calc",
            CodebookName.State => "state",
            CodebookName.Content => "cnt",
            CodebookName.Command => "cmd",
            CodebookName.Type => "type",
            CodebookName.Detail => "detail",
            _ => throw new InvalidOperationException("Unknown metadata tag: " + Name),
        };

        builder.Append(prefix);
        builder.Append(IsCustom ? '~' : '-');
        builder.Append(Value);
        builder.Append('/');
    }
}
