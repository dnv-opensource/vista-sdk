import { LocationParsingErrorBuilder } from "./internal/LocationParsingErrorBuilder";
import {
    LocationValidationResult,
    RelativeLocation as RelativeLocations,
} from "./types/Location";
import { LocationsDto } from "./types/LocationDto";
import { isNullOrWhiteSpace, tryParseInt } from "./util/util";
import { VisVersion } from "./VisVersion";

const charCodeZero = "0".charCodeAt(0);
const charCodeNine = "9".charCodeAt(0);

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

    public constructor(visVersion: VisVersion, dto: LocationsDto) {
        this.visVersion = visVersion;
        this._locationCodes = [];
        this._relativeLocations = [];

        for (const relativeLocationDto of dto.items) {
            this._locationCodes.push(relativeLocationDto.code);
            this._relativeLocations.push({
                code: relativeLocationDto.code,
                name: relativeLocationDto.name,
                definition: relativeLocationDto.definition ?? undefined,
            });
        }
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
        let digitStartIndex: number | null = null;
        let lastLetterIndex: number | null = null;
        let charsStartIndex: number | null = null;
        let n: number | null = null;

        for (let i = 0; i < span.length; i++) {
            const ch = span.charAt(i);

            if (isDigitCode(ch)) {
                if (digitStartIndex === null) {
                    digitStartIndex = i;
                    if (lastLetterIndex !== null) {
                        addError(
                            LocationValidationResult.Invalid,
                            `Invalid location: numeric location should start before location code(s) in location: '${location}'`
                        );
                        return;
                    }
                } else {
                    if (lastLetterIndex !== null) {
                        if (lastLetterIndex < digitStartIndex) {
                            addError(
                                LocationValidationResult.Invalid,
                                `Invalid location: numeric location should start before location code(s) in location: '${location}'`
                            );
                            return;
                        } else if (lastLetterIndex > digitStartIndex && lastLetterIndex < i) {
                            addError(
                                LocationValidationResult.Invalid,
                                `Invalid location: cannot have multiple separated digits in location: '${location}'`
                            );
                            return;
                        }
                    }

                    n = parseInt(span.slice(digitStartIndex, i - digitStartIndex), 10);

                    if (n < 0) {
                        addError(
                            LocationValidationResult.Invalid,
                            `Invalid location: negative numeric location is not allowed: '${location}'`
                        );
                        return;
                    }
                }
            } else {
                if (ch === 'N' || !this._locationCodes.includes(ch)) {
                    const invalidChars = Array.from(location)
                        .filter(c => !isDigitCode(c) && (c === 'N' || !this._locationCodes.includes(c)))
                        .map(c => `'${c}'`)
                        .join(',');
                    addError(
                        LocationValidationResult.InvalidCode,
                        `Invalid location code: '${location}' with invalid location code(s): ${invalidChars}`
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

                lastLetterIndex = i;
            }
        }

        return new Location(location);


        function addError(type: LocationValidationResult, message: string) {
            errorBuilder?.push({
                type,
                message,
            });
        }

        function isDigitCode(ch: string) {
            return(ch.charCodeAt(0) >= charCodeZero && ch.charCodeAt(0) <= charCodeNine);
        }
    }

    public get relativeLocations() {
        return this._relativeLocations;
    }

    public get locationCodes() {
        return this._locationCodes;
    }
}
