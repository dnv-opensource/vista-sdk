import { ParsingState } from "../types/LocalId";
import { Error, ErrorBuilder } from "./ErrorBuilder";

export class LocalIdErrorBuilder extends ErrorBuilder<ParsingState> {
    readonly predefinedMessages: Map<ParsingState, string>;

    constructor(errors?: Error<ParsingState>[]) {
        super(errors);
        this.predefinedMessages = this.setPredefinedMessages();
    }

    private setPredefinedMessages(): Map<ParsingState, string> {
        const map = new Map<ParsingState, string>();

        map.set(ParsingState.NamingRule, "Missing or invalid naming rule");
        map.set(ParsingState.VisVersion, "Missing or invalid vis version");
        map.set(ParsingState.PrimaryItem, "Missing or invalid primary item");
        map.set(ParsingState.SecondaryItem, "Invalid secondary item");
        map.set(ParsingState.MetaQty, "Invalid metadata tag: Quantity");
        map.set(ParsingState.MetaCnt, "Invalid metadata tag: Content");
        map.set(ParsingState.MetaCmd, "Invalid metadata tag: Command");
        map.set(ParsingState.MetaPos, "Invalid metadata tag: Position");
        map.set(ParsingState.MetaCalc, "Invalid metadata tag: Calculation");
        map.set(ParsingState.MetaState, "Invalid metadata tag: State");
        map.set(ParsingState.MetaType, "Invalid metadata tag: Type");
        map.set(ParsingState.MetaDetail, "Invalid metadata tag: Detail");
        map.set(ParsingState.EmptyState, "Missing primary path or metadata");

        return map;
    }
}
