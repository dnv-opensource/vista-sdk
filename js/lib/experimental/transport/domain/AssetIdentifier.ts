import { ImoNumber } from "../../../ImoNumber";
import { isNullOrWhiteSpace } from "../../../util/util";

export class AssetIdentifier {
    private readonly _tag: number;
    private readonly _imoNumber?: ImoNumber;
    private readonly _otherId?: string;

    private constructor(value: ImoNumber | string) {
        if (typeof value === "string") {
            this._tag = 2;
            this._otherId = value;
        } else {
            this._tag = 1;
            this._imoNumber = value;
        }
    }

    public get imoNumber() {
        return this._tag === 1 ? this._imoNumber : undefined;
    }

    public get otherId() {
        return this._tag === 2 ? this._otherId : undefined;
    }

    public match<T>(
        onImoNumber: (imoNumber: ImoNumber) => T,
        onOtherId: (otherId: string) => T
    ): T {
        switch (this._tag) {
            case 1:
                return onImoNumber(this._imoNumber!);
            case 2:
                return onOtherId(this._otherId!);
            default:
                throw new Error("Tried to match on invalid ShipId");
        }
    }

    public switch(
        onImoNumber: (imoNumber: ImoNumber) => void,
        onOtherId: (otherId: string) => void
    ): void {
        switch (this._tag) {
            case 1:
                onImoNumber(this._imoNumber!);
                break;
            case 2:
                onOtherId(this._otherId!);
                break;
            default:
                throw new Error("Tried to switch on invalid ShipId");
        }
    }

    public toString(): string {
        switch (this._tag) {
            case 1:
                return this._imoNumber!.toString();
            case 2:
                return this._otherId!;
            default:
                throw new Error("Invalid ShipId");
        }
    }

    public static parse(value?: string): AssetIdentifier {
        if (isNullOrWhiteSpace(value))
            throw new Error(`${AssetIdentifier.name}.parse: value is null`);

        let imo: ImoNumber | undefined = undefined;
        if (value.startsWith("IMO") && (imo = ImoNumber.tryParse(value))) {
            return new AssetIdentifier(imo);
        } else {
            return new AssetIdentifier(value);
        }
    }
}
