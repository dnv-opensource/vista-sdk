import {
    Gmod,
    Codebooks,
    LocalIdBuilder,
    GmodPath,
    MetadataTag,
    CodebookName,
} from ".";
import { LocalIdErrorBuilder } from "./internal/LocalIdErrorBuilder";
import { ParsingState } from "./types/LocalId";
import { isNullOrWhiteSpace } from "./util/util";

type ParseContext = {
    state: ParsingState;
    i: number;
    segment?: string;
    span: string;
};
export class LocalIdParser {
    public static parse(
        localIdStr: string | undefined,
        gmod: Gmod,
        codebooks: Codebooks,
        errorBuilder?: LocalIdErrorBuilder
    ): LocalIdBuilder {
        const localId = LocalIdParser.tryParse(
            localIdStr,
            gmod,
            codebooks,
            errorBuilder
        );
        if (!localId)
            throw new Error("Couldnt parse local ID from: " + localIdStr);
        return localId;
    }

    public static tryParse(
        localIdStr: string | undefined,
        gmod: Gmod,
        codebooks: Codebooks,
        errorBuilder?: LocalIdErrorBuilder
    ): LocalIdBuilder | undefined {
        if (!localIdStr || isNullOrWhiteSpace(localIdStr))
            throw new Error("Invalid LocalId string");
        if (localIdStr.length === 0) return;
        if (localIdStr.charAt(0) !== "/") {
            if (errorBuilder)
                errorBuilder.push({
                    type: ParsingState.Formatting,
                    message: "Invalid format: missing '/' as first character",
                });
            else return;
        }

        const namingRule: string = LocalIdBuilder.namingRule;
        const visVersion = gmod.visVersion;
        let verbose = false;
        let primaryItem: GmodPath | undefined = undefined;
        let secondaryItem: GmodPath | undefined = undefined;

        let qty: MetadataTag | undefined = undefined;
        let calc: MetadataTag | undefined = undefined;
        let cnt: MetadataTag | undefined = undefined;
        let pos: MetadataTag | undefined = undefined;
        let stateTag: MetadataTag | undefined = undefined;
        let cmd: MetadataTag | undefined = undefined;
        let type: MetadataTag | undefined = undefined;
        let detail: MetadataTag | undefined = undefined;

        let primaryItemStart = -1;
        let secondaryItemStart = -1;

        const context: ParseContext = {
            state: ParsingState.NamingRule,
            i: 1,
            span: localIdStr,
        };

        while (context.state <= ParsingState.MetaDetail) {
            if (context.i >= context.span.length) break;

            const nextSlash = context.span.slice(context.i).indexOf("/");
            context.segment =
                nextSlash === -1
                    ? context.span.slice(context.i)
                    : context.span.slice(context.i, context.i + nextSlash);
            switch (context.state) {
                case ParsingState.NamingRule:
                    if (context.segment !== namingRule) {
                        if (errorBuilder)
                            errorBuilder.push(ParsingState.NamingRule);
                        else return;
                    }

                    this.advanceParser(
                        context,
                        context.i,
                        context.segment,
                        context.state
                    );
                    break;
                case ParsingState.VisVersion:
                    if (!context.segment.startsWith("vis-")) {
                        if (errorBuilder)
                            errorBuilder.push(ParsingState.VisVersion);
                        else return;
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
                        const dashIndex = context.segment.indexOf("-");
                        const code =
                            dashIndex === -1
                                ? context.segment
                                : context.segment.slice(0, dashIndex);
                        if (primaryItemStart === -1) {
                            if (!gmod.tryGetNode(code)) {
                                let nextState: ParsingState = context.state;

                                const [sec, meta] = [
                                    context.segment.startsWith("sec"),
                                    context.segment.startsWith("meta"),
                                ];
                                switch (true) {
                                    case !sec && !meta:
                                        break;
                                    case sec && !meta:
                                        nextState = ParsingState.SecondaryItem;
                                        break;
                                    case !sec && meta:
                                        nextState = ParsingState.MetaQty;
                                        break;
                                }
                                this.advanceParser(
                                    context,
                                    context.i,
                                    context.segment,
                                    context.state,
                                    nextState
                                );
                            } else {
                                primaryItemStart = context.i;
                                this.advanceParser(
                                    context,
                                    context.i,
                                    context.segment
                                );
                            }
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
                                    nextState = ParsingState.MetaQty;
                                    break;
                                case !sec && !meta && tilde:
                                    nextState = ParsingState.ItemDescription;
                                    break;
                            }

                            if (nextState !== context.state) {
                                const path = context.span.slice(
                                    primaryItemStart,
                                    context.i - 1
                                );
                                const gmodPath = gmod.tryParsePath(path);
                                if (!gmodPath) {
                                    if (errorBuilder) {
                                        errorBuilder.push({
                                            type: ParsingState.PrimaryItem,
                                            message:
                                                "Invalid GmodPath: Primary item " +
                                                path,
                                        });
                                        this.advanceParser(
                                            context,
                                            context.i,
                                            context.segment
                                        );
                                        continue;
                                    } else {
                                        return;
                                    }
                                } else {
                                    primaryItem = gmodPath;
                                }

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
                                if (errorBuilder)
                                    errorBuilder.push({
                                        type: ParsingState.PrimaryItem,
                                        message:
                                            "Invalid GmodNode in Primary item: " +
                                            code,
                                    });
                                else return;
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
                        const dashIndex = context.segment.indexOf("-");
                        const code =
                            dashIndex === -1
                                ? context.segment
                                : context.segment.slice(0, dashIndex);
                        if (secondaryItemStart === -1) {
                            if (!gmod.tryGetNode(code)) {
                                if (errorBuilder)
                                    errorBuilder.push({
                                        type: ParsingState.SecondaryItem,
                                        message:
                                            "Invalid GmodNode in Secondary item: " +
                                            code,
                                    });
                                else return;
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
                                    nextState = ParsingState.MetaQty;
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
                                const gmodPath = gmod.tryParsePath(path);
                                if (!gmodPath) {
                                    if (errorBuilder) {
                                        errorBuilder.push({
                                            type: ParsingState.PrimaryItem,
                                            message:
                                                "Invalid GmodPath: Secondary item " +
                                                path,
                                        });
                                        this.advanceParser(
                                            context,
                                            context.i,
                                            context.segment
                                        );
                                        continue;
                                    } else {
                                        return;
                                    }
                                } else {
                                    secondaryItem = gmodPath;
                                }

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
                                if (errorBuilder)
                                    errorBuilder.push({
                                        type: ParsingState.PrimaryItem,
                                        message:
                                            "Invalid GmodNode in Secondary item: " +
                                            code,
                                    });
                                else return;
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
                    verbose = true;

                    const metaStr = "/meta";
                    const metaIndex = context.span.indexOf(metaStr);
                    if (metaIndex === -1) {
                        if (errorBuilder)
                            errorBuilder.push({
                                type: ParsingState.ItemDescription,
                                message: "Could not /meta in string",
                            });
                        else return;
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
                case ParsingState.MetaQty:
                    {
                        const res = this.parseMetatag(
                            CodebookName.Quantity,
                            context,
                            context.state,
                            context.i,
                            context.segment,
                            codebooks,
                            qty,
                            errorBuilder
                        );
                        if (!res) {
                            if (errorBuilder) continue;
                            else return;
                        }
                        qty = res;
                    }
                    break;
                case ParsingState.MetaCnt:
                    {
                        const res = this.parseMetatag(
                            CodebookName.Content,
                            context,
                            context.state,
                            context.i,
                            context.segment,
                            codebooks,
                            cnt,
                            errorBuilder
                        );
                        if (!res) {
                            if (errorBuilder) continue;
                            else return;
                        }
                        cnt = res;
                    }
                    break;
                case ParsingState.MetaCalc:
                    {
                        const res = this.parseMetatag(
                            CodebookName.Calculation,
                            context,
                            context.state,
                            context.i,
                            context.segment,
                            codebooks,
                            calc,
                            errorBuilder
                        );
                        if (!res) {
                            if (errorBuilder) continue;
                            else return;
                        }
                        calc = res;
                    }
                    break;
                case ParsingState.MetaState:
                    {
                        const res = this.parseMetatag(
                            CodebookName.State,
                            context,
                            context.state,
                            context.i,
                            context.segment,
                            codebooks,
                            stateTag,
                            errorBuilder
                        );
                        if (!res) {
                            if (errorBuilder) continue;
                            else return;
                        }
                        stateTag = res;
                    }
                    break;
                case ParsingState.MetaCmd:
                    {
                        const res = this.parseMetatag(
                            CodebookName.Command,
                            context,
                            context.state,
                            context.i,
                            context.segment,
                            codebooks,
                            cmd,
                            errorBuilder
                        );
                        if (!res) {
                            if (errorBuilder) continue;
                            else return;
                        }
                        cmd = res;
                    }
                    break;
                case ParsingState.MetaType:
                    {
                        const res = this.parseMetatag(
                            CodebookName.Type,
                            context,
                            context.state,
                            context.i,
                            context.segment,
                            codebooks,
                            type,
                            errorBuilder
                        );
                        if (!res) {
                            if (errorBuilder) continue;
                            else return;
                        }
                        type = res;
                    }
                    break;
                case ParsingState.MetaPos:
                    {
                        const res = this.parseMetatag(
                            CodebookName.Position,
                            context,
                            context.state,
                            context.i,
                            context.segment,
                            codebooks,
                            pos,
                            errorBuilder
                        );
                        if (!res) {
                            if (errorBuilder) continue;
                            else return;
                        }
                        pos = res;
                    }
                    break;
                case ParsingState.MetaDetail:
                    {
                        const res = this.parseMetatag(
                            CodebookName.Detail,
                            context,
                            context.state,
                            context.i,
                            context.segment,
                            codebooks,
                            detail,
                            errorBuilder
                        );
                        if (!res) {
                            if (errorBuilder) continue;
                            else return;
                        }
                        detail = res;
                    }
                    break;
            }
        }

        const builder = LocalIdBuilder.create(visVersion)
            .withPrimaryItem(primaryItem)
            .withSecondaryItem(secondaryItem)
            .withVerboseMode(verbose)
            .tryWithMetadataTag(qty)
            .tryWithMetadataTag(cnt)
            .tryWithMetadataTag(calc)
            .tryWithMetadataTag(stateTag)
            .tryWithMetadataTag(cmd)
            .tryWithMetadataTag(type)
            .tryWithMetadataTag(pos)
            .tryWithMetadataTag(detail);

        if (builder.isEmptyMetadata) {
            errorBuilder?.push({
                type: ParsingState.EmptyState,
                message: "Missing at least one metadata tag",
            });
        }

        return builder;
    }

    private static parseMetatag(
        codebookName: CodebookName,
        context: ParseContext,
        state: ParsingState,
        i: number,
        segment: string,
        codebooks: Codebooks,
        tag?: MetadataTag,
        errorBuilder?: LocalIdErrorBuilder
    ): MetadataTag | undefined {
        if (!codebooks)
            return;

        let dashIndex = segment.indexOf("-");
        if (dashIndex === -1)
            dashIndex = segment.indexOf('~');
        if (dashIndex === -1)
        {
            if (errorBuilder) {
                errorBuilder.push({
                    type: state,
                    message: 'Invalid metadata tag: missing "-" in ' + segment,
                });
            }
            this.advanceParser(context, i, segment, state);
            return;
        }

        const actualPrefix = segment.slice(0, dashIndex);

        const actualState = this.metaPrefixToState(actualPrefix);

        if (actualState === undefined || actualState < state) {
            if (errorBuilder)
                errorBuilder.push({
                    type: state,
                    message:
                        "Invalid metadata tag: unknown prefix " + actualPrefix,
                });
            this.advanceParser(context, i, segment, state);
            return;
        }

        const value = segment.slice(dashIndex + 1);
        if (value.length === 0) {
            if (errorBuilder)
                errorBuilder.push({
                    type: state,
                    message:
                        "Invalid " +
                        CodebookName[codebookName] +
                        " metadata tag: missing value",
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
            return tag;
        }

        const res = codebooks.tryCreateTag(codebookName, value);
        if (!res) {
            if (errorBuilder)
                errorBuilder.push({
                    type: state,
                    message:
                        "Invalid " +
                        CodebookName[codebookName] +
                        " metadata tag: failed to create",
                });
        }
        this.advanceParser(context, i, segment, state);
        return res;
    }

    static metaPrefixToState(prefix: string): ParsingState | undefined {
        switch (prefix) {
            case "qty":
                return ParsingState.MetaQty;
            case "cnt":
                return ParsingState.MetaCnt;
            case "calc":
                return ParsingState.MetaCalc;
            case "state":
                return ParsingState.MetaState;
            case "cmd":
                return ParsingState.MetaCmd;
            case "pos":
                return ParsingState.MetaPos;
            case "type":
                return ParsingState.MetaType;
            case "detail":
                return ParsingState.MetaDetail;
            default:
                return;
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
