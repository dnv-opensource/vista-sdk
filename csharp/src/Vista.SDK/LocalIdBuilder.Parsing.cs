using System.Diagnostics.CodeAnalysis;
using Vista.SDK.Internal;

namespace Vista.SDK;

public sealed partial record class LocalIdBuilder
{
    public static LocalIdBuilder Parse(string localIdStr)
    {
        if (!TryParse(localIdStr, out var localId))
            throw new ArgumentException("Couldn't parse local ID from: " + localIdStr);
        return localId;
    }

    public static LocalIdBuilder Parse(string localIdStr, ref LocalIdErrorBuilder errorBuilder)
    {
        if (!TryParse(localIdStr, out errorBuilder, out var localId))
            throw new ArgumentException("Couldn't parse local ID from: " + localIdStr);
        return localId;
    }

    public static bool TryParse(
        string localIdStr,
        [MaybeNullWhen(false)] out LocalIdBuilder localId
    )
    {
        return TryParse(localIdStr, out _, out localId);
    }

    public static bool TryParse(
        string localIdStr,
        out LocalIdErrorBuilder errorBuilder,
        [MaybeNullWhen(false)] out LocalIdBuilder localId
    )
    {
        localId = null;
        errorBuilder = LocalIdErrorBuilder.Empty;
        if (localIdStr is null)
            throw new ArgumentNullException(nameof(localIdStr));
        if (localIdStr.Length == 0)
            return false;
        if (localIdStr[0] != '/')
        {
            AddError(
                ref errorBuilder,
                ParsingState.Formatting,
                "Invalid format: missing '/' as first character"
            );
            //return false;
        }

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
        string? predefinedMessage = null;

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
                    {
                        AddError(ref errorBuilder, ParsingState.NamingRule, predefinedMessage);
                        return false;
                    }
                    AdvanceParser(ref i, in segment, ref state);
                    break;
                case ParsingState.VisVersion:
                    if (!segment.StartsWith("vis-".AsSpan()))
                    {
                        AddError(ref errorBuilder, ParsingState.VisVersion, predefinedMessage);
                        return false;
                    }

                    if (!VisVersions.TryParse(segment.Slice("vis-".Length), out visVersion))
                    {
                        AddError(ref errorBuilder, ParsingState.VisVersion, predefinedMessage);
                        return false;
                    }

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
                                AddError(
                                    ref errorBuilder,
                                    ParsingState.PrimaryItem,
                                    $"Invalid start GmodNode in Primary item: {code.ToString()}"
                                );

                                primaryItemStart = i;
                                AdvanceParser(ref i, in segment);

                                //(var nextStateIndex, var endOfNextStateIndex) = GetNextStateIndexes(
                                //    span,
                                //    state
                                //);
                                //var nextSegment = span.Slice(nextStateIndex + 1);

                                //var nextState = (
                                //    nextSegment.StartsWith("sec".AsSpan()),
                                //    nextSegment.StartsWith("meta".AsSpan()),
                                //    nextSegment[0] == '~'
                                //) switch
                                //{
                                //    (false, false, false) => state,
                                //    (true, false, false) => ParsingState.SecondaryItem,
                                //    (false, true, false) => ParsingState.MetaQty,
                                //    (false, false, true) => ParsingState.ItemDescription,
                                //    _ => throw new Exception("Inconsistent parsing state"),
                                //};
                                //if (nextState == state)
                                //{
                                //    AddError(
                                //        ref errorBuilder,
                                //        ParsingState.PrimaryItem,
                                //        "Invalid or missing '/meta' prefix after Primary item"
                                //    );
                                //    return false;
                                //}

                                //var primaryItemPath = span.Slice(i, nextStateIndex - i);

                                //AddError(
                                //    ref errorBuilder,
                                //    ParsingState.PrimaryItem,
                                //    $"Invalid GmodPath in Primary item: {primaryItemPath.ToString()}"
                                //);

                                //i = endOfNextStateIndex;
                                //AdvanceParser(ref state, nextState);
                                //break;
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
                                {
                                    // Displays the full GmodPath when first part of PrimaryItem is invalid
                                    AddError(
                                        ref errorBuilder,
                                        ParsingState.PrimaryItem,
                                        $"Invalid GmodPath in Primary item: {path.ToString()}"
                                    );

                                    (var nextStateIndex, var endOfNextStateIndex) =
                                        GetNextStateIndexes(span, state);

                                    var nextSegment = span.Slice(nextStateIndex + 1);

                                    nextState = (
                                        nextSegment.StartsWith("sec".AsSpan()),
                                        nextSegment.StartsWith("meta".AsSpan()),
                                        nextSegment[0] == '~'
                                    ) switch
                                    {
                                        (false, false, false) => state,
                                        (true, false, false) => ParsingState.SecondaryItem,
                                        (false, true, false) => ParsingState.MetaQty,
                                        (false, false, true) => ParsingState.ItemDescription,
                                        _ => throw new Exception("Inconsistent parsing state"),
                                    };

                                    if (nextState == state)
                                    {
                                        AddError(
                                            ref errorBuilder,
                                            ParsingState.PrimaryItem,
                                            "Invalid or missing '/meta' prefix after Primary item"
                                        );
                                        return false;
                                    }
                                    i = endOfNextStateIndex;
                                    AdvanceParser(ref state, nextState);
                                    break; //return false;
                                }

                                if (segment[0] == '~')
                                    AdvanceParser(ref state, nextState);
                                else
                                    AdvanceParser(ref i, in segment, ref state, nextState);
                                break;
                            }

