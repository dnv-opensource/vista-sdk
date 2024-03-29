import { CodebookName } from "../CodebookName";
import { Codebooks } from "../Codebooks";
import { Gmod } from "../Gmod";
import { GmodPath } from "../GmodPath";
import { Locations } from "../Location";
import { MetadataTag } from "../MetadataTag";
import { VIS } from "../VIS";
import { VisVersions } from "../VisVersion";
import { LocalIdParsingErrorBuilder } from "../internal/LocalIdParsingErrorBuilder";
import { parseVisVersion } from "../internal/Parsing";
import { ParsingState } from "../types/LocalId";
import { isNullOrWhiteSpace } from "../util/util";
import { PMSLocalIdBuilder } from "./PMSLocalId.Builder";

type NextStateIndexTuple = {
    nextStateIndex: number;
    endOfNextStateIndex: number;
};

type ParseContext = {
    state: ParsingState;
    i: number;
    segment?: string;
    span: string;
};

export class PMSLocalIdParser {
    public static parse(
        localIdStr: string | undefined,
        gmod: Gmod,
        codebooks: Codebooks,
        locations: Locations,
        errorBuilder?: LocalIdParsingErrorBuilder
    ): PMSLocalIdBuilder {
        const localId = PMSLocalIdParser.tryParse(
            localIdStr,
            gmod,
            codebooks,
            locations,
            errorBuilder
        );
        if (!localId)
            throw new Error("Couldn't parse local ID from: " + localIdStr);
        return localId;
    }

    public static async parseAsync(
        localIdStr: string | undefined,
        errorBuilder?: LocalIdParsingErrorBuilder
    ): Promise<PMSLocalIdBuilder> {
        const localId = await this.tryParseAsync(localIdStr, errorBuilder);

        if (!localId)
            throw new Error("Couldn't parse local ID from: " + localIdStr);
        return localId;
    }

