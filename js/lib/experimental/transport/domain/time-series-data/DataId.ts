import { Codebooks } from "../../../../Codebooks";
import { Gmod } from "../../../../Gmod";
import { LocalId } from "../../../../LocalId";
import { LocalIdBuilder } from "../../../../LocalId.Builder";
import { Locations } from "../../../../Location";
import { LocalIdParsingErrorBuilder } from "../../../../internal/LocalIdParsingErrorBuilder";
import { isNullOrWhiteSpace } from "../../../../util/util";
import { PMSLocalId } from "../../../PMSLocalId";
import { PMSLocalIdBuilder } from "../../../PMSLocalId.Builder";

export class DataId {
    private readonly _tag: number;
    private readonly _localId?: LocalId;
    private readonly _pmsLocalId?: PMSLocalId;
    private readonly _shortId?: string;

    public constructor(value: LocalId | PMSLocalId | string) {
        if (value instanceof LocalId) {
            this._tag = 1;
            this._localId = value;
        } else if (value instanceof PMSLocalId) {
            this._tag = 2;
            this._pmsLocalId = value;
        } else {
            this._tag = 3;
            this._shortId = value;
        }
    }

    public get isLocalId(): boolean {
        return this._tag == 1;
    }

    public get isPMSLocalId(): boolean {
        return this._tag == 2;
    }

    public get isShortId(): boolean {
        return this._tag == 3;
    }

    public get localId() {
        return this._tag === 1 ? this._localId : undefined;
    }

    public get pmsLocalId() {
        return this._tag === 2 ? this._pmsLocalId : undefined;
    }

    public get shortId() {
        return this._tag === 3 ? this._shortId : undefined;
    }

    public match<T>(
        onLocalId: (localId: LocalId) => T,
        onPMSLocalId: (pmsLocalId: PMSLocalId) => T,
        onShortId: (shortId: string) => T
    ): T {
        switch (this._tag) {
            case 1:
                return onLocalId(this._localId!);
            case 2:
                return onPMSLocalId(this._pmsLocalId!);
            case 3:
                return onShortId(this._shortId!);
            default:
                throw new Error("Tried to match on invalid DataChannelId");
        }
    }

    public switch(
        onLocalId: (localId: LocalId) => void,
        onPMSLocalId: (pmsLocalId: PMSLocalId) => void,
        onShortId: (shortId: string) => void
    ): void {
        switch (this._tag) {
            case 1:
                onLocalId(this._localId!);
                break;
            case 2:
                onPMSLocalId(this._pmsLocalId!);
                break;
            case 3:
                onShortId(this._shortId!);
                break;
            default:
                throw new Error("Tried to switch on invalid DataChannelId");
        }
    }

    public toString(): string {
        switch (this._tag) {
            case 1:
                return this._localId!.toString();
            case 2:
                return this._pmsLocalId!.toString();
            case 3:
                return this._shortId!;
            default:
                throw new Error("Invalid DataChannelId");
        }
    }

    public static parse(
        value: string | undefined,
        gmod: Gmod,
        codebooks: Codebooks,
        locations: Locations,
        errorBuilder?: LocalIdParsingErrorBuilder
    ): DataId {
        if (isNullOrWhiteSpace(value))
            throw new Error(`${this.parse.name}: value is null`);

        let localId: LocalIdBuilder | undefined = undefined;
        let pmsLocalId: PMSLocalIdBuilder | undefined = undefined;

        if (
            !!(localId = LocalIdBuilder.tryParse(
                value,
                gmod,
                codebooks,
                locations,
                errorBuilder
            ))
        ) {
            return new DataId(localId.build());
        } else if (
            !!(pmsLocalId = PMSLocalIdBuilder.tryParse(
                value,
                gmod,
                codebooks,
                locations,
                errorBuilder
            ))
        ) {
            return new DataId(pmsLocalId.build());
        } else {
            return new DataId(value);
        }
    }

    public static async parseAsync(
        value: string | undefined,
        errorBuilder?: LocalIdParsingErrorBuilder
    ) {
        if (isNullOrWhiteSpace(value))
            throw new Error(`${this.parse.name}: value is null`);

        let localId: LocalIdBuilder | undefined = undefined;
        let pmsLocalId: PMSLocalIdBuilder | undefined = undefined;

        if (
            (localId = await LocalIdBuilder.tryParseAsync(value, errorBuilder))
        ) {
            return new DataId(localId.build());
        } else if (
            (pmsLocalId = await PMSLocalIdBuilder.tryParseAsync(
                value,
                errorBuilder
            ))
        ) {
            return new DataId(pmsLocalId.build());
        } else {
            return new DataId(value);
        }
    }
}
