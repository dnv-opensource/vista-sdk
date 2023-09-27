using System.Diagnostics.CodeAnalysis;
using Vista.SDK.Internal;

namespace Vista.SDK.Experimental;

internal enum PMSLocalIdParsingState
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
    MetaFunctionalServices,
    MetaMaintenanceCategory,
    MetaActivityType,
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

partial record class PMSLocalIdBuilder
{
    public static PMSLocalIdBuilder Parse(string localIdStr)
    {
        if (!TryParse(localIdStr, out var errors, out var localId))
            throw new ArgumentException($"Couldn't parse local ID from: '{localIdStr}'. {errors}");
        return localId;
    }

    public static bool TryParse(string localIdStr, [MaybeNullWhen(false)] out PMSLocalIdBuilder localId)
    {
        return TryParse(localIdStr, out _, out localId);
    }

    public static bool TryParse(
        string localIdStr,
        out ParsingErrors errors,
        [MaybeNullWhen(false)] out PMSLocalIdBuilder localId
    )
    {
        var errorBuilder = PMSLocalIdParsingErrorBuilder.Empty;

        var result = TryParseInternal(localIdStr, ref errorBuilder, out localId);
        errors = errorBuilder.Build();

        return result;
    }

    internal static bool TryParseInternal(
        string localIdStr,
        ref PMSLocalIdParsingErrorBuilder errorBuilder,
        [MaybeNullWhen(false)] out PMSLocalIdBuilder localId
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
                PMSLocalIdParsingState.Formatting,
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
        MetadataTag? stateTag = null;
        MetadataTag? cmd = null;
        MetadataTag? func = null;
        MetadataTag? maint = null;
        MetadataTag? act = null;
        MetadataTag? pos = null;
        MetadataTag? detail = null;
        bool verbose = false;
        string? predefinedMessage = null;
        bool invalidSecondaryItem = false;

        var primaryItemStart = -1;
        var secondaryItemStart = -1;

        var state = PMSLocalIdParsingState.NamingRule;
        int i = 1;
        while (state <= PMSLocalIdParsingState.MetaDetail)
        {
            var nextStart = Math.Min(span.Length, i);
            var nextSlash = span.Slice(nextStart).IndexOf('/');
            var segment = nextSlash == -1 ? span.Slice(nextStart) : span.Slice(nextStart, nextSlash);
            switch (state)
            {
                case PMSLocalIdParsingState.NamingRule:
                    if (segment.Length == 0)
                    {
                        AddError(ref errorBuilder, PMSLocalIdParsingState.NamingRule, predefinedMessage);
                        state++;
                        break;
                    }

                    if (!segment.SequenceEqual(NamingRule.AsSpan()))
                    {
                        AddError(ref errorBuilder, PMSLocalIdParsingState.NamingRule, predefinedMessage);
                        return false;
                    }
                    AdvanceParser(ref i, in segment, ref state);
                    break;
                case PMSLocalIdParsingState.VisVersion:
                    if (segment.Length == 0)
                    {
                        AddError(ref errorBuilder, PMSLocalIdParsingState.VisVersion, predefinedMessage);
                        state++;
                        break;
                    }

                    if (!segment.StartsWith("vis-".AsSpan()))
                    {
                        AddError(ref errorBuilder, PMSLocalIdParsingState.VisVersion, predefinedMessage);
                        return false;
                    }

                    if (!VisVersions.TryParse(segment.Slice("vis-".Length), out visVersion))
                    {
                        AddError(ref errorBuilder, PMSLocalIdParsingState.VisVersion, predefinedMessage);
                        return false;
                    }

                    gmod = VIS.Instance.GetGmod(visVersion);
                    codebooks = VIS.Instance.GetCodebooks(visVersion);
                    if (gmod is null || codebooks is null)
                        return false;

                    AdvanceParser(ref i, in segment, ref state);
                    break;
                case PMSLocalIdParsingState.PrimaryItem:

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
                                        PMSLocalIdParsingState.PrimaryItem,
                                        $"Invalid GmodPath in Primary item: {path.ToString()}"
                                    );
                                }
                            }
                            else
                            {
                                AddError(ref errorBuilder, PMSLocalIdParsingState.PrimaryItem, predefinedMessage);
                            }
                            AddError(
                                ref errorBuilder,
                                PMSLocalIdParsingState.PrimaryItem,
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
                                    PMSLocalIdParsingState.PrimaryItem,
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
                                (true, false, false) => PMSLocalIdParsingState.SecondaryItem,
                                (false, true, false) => PMSLocalIdParsingState.MetaQuantity,
                                (false, false, true) => PMSLocalIdParsingState.ItemDescription,
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
                                        PMSLocalIdParsingState.PrimaryItem,
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
                                    PMSLocalIdParsingState.PrimaryItem,
                                    $"Invalid GmodNode in Primary item: {code.ToString()}"
                                );
                                (var nextStateIndex, var endOfNextStateIndex) = GetNextStateIndexes(span, state);

