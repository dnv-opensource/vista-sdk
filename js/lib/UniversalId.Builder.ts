import { Codebooks, Gmod, UniversalId } from ".";
import { ImoNumber } from "./ImoNumber";
import { LocalIdParsingErrorBuilder } from "./internal/LocalIdParsingErrorBuilder";
import { LocalIdBuilder } from "./LocalId.Builder";
import { Locations } from "./Location";
import { ParsingState } from "./types/LocalId";
import { UniversalIdParser } from "./UniversalId.Parsing";
import { VisVersion } from "./VisVersion";

export class UniversalIdBuilder {
    public static readonly namingEntity = "data.dnv.com";
    private _localId?: LocalIdBuilder;
    public imoNumber?: ImoNumber;

    public get localId() {
        return this._localId;
    }

    public isValid(locations: Locations): boolean {
        return !!this.imoNumber?.isValid && !!this.localId?.isValid(locations);
    }

    public static create(visVersion: VisVersion) {
        return new UniversalIdBuilder().withLocalId(
            LocalIdBuilder.create(visVersion)
        );
    }

    public equals(other?: UniversalIdBuilder): boolean {
        if (!other) return false;
        return (
            this.imoNumber === other.imoNumber &&
            !!this.localId?.equals(other.localId)
        );
    }

    public clone() {
        return Object.assign(new UniversalIdBuilder(), this);
    }

    public validate(
        locations: Locations,
        errorBuilder = new LocalIdParsingErrorBuilder()
    ): LocalIdParsingErrorBuilder {
        if (!this.imoNumber?.isValid) {
            errorBuilder.push({
                type: ParsingState.EmptyState,
                message: "Missing or invalid IMO number",
            });
        }
        this._localId?.validate(locations, errorBuilder);
        return errorBuilder;
    }

    public static parse(
        universalId: string,
        gmod: Gmod,
        codebooks: Codebooks,
        locations: Locations,
        errorBuilder?: LocalIdParsingErrorBuilder
    ) {
        return UniversalIdParser.parse(
            universalId,
            gmod,
            codebooks,
            locations,
            errorBuilder
        );
    }

    public static async parseAsync(
        universalIdString: string | undefined,
        errorBuilder?: LocalIdParsingErrorBuilder
    ) {
        return UniversalIdParser.parseAsync(universalIdString, errorBuilder);
    }

    public static tryParse(
        universalId: string,
        gmod: Gmod,
        codebooks: Codebooks,
        locations: Locations,
        errorBuilder?: LocalIdParsingErrorBuilder
    ): UniversalIdBuilder | undefined {
        return UniversalIdParser.tryParse(
            universalId,
            gmod,
            codebooks,
            locations,
            errorBuilder
        );
    }

    public static async tryParseAsync(
        universalIdString: string | undefined,
        errorBuilder?: LocalIdParsingErrorBuilder
    ) {
        return UniversalIdParser.tryParseAsync(universalIdString, errorBuilder);
    }

    public build(): UniversalId {
        return new UniversalId(this);
    }

    public toString() {
        const builder: string[] = [];

        if (!this.imoNumber)
            throw new Error("Invalid Universal Id state: Missing IMO Number");
        if (!this.localId)
            throw new Error("Invalid Universal Id state: Missing LocalId");

        builder.push(UniversalIdBuilder.namingEntity);
        builder.push("/");
        builder.push(this.imoNumber.toString());
        this.localId.toString(builder);

        return builder.join("");
    }

    public withLocalId(localId?: LocalIdBuilder) {
        return this.with((s) => (s._localId = localId));
    }

    public withImoNumber(imoNumber?: ImoNumber) {
        return this.with((s) => (s.imoNumber = imoNumber));
    }

    public with(u: (state: UniversalIdBuilder) => void): UniversalIdBuilder {
        const n = this.clone();
        u && u(n);
        return n;
    }
}
