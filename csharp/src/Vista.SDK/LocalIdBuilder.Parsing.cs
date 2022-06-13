using System.Diagnostics.CodeAnalysis;

namespace Vista.SDK;

public sealed partial record class LocalIdBuilder
{
    public static LocalIdBuilder Parse(string localIdStr)
    {
        if (!TryParse(localIdStr, out var localId))
            throw new ArgumentException("Could not parse local ID: " + localIdStr);

        return localId;
    }

    public static bool TryParse(
        string localIdStr,
        [MaybeNullWhen(false)] out LocalIdBuilder localId
    )
    {
        localId = null;
        if (localIdStr is null)
            throw new ArgumentNullException(nameof(localIdStr));
        if (localIdStr.Length == 0)
            return false;
        if (localIdStr[0] != '/')
            return false;

        ReadOnlySpan<char> span = localIdStr.AsSpan();

        VisVersion visVersion = (VisVersion)int.MaxValue;
        Gmod? gmod = null;
        Codebooks? codebooks = null;
        GmodPath? primaryItem = null;
        GmodPath? secondaryItem = null;
        MetadataTag? qty = null;
        MetadataTag? cnt = null;
        MetadataTag? calc = null;
        MetadataTag? stateTag = null;
        MetadataTag? cmd = null;
        MetadataTag? type = null;
        MetadataTag? pos = null;
        MetadataTag? detail = null;
        bool verbose = false;

        var primaryItemStart = -1;
        var secondaryItemStart = -1;

        var state = ParsingState.NamingRule;
        int i = 1;
        while (state <= ParsingState.MetaDetail)
        {
            if (i >= span.Length)
                break; // We've consumed the string

            var nextSlash = span.Slice(i).IndexOf('/');
            var segment = nextSlash == -1 ? span.Slice(i) : span.Slice(i, nextSlash);
            switch (state)
            {
                case ParsingState.NamingRule:
                    if (!segment.SequenceEqual(NamingRule.AsSpan()))
                        return false;

                    AdvanceParser(ref i, in segment, ref state);
                    break;
                case ParsingState.VisVersion:
                    if (!segment.StartsWith("vis-".AsSpan()))
                        return false;
                    if (!VisVersions.TryParse(segment.Slice("vis-".Length), out visVersion))
                        return false;

                    gmod = VIS.Instance.GetGmod(visVersion);
                    codebooks = VIS.Instance.GetCodebooks(visVersion);
                    if (gmod is null || codebooks is null)
                        return false;

                    AdvanceParser(ref i, in segment, ref state);
                    break;
                case ParsingState.PrimaryItem:

                    {
                        var dashIndex = segment.IndexOf('-');
                        var code = dashIndex == -1 ? segment : segment.Slice(0, dashIndex);

                        if (gmod is null)
                            return false;

                        if (primaryItemStart == -1)
                        {
                            if (!gmod.TryGetNode(code, out _))
                            {
                                var nextState = (
                                    segment.StartsWith("sec".AsSpan()),
                                    segment.StartsWith("meta".AsSpan())
                                ) switch
                                {
                                    (false, false) => state,
                                    (true, false) => ParsingState.SecondaryItem,
                                    (false, true) => ParsingState.MetaQty,
                                    _ => throw new Exception("Inconsistent parsing state"),
                                };
                                if (nextState == state)
                                    return false;

                                AdvanceParser(ref i, in segment, ref state, nextState);
                            }
                            else
                            {
                                primaryItemStart = i;
                                AdvanceParser(ref i, in segment);
                            }
                        }
                        else
                        {
                            var nextState = (
                                segment.StartsWith("sec".AsSpan()),
                                segment.StartsWith("meta".AsSpan()),
                                segment[0] == '~'
                            ) switch
                            {
                                (false, false, false) => state,
                                (true, false, false) => ParsingState.SecondaryItem,
                                (false, true, false) => ParsingState.MetaQty,
                                (false, false, true) => ParsingState.ItemDescription,
                                _ => throw new Exception("Inconsistent parsing state"),
                            };

                            if (nextState != state)
                            {
                                var path = span.Slice(primaryItemStart, i - 1 - primaryItemStart);
                                if (!gmod.TryParsePath(path.ToString(), out primaryItem))
                                    return false;

                                if (segment[0] == '~')
                                    AdvanceParser(ref state, nextState);
                                else
                                    AdvanceParser(ref i, in segment, ref state, nextState);
                                break;
                            }

                            if (!gmod.TryGetNode(code, out _))
                                return false;

                            AdvanceParser(ref i, in segment);
                        }
                    }
                    break;
                case ParsingState.SecondaryItem:

                    {
                        var dashIndex = segment.IndexOf('-');
                        var code = dashIndex == -1 ? segment : segment.Slice(0, dashIndex);

                        if (gmod is null)
                            return false;

                        if (secondaryItemStart == -1)
                        {
                            if (!gmod.TryGetNode(code, out _))
                            {
                                return false;
                            }

                            secondaryItemStart = i;
                            AdvanceParser(ref i, in segment);
                        }
                        else
                        {
                            var nextState = (
                                segment.StartsWith("meta".AsSpan()),
                                segment[0] == '~'
                            ) switch
                            {
                                (false, false) => state,
                                (true, false) => ParsingState.MetaQty,
                                (false, true) => ParsingState.ItemDescription,
                                _ => throw new Exception("Inconsistent parsing state"),
                            };

                            if (nextState != state)
                            {
                                var path = span.Slice(
                                    secondaryItemStart,
                                    i - 1 - secondaryItemStart
                                );
                                if (!gmod.TryParsePath(path.ToString(), out secondaryItem))
                                    return false;

                                if (segment[0] == '~')
                                    AdvanceParser(ref state, nextState);
                                else
                                    AdvanceParser(ref i, in segment, ref state, nextState);
                                break;
                            }

                            if (!gmod.TryGetNode(code, out _))
                                return false;

                            AdvanceParser(ref i, in segment);
                        }
                    }
                    break;
                case ParsingState.ItemDescription:
                    verbose = true;

                    var metaIndex = span.IndexOf("/meta".AsSpan());
                    if (metaIndex == -1)
                        return false;

                    segment = span.Slice(i, (metaIndex + "/meta".Length) - i);

                    AdvanceParser(ref i, in segment, ref state);
                    break;
                case ParsingState.MetaQty:

                    {
                        var result = ParseMetatag(
                            CodebookName.Quantity,
                            ref state,
                            ref i,
                            in segment,
                            ref qty,
                            codebooks
                        );
                        if (!result)
                            return false;
                    }
                    break;
                case ParsingState.MetaCnt:

                    {
                        var result = ParseMetatag(
                            CodebookName.Content,
                            ref state,
                            ref i,
                            in segment,
                            ref cnt,
                            codebooks
                        );
                        if (!result)
                            return false;
                    }
                    break;
                case ParsingState.MetaCalc:

                    {
                        var result = ParseMetatag(
                            CodebookName.Calculation,
                            ref state,
                            ref i,
                            in segment,
                            ref calc,
                            codebooks
                        );
                        if (!result)
                            return false;
                    }
                    break;
                case ParsingState.MetaState:

                    {
                        var result = ParseMetatag(
                            CodebookName.State,
                            ref state,
                            ref i,
                            in segment,
                            ref stateTag,
                            codebooks
                        );
                        if (!result)
                            return false;
                    }
                    break;
                case ParsingState.MetaCmd:

                    {
                        var result = ParseMetatag(
                            CodebookName.Command,
                            ref state,
                            ref i,
                            in segment,
                            ref cmd,
                            codebooks
                        );
                        if (!result)
                            return false;
                    }
                    break;
                case ParsingState.MetaType:

                    {
                        var result = ParseMetatag(
                            CodebookName.Type,
                            ref state,
                            ref i,
                            in segment,
                            ref type,
                            codebooks
                        );
                        if (!result)
                            return false;
                    }
                    break;
                case ParsingState.MetaPos:

                    {
                        var result = ParseMetatag(
                            CodebookName.Position,
                            ref state,
                            ref i,
                            in segment,
                            ref pos,
                            codebooks
                        );
                        if (!result)
                            return false;
                    }
                    break;
                case ParsingState.MetaDetail:

                    {
                        var result = ParseMetatag(
                            CodebookName.Detail,
                            ref state,
                            ref i,
                            in segment,
                            ref detail,
                            codebooks
                        );
                        if (!result)
                            return false;
                    }
                    break;
            }
        }

        localId = Create(visVersion)
            .WithPrimaryItem(primaryItem)
            .WithSecondaryItem(secondaryItem)
            .WithVerboseMode(verbose)
            .TryWithMetadataTag(in qty)
            .TryWithMetadataTag(in cnt)
            .TryWithMetadataTag(in calc)
            .TryWithMetadataTag(in stateTag)
            .TryWithMetadataTag(in cmd)
            .TryWithMetadataTag(in type)
            .TryWithMetadataTag(in pos)
            .TryWithMetadataTag(in detail);

        return true;

        static bool ParseMetatag(
            CodebookName codebookName,
            ref ParsingState state,
            ref int i,
            in ReadOnlySpan<char> segment,
            ref MetadataTag? tag,
            Codebooks? codebooks
        )
        {
            if (codebooks is null)
                return false;

            var dashIndex = segment.IndexOf('-');
            if (dashIndex == -1)
                dashIndex = segment.IndexOf('~');
            if (dashIndex == -1)
                return false;

            var actualPrefix = segment.Slice(0, dashIndex);

            var actualState = MetaPrefixToState(actualPrefix);
            if (actualState is null)
                return false;
            if (actualState < state)
                return false;

            if (actualState > state)
            {
                AdvanceParser(ref state, actualState.Value);
                return true;
            }

            var value = segment.Slice(dashIndex + 1);
            if (value.Length == 0)
                return false;

            tag = codebooks.TryCreateTag(codebookName, value.ToString());
            if (tag is null)
                return false;
            AdvanceParser(ref i, in segment, ref state);

            return true;
        }

        static ParsingState? MetaPrefixToState(ReadOnlySpan<char> prefix)
        {
            if (prefix.SequenceEqual("qty".AsSpan()))
                return ParsingState.MetaQty;
            if (prefix.SequenceEqual("cnt".AsSpan()))
                return ParsingState.MetaCnt;
            if (prefix.SequenceEqual("calc".AsSpan()))
                return ParsingState.MetaCalc;
            if (prefix.SequenceEqual("state".AsSpan()))
                return ParsingState.MetaState;
            if (prefix.SequenceEqual("cmd".AsSpan()))
                return ParsingState.MetaCmd;
            if (prefix.SequenceEqual("type".AsSpan()))
                return ParsingState.MetaType;
            if (prefix.SequenceEqual("pos".AsSpan()))
                return ParsingState.MetaPos;
            if (prefix.SequenceEqual("detail".AsSpan()))
                return ParsingState.MetaDetail;

            return null;
        }
    }

    static void AdvanceParser(ref int i, in ReadOnlySpan<char> segment, ref ParsingState state)
    {
        state++;
        i += segment.Length + 1;
    }

    static void AdvanceParser(ref int i, in ReadOnlySpan<char> segment) => i += segment.Length + 1;

    static void AdvanceParser(ref ParsingState state, ParsingState to) => state = to;

    static void AdvanceParser(
        ref int i,
        in ReadOnlySpan<char> segment,
        ref ParsingState state,
        ParsingState to
    )
    {
        i += segment.Length + 1;
        state = to;
    }

    enum ParsingState
    {
        NamingRule,
        VisVersion,
        PrimaryItem,
        SecondaryItem,
        ItemDescription,
        MetaQty,
        MetaCnt,
        MetaCalc,
        MetaState,
        MetaCmd,
        MetaType,
        MetaPos,
        MetaDetail,
    }
}
