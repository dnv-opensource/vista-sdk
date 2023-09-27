using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using Vista.SDK.Internal;

namespace Vista.SDK;

internal enum LocalIdParsingState
{
    NamingRule,
    VisVersion,
    PrimaryItem,
    SecondaryItem,
    ItemDescription,
    MetaQuantity,
    MetaContent,
    MetaCalculation,
    MetaState,
    MetaCommand,
    MetaType,
    MetaPosition,
    MetaDetail,

    // For "other" errors
    EmptyState = 100,
    Formatting = 101,
    Completeness = 102,

    // UniversalId
    NamingEntity = 200,
    IMONumber = 201
}

partial record class LocalIdBuilder
{
    public static LocalIdBuilder Parse(string localIdStr)
    {
        if (!TryParse(localIdStr, out var errors, out var localId))
            throw new ArgumentException($"Couldn't parse local ID from: '{localIdStr}'. {errors}");

        return localId;
    }

    public static bool TryParse(string localIdStr, [MaybeNullWhen(false)] out LocalIdBuilder localId)
    {
        return TryParse(localIdStr, out _, out localId);
    }

    public static bool TryParse(
        string localIdStr,
        out ParsingErrors errors,
        [MaybeNullWhen(false)] out LocalIdBuilder localId
    )
    {
        var errorBuilder = LocalIdParsingErrorBuilder.Empty;

        var result = TryParseInternal(localIdStr, ref errorBuilder, out localId);
        errors = errorBuilder.Build();
        return result;
    }