    public static tryParse(
        localIdStr: string | undefined,
        gmod: Gmod,
        codebooks: Codebooks,
        locations: Locations,
        errorBuilder?: LocalIdParsingErrorBuilder
    ): PMSLocalIdBuilder | undefined {
        if (!localIdStr || isNullOrWhiteSpace(localIdStr))
            throw new Error("Invalid LocalId string");
        if (localIdStr.length === 0) return;
        if (localIdStr.charAt(0) !== "/") {
            errorBuilder?.push({
                type: ParsingState.Formatting,
                message: "Invalid format: missing '/' as first character",
            });
            return;
        }

        const namingRule: string = PMSLocalIdBuilder.namingRule;
        const visVersion = gmod.visVersion;

        let primaryItem: GmodPath | undefined = undefined;
        let secondaryItem: GmodPath | undefined = undefined;

        let qty: MetadataTag | undefined = undefined;
        let cnt: MetadataTag | undefined = undefined;
        let pos: MetadataTag | undefined = undefined;
        let stateTag: MetadataTag | undefined = undefined;
        let cmd: MetadataTag | undefined = undefined;
        let func: MetadataTag | undefined = undefined;
        let maint: MetadataTag | undefined = undefined;
        let act: MetadataTag | undefined = undefined;
        let detail: MetadataTag | undefined = undefined;
        let verbose = false;
        let invalidSecondaryItem = false;

        let primaryItemStart = -1;
        let secondaryItemStart = -1;

        const context: ParseContext = {
            state: ParsingState.NamingRule,
            i: 1,
            span: localIdStr,
        };

        while (context.state <= ParsingState.MetaDetail) {
            const nextStart = Math.min(context.span.length, context.i);
            const nextSlash = context.span.slice(nextStart).indexOf("/");
            context.segment =
                nextSlash === -1
                    ? context.span.slice(nextStart)
                    : context.span.slice(nextStart, nextStart + nextSlash);
            switch (context.state) {
                case ParsingState.NamingRule:
                    if (context.segment.length == 0) {
                        errorBuilder?.push(ParsingState.NamingRule);
                        context.state++;
                        break;
                    }

                    if (context.segment !== namingRule) {
                        errorBuilder?.push(ParsingState.NamingRule);
                        return;
                    }
                    this.advanceParser(
                        context,
                        context.i,
                        context.segment,
                        context.state
                    );
                    break;
                case ParsingState.VisVersion:
                    if (context.segment.length == 0) {
                        errorBuilder?.push(ParsingState.VisVersion);
                        context.state++;
                        break;
                    }

                    if (!context.segment.startsWith("vis-")) {
                        errorBuilder?.push(ParsingState.VisVersion);
                        return;
                    }
                    const version = VisVersions.tryParse(
                        context.segment.slice("vis-".length)
                    );

                    if (!version) {
                        errorBuilder?.push(ParsingState.VisVersion);
                        return;
                    }

                    this.advanceParser(
                        context,
                        context.i,
                        context.segment,
                        context.state
                    );
                    break;
                case ParsingState.PrimaryItem:
                    {
                        if (context.segment.length == 0) {
                            if (primaryItemStart != -1) {
                                if (!gmod) return;

                                const path = context.span.slice(
                                    primaryItemStart,
                                    context.i - 1
                                ); // context.i - 1

                                const gmodPath = gmod.tryParsePath(
                                    path,
                                    locations
                                );
                                if (gmodPath === undefined) {
                                    errorBuilder?.push({
                                        type: ParsingState.PrimaryItem,
                                        message:
                                            "Invalid GmodPath in Primary item: " +
                                            path,
                                    });
                                }
                            } else {
                                errorBuilder?.push(ParsingState.PrimaryItem);
                            }
                            errorBuilder?.push({
                                type: ParsingState.PrimaryItem,
                                message:
                                    "Invalid or missing '/meta' prefix after Primary item",
                            });
                            context.state++;
                            break;
                        }

                        const dashIndex = context.segment.indexOf("-");
                        const code =
                            dashIndex === -1
                                ? context.segment
                                : context.segment.slice(0, dashIndex);
                        if (primaryItemStart === -1) {
                            if (!gmod.tryGetNode(code)) {
                                errorBuilder?.push({
                                    type: ParsingState.PrimaryItem,
                                    message:
                                        "Invalid start GmodNode in Primary item: " +
                                        code,
                                });
                            }
                            primaryItemStart = context.i;
                            this.advanceParser(
                                context,
                                context.i,
                                context.segment
                            );
                        } else {
                            let nextState: ParsingState = context.state;
                            const [sec, meta, tilde] = [
                                context.segment.startsWith("sec"),
                                context.segment.startsWith("meta"),
                                context.segment[0] === "~",
                            ];
                            switch (true) {
                                case !sec && !meta && !tilde:
                                    break;
                                case sec && !meta && !tilde:
                                    nextState = ParsingState.SecondaryItem;
                                    break;
                                case !sec && meta && !tilde:
                                    nextState = ParsingState.MetaQuantity;
                                    break;
                                case !sec && !meta && tilde:
                                    nextState = ParsingState.ItemDescription;
                                    break;
                            }

                            if (nextState !== context.state) {
                                const path = context.span.slice(
                                    primaryItemStart,
                                    context.i - 1
                                ); // context.i - 1

                                const gmodPath = gmod.tryParsePath(
                                    path,
                                    locations
                                );
                                if (gmodPath === undefined) {
                                    // Displays the full GmodPath when first part of PrimaryItem is invalid
                                    errorBuilder?.push({
                                        type: ParsingState.PrimaryItem,
                                        message:
                                            "Invalid GmodPath in Primary item: " +
                                            path,
                                    });

                                    const {
                                        nextStateIndex,
                                        endOfNextStateIndex,
                                    } = this.getNextStateIndexes(
                                        context.span,
                                        context.state
                                    );

                                    context.i = endOfNextStateIndex;
                                    this.advanceParser(
                                        context,
                                        undefined,
                                        undefined,
                                        context.state,
                                        nextState
                                    );
                                    break;
                                }
                                primaryItem = gmodPath;

                                if (context.segment[0] === "~")
                                    this.advanceParser(
                                        context,
                                        undefined,
                                        undefined,
                                        context.state,
                                        nextState
                                    );
                                else
                                    this.advanceParser(
                                        context,
                                        context.i,
                                        context.segment,
                                        context.state,
                                        nextState
                                    );
                                break;
                            }
                            if (!gmod.tryGetNode(code)) {
                                errorBuilder?.push({
                                    type: ParsingState.PrimaryItem,
                                    message:
                                        "Invalid GmodNode in Primary item: " +
                                        code,
                                });

                                const { nextStateIndex, endOfNextStateIndex } =
                                    this.getNextStateIndexes(
                                        context.span,
                                        context.state
                                    );
                                if (nextStateIndex === -1) {
                                    errorBuilder?.push({
                                        type: ParsingState.PrimaryItem,
                                        message:
                                            "Invalid or missing '/meta' prefix after Primary item",
                                    });
                                    return;
                                }
                                const nextSegment = context.span.slice(
                                    nextStateIndex + 1
                                );

                                const [sec, meta, tilde] = [
                                    nextSegment.startsWith("sec"),
                                    nextSegment.startsWith("meta"),
                                    nextSegment[0] === "~",
                                ];
                                switch (true) {
                                    case !sec && !meta && !tilde:
                                        break;
                                    case sec && !meta && !tilde:
                                        nextState = ParsingState.SecondaryItem;
                                        break;
                                    case !sec && meta && !tilde:
                                        nextState = ParsingState.MetaQuantity;
                                        break;
                                    case !sec && !meta && tilde:
                                        nextState =
                                            ParsingState.ItemDescription;
                                        break;
                                }
                                // Displays the invalid middle parts of PrimaryItem and not the whole GmodPath
                                const invalidPrimaryItemPath =
                                    context.span.slice(
                                        context.i,
                                        nextStateIndex
                                    );
                                errorBuilder?.push({
                                    type: ParsingState.PrimaryItem,
                                    message:
                                        "Invalid GmodPath: Last part in Primary item: " +
                                        invalidPrimaryItemPath,
                                });

                                context.i = endOfNextStateIndex;
                                this.advanceParser(
                                    context,
                                    undefined,
                                    undefined,
                                    context.state,
                                    nextState
                                );
                                break;
                            }
                            this.advanceParser(
                                context,
                                context.i,
                                context.segment
                            );
                        }
                    }
                    break;
                case ParsingState.SecondaryItem:
                    {
                        if (context.segment.length == 0) {
                            context.state++;
                            break;
                        }

                        const dashIndex = context.segment.indexOf("-");
                        const code =
                            dashIndex === -1
                                ? context.segment
                                : context.segment.slice(0, dashIndex);
                        if (secondaryItemStart === -1) {
                            if (!gmod.tryGetNode(code)) {
                                errorBuilder?.push({
                                    type: ParsingState.SecondaryItem,
                                    message:
                                        "Invalid start GmodNode in Secondary item: " +
                                        code,
                                });
                            }

                            secondaryItemStart = context.i;
                            this.advanceParser(
                                context,
                                context.i,
                                context.segment
                            );
                        } else {
                            let nextState: ParsingState = context.state;
                            const [meta, tilde] = [
                                context.segment.startsWith("meta"),
                                context.segment[0] === "~",
                            ];

                            switch (true) {
                                case !meta && !tilde:
                                    break;
                                case meta && !tilde:
                                    nextState = ParsingState.MetaQuantity;
                                    break;
                                case !meta && tilde:
                                    nextState = ParsingState.ItemDescription;
                                    break;
                            }

                            if (nextState !== context.state) {
                                const path = context.span.slice(
                                    secondaryItemStart,
                                    context.i - 1
                                );
                                const gmodPath = gmod.tryParsePath(
                                    path,
                                    locations
                                );
                                if (gmodPath === undefined) {
                                    // Displays the full GmodPath when first part of SecondaryItem is invalid
                                    invalidSecondaryItem = true;
                                    errorBuilder?.push({
                                        type: ParsingState.SecondaryItem,
                                        message:
                                            "Invalid GmodPath in Secondary item: " +
                                            path,
                                    });

                                    const {
                                        nextStateIndex,
                                        endOfNextStateIndex,
                                    } = this.getNextStateIndexes(
                                        context.span,
                                        context.state
                                    );
                                    context.i = endOfNextStateIndex;
                                    this.advanceParser(
                                        context,
                                        undefined,
                                        undefined,
                                        context.state,
                                        nextState
                                    );
                                    break;
                                }
                                secondaryItem = gmodPath;

                                if (context.segment[0] === "~")
                                    this.advanceParser(
                                        context,
                                        undefined,
                                        undefined,
                                        context.state,
                                        nextState
                                    );
                                else
                                    this.advanceParser(
                                        context,
                                        context.i,
                                        context.segment,
                                        context.state,
                                        nextState
                                    );

                                break;
                            }
                            if (!gmod.tryGetNode(code)) {
                                errorBuilder?.push({
                                    type: ParsingState.SecondaryItem,
                                    message:
                                        "Invalid GmodNode in Secondary item: " +
                                        code,
                                });

                                const { nextStateIndex, endOfNextStateIndex } =
                                    this.getNextStateIndexes(
                                        context.span,
                                        context.state
                                    );
                                if (nextStateIndex === -1) {
                                    errorBuilder?.push({
                                        type: ParsingState.SecondaryItem,
                                        message:
                                            "Invalid or missing '/meta' prefix after Secondary item",
                                    });
                                    return;
                                }

                                const nextSegment = context.span.slice(
                                    nextStateIndex + 1
                                );
                                const [meta, tilde] = [
                                    nextSegment.startsWith("meta"),
                                    nextSegment[0] === "~",
                                ];
                                switch (true) {
                                    case !meta && !tilde:
                                        break;
                                    case meta && !tilde:
                                        nextState = ParsingState.MetaQuantity;
                                        break;
                                    case !meta && tilde:
                                        nextState =
                                            ParsingState.ItemDescription;
                                        break;
                                }
                                const invalidSecondaryItemPath =
                                    context.span.slice(
                                        context.i,
                                        nextStateIndex
                                    );
                                errorBuilder?.push({
                                    type: ParsingState.SecondaryItem,
                                    message:
                                        "Invalid GmodPath: Last part in Secondary item: " +
                                        invalidSecondaryItemPath,
                                });

                                invalidSecondaryItem = true;
                                context.i = endOfNextStateIndex;
                                this.advanceParser(
                                    context,
                                    undefined,
                                    undefined,
                                    context.state,
                                    nextState
                                );
                                break;
                            }
                            this.advanceParser(
                                context,
                                context.i,
                                context.segment
                            );
                        }
                    }
                    break;
                case ParsingState.ItemDescription:
                    if (context.segment.length == 0) {
                        context.state++;
                        break;
                    }

                    verbose = true;

                    const metaStr = "/meta";

                    const metaIndex = context.span.indexOf(metaStr);
                    if (metaIndex === -1) {
                        errorBuilder?.push(ParsingState.ItemDescription);

                        return;
                    }

                    context.segment = context.span.slice(
                        context.i,
                        metaIndex + metaStr.length
                    );
                    this.advanceParser(
                        context,
                        context.i,
                        context.segment,
                        context.state
                    );
                    break;
                case ParsingState.MetaQuantity:
                    {
                        if (context.segment.length == 0) {
                            context.state++;
                            break;
                        }

                        const res = this.parseMetatag(
                            CodebookName.Quantity,
                            context,
                            context.state,
                            context.i,
                            context.segment,
                            codebooks,
                            errorBuilder,

                            qty
                        );
                        if (!res) continue;
                        qty = res;
                    }
                    break;
                case ParsingState.MetaContent:
                    {
                        if (context.segment.length == 0) {
                            context.state++;
                            break;
                        }

                        const res = this.parseMetatag(
                            CodebookName.Content,
                            context,
                            context.state,
                            context.i,
                            context.segment,
                            codebooks,
                            errorBuilder,
                            cnt
                        );
                        if (!res) continue;
                        cnt = res;
                    }
                    break;
                case ParsingState.MetaState:
                    {
                        if (context.segment.length == 0) {
                            context.state++;
                            break;
                        }

                        const res = this.parseMetatag(
                            CodebookName.State,
                            context,
                            context.state,
                            context.i,
                            context.segment,
                            codebooks,
                            errorBuilder,

                            stateTag
                        );
                        if (!res) continue;
                        stateTag = res;
                    }
                    break;
                case ParsingState.MetaCommand:
                    {
                        if (context.segment.length == 0) {
                            context.state++;
                            break;
                        }

                        const res = this.parseMetatag(
                            CodebookName.Command,
                            context,
                            context.state,
                            context.i,
                            context.segment,
                            codebooks,
                            errorBuilder,

                            cmd
                        );
                        if (!res) continue;
                        cmd = res;
                    }
                    break;
                case ParsingState.MetaFunctionalServices:
                    {
                        if (context.segment.length == 0) {
                            context.state++;
                            break;
                        }

                        const res = this.parseMetatag(
                            CodebookName.FunctionalServices,
                            context,
                            context.state,
                            context.i,
                            context.segment,
                            codebooks,
                            errorBuilder,
                            func
                        );
                        if (!res) continue;
                        func = res;
                    }
                    break;
                case ParsingState.MetaMaintenanceCategory:
                    {
                        if (context.segment.length == 0) {
                            context.state++;
                            break;
                        }

                        const res = this.parseMetatag(
                            CodebookName.MaintenanceCategory,
                            context,
                            context.state,
                            context.i,
                            context.segment,
                            codebooks,
                            errorBuilder,
                            maint
                        );
                        if (!res) continue;
                        maint = res;
                    }
                    break;
                case ParsingState.MetaActivityType:
                    {
                        if (context.segment.length == 0) {
                            context.state++;
                            break;
                        }

                        const res = this.parseMetatag(
                            CodebookName.ActivityType,
                            context,
                            context.state,
                            context.i,
                            context.segment,
                            codebooks,
                            errorBuilder,
                            act
                        );
                        if (!res) continue;
                        act = res;
                    }
                    break;
                case ParsingState.MetaPosition:
                    {
                        if (context.segment.length == 0) {
                            context.state++;
                            break;
                        }

                        const res = this.parseMetatag(
                            CodebookName.Position,
                            context,
                            context.state,
                            context.i,
                            context.segment,
                            codebooks,

                            errorBuilder,
                            pos
                        );
                        if (!res) continue;
                        pos = res;
                    }
                    break;
                case ParsingState.MetaDetail:
                    {
                        if (context.segment.length == 0) {
                            context.state++;
                            break;
                        }

                        const res = this.parseMetatag(
                            CodebookName.Detail,
                            context,
                            context.state,
                            context.i,
                            context.segment,
                            codebooks,
                            errorBuilder,

                            detail
                        );
                        if (!res) continue;
                        detail = res;
                    }
                    break;
                default:
                    this.advanceParser(
                        context,
                        context.i,
                        context.segment,
                        context.state
                    );
                    break;
            }
        }

        const builder = PMSLocalIdBuilder.create(visVersion)
            .tryWithPrimaryItem(primaryItem)
            .tryWithSecondaryItem(secondaryItem)
            .withVerboseMode(verbose)
            .tryWithMetadataTag(qty)
            .tryWithMetadataTag(cnt)
            .tryWithMetadataTag(stateTag)
            .tryWithMetadataTag(cmd)
            .tryWithMetadataTag(func)
            .tryWithMetadataTag(maint)
            .tryWithMetadataTag(act)
            .tryWithMetadataTag(pos)
            .tryWithMetadataTag(detail);

        if (builder.isEmptyMetadata) {
            errorBuilder?.push({
                type: ParsingState.Completeness,
                message:
                    "No metadata tags specified. Local IDs require atleast 1 metadata tag.",
            });
        }

        return !errorBuilder?.hasError && !invalidSecondaryItem
            ? builder
            : undefined;
    }

