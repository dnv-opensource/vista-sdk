using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Vista.SDK.Internal;

namespace Vista.SDK;

partial record class UniversalIdBuilder
{
    public static UniversalIdBuilder Parse(string universalIdStr)
    {
        if (!TryParse(universalIdStr, out var errors, out var universalId))
            throw new ArgumentException(
                $"Couldn't parse universal ID from: '{universalIdStr}'. {errors}"
            );
        return universalId;
    }

    public static bool TryParse(
        string universalId,
        [MaybeNullWhen(false)] out UniversalIdBuilder universalIdBuilder
    )
    {
        return TryParse(universalId, out _, out universalIdBuilder);
    }

    public static bool TryParse(
        string universalId,
        out ParsingErrors errors,
        [MaybeNullWhen(false)] out UniversalIdBuilder universalIdBuilder
    )
    {
        universalIdBuilder = null;

        var errorBuilder = LocalIdParsingErrorBuilder.Empty;

        if (universalId is null)
        {
            AddError(
                ref errorBuilder,
                LocalIdParsingState.NamingRule,
                "Failed to find localId start segment"
            );
            errors = errorBuilder.Build();
            return false;
        }
        if (universalId.Length == 0)
        {
            AddError(
                ref errorBuilder,
                LocalIdParsingState.NamingRule,
                "Failed to find localId start segment"
            );
            errors = errorBuilder.Build();
            return false;
        }

        var localIdStartIndex = universalId.IndexOf("/dnv-v");
        if (localIdStartIndex == -1)
        {
            AddError(
                ref errorBuilder,
                LocalIdParsingState.NamingRule,
                "Failed to find localId start segment"
            );
            errors = errorBuilder.Build();
            return false;
        }

        var universalIdSegment = universalId.Substring(0, localIdStartIndex);
        var localIdSegment = universalId.Substring(localIdStartIndex);

        ImoNumber? imoNumber = null;

        var localIdBuilder = LocalIdBuilder.TryParseInternal(
            localIdSegment,
            ref errorBuilder,
            out var b
        )
            ? b
            : null;

        if (localIdBuilder is null)
        {
            // Dont need additional error, as the localIdBuilder does it for us
            errors = errorBuilder.Build();
            return false;
        }

        ReadOnlySpan<char> span = universalIdSegment.AsSpan();
        var state = LocalIdParsingState.NamingEntity;
        int i = 0;

        while (state <= LocalIdParsingState.IMONumber)
        {
            if (i >= span.Length)
                break; // We've consumed the string
            var nextSlash = span.Slice(i).IndexOf('/');
            var segment = nextSlash == -1 ? span.Slice(i) : span.Slice(i, nextSlash);

            switch (state)
            {
                case LocalIdParsingState.NamingEntity:
                    if (!NamingEntity.AsSpan().SequenceEqual(segment))
                    {
                        AddError(
                            ref errorBuilder,
                            state,
                            "Naming entity segment didnt match. Found: " + segment.ToString()
                        );
                        break;
                    }
                    break;
                case LocalIdParsingState.IMONumber:
                    if (!SDK.ImoNumber.TryParse(segment, out var imo))
                    {
                        AddError(ref errorBuilder, state, "Invalid IMO number segment");
                        break;
                    }
                    else
                    {
                        imoNumber = imo;
                    }
                    break;
            }
            state++;
            i += segment.Length + 1;
        }

        var visVersion = localIdBuilder.VisVersion;
        if (visVersion is null)
        {
            AddError(ref errorBuilder, LocalIdParsingState.VisVersion, null);
            errors = errorBuilder.Build();
            return false;
        }

        universalIdBuilder = Create(visVersion.Value)
            .TryWithLocalId(in localIdBuilder)
            .TryWithImoNumber(in imoNumber);

        errors = errorBuilder.Build();
        return true;
    }

    static void AddError(
        ref LocalIdParsingErrorBuilder errorBuilder,
        LocalIdParsingState state,
        string? message
    )
    {
        if (!errorBuilder.HasError)
            errorBuilder = LocalIdParsingErrorBuilder.Create();

        errorBuilder.AddError(state, message);
    }
}
