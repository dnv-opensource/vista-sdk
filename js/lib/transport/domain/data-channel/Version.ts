import { VisVersion, VisVersionExtension, VisVersions } from "../../..";
import { isNullOrWhiteSpace } from "../../../util/util";

export class Version {
    private readonly _tag: number;
    private readonly _visVersion?: VisVersion;
    private readonly _otherVersion?: string;

    private constructor(version: VisVersion | string) {
        if (typeof version === "string") {
            const parsed = VisVersions.tryParse(version);
            if (parsed !== undefined) {
                this._tag = 1;
                this._visVersion = parsed;
            } else {
                this._tag = 2;
                this._otherVersion = version;
            }
        } else {
            this._tag = 1;
            this._visVersion = version;
        }
    }

    public match<T>(
        onVisVersion: (version: VisVersion) => T,
        onOtherVersion: (version: string) => T,
    ): T {
        switch (this._tag) {
            case 1:
                return onVisVersion(this._visVersion!);
            case 2:
                return onOtherVersion(this._otherVersion!);
            default:
                throw new Error("Tried to match on invalid Version");
        }
    }

    public switch(
        onVisVersion: (version: VisVersion) => void,
        onOtherVersion: (version: string) => void,
    ) {
        switch (this._tag) {
            case 1:
                return onVisVersion(this._visVersion!);
            case 2:
                return onOtherVersion(this._otherVersion!);
            default:
                throw new Error("Tried to switch on invalid Version");
        }
    }

    public toString() {
        switch (this._tag) {
            case 1:
                return VisVersionExtension.toVersionString(this._visVersion!);
            case 2:
                return this._otherVersion!;
            default:
                throw new Error("Invalid Version");
        }
    }

    public static parse(version?: VisVersion | string): Version {
        if (typeof version === "string") {
            return new Version(version);
        }
        if (isNullOrWhiteSpace(version))
            throw new Error(`${Version.name}.parse: value is null`);

        return new Version(version);
    }
}
