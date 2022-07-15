import { ImoNumber } from "./ImoNumber";
import { LocalIdParsingErrorBuilder } from "./internal/LocalIdParsingErrorBuilder";
import { LocalIdBuilder } from "./LocalId.Builder";
import { ParsingState } from "./types/LocalId";
import { VisVersion } from "./VisVersion";

export class UniversalIdBuilder {
    public readonly namingEntity = "data.dnv.com";
    private _localId?: LocalIdBuilder;
    public imoNumber?: ImoNumber;

    public get localId() {
        return this._localId;
    }

    public get isValid(): boolean {
        return !!this.imoNumber?.isValid && !!this.localId?.isValid;
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
        errorBuilder = new LocalIdParsingErrorBuilder()
    ): LocalIdParsingErrorBuilder {
        if (!this.imoNumber?.isValid) {
            errorBuilder.push({
                type: ParsingState.EmptyState,
                message: "Missing or invalid IMO number",
            });
        }
        this._localId?.validate(errorBuilder);
        return errorBuilder;
    }

    public toString() {
        const builder: string[] = [];

        if (!this.imoNumber)
            throw new Error("Invalid Universal Id state: Missing IMO Number");
        if (!this.localId)
            throw new Error("Invalid Universal Id state: Missing LocalId");

        builder.push(this.namingEntity);
        builder.push("/");
        builder.push("IMO");
        builder.push(this.imoNumber.toString());
        builder.push("/");
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
