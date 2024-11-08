import { LocationParsingErrorBuilder } from "./internal/LocationParsingErrorBuilder";
import {
    LocationValidationResult,
    RelativeLocation as RelativeLocations,
} from "./types/Location";
import { LocationsDto } from "./types/LocationDto";
import { isNullOrWhiteSpace } from "./util/util";
import { VisVersion } from "./VisVersion";

const charCodeZero = "0".charCodeAt(0);
const charCodeNine = "9".charCodeAt(0);

export enum LocationGroup {
    Number,
    Side,
    Vertical,
    Transverse,
    Longitudinal,
}

export class Location {
    public readonly value: string;

    public constructor(value: string) {
        this.value = value;
    }

    public toString(): string {
        return this.value;
    }
    public clone() {
        return new Location(this.value);
    }
    public equals(other?: Location) {
        if (!other) return false;
        return this.value === other.value;
    }
}

export class Locations {
    public visVersion: VisVersion;

    private _locationCodes: string[];
    private _relativeLocations: RelativeLocations[];

    public reversedGroups: Map<string, LocationGroup>;

    public groups: Map<LocationGroup, RelativeLocations[]>;

    public constructor(visVersion: VisVersion, dto: LocationsDto) {
        this.visVersion = visVersion;
        this._locationCodes = [];
        this._relativeLocations = [];

        const groups = new Map<LocationGroup, RelativeLocations[]>();
        this.reversedGroups = new Map();

        for (const relativeLocationDto of dto.items) {
            this._locationCodes.push(relativeLocationDto.code);
            const relativeLocation = {
                code: relativeLocationDto.code,
                name: relativeLocationDto.name,
                definition: relativeLocationDto.definition ?? undefined,
                location: new Location(relativeLocationDto.code),
            };
            this._relativeLocations.push(relativeLocation);
            // TODO : Create new LocationGroup for H and V
            if (["H", "V"].includes(relativeLocationDto.code)) continue;
            const key = {
                N: LocationGroup.Number,
                P: LocationGroup.Side,
                C: LocationGroup.Side,
                S: LocationGroup.Side,
                U: LocationGroup.Vertical,
                M: LocationGroup.Vertical,
                L: LocationGroup.Vertical,
                I: LocationGroup.Transverse,
                O: LocationGroup.Transverse,
                F: LocationGroup.Longitudinal,
                A: LocationGroup.Longitudinal,
            }[relativeLocationDto.code];

            if (key === undefined)
                throw new Error(
                    `Unsupported code: ${relativeLocationDto.code}`
                );
            if (!groups.has(key)) groups.set(key, []);

            if (key === LocationGroup.Number) continue;
            this.reversedGroups.set(relativeLocationDto.code, key);
            groups.get(key)?.push(relativeLocation);
        }

        this.groups = groups;
    }

    public parse(
        locationStr: string,
        errorBuilder?: LocationParsingErrorBuilder
    ): Location {
        const location = this.tryParse(locationStr, errorBuilder);
        if (!location) {
            throw new Error(`Invalid value for location: ${locationStr}`);
        }

        return location;
    }

    public tryParse(
        locationStr?: string | null | undefined,
        errorBuilder?: LocationParsingErrorBuilder
    ): Location | undefined {
        return this.tryParseInternal(locationStr, errorBuilder);
    }

