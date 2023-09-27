import { Location, LocationGroup, Locations, isDigitCode } from "./Location";
import { VisVersion } from "./VisVersion";

export class LocationBuilder {
    public number?: number;
    public side?: string;
    public vertical?: string;
    public transverse?: string;
    public longitudinal?: string;

    public visVersion: VisVersion;

    protected _reversedGroups: Map<string, LocationGroup>;

    private constructor(
        visVersion: VisVersion,
        reversedGroups: Map<string, LocationGroup>
    ) {
        this.visVersion = visVersion;
        this._reversedGroups = reversedGroups;
    }

    public static create(locations: Locations) {
        return new LocationBuilder(
            locations.visVersion,
            locations.reversedGroups
        );
    }

    public get(group: LocationGroup): number | string | undefined {
        return {
            [LocationGroup.Number]: this.number,
            [LocationGroup.Side]: this.side,
            [LocationGroup.Vertical]: this.vertical,
            [LocationGroup.Transverse]: this.transverse,
            [LocationGroup.Longitudinal]: this.longitudinal,
        }[group];
    }

    public tryWithLocation(value?: Location): LocationBuilder {
        if (!value) return this;
        return this.withLocation(value);
    }

    public withLocation(value: Location): LocationBuilder {
        let builder = this.clone();

        const span = value.toString();

        let n: number | undefined = undefined;

        for (let i = 0; i < span.length; i++) {
            const ch = span.charAt(i);

            if (isDigitCode(ch)) {
                if (n === undefined) n = +ch;
                else {
                    n = parseInt(span.slice(0, i + 1));
                    if (isNaN(n))
                        throw new Error("Should include a valid number");
                }
                continue;
            }
            builder = builder.withValue(ch);
        }

        if (n !== undefined) builder = builder.withNumber(n);

        return builder;
    }

    public withNumber(value: number): LocationBuilder {
        return this.withValueInternal(LocationGroup.Number, value);
    }

    public withSide(value: string): LocationBuilder {
        return this.withValueInternal(LocationGroup.Side, value);
    }

    public withVertical(value: string): LocationBuilder {
        return this.withValueInternal(LocationGroup.Vertical, value);
    }

    public withTransverse(value: string): LocationBuilder {
        return this.withValueInternal(LocationGroup.Transverse, value);
    }

    public withLongitudinal(value: string): LocationBuilder {
        return this.withValueInternal(LocationGroup.Longitudinal, value);
    }

    public withValue(
        value: number | string,
        group: LocationGroup | undefined = undefined
    ): LocationBuilder {
        if (group !== undefined) return this.withValueInternal(group, value);
        if (typeof value === "number") {
            return this.withValueInternal(LocationGroup.Number, value);
        } else {
            const key = this._reversedGroups.get(value);
            if (!key)
                throw new Error(
                    `The value ${value} is an invalid Locations value`
                );
            return this.withValueInternal(key, value);
        }
    }

    private withValueInternal(
        group: LocationGroup,
        value: number | string
    ): LocationBuilder {
        if (group === LocationGroup.Number) {
            if (typeof value !== "number")
                throw new Error("Value should be number");
            if (value < 1) throw new Error("Value should be greater than 0");

            return this.with((s) => (s.number = value));
        }

        if (typeof value !== "string")
            throw new Error("Value should be a string");
        if (value.length !== 1)
            throw new Error("Value should be a single character");

        const key = this._reversedGroups.get(value);
        if (key === undefined || key !== group) {
            throw new Error(
                `The value ${value} is an invalid ${LocationGroup[group]} value`
            );
        }

        return {
            [LocationGroup.Side]: this.with((s) => (s.side = value)),
            [LocationGroup.Vertical]: this.with((s) => (s.vertical = value)),
            [LocationGroup.Transverse]: this.with(
                (s) => (s.transverse = value)
            ),
            [LocationGroup.Longitudinal]: this.with(
                (s) => (s.longitudinal = value)
            ),
        }[key];
    }

    public withoutNumber(): LocationBuilder {
        return this.withoutValue(LocationGroup.Number);
    }

    public withoutSide(): LocationBuilder {
        return this.withoutValue(LocationGroup.Side);
    }

    public withoutVertical(): LocationBuilder {
        return this.withoutValue(LocationGroup.Vertical);
    }

    public withoutTransverse(): LocationBuilder {
        return this.withoutValue(LocationGroup.Transverse);
    }

    public withoutLongitudinal(): LocationBuilder {
        return this.withoutValue(LocationGroup.Longitudinal);
    }

    public withoutValue(group: LocationGroup) {
        return {
            [LocationGroup.Number]: this.with((s) => (s.number = undefined)),
            [LocationGroup.Side]: this.with((s) => (s.side = undefined)),
            [LocationGroup.Vertical]: this.with(
                (s) => (s.vertical = undefined)
            ),
            [LocationGroup.Transverse]: this.with(
                (s) => (s.transverse = undefined)
            ),
            [LocationGroup.Longitudinal]: this.with(
                (s) => (s.longitudinal = undefined)
            ),
        }[group];
    }

    public with(u: { (state: LocationBuilder): void }): LocationBuilder {
        const n = this.clone();
        u && u(n);
        return n;
    }

    public clone(): LocationBuilder {
        return Object.assign(
            new LocationBuilder(this.visVersion, this._reversedGroups),
            this
        );
    }

    public build(): Location {
        return new Location(this.toString());
    }

    public toString(): string {
        const items = [
            this.side,
            this.vertical,
            this.transverse,
            this.longitudinal,
        ];

        let s = "";

        if (this.number !== undefined) s += this.number;

        for (const item of items) {
            if (item === undefined) continue;
            s += item;
        }

        return s.split("").sort().join("");
    }
}
