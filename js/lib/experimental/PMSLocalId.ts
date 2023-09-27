import { Codebooks } from "../Codebooks";
import { Gmod } from "../Gmod";
import { GmodPath } from "../GmodPath";
import { ILocalIdGeneric } from "../ILocalId";
import { Locations } from "../Location";
import { MetadataTag } from "../MetadataTag";
import { VisVersion } from "../VisVersion";
import { LocalIdParsingErrorBuilder } from "../internal/LocalIdParsingErrorBuilder";
import { PMSLocalIdBuilder } from "./PMSLocalId.Builder";

export class PMSLocalId implements ILocalIdGeneric<PMSLocalId> {
    private readonly _builder: PMSLocalIdBuilder;

    public get visVersion(): VisVersion {
        return this._builder.visVersion!;
    }

    public get verboseMode(): boolean {
        return this._builder.verboseMode;
    }
    public get primaryItem(): GmodPath {
        return this._builder.primaryItem!;
    }
    public get secondaryItem(): GmodPath | undefined {
        return this._builder.secondaryItem;
    }
    public get hasCustomTag(): boolean {
        return this._builder.hasCustomTag;
    }
    public get metadataTags(): MetadataTag[] {
        return this._builder.metadataTags;
    }

    public get builder() {
        return this._builder;
    }

    constructor(builder: PMSLocalIdBuilder) {
        if (builder.isEmpty)
            throw new Error(
                "PMSLocalId cannot be constructed from empty PMSLocalIdBuilder"
            );
        if (!builder.isValid)
            throw new Error(
                "PMSLocalId cannot be constructed from invalid PMSLocalIdBuilder"
            );
        this._builder = builder;
    }

    public toString(): string {
        return this._builder.toString();
    }

    public static parse(
        localIdStr: string | undefined,
        gmod: Gmod,
        codebooks: Codebooks,
        locations: Locations,
        errorBuilder?: LocalIdParsingErrorBuilder
    ): PMSLocalId {
        return PMSLocalIdBuilder.parse(
            localIdStr,
            gmod,
            codebooks,
            locations,
            errorBuilder
        ).build();
    }

    public static parseAsync(
        localIdStr: string,
        errorBuilder?: LocalIdParsingErrorBuilder
    ) {
        return PMSLocalIdBuilder.parseAsync(localIdStr, errorBuilder);
    }
}
