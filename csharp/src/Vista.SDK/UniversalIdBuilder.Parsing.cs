using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Vista.SDK.Internal;

namespace Vista.SDK;

public sealed partial record UniversalIdBuilder
{
    public static UniversalIdBuilder Parse(string localIdStr)
    {
        if (!TryParse(localIdStr, out var localId))
            throw new ArgumentException("Couldn't parse local ID from: " + localIdStr);
        return localId;
    }

    public static UniversalIdBuilder Parse(
        string localIdStr,
        out LocalIdParsingErrorBuilder errorBuilder
    )
    {
        if (!TryParse(localIdStr, out errorBuilder, out var localId))
            throw new ArgumentException("Couldn't parse local ID from: " + localIdStr);
        return localId;
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
        out LocalIdParsingErrorBuilder errorBuilder,
        [MaybeNullWhen(false)] out UniversalIdBuilder universalIdBuilder
    )
    {
        universalIdBuilder = null;

        errorBuilder = LocalIdParsingErrorBuilder.Empty;
        if (universalId is null)
            throw new ArgumentNullException(nameof(universalId));
        if (universalId.Length == 0)
            return false;

        var localIdStartIndex = universalId.IndexOf("/dnv-v");
        if (localIdStartIndex == -1)
        {
            AddError(
                ref errorBuilder,
                ParsingState.NamingRule,
                "Failed to find localId start segment"
            );
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
            // Dont need additional error, as the localIdBuilder does it for us
            return false;

        ReadOnlySpan<char> span = universalIdSegment.AsSpan();
        var state = ParsingState.NamingEntity;
        int i = 0;

        while (state <= ParsingState.IMONumber)
        {
            if (i >= span.Length)
                break; // We've consumed the string
            var nextSlash = span.Slice(i).IndexOf('/');
            var segment = nextSlash == -1 ? span.Slice(i) : span.Slice(i, nextSlash);

            switch (state)
            {
                case ParsingState.NamingEntity:
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
                case ParsingState.IMONumber:
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
            AddError(ref errorBuilder, ParsingState.VisVersion, null);
            return false;
        }

        universalIdBuilder = Create(visVersion.Value)
            .WithLocalId(localIdBuilder)
            .WithImoNumber(imoNumber);
        return true;
    }

    static void AddError(
        ref LocalIdParsingErrorBuilder errorBuilder,
        ParsingState state,
        string? message
    )
    {
        if (!errorBuilder.HasError)
            errorBuilder = LocalIdParsingErrorBuilder.Create();

        errorBuilder.AddError(state, message);
    }
}