                            if (!gmod.TryGetNode(code, out _))
                            {
                                AddError(
                                    ref errorBuilder,
                                    ParsingState.PrimaryItem,
                                    $"Invalid GmodNode in Primary item: {code.ToString()}"
                                );
                                (var nextStateIndex, var endOfNextStateIndex) = GetNextStateIndexes(
                                    span,
                                    state
                                );

                                var nextSegment = span.Slice(nextStateIndex + 1);

                                nextState = (
                                    nextSegment.StartsWith("sec".AsSpan()),
                                    nextSegment.StartsWith("meta".AsSpan()),
                                    nextSegment[0] == '~'
                                ) switch
                                {
                                    (false, false, false) => state,
                                    (true, false, false) => ParsingState.SecondaryItem,
                                    (false, true, false) => ParsingState.MetaQty,
                                    (false, false, true) => ParsingState.ItemDescription,
                                    _ => throw new Exception("Inconsistent parsing state"),
                                };
                                // Displays the invalid middle parts of secondaryItem and not the whole GmodPath

                                var invalidPrimaryItemPath = span.Slice(i, nextStateIndex - i);

                                AddError(
                                    ref errorBuilder,
                                    ParsingState.PrimaryItem,
                                    $"Invalid GmodPath: Last part in Primary item: {invalidPrimaryItemPath.ToString()}"
                                );

                                if (nextState == state)
                                {
                                    AddError(
                                        ref errorBuilder,
                                        ParsingState.PrimaryItem,
                                        "Invalid or missing '/meta' prefix after Primary item"
                                    );
                                    return false;
                                }

                                i = endOfNextStateIndex;
                                AdvanceParser(ref state, nextState);
                                break; //return false;
                            }

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
                                AddError(
                                    ref errorBuilder,
                                    ParsingState.SecondaryItem,
                                    $"Invalid GmodNode in Secondary item: {code.ToString()}"
                                );

                                (var nextStateIndex, var endOfNextStateIndex) = GetNextStateIndexes(
                                    span,
                                    state
                                );

                                var nextSegment = span.Slice(nextStateIndex + 1);

                                var nextState = (
                                    nextSegment.StartsWith("meta".AsSpan()),
                                    nextSegment[0] == '~'
                                ) switch
                                {
                                    (false, false) => state,
                                    (true, false) => ParsingState.MetaQty,
                                    (false, true) => ParsingState.ItemDescription,
                                    _ => throw new Exception("Inconsistent parsing state"),
                                };

                                var path = span.Slice(i, nextStateIndex - i);

                                AddError(
                                    ref errorBuilder,
                                    ParsingState.SecondaryItem,
                                    $"Invalid GmodPath in Secondary item: {path.ToString()}"
                                );
                                if (nextState == state)
                                {
                                    AddError(
                                        ref errorBuilder,
                                        ParsingState.SecondaryItem,
                                        "Invalid or missing '/meta' prefix after Secondary item"
                                    );
                                    return false;
                                }

                                i = endOfNextStateIndex;

                                AdvanceParser(ref state, nextState);
                                break; //return false;
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
                                {
                                    AddError(
                                        ref errorBuilder,
                                        ParsingState.SecondaryItem,
                                        $"Invalid GmodPath in Secondary item: {path.ToString()}"
                                    );

                                    //TODO: Advance to next state
                                    break; //return false;
                                }

