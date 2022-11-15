import { RelativeLocation as RelativeLocations } from "./types/Location";
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

    public isValid(location: string | undefined): boolean {
        if (!location || isNullOrWhiteSpace(location)) return false;
        if (location.trim().length !== location.length) return false;
        const locationWithoutNumber = [...location].filter(
            (l) => !(typeof tryParseInt(l) === "number")
        );
        const invalidLocationCode = locationWithoutNumber.some(
            (l) => !this._locationCodes.includes(l) || l === "N"
        );
        if (invalidLocationCode) return false;
        const numberNotAtStart = [...location].some(
            (l) =>
                typeof tryParseInt(l) === "number" &&
                typeof tryParseInt(location[0]) !== "number"
        );

        const alphabeticallySorted = [...locationWithoutNumber].sort();
        const notAlphabeticallySorted =
            JSON.stringify(locationWithoutNumber) !==
            JSON.stringify(alphabeticallySorted);

        const notUpperCase = locationWithoutNumber.some(
            (l) => l === l.toLowerCase()
        );

        const locationWithNumber = [...location].filter(
            (l) => typeof tryParseInt(l) === "number"
        );
        const alphaNumericLocation =
            locationWithNumber.concat(alphabeticallySorted);

        const notNumericalSorted =
            JSON.stringify([...location]) !==
            JSON.stringify(alphaNumericLocation);
        if (
            numberNotAtStart ||
            notAlphabeticallySorted ||
            notUpperCase ||
            notNumericalSorted
        )
            return false;

        return true;
    }

    public tryParse(value?: string): Location | undefined {
        if (!this.isValid(value)) return;

        return new Location(value!);
    }

    public parse(value: string): Location {
        const location = this.tryParse(value);
        if (!location) {
            throw new Error(`Invalid value for location: ${value}`);
        }

        return location;
    }

    public get relativeLocations() {
        return this._relativeLocations;
    }

    public get locationCodes() {
        return this._locationCodes;
    }
}