    public static async tryParseAsync(
        localIdStr: string | undefined,
        errorBuilder?: LocalIdParsingErrorBuilder
    ): Promise<PMSLocalIdBuilder | undefined> {
        const version = parseVisVersion(localIdStr, errorBuilder);
        if (!version) return;

        const gmod = await VIS.instance.getGmod(version);
        const codebooks = await VIS.instance.getCodebooks(version);
        const locations = await VIS.instance.getLocations(version);

        return this.tryParse(
            localIdStr,
            gmod,
            codebooks,
            locations,
            errorBuilder
        );
    }

    private static parseMetatag(
        codebookName: CodebookName,
        context: ParseContext,
        state: ParsingState,
        i: number,
        segment: string,
        codebooks: Codebooks,
        errorBuilder?: LocalIdParsingErrorBuilder,
        tag?: MetadataTag
    ): MetadataTag | undefined {
        if (!codebooks) return;

        const dashIndex = segment.indexOf("-");
        const tildeIndex = segment.indexOf("~");
        const prefixIndex = dashIndex === -1 ? tildeIndex : dashIndex;

        if (prefixIndex === -1) {
            errorBuilder?.push({
                type: state,
                message:
                    "Invalid metadata tag: missing prefix '-' or '~' in " +
                    segment,
            });
            this.advanceParser(context, i, segment, state);
            return;
        }

        const actualPrefix = segment.slice(0, prefixIndex);

        const actualState = this.metaPrefixToState(actualPrefix);

        if (actualState === undefined || actualState < state) {
            errorBuilder?.push({
                type: state,
                message: "Invalid metadata tag: unknown prefix " + actualPrefix,
            });
            this.advanceParser(context, i, segment, state);
            return;
        }

        if (actualState > state) {
            this.advanceParser(
                context,
                undefined,
                undefined,
                state,
                actualState
            );
            return;
        }

        const value = segment.slice(prefixIndex + 1);
        if (value.length === 0) {
            errorBuilder?.push({
                type: state,
                message:
                    "Invalid " +
                    CodebookName[codebookName] +
                    " metadata tag: missing value",
            });
            this.advanceParser(context, i, segment, state);
            return;
        }
        const nextState = PMSLocalIdParser.nextParsingState(actualState);

        tag = codebooks.tryCreateTag(codebookName, value);
        if (tag === undefined) {
            if (prefixIndex === tildeIndex)
                errorBuilder?.push({
                    type: state,
                    message:
                        "Invalid custom " +
                        CodebookName[codebookName] +
                        " metadata tag: failed to create " +
                        value,
                });
            else
                errorBuilder?.push({
                    type: state,
                    message:
                        "Invalid " +
                        CodebookName[codebookName] +
                        " metadata tag: failed to create " +
                        value,
                });
            this.advanceParser(context, i, segment, state);
            return;
        }
        if (prefixIndex === dashIndex && tag.prefix === "~")
            errorBuilder?.push({
                type: state,
                message:
                    "Invalid " +
                    CodebookName[codebookName] +
                    " metadata tag: '" +
                    value +
                    "'. Use prefix '~' for custom values",
            });

        if (nextState === undefined)
            this.advanceParser(context, i, segment, state);
        else this.advanceParser(context, i, segment, state, nextState);

        return tag;
    }