                                if (segment[0] == '~')
                                    AdvanceParser(ref state, nextState);
                                else
                                    AdvanceParser(ref i, in segment, ref state, nextState);
                                break;
                            }

                            if (!gmod.TryGetNode(code, out _))
                            {
                                AddError(
                                    ref errorBuilder,
                                    ParsingState.SecondaryItem,
                                    $"Invalid GmodNode in Secondary item: {code.ToString()}"
                                );

                                (var nextStateIndex, var endOfNextStateIndex) = GetNextStateIndexes(
                                    span,
                                    state
                                );

                                var nextSegment = span.Slice(nextStateIndex + 1);

                                nextState = (
                                    nextSegment.StartsWith("meta".AsSpan()),
                                    nextSegment[0] == '~'
                                ) switch
                                {
                                    (false, false) => state,
                                    (true, false) => ParsingState.MetaQty,
                                    (false, true) => ParsingState.ItemDescription,
                                    _ => throw new Exception("Inconsistent parsing state"),
                                };

                                var path = span.Slice(
                                    secondaryItemStart,
                                    nextStateIndex - secondaryItemStart
                                );

                                AddError(
                                    ref errorBuilder,
                                    ParsingState.SecondaryItem,
                                    $"Invalid GmodPath in Secondary item: {path.ToString()}"
                                );
                                if (nextState == state)
                                {
                                    AddError(
                                        ref errorBuilder,
                                        ParsingState.SecondaryItem,
                                        "Invalid or missing '/meta' prefix after Secondary item"
                                    );
                                    return false;
                                }

                                i = endOfNextStateIndex;

                                AdvanceParser(ref state, nextState);
                                break; //return false;
                            }
                            AdvanceParser(ref i, in segment);
                        }
                    }
                    break;
                case ParsingState.ItemDescription:
                    verbose = true;

                    var metaIndex = span.IndexOf("/meta".AsSpan());
                    if (metaIndex == -1)
                    {
                        AddError(ref errorBuilder, ParsingState.ItemDescription, predefinedMessage);
                        return false;
                    }

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
                            codebooks,
                            ref errorBuilder
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
                            codebooks,
                            ref errorBuilder
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
                            codebooks,
                            ref errorBuilder
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
                            codebooks,
                            ref errorBuilder
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
                            codebooks,
                            ref errorBuilder
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
                            codebooks,
                            ref errorBuilder
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
                            codebooks,
                            ref errorBuilder
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
                            codebooks,
                            ref errorBuilder
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

        return localId.IsValid;

        static bool ParseMetatag(
            CodebookName codebookName,
            ref ParsingState state,
            ref int i,
            in ReadOnlySpan<char> segment,
            ref MetadataTag? tag,
            Codebooks? codebooks,
            ref LocalIdErrorBuilder errorBuilder
        )
        {
            if (codebooks is null)
                return false;

            var dashIndex = segment.IndexOf('-');
            if (dashIndex == -1)
                dashIndex = segment.IndexOf('~');
            if (dashIndex == -1)
            {
                AddError(
                    ref errorBuilder,
                    state,
                    $"Invalid metadata tag: missing '-' in {segment.ToString()} "
                );
                return false;
            }

            var actualPrefix = segment.Slice(0, dashIndex);

            var actualState = MetaPrefixToState(actualPrefix);
            if (actualState is null || actualState < state)
            {
                AddError(
                    ref errorBuilder,
                    state,
                    $"Invalid metadata tag: unknown prefix {actualPrefix.ToString()}"
                );
                return false;
            }

            if (actualState > state)
            {
                AdvanceParser(ref state, actualState.Value);
                return true;
            }

            var value = segment.Slice(dashIndex + 1);
            if (value.Length == 0)
            {
                AddError(
                    ref errorBuilder,
                    state,
                    $"Invalid {codebookName} metadata tag: missing value"
                );
                return false;
            }

            tag = codebooks.TryCreateTag(codebookName, value.ToString());
            if (tag is null)
            {
                AddError(
                    ref errorBuilder,
                    state,
                    $"Invalid {codebookName} metadata tag: failed to create {value.ToString()}"
                );
                return false;
            }
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

        static void AddError(
            ref LocalIdErrorBuilder errorBuilder,
            ParsingState state,
            string? message
        )
        {
            if (!errorBuilder.HasError)
                errorBuilder = LocalIdErrorBuilder.Create();

            errorBuilder.AddError(state, message);
        }

        static (int NextIndex, int NextStateIndex) GetNextStateIndexes(
            ReadOnlySpan<char> span,
            ParsingState state
        )
        {
            var customIndex = span.IndexOf("~".AsSpan());
            var endOfCustomIndex = (customIndex + "~".Length + 1);

            var metaIndex = span.IndexOf("/meta".AsSpan());
            var endOfMetaIndex = (metaIndex + "/meta".Length + 1);

            switch (state)
            {
                case (ParsingState.PrimaryItem):
                {
                    var secIndex = span.IndexOf("/sec".AsSpan());
                    var endOfSecIndex = (secIndex + "/sec".Length + 1);
                    return secIndex != -1
                      ? (secIndex, endOfSecIndex)
                      : customIndex != -1
                          ? (customIndex, endOfCustomIndex)
                          : (metaIndex, endOfMetaIndex);
                }

                case (ParsingState.SecondaryItem):
                    return customIndex != -1
                      ? (customIndex, endOfCustomIndex)
                      : (metaIndex, endOfMetaIndex);

                default:
                    return (metaIndex, endOfMetaIndex);
            }
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
}