                                if (nextStateIndex == -1)
                                {
                                    AddError(
                                        ref errorBuilder,
                                        PMSLocalIdParsingState.PrimaryItem,
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
                                    (true, false, false) => PMSLocalIdParsingState.SecondaryItem,
                                    (false, true, false) => PMSLocalIdParsingState.MetaQuantity,
                                    (false, false, true) => PMSLocalIdParsingState.ItemDescription,
                                    _ => throw new Exception("Inconsistent parsing state"),
                                };

                                // Displays the invalid middle parts of PrimaryItem and not the whole GmodPath
                                var invalidPrimaryItemPath = span.Slice(i, nextStateIndex - i);

                                AddError(
                                    ref errorBuilder,
                                    PMSLocalIdParsingState.PrimaryItem,
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
                case PMSLocalIdParsingState.SecondaryItem:

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
                                    PMSLocalIdParsingState.SecondaryItem,
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
                                (true, false) => PMSLocalIdParsingState.MetaQuantity,
                                (false, true) => PMSLocalIdParsingState.ItemDescription,
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
                                        PMSLocalIdParsingState.SecondaryItem,
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
                                    PMSLocalIdParsingState.SecondaryItem,
                                    $"Invalid GmodNode in Secondary item: {code.ToString()}"
                                );

                                (var nextStateIndex, var endOfNextStateIndex) = GetNextStateIndexes(span, state);
                                if (nextStateIndex == -1)
                                {
                                    AddError(
                                        ref errorBuilder,
                                        PMSLocalIdParsingState.SecondaryItem,
                                        "Invalid or missing '/meta' prefix after Secondary item"
                                    );
                                    return false;
                                }

                                var nextSegment = span.Slice(nextStateIndex + 1);

                                nextState = (nextSegment.StartsWith("meta".AsSpan()), nextSegment[0] == '~') switch
                                {
                                    (true, false) => PMSLocalIdParsingState.MetaQuantity,
                                    (false, true) => PMSLocalIdParsingState.ItemDescription,
                                    _ => throw new Exception("Inconsistent parsing state"),
                                };

                                var invalidSecondaryItemPath = span.Slice(i, nextStateIndex - i);

                                AddError(
                                    ref errorBuilder,
                                    PMSLocalIdParsingState.SecondaryItem,
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
                case PMSLocalIdParsingState.ItemDescription:
                    if (segment.Length == 0)
                    {
                        state++;
                        break;
                    }

                    verbose = true;

                    var metaIndex = span.IndexOf("/meta".AsSpan());
                    if (metaIndex == -1)
                    {
                        AddError(ref errorBuilder, PMSLocalIdParsingState.ItemDescription, predefinedMessage);
                        return false;
                    }

                    segment = span.Slice(i, (metaIndex + "/meta".Length) - i);

                    AdvanceParser(ref i, in segment, ref state);
                    break;
                case PMSLocalIdParsingState.MetaQuantity:

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
                case PMSLocalIdParsingState.MetaContent:

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
                case PMSLocalIdParsingState.MetaState:

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
                case PMSLocalIdParsingState.MetaCommand:

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
                case PMSLocalIdParsingState.MetaFunctionalServices:

                    {
                        if (segment.Length == 0)
                        {
                            state++;
                            break;
                        }

                        var result = ParseMetatag(
                            CodebookName.FunctionalServices,
                            ref state,
                            ref i,
                            in segment,
                            ref func,
                            codebooks,
                            ref errorBuilder
                        );
                        if (!result)
                            return false;
                    }
                    break;
                case PMSLocalIdParsingState.MetaMaintenanceCategory:

                    {
                        if (segment.Length == 0)
                        {
                            state++;
                            break;
                        }

                        var result = ParseMetatag(
                            CodebookName.MaintenanceCategory,
                            ref state,
                            ref i,
                            in segment,
                            ref maint,
                            codebooks,
                            ref errorBuilder
                        );
                        if (!result)
                            return false;
                    }
                    break;
                case PMSLocalIdParsingState.MetaActivityType:

                    {
                        if (segment.Length == 0)
                        {
                            state++;
                            break;
                        }

                        var result = ParseMetatag(
                            CodebookName.ActivityType,
                            ref state,
                            ref i,
                            in segment,
                            ref act,
                            codebooks,
                            ref errorBuilder
                        );
                        if (!result)
                            return false;
                    }
                    break;
                case PMSLocalIdParsingState.MetaPosition:

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
                case PMSLocalIdParsingState.MetaDetail:

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
            .TryWithMetadataTag(in stateTag)
            .TryWithMetadataTag(in cmd)
            .TryWithMetadataTag(in func)
            .TryWithMetadataTag(in maint)
            .TryWithMetadataTag(in act)
            .TryWithMetadataTag(in pos)
            .TryWithMetadataTag(in detail);

        if (localId.IsEmptyMetadata)
        {
            AddError(
                ref errorBuilder,
                PMSLocalIdParsingState.Completeness,
                "No metadata tags specified. Local IDs require atleast 1 metadata tag."
            );
        }

        return (!errorBuilder.HasError && !invalidSecondaryItem);

        static bool ParseMetatag(
            CodebookName codebookName,
            ref PMSLocalIdParsingState state,
            ref int i,
            in ReadOnlySpan<char> segment,
            ref MetadataTag? tag,
            Codebooks? codebooks,
            ref PMSLocalIdParsingErrorBuilder errorBuilder
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

        static PMSLocalIdParsingState? MetaPrefixToState(ReadOnlySpan<char> prefix)
        {
            if (prefix.SequenceEqual("qty".AsSpan()))
                return PMSLocalIdParsingState.MetaQuantity;
            if (prefix.SequenceEqual("cnt".AsSpan()))
                return PMSLocalIdParsingState.MetaContent;
            if (prefix.SequenceEqual("state".AsSpan()))
                return PMSLocalIdParsingState.MetaState;
            if (prefix.SequenceEqual("cmd".AsSpan()))
                return PMSLocalIdParsingState.MetaCommand;
            if (prefix.SequenceEqual("funct.svc".AsSpan()))
                return PMSLocalIdParsingState.MetaFunctionalServices;
            if (prefix.SequenceEqual("maint.cat".AsSpan()))
                return PMSLocalIdParsingState.MetaMaintenanceCategory;
            if (prefix.SequenceEqual("act.type".AsSpan()))
                return PMSLocalIdParsingState.MetaActivityType;
            if (prefix.SequenceEqual("pos".AsSpan()))
                return PMSLocalIdParsingState.MetaPosition;
            if (prefix.SequenceEqual("detail".AsSpan()))
                return PMSLocalIdParsingState.MetaDetail;

            return null;
        }

        static PMSLocalIdParsingState? NextParsingState(PMSLocalIdParsingState prev) =>
            prev switch
            {
                PMSLocalIdParsingState.MetaQuantity => PMSLocalIdParsingState.MetaContent,
                PMSLocalIdParsingState.MetaContent => PMSLocalIdParsingState.MetaState,
                PMSLocalIdParsingState.MetaState => PMSLocalIdParsingState.MetaCommand,
                PMSLocalIdParsingState.MetaCommand => PMSLocalIdParsingState.MetaFunctionalServices,
                PMSLocalIdParsingState.MetaFunctionalServices => PMSLocalIdParsingState.MetaMaintenanceCategory,
                PMSLocalIdParsingState.MetaMaintenanceCategory => PMSLocalIdParsingState.MetaActivityType,
                PMSLocalIdParsingState.MetaActivityType => PMSLocalIdParsingState.MetaPosition,
                PMSLocalIdParsingState.MetaPosition => PMSLocalIdParsingState.MetaDetail,
                _ => null
            };

        static void AddError(
            ref PMSLocalIdParsingErrorBuilder errorBuilder,
            PMSLocalIdParsingState state,
            string? message
        )
        {
            if (!errorBuilder.HasError)
                errorBuilder = PMSLocalIdParsingErrorBuilder.Create();

            errorBuilder.AddError(state, message);
        }

        static (int NextIndex, int EndOfNextStateIndex) GetNextStateIndexes(
            ReadOnlySpan<char> span,
            PMSLocalIdParsingState state
        )
        {
            var customIndex = span.IndexOf("~".AsSpan());
            var endOfCustomIndex = (customIndex + "~".Length + 1);

            var metaIndex = span.IndexOf("/meta".AsSpan());
            var endOfMetaIndex = (metaIndex + "/meta".Length + 1);

            switch (state)
            {
                case (PMSLocalIdParsingState.PrimaryItem):
                {
                    var secIndex = span.IndexOf("/sec".AsSpan());
                    var endOfSecIndex = (secIndex + "/sec".Length + 1);
                    return secIndex != -1
                        ? (secIndex, endOfSecIndex)
                        : customIndex != -1
                            ? (customIndex, endOfCustomIndex)
                            : (metaIndex, endOfMetaIndex);
                }

                case (PMSLocalIdParsingState.SecondaryItem):
                    return customIndex != -1 ? (customIndex, endOfCustomIndex) : (metaIndex, endOfMetaIndex);

                default:
                    return (metaIndex, endOfMetaIndex);
            }
        }
    }

    static void AdvanceParser(ref int i, in ReadOnlySpan<char> segment, ref PMSLocalIdParsingState state)
    {
        state++;
        i += segment.Length + 1;
    }

    static void AdvanceParser(ref int i, in ReadOnlySpan<char> segment) => i += segment.Length + 1;

    static void AdvanceParser(ref PMSLocalIdParsingState state, PMSLocalIdParsingState to) => state = to;

    static void AdvanceParser(
        ref int i,
        in ReadOnlySpan<char> segment,
        ref PMSLocalIdParsingState state,
        PMSLocalIdParsingState to
    )
    {
        i += segment.Length + 1;
        state = to;
    }
}
