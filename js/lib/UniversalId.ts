import { Gmod, Codebooks, LocalIdParsingErrorBuilder, LocalId } from ".";
import { Locations } from "./Location";
import { UniversalIdBuilder } from "./UniversalId.Builder";

export class UniversalId {
    private _builder: UniversalIdBuilder;
    private _localId: LocalId;

    public constructor(builder: UniversalIdBuilder) {
        if (!builder.isValid) throw new Error("Invalid UniversalId state");
        this._builder = builder;
        this._localId = builder.localId!.build();
    }

    public get localId(): LocalId {
        return this._localId;
    }

    public get builder(): UniversalIdBuilder {
        return this._builder;
    }

    public get imoNumber() {
        return this._builder.imoNumber!;
    }

    public equals(other?: UniversalId): boolean {
        if (!other) return false;
        return this._builder.equals(other.builder);
    }

    public toString() {
        return this._builder.toString();
    }

    public static parse(
        universalId: string,
        gmod: Gmod,
        codebooks: Codebooks,
        locations: Locations,
        errorBuilder?: LocalIdParsingErrorBuilder
    ) {
        return new UniversalId(
            UniversalIdBuilder.parse(
                universalId,
                gmod,
                codebooks,
                locations,
                errorBuilder
            )
        );
    }

    public static async parseAsync(
        universalIdString: string | undefined,
        errorBuilder?: LocalIdParsingErrorBuilder
    ) {
        return (
            await UniversalIdBuilder.parseAsync(universalIdString, errorBuilder)
        ).build();
    }
}