    internal static bool TryParseInternal(
        string localIdStr,
        ref LocalIdParsingErrorBuilder errorBuilder,
        [MaybeNullWhen(false)] out LocalIdBuilder localId
    )
    {
        localId = null;
        if (localIdStr is null)
            throw new ArgumentNullException(nameof(localIdStr));
        if (localIdStr.Length == 0)
            return false;
        if (localIdStr[0] != '/')
        {
            AddError(
                ref errorBuilder,
                LocalIdParsingState.Formatting,
                "Invalid format: missing '/' as first character"
            );
            return false;
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
        bool invalidSecondaryItem = false;

        var primaryItemStart = -1;
        var secondaryItemStart = -1;

        var state = LocalIdParsingState.NamingRule;
        int i = 1;
        while (state <= LocalIdParsingState.MetaDetail)
        {
            var nextStart = Math.Min(span.Length, i);
            var nextSlash = span.Slice(nextStart).IndexOf('/');
            var segment = nextSlash == -1 ? span.Slice(nextStart) : span.Slice(nextStart, nextSlash);
            switch (state)
            {
                case LocalIdParsingState.NamingRule:
                    if (segment.Length == 0)
                    {
                        AddError(ref errorBuilder, LocalIdParsingState.NamingRule, predefinedMessage);
                        state++;
                        break;
                    }

                    if (!segment.SequenceEqual(NamingRule.AsSpan()))
                    {
                        AddError(ref errorBuilder, LocalIdParsingState.NamingRule, predefinedMessage);
                        return false;
                    }
                    AdvanceParser(ref i, in segment, ref state);
                    break;
                case LocalIdParsingState.VisVersion:
                    if (segment.Length == 0)
                    {
                        AddError(ref errorBuilder, LocalIdParsingState.VisVersion, predefinedMessage);
                        state++;
                        break;
                    }

                    if (!segment.StartsWith("vis-".AsSpan()))
                    {
                        AddError(ref errorBuilder, LocalIdParsingState.VisVersion, predefinedMessage);
                        return false;
                    }

                    if (!VisVersions.TryParse(segment.Slice("vis-".Length), out visVersion))
                    {
                        AddError(ref errorBuilder, LocalIdParsingState.VisVersion, predefinedMessage);
                        return false;
                    }

                    gmod = VIS.Instance.GetGmod(visVersion);
                    codebooks = VIS.Instance.GetCodebooks(visVersion);
                    if (gmod is null || codebooks is null)
                        return false;

                    AdvanceParser(ref i, in segment, ref state);
                    break;
                case LocalIdParsingState.PrimaryItem:

                    {
                        if (segment.Length == 0)
                        {
                            if (primaryItemStart != -1)
                            {
                                if (gmod is null)
                                    return false;

                                var path = span.Slice(primaryItemStart, i - 1 - primaryItemStart);
                                if (!gmod.TryParsePath(path.ToString(), out primaryItem))
                                {
                                    // Displays the full GmodPath when first part of PrimaryItem is invalid
                                    AddError(
                                        ref errorBuilder,
                                        LocalIdParsingState.PrimaryItem,
                                        $"Invalid GmodPath in Primary item: {path.ToString()}"
                                    );
                                }
                            }
                            else
                            {
                                AddError(ref errorBuilder, LocalIdParsingState.PrimaryItem, predefinedMessage);
                            }
                            AddError(
                                ref errorBuilder,
                                LocalIdParsingState.PrimaryItem,
                                "Invalid or missing '/meta' prefix after Primary item"
                            );
                            state++;
                            break;
                        }

                        var dashIndex = segment.IndexOf('-');
                        var code = dashIndex == -1 ? segment : segment.Slice(0, dashIndex);

                        if (gmod is null)
                            return false;

                        if (primaryItemStart == -1)
                        {
                            if (!gmod.TryGetNode(code, out _))
                                AddError(
                                    ref errorBuilder,
                                    LocalIdParsingState.PrimaryItem,
                                    $"Invalid start GmodNode in Primary item: {code.ToString()}"
                                );
                            primaryItemStart = i;
                            AdvanceParser(ref i, in segment);
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
                                (true, false, false) => LocalIdParsingState.SecondaryItem,
                                (false, true, false) => LocalIdParsingState.MetaQuantity,
                                (false, false, true) => LocalIdParsingState.ItemDescription,
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
                                        LocalIdParsingState.PrimaryItem,
                                        $"Invalid GmodPath in Primary item: {path.ToString()}"
                                    );

                                    (var _, var endOfNextStateIndex) = GetNextStateIndexes(span, state);
                                    i = endOfNextStateIndex;
                                    AdvanceParser(ref state, nextState);
                                    break;
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
                                    LocalIdParsingState.PrimaryItem,
                                    $"Invalid GmodNode in Primary item: {code.ToString()}"
                                );
                                (var nextStateIndex, var endOfNextStateIndex) = GetNextStateIndexes(span, state);

                                if (nextStateIndex == -1)
                                {
                                    AddError(
                                        ref errorBuilder,
                                        LocalIdParsingState.PrimaryItem,
                                        "Invalid or missing '/meta' prefix after Primary item"
                                    );
                                    return false;
                                }

                                var nextSegment = span.Slice(nextStateIndex + 1);

                                nextState = (
                                    nextSegment.StartsWith("sec".AsSpan()),
                                    nextSegment.StartsWith("meta".AsSpan()),
                                    nextSegment[0] == '~'
                                ) switch
                                {
                                    (true, false, false) => LocalIdParsingState.SecondaryItem,
                                    (false, true, false) => LocalIdParsingState.MetaQuantity,
                                    (false, false, true) => LocalIdParsingState.ItemDescription,
                                    _ => throw new Exception("Inconsistent parsing state"),
                                };

                                // Displays the invalid middle parts of PrimaryItem and not the whole GmodPath
                                var invalidPrimaryItemPath = span.Slice(i, nextStateIndex - i);

                                AddError(
                                    ref errorBuilder,
                                    LocalIdParsingState.PrimaryItem,
                                    $"Invalid GmodPath: Last part in Primary item: {invalidPrimaryItemPath.ToString()}"
                                );

                                i = endOfNextStateIndex;
                                AdvanceParser(ref state, nextState);
                                break;
                            }

                            AdvanceParser(ref i, in segment);
                        }
                    }
                    break;
                case LocalIdParsingState.SecondaryItem:

                    {
                        if (segment.Length == 0)
                        {
                            state++;
                            break;
                        }

                        var dashIndex = segment.IndexOf('-');
                        var code = dashIndex == -1 ? segment : segment.Slice(0, dashIndex);
                        if (gmod is null)
                            return false;

                        if (secondaryItemStart == -1)
                        {
                            if (!gmod.TryGetNode(code, out _))
                                AddError(
                                    ref errorBuilder,
                                    LocalIdParsingState.SecondaryItem,
                                    $"Invalid start GmodNode in Secondary item: {code.ToString()}"
                                );

                            secondaryItemStart = i;
                            AdvanceParser(ref i, in segment);
                        }
                        else
                        {
                            var nextState = (segment.StartsWith("meta".AsSpan()), segment[0] == '~') switch
                            {
                                (false, false) => state,
                                (true, false) => LocalIdParsingState.MetaQuantity,
                                (false, true) => LocalIdParsingState.ItemDescription,
                                _ => throw new Exception("Inconsistent parsing state"),
                            };

                            if (nextState != state)
                            {
                                var path = span.Slice(secondaryItemStart, i - 1 - secondaryItemStart);
                                if (!gmod.TryParsePath(path.ToString(), out secondaryItem))
                                {
                                    // Displays the full GmodPath when first part of SecondaryItem is invalid
                                    invalidSecondaryItem = true;
                                    AddError(
                                        ref errorBuilder,
                                        LocalIdParsingState.SecondaryItem,
                                        $"Invalid GmodPath in Secondary item: {path.ToString()}"
                                    );

                                    (var _, var endOfNextStateIndex) = GetNextStateIndexes(span, state);
                                    i = endOfNextStateIndex;
                                    AdvanceParser(ref state, nextState);
                                    break;
                                }

                                if (segment[0] == '~')
                                    AdvanceParser(ref state, nextState);
                                else
                                    AdvanceParser(ref i, in segment, ref state, nextState);
                                break;
                            }

                            if (!gmod.TryGetNode(code, out _))
                            {
                                invalidSecondaryItem = true;
                                AddError(
                                    ref errorBuilder,
                                    LocalIdParsingState.SecondaryItem,
                                    $"Invalid GmodNode in Secondary item: {code.ToString()}"
                                );

                                (var nextStateIndex, var endOfNextStateIndex) = GetNextStateIndexes(span, state);
                                if (nextStateIndex == -1)
                                {
                                    AddError(
                                        ref errorBuilder,
                                        LocalIdParsingState.SecondaryItem,
                                        "Invalid or missing '/meta' prefix after Secondary item"
                                    );
                                    return false;
                                }

                                var nextSegment = span.Slice(nextStateIndex + 1);

                                nextState = (nextSegment.StartsWith("meta".AsSpan()), nextSegment[0] == '~') switch
                                {
                                    (true, false) => LocalIdParsingState.MetaQuantity,
                                    (false, true) => LocalIdParsingState.ItemDescription,
                                    _ => throw new Exception("Inconsistent parsing state"),
                                };

                                var invalidSecondaryItemPath = span.Slice(i, nextStateIndex - i);

                                AddError(
                                    ref errorBuilder,
                                    LocalIdParsingState.SecondaryItem,
                                    $"Invalid GmodPath: Last part in Secondary item: {invalidSecondaryItemPath.ToString()}"
                                );

                                i = endOfNextStateIndex;

                                AdvanceParser(ref state, nextState);
                                break;
                            }
                            AdvanceParser(ref i, in segment);
                        }
                    }
                    break;
                case LocalIdParsingState.ItemDescription:
                    if (segment.Length == 0)
                    {
                        state++;
                        break;
                    }

                    verbose = true;

                    var metaIndex = span.IndexOf("/meta".AsSpan());
                    if (metaIndex == -1)
                    {
                        AddError(ref errorBuilder, LocalIdParsingState.ItemDescription, predefinedMessage);
                        return false;
                    }

                    segment = span.Slice(i, (metaIndex + "/meta".Length) - i);

                    AdvanceParser(ref i, in segment, ref state);
                    break;
                case LocalIdParsingState.MetaQuantity:

                    {
                        if (segment.Length == 0)
                        {
                            state++;
                            break;
                        }

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
                case LocalIdParsingState.MetaContent:

                    {
                        if (segment.Length == 0)
                        {
                            state++;
                            break;
                        }

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
                case LocalIdParsingState.MetaCalculation:

                    {
                        if (segment.Length == 0)
                        {
                            state++;
                            break;
                        }

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
                case LocalIdParsingState.MetaState:

                    {
                        if (segment.Length == 0)
                        {
                            state++;
                            break;
                        }

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
                case LocalIdParsingState.MetaCommand:

                    {
                        if (segment.Length == 0)
                        {
                            state++;
                            break;
                        }

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
                case LocalIdParsingState.MetaType:

                    {
                        if (segment.Length == 0)
                        {
                            state++;
                            break;
                        }

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
                case LocalIdParsingState.MetaPosition:

                    {
                        if (segment.Length == 0)
                        {
                            state++;
                            break;
                        }

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
                case LocalIdParsingState.MetaDetail:

                    {
                        if (segment.Length == 0)
                        {
                            state++;
                            break;
                        }

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
                default:
                    AdvanceParser(ref i, in segment, ref state);
                    break;
            }
        }

        localId = Create(visVersion)
            .TryWithPrimaryItem(in primaryItem)
            .TryWithSecondaryItem(in secondaryItem)
            .WithVerboseMode(in verbose)
            .TryWithMetadataTag(in qty)
            .TryWithMetadataTag(in cnt)
            .TryWithMetadataTag(in calc)
            .TryWithMetadataTag(in stateTag)
            .TryWithMetadataTag(in cmd)
            .TryWithMetadataTag(in type)
            .TryWithMetadataTag(in pos)
            .TryWithMetadataTag(in detail);

        if (localId.IsEmptyMetadata)
        {
            AddError(
                ref errorBuilder,
                LocalIdParsingState.Completeness,
                "No metadata tags specified. Local IDs require atleast 1 metadata tag."
            );
        }

        return (!errorBuilder.HasError && !invalidSecondaryItem);

        static bool ParseMetatag(
            CodebookName codebookName,
            ref LocalIdParsingState state,
            ref int i,
            in ReadOnlySpan<char> segment,
            ref MetadataTag? tag,
            Codebooks? codebooks,
            ref LocalIdParsingErrorBuilder errorBuilder
        )
        {
            if (codebooks is null)
                return false;

            var dashIndex = segment.IndexOf('-');
            var tildeIndex = segment.IndexOf('~');
            var prefixIndex = dashIndex == -1 ? tildeIndex : dashIndex;
            if (prefixIndex == -1)
            {
                AddError(
                    ref errorBuilder,
                    state,
                    $"Invalid metadata tag: missing prefix '-' or '~' in {segment.ToString()}"
                );
                AdvanceParser(ref i, in segment, ref state);
                return true;
            }

            var actualPrefix = segment.Slice(0, prefixIndex);

            var actualState = MetaPrefixToState(actualPrefix);
            if (actualState is null || actualState < state)
            {
                AddError(ref errorBuilder, state, $"Invalid metadata tag: unknown prefix {actualPrefix.ToString()}");
                return false;
            }

            if (actualState > state)
            {
                AdvanceParser(ref state, actualState.Value);
                return true;
            }

            var nextState = NextParsingState(actualState.Value);

            var value = segment.Slice(prefixIndex + 1);
            if (value.Length == 0)
            {
                AddError(ref errorBuilder, state, $"Invalid {codebookName} metadata tag: missing value");
                return false;
            }

            tag = codebooks.TryCreateTag(codebookName, value.ToString());
            if (tag is null)
            {
                if (prefixIndex == tildeIndex)
                    AddError(
                        ref errorBuilder,
                        state,
                        $"Invalid custom {codebookName} metadata tag: failed to create {value.ToString()}"
                    );
                else
                    AddError(
                        ref errorBuilder,
                        state,
                        $"Invalid {codebookName} metadata tag: failed to create {value.ToString()}"
                    );

                AdvanceParser(ref i, in segment, ref state);
                return true;
            }

            if (prefixIndex == dashIndex && tag.Value.Prefix == '~')
                AddError(
                    ref errorBuilder,
                    state,
                    $"Invalid {codebookName} metadata tag: '{value.ToString()}'. Use prefix '~' for custom values"
                );
            if (nextState is null)
                AdvanceParser(ref i, in segment, ref state);
            else
                AdvanceParser(ref i, in segment, ref state, nextState.Value);
            return true;
        }

        static LocalIdParsingState? MetaPrefixToState(ReadOnlySpan<char> prefix)
        {
            if (prefix.SequenceEqual("qty".AsSpan()))
                return LocalIdParsingState.MetaQuantity;
            if (prefix.SequenceEqual("cnt".AsSpan()))
                return LocalIdParsingState.MetaContent;
            if (prefix.SequenceEqual("calc".AsSpan()))
                return LocalIdParsingState.MetaCalculation;
            if (prefix.SequenceEqual("state".AsSpan()))
                return LocalIdParsingState.MetaState;
            if (prefix.SequenceEqual("cmd".AsSpan()))
                return LocalIdParsingState.MetaCommand;
            if (prefix.SequenceEqual("type".AsSpan()))
                return LocalIdParsingState.MetaType;
            if (prefix.SequenceEqual("pos".AsSpan()))
                return LocalIdParsingState.MetaPosition;
            if (prefix.SequenceEqual("detail".AsSpan()))
                return LocalIdParsingState.MetaDetail;

            return null;
        }

        static LocalIdParsingState? NextParsingState(LocalIdParsingState prev) =>
            prev switch
            {
                LocalIdParsingState.MetaQuantity => LocalIdParsingState.MetaContent,
                LocalIdParsingState.MetaContent => LocalIdParsingState.MetaCalculation,
                LocalIdParsingState.MetaCalculation => LocalIdParsingState.MetaState,
                LocalIdParsingState.MetaState => LocalIdParsingState.MetaCommand,
                LocalIdParsingState.MetaCommand => LocalIdParsingState.MetaType,
                LocalIdParsingState.MetaType => LocalIdParsingState.MetaPosition,
                LocalIdParsingState.MetaPosition => LocalIdParsingState.MetaDetail,
                _ => null
            };

        static void AddError(ref LocalIdParsingErrorBuilder errorBuilder, LocalIdParsingState state, string? message)
        {
            if (!errorBuilder.HasError)
                errorBuilder = LocalIdParsingErrorBuilder.Create();

            errorBuilder.AddError(state, message);
        }

        static (int NextIndex, int EndOfNextStateIndex) GetNextStateIndexes(
            ReadOnlySpan<char> span,
            LocalIdParsingState state
        )
        {
            var customIndex = span.IndexOf("~".AsSpan());
            var endOfCustomIndex = (customIndex + "~".Length + 1);

            var metaIndex = span.IndexOf("/meta".AsSpan());
            var endOfMetaIndex = (metaIndex + "/meta".Length + 1);
            var isVerbose = customIndex < metaIndex;

            switch (state)
            {
                case (LocalIdParsingState.PrimaryItem):
                {
                    var secIndex = span.IndexOf("/sec".AsSpan());
                    var endOfSecIndex = (secIndex + "/sec".Length + 1);

                    if (secIndex != -1)
                        return (secIndex, endOfSecIndex);

                    if (isVerbose && customIndex != -1)
                        return (customIndex, endOfCustomIndex);

                    return (metaIndex, endOfMetaIndex);
                }

                case (LocalIdParsingState.SecondaryItem):
                    if (isVerbose && customIndex != -1)
                        return (customIndex, endOfCustomIndex);
                    return (metaIndex, endOfMetaIndex);

                default:
                    return (metaIndex, endOfMetaIndex);
            }
        }
    }

    static void AdvanceParser(ref int i, in ReadOnlySpan<char> segment, ref LocalIdParsingState state)
    {
        state++;
        i += segment.Length + 1;
    }

    static void AdvanceParser(ref int i, in ReadOnlySpan<char> segment) => i += segment.Length + 1;

    static void AdvanceParser(ref LocalIdParsingState state, LocalIdParsingState to) => state = to;

    static void AdvanceParser(
        ref int i,
        in ReadOnlySpan<char> segment,
        ref LocalIdParsingState state,
        LocalIdParsingState to
    )
    {
        i += segment.Length + 1;
        state = to;
    }
}
