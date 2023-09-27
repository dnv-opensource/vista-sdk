import { CodebookName } from "./CodebookName";
import { Codebooks } from "./Codebooks";
import { Gmod } from "./Gmod";
import { GmodPath } from "./GmodPath";
import { ILocalIdGeneric } from "./ILocalId";
import { LocalIdParsingErrorBuilder } from "./internal/LocalIdParsingErrorBuilder";
import { LocalIdBuilder } from "./LocalId.Builder";
import { LocalIdParser } from "./LocalId.Parsing";
import { Locations } from "./Location";
import { MetadataTag } from "./MetadataTag";

export class LocalId implements ILocalIdGeneric<LocalId> {
    protected _builder: LocalIdBuilder;

    public constructor(builder: LocalIdBuilder) {
        if (!builder.isValid)
            throw new Error("Invalid LocalId state:" + builder.toString());
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

    public get hasCustomTag(): boolean {
        return this._builder.hasCustomTag;
    }

    public get visVersion() {
        return this._builder.visVersion!;
    }

    public get primaryItem(): GmodPath {
        return this._builder.primaryItem!;
    }

    public get secondaryItem() {
        return this._builder.secondaryItem;
    }

    public get builder() {
        return this._builder;
    }

    public get metadataTags(): MetadataTag[] {
        return this._builder.metadataTags;
    }

    public static parse(
        localIdStr: string,
        gmod: Gmod,
        codebooks: Codebooks,
        locations: Locations,
        errorBuilder?: LocalIdParsingErrorBuilder
    ) {
        return LocalIdParser.parse(
            localIdStr,
            gmod,
            codebooks,
            locations,
            errorBuilder
        ).build();
    }

    public static async parseAsync(
        localIdString: string | undefined,
        errorBuilder?: LocalIdParsingErrorBuilder
    ) {
        return (
            await LocalIdParser.parseAsync(localIdString, errorBuilder)
        ).build();
    }
}
