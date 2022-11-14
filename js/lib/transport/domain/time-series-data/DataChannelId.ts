import {
    Codebooks,
    Gmod,
    LocalId,
    LocalIdBuilder,
    LocalIdParsingErrorBuilder,
} from "../../..";
import { Locations } from "../../../Location";
import { isNullOrWhiteSpace } from "../../../util/util";

export class DataChannelId {
    private readonly _tag: number;
    private readonly _localId?: LocalId;
    private readonly _shortId?: string;

    private constructor(value: LocalId | string) {
        if (typeof value === "string") {
            this._tag = 2;
            this._shortId = value;
        } else {
            this._tag = 1;
            this._localId = value;
        }
    }

    public get isLocalId(): boolean {
        return this._tag == 1;
    }

    public get isShortId(): boolean {
        return this._tag == 2;
    }

    public get localId() {
        return this._tag === 1 ? this._localId : undefined;
    }

    public get shortId() {
        return this._tag === 2 ? this._shortId : undefined;
    }

    public match<T>(
        onLocalId: (localId: LocalId) => T,
        onShortId: (shortId: string) => T
    ): T {
        switch (this._tag) {
            case 1:
                return onLocalId(this._localId!);
            case 2:
                return onShortId(this._shortId!);
            default:
                throw new Error("Tried to match on invalid DataChannelId");
        }
    }

    public switch(
        onLocalId: (localId: LocalId) => void,
        onShortId: (shortId: string) => void
    ): void {
        switch (this._tag) {
            case 1:
                onLocalId(this._localId!);
                break;
            case 2:
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
    ): DataChannelId {
        if (isNullOrWhiteSpace(value))
            throw new Error(`${this.parse.name}: value is null`);

        let localId: LocalIdBuilder | undefined = undefined;

        if (
            (localId = LocalIdBuilder.tryParse(
                value,
                gmod,
                codebooks,
                locations,
                errorBuilder
            ))
        ) {
            return new DataChannelId(localId.build());
        } else {
            return new DataChannelId(value);
        }
    }

    public static async parseAsync(
        value: string | undefined,
        errorBuilder?: LocalIdParsingErrorBuilder
    ) {
        if (isNullOrWhiteSpace(value))
            throw new Error(`${this.parse.name}: value is null`);

        let localId: LocalIdBuilder | undefined = undefined;

        if (
            (localId = await LocalIdBuilder.tryParseAsync(value, errorBuilder))
        ) {
            return new DataChannelId(localId.build());
        } else {
            return new DataChannelId(value);
        }
    }
}
