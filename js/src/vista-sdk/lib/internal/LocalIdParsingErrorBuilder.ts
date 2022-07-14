import { ParsingState } from "../types/LocalId";
import { Error, ErrorBuilder } from "./ErrorBuilder";

export class LocalIdParsingErrorBuilder extends ErrorBuilder<ParsingState> {
    readonly predefinedMessages: Map<ParsingState, string>;

    public static readonly Empty : LocalIdParsingErrorBuilder = new LocalIdParsingErrorBuilder();

    public static Create() : LocalIdParsingErrorBuilder{
        return new LocalIdParsingErrorBuilder();
    }
    constructor(errors?: Error<ParsingState>[]) {
        super(errors);
        this.predefinedMessages = this.setPredefinedMessages();
    }

    public get hasError(): boolean {
        return this.errors.length > 0;
    }

    private setPredefinedMessages(): Map<ParsingState, string> {
        const map = new Map<ParsingState, string>();

        map.set(ParsingState.NamingRule, "Missing or invalid naming rule");
        map.set(ParsingState.VisVersion, "Missing or invalid vis version");
        map.set(ParsingState.PrimaryItem, "Missing or invalid primary item");
        map.set(ParsingState.SecondaryItem, "Invalid secondary item");
        map.set(ParsingState.ItemDescription, "Missing or invalid /meta prefix");
        map.set(ParsingState.MetaQuantity, "Invalid metadata tag: Quantity");
        map.set(ParsingState.MetaContent, "Invalid metadata tag: Content");
        map.set(ParsingState.MetaCommand, "Invalid metadata tag: Command");
        map.set(ParsingState.MetaPosition, "Invalid metadata tag: Position");
        map.set(ParsingState.MetaCalculation, "Invalid metadata tag: Calculation");
        map.set(ParsingState.MetaState, "Invalid metadata tag: State");
        map.set(ParsingState.MetaType, "Invalid metadata tag: Type");
        map.set(ParsingState.MetaDetail, "Invalid metadata tag: Detail");
        map.set(ParsingState.EmptyState, "Missing primary path or metadata");

        return map;
    }
}
