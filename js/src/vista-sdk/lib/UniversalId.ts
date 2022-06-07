import { UniversalIdBuilder } from ".";

export class UniversalId {
    private _builder: UniversalIdBuilder;
    public constructor(builder: UniversalIdBuilder) {
        if (!builder.isValid) throw new Error("Invalid UniversalId state");
        this._builder = builder;
    }

    public get builder(): UniversalIdBuilder {
        return this._builder;
    }

    public get imoNumber() {
        return this._builder.imoNumber;
    }

    public equals(other?: UniversalId): boolean {
        if (!other) return false;
        return this._builder.equals(other.builder);
    }

    public toString() {
        return this._builder.toString();
    }
}
