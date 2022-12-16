import { LocationParsingErrorBuilder } from "./internal/LocationParsingErrorBuilder";
import {
    LocationValidationResult,
    RelativeLocation as RelativeLocations,
} from "./types/Location";
import { LocationsDto, RelativeLocationsDto } from "./types/LocationDto";
import { isNullOrWhiteSpace, tryParseInt } from "./util/util";
import { VisVersion } from "./VisVersion";

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
        value: string,
        errorBuilder?: LocationParsingErrorBuilder
    ): Location {
        const location = this.tryParse(value, errorBuilder);
        if (!location) {
            throw new Error(`Invalid value for location: ${value}`);
        }

        return location;
    }

    public tryParse(
        nodeLocation?: string | undefined | null,
        errorBuilder?: LocationParsingErrorBuilder
    ): Location | undefined {
        if (!this.isValid(nodeLocation, errorBuilder)) return;

        return new Location(nodeLocation!);
    }

    public isValid(
        nodeLocation: Location | string | undefined | null,
        errorBuilder?: LocationParsingErrorBuilder
    ): boolean {
        const location =
            typeof nodeLocation === "string"
                ? nodeLocation
                : nodeLocation?.toString();
        if (!location) return true;

        if (isNullOrWhiteSpace(location)) {
            errorBuilder?.push({
                type: LocationValidationResult.NullOrWhiteSpace,
                message: "Invalid location: contains only whitespace",
            });
            return false;
        }
        if (location.trim().length !== location.length) {
            errorBuilder?.push({
                type: LocationValidationResult.Invalid,
                message:
                    "Invalid location with leading and/or trailing whitespace: " +
                    location,
            });
            return false;
        }
        if (location.indexOf(" ") >= 0) {
            errorBuilder?.push({
                type: LocationValidationResult.Invalid,
                message: "Invalid location containing whitespace: " + location,
            });
            return false;
        }

        const locationWithoutNumber = [...location].filter(
            (l) => !(typeof tryParseInt(l) === "number")
        );

        const invalidLocationCodes = locationWithoutNumber.filter(
            (l) => !this._locationCodes.includes(l) || l === "N"
        );

        if (invalidLocationCodes.length > 0) {
            const invalidChars = invalidLocationCodes.join(",");
            errorBuilder?.push({
                type: LocationValidationResult.InvalidCode,
                message: `Invalid location code: ${location} with invalid character(s): ${invalidChars}`,
            });
        }
        const numberNotAtStart = [...location].some(
            (l) =>
                typeof tryParseInt(l) === "number" &&
                typeof tryParseInt(location[0]) !== "number"
        );
        if (numberNotAtStart)
            errorBuilder?.push({
                type: LocationValidationResult.Invalid,
                message:
                    "Invalid location: numbers should start before characters in location: " +
                    location,
            });

        const alphabeticallySorted = [...locationWithoutNumber].sort((a, b) =>
            a.toLowerCase().localeCompare(b.toLowerCase())
        );
        const notAlphabeticallySorted =
            JSON.stringify(locationWithoutNumber) !==
            JSON.stringify(alphabeticallySorted);
        if (notAlphabeticallySorted)
            errorBuilder?.push({
                type: LocationValidationResult.InvalidOrder,
                message: `Invalid location ${location}: not alphabetically sorted`,
            });

        const notUpperCase = locationWithoutNumber.some(
            (l) => l === l.toLowerCase()
        );
        if (notUpperCase)
            errorBuilder?.push({
                type: LocationValidationResult.Invalid,
                message: `Invalid location ${location}: characters can only be uppercase`,
            });

        const locationWithNumber = [...location].filter(
            (l) => typeof tryParseInt(l) === "number"
        );
        const alphaNumericLocation = locationWithNumber.concat(
            locationWithoutNumber
        );
        const notNumericalSorted =
            JSON.stringify([...location]) !==
            JSON.stringify(alphaNumericLocation);
        if (notNumericalSorted)
            errorBuilder?.push({
                type: LocationValidationResult.InvalidOrder,
                message: `Invalid location ${location}: not numerically sorted`,
            });

        if (errorBuilder?.hasError) return false;
        return true;
    }

    public get relativeLocations() {
        return this._relativeLocations;
    }

    public get locationCodes() {
        return this._locationCodes;
    }
}