    static metaPrefixToState(prefix: string): ParsingState | undefined {
        switch (prefix) {
            case "qty":
                return ParsingState.MetaQuantity;
            case "cnt":
                return ParsingState.MetaContent;

            case "state":
                return ParsingState.MetaState;
            case "cmd":
                return ParsingState.MetaCommand;
            case "funct.svc":
                return ParsingState.MetaFunctionalServices;
            case "maint.cat":
                return ParsingState.MetaMaintenanceCategory;
            case "act.type":
                return ParsingState.MetaActivityType;
            case "pos":
                return ParsingState.MetaPosition;
            case "detail":
                return ParsingState.MetaDetail;
            default:
                return;
        }
    }

    static nextParsingState(prev: ParsingState): ParsingState | undefined {
        switch (prev) {
            case ParsingState.MetaQuantity:
                return ParsingState.MetaContent;
            case ParsingState.MetaContent:
                return ParsingState.MetaState;
            case ParsingState.MetaState:
                return ParsingState.MetaCommand;
            case ParsingState.MetaCommand:
                return ParsingState.MetaFunctionalServices;
            case ParsingState.MetaFunctionalServices:
                return ParsingState.MetaMaintenanceCategory;
            case ParsingState.MetaMaintenanceCategory:
                return ParsingState.MetaActivityType;
            case ParsingState.MetaActivityType:
                return ParsingState.MetaPosition;
            case ParsingState.MetaPosition:
                return ParsingState.MetaDetail;
        }
    }

