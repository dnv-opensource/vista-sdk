import { LocalIdBuilder, CodebookName, MetadataTag, Gmod, Codebooks } from ".";
import { LocalIdParsingErrorBuilder } from "./internal/LocalIdParsingErrorBuilder";
import { LocalIdParser } from "./LocalId.Parsing";

export class LocalId {
    protected _builder: LocalIdBuilder;

    public constructor(builder: LocalIdBuilder) {
        if (!builder.isValid) throw new Error("Invalid LocalId state");
        this._builder = builder;
    }

    public equals(other?: LocalId) {
        if (!other) return false;
        return this._builder.equals(other.builder);
    }

    public toString() {
        return this._builder.toString();
    }

    public getMetadataTag(name: CodebookName): MetadataTag | undefined {
        return this._builder.getMetadataTag(name);
    }

    public get verboseMode() {
        return this._builder.verboseMode;
    }

    public get primaryItem() {
        return this._builder.primaryItem;
    }

    public get secondaryItem() {
        return this._builder.secondaryItem;
    }

    public get builder() {
        return this._builder;
    }

public static parse(
        localIdStr: string,
        gmod: Gmod,
        codebooks: Codebooks,
        errorBuilder?: LocalIdParsingErrorBuilder
    ) {
        return LocalIdParser.parse(
            localIdStr,
            gmod,
            codebooks,
            errorBuilder
        ).build();
    }
}