    private tryParseInternal(
        location: string | null | undefined,
        errorBuilder?: LocationParsingErrorBuilder
    ): Location | undefined {
        if (!location) return;

        if (isNullOrWhiteSpace(location)) {
            addError(
                LocationValidationResult.NullOrWhiteSpace,
                "Invalid location: contains only whitespace"
            );
            return;
        }

        const span = location;

        let prevDigitIndex: number | null = null;
        let digitStartIndex: number | null = null;
        let charsStartIndex: number | null = null;
        let number: number | null = null;

        const charDict = new LocationCharDict();

        for (let i = 0; i < span.length; i++) {
            const ch = span.charAt(i);

            if (isDigitCode(ch)) {
                // First digit should be at index 0
                if (digitStartIndex === null && i !== 0) {
                    addError(
                        LocationValidationResult.Invalid,
                        `Invalid location: numeric location should start before location code(s) in location: '${location}'`
                    );
                    return;
                }

                // Other digits should neighbor the first
                if (prevDigitIndex !== null && prevDigitIndex !== i - 1) {
                    addError(
                        LocationValidationResult.Invalid,
                        `Invalid location: cannot have multiple separated digits in location: '${location}'`
                    );
                    return;
                }
                if (digitStartIndex === null) {
                    number = +ch;
                    digitStartIndex = i;
                } else {
                    number = parseInt(span.slice(digitStartIndex, i + 1), 10);
                    if (isNaN(number)) {
                        addError(
                            LocationValidationResult.Invalid,
                            `Invalid location: failed to parse numeric location: '${location}'`
                        );
                        return;
                    }
                }
                prevDigitIndex = i;
            } else {
                const group = this.reversedGroups.get(ch);
                if (group === undefined) {
                    const invalidChars = Array.from(location)
                        .filter(
                            (c) =>
                                !isDigitCode(c) &&
                                (c === "N" || !this._locationCodes.includes(c))
                        )
                        .map((c) => `'${c}'`)
                        .join(",");

                    addError(
                        LocationValidationResult.InvalidCode,
                        `Invalid location code: '${location}' with invalid location code(s): ${invalidChars}`
                    );
                    return;
                }

                if (!charDict.tryAdd(group, ch)) {
                    addError(
                        LocationValidationResult.Invalid,
                        `Invalid location: Multiple '${
                            LocationGroup[group]
                        }' values. Got both '${charDict.get(
                            group
                        )}' and '${ch}' in '${location}'`
                    );
                    return;
                }

                if (charsStartIndex === null) {
                    charsStartIndex = i;
                } else if (i > 0) {
                    const prevCh = span.charAt(i - 1);
                    if (ch.localeCompare(prevCh) < 0) {
                        addError(
                            LocationValidationResult.InvalidOrder,
                            `Invalid location: '${location}' not alphabetically sorted`
                        );
                        return;
                    }
                }
            }
        }

        return new Location(location);

        function addError(type: LocationValidationResult, message: string) {
            errorBuilder?.push({
                type,
                message,
            });
        }
    }

    public get relativeLocations() {
        return this._relativeLocations;
    }

    public get locationCodes() {
        return this._locationCodes;
    }
}

export function isDigitCode(ch: string) {
    return ch.charCodeAt(0) >= charCodeZero && ch.charCodeAt(0) <= charCodeNine;
}

class LocationCharDict {
    private _side?: string;
    private _vertical?: string;
    private _transverse?: string;
    private _longitudinal?: string;

    public has(key: LocationGroup): boolean {
        if (key === LocationGroup.Number)
            throw new Error("numbers arent handled here");
        return {
            [LocationGroup.Side]: this._side !== undefined,
            [LocationGroup.Vertical]: this._vertical !== undefined,
            [LocationGroup.Transverse]: this._transverse !== undefined,
            [LocationGroup.Longitudinal]: this._longitudinal !== undefined,
        }[key];
    }

    public get(key: LocationGroup): string | undefined {
        if (key === LocationGroup.Number)
            throw new Error("numbers arent handled here");
        return {
            [LocationGroup.Side]: this._side,
            [LocationGroup.Vertical]: this._vertical,
            [LocationGroup.Transverse]: this._transverse,
            [LocationGroup.Longitudinal]: this._longitudinal,
        }[key];
    }

    public tryAdd(key: LocationGroup, value: string): boolean {
        if (this.has(key)) return false;

        switch (key) {
            case LocationGroup.Side:
                this._side = value;
                break;
            case LocationGroup.Vertical:
                this._vertical = value;
                break;
            case LocationGroup.Transverse:
                this._transverse = value;
                break;
            case LocationGroup.Longitudinal:
                this._longitudinal = value;
                break;
        }
        return true;
    }
}