    static getNextStateIndexes(
        span: string,
        state: ParsingState
    ): NextStateIndexTuple {
        const customIndex = span.indexOf("~");
        const endOfCustomIndex = customIndex + "~".length + 1;

        const metaIndex = span.indexOf("/meta");
        const endOfMetaIndex = metaIndex + "/meta".length + 1;

        switch (state) {
            case ParsingState.PrimaryItem: {
                const secIndex = span.indexOf("/sec");
                const endOfSecIndex = secIndex + "/sec".length + 1;
                return secIndex != -1
                    ? {
                          nextStateIndex: secIndex,
                          endOfNextStateIndex: endOfSecIndex,
                      }
                    : customIndex != -1
                    ? {
                          nextStateIndex: customIndex,
                          endOfNextStateIndex: endOfCustomIndex,
                      }
                    : {
                          nextStateIndex: metaIndex,
                          endOfNextStateIndex: endOfMetaIndex,
                      };
            }
            case ParsingState.SecondaryItem:
                return customIndex != -1
                    ? {
                          nextStateIndex: customIndex,
                          endOfNextStateIndex: endOfCustomIndex,
                      }
                    : {
                          nextStateIndex: metaIndex,
                          endOfNextStateIndex: endOfMetaIndex,
                      };
            default:
                return {
                    nextStateIndex: metaIndex,
                    endOfNextStateIndex: endOfMetaIndex,
                };
        }
    }

    static advanceParser(
        context: ParseContext,
        i?: number,
        segment?: string,
        state?: ParsingState,
        to?: ParsingState
    ) {
        if (i !== undefined && segment !== undefined) {
            context.i += segment.length + 1;
        }

        if (state !== undefined && to !== undefined) {
            context.state = to;
            return;
        }

        if (state !== undefined) {
            context.state++;
            return;
        }
    }
}
