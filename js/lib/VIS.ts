import { CodebooksDto } from "./types/CodebookDto";
import { VisVersion, VisVersionExtension, VisVersions } from "./VisVersion";
import { Client } from "./Client";

import LRUCache from "lru-cache";
import { GmodDto } from "./types/GmodDto";
import { Gmod } from "./Gmod";
import { Codebooks } from "./Codebooks";
import { LocationsDto } from "./types/LocationDto";
import { Locations } from "./Location";

export class VIS {
    public static readonly instance = new VIS();

    public static readonly latestVisVersion: VisVersion = VisVersion.v3_9a;

    private readonly _gmodDtoCache: LRUCache<VisVersion, Promise<GmodDto>>;
    private readonly _gmodCache: LRUCache<VisVersion, Gmod>;
    private readonly _codebooksDtoCache: LRUCache<
        VisVersion,
        Promise<CodebooksDto>
    >;
    private readonly _codebooksCache: LRUCache<VisVersion, Codebooks>;
    private readonly _locationDtoCache: LRUCache<
        VisVersion,
        Promise<LocationsDto>
    >;
    private readonly _locationCache: LRUCache<VisVersion, Locations>;

    public constructor() {
        this._gmodDtoCache = new LRUCache(this.options);
        this._gmodCache = new LRUCache(this.options);
        this._codebooksDtoCache = new LRUCache(this.options);
        this._codebooksCache = new LRUCache(this.options);
        this._locationDtoCache = new LRUCache(this.options);
        this._locationCache = new LRUCache(this.options);
    }

    private readonly options = {
        max: 10,
        ttl: 60 * 60 * 1000,
    };

    public async getGmodDto(visVersion: VisVersion): Promise<GmodDto> {
        let gmodDto: Promise<GmodDto> | undefined =
            this._gmodDtoCache.get(visVersion);

        if (gmodDto) {
            return await gmodDto;
        }

        gmodDto = Client.visGetGmod(visVersion);
        this._gmodDtoCache.set(visVersion, gmodDto);

        return gmodDto;
    }

    public async getGmod(visVersion: VisVersion): Promise<Gmod> {
        let gmod: Gmod | undefined = this._gmodCache.get(visVersion);

        if (gmod) {
            return gmod;
        }

        gmod = new Gmod(visVersion, await this.getGmodDto(visVersion));
        this._gmodCache.set(visVersion, gmod);
        return gmod;
    }

    public async getGmodsMap(
        visVersions: VisVersion[]
    ): Promise<Map<VisVersion, Gmod>> {
        var invalidVisVersions = visVersions.filter(
            (v) => !VisVersionExtension.isValid(v)
        );
        if (invalidVisVersions.length > 0) {
            throw new Error(
                "Invalid VIS versions provided: " +
                    invalidVisVersions.join(", ")
            );
        }

        const versions = new Set(visVersions);
        const gmodPromises = Array.from(versions).map(async (v) => ({
            visVersion: v,
            gmod: await this.getGmod(v),
        }));

        const gmods = await Promise.all(gmodPromises);

        return new Map(
            Object.assign({}, ...gmods.map((g) => ({ [g.visVersion]: g.gmod })))
        );
    }

    // private async getGmodVersioningDto() : Promise<GmodVersioningDto> {
    //     return await Client.visGetGmodVersioning();
    // }

    // public async getGmodVersioning() : Promise<GmodVersioning> {
    //     const dto = await this.getGmodVersioningDto();
    //     return  new GmodVersioning(dto);
    // }

    private async getCodebooksDto(
        visVersion: VisVersion
    ): Promise<CodebooksDto> {
        let codebooksDto: Promise<CodebooksDto> | undefined =
            this._codebooksDtoCache.get(visVersion);
        if (codebooksDto) return await codebooksDto;

        codebooksDto = Client.visGetCodebooks(visVersion);

        this._codebooksDtoCache.set(visVersion, codebooksDto);
        return codebooksDto;
    }

    public async getCodebooks(visVersion: VisVersion): Promise<Codebooks> {
        let codebooks: Codebooks | undefined =
            this._codebooksCache.get(visVersion);
        if (codebooks) return codebooks;

        codebooks = new Codebooks(
            visVersion,
            await this.getCodebooksDto(visVersion)
        );

        this._codebooksCache.set(visVersion, codebooks);
        return codebooks;
    }

    public async getCodebooksMap(
        visVersions: VisVersion[]
    ): Promise<Map<VisVersion, Codebooks>> {
        var invalidVisVersions = visVersions.filter(
            (v) => !VisVersionExtension.isValid(v)
        );
        if (invalidVisVersions.length > 0) {
            throw new Error(
                "Invalid VIS versions provided: " +
                    invalidVisVersions.join(", ")
            );
        }

        const versions = new Set(visVersions);
        const codebookPromises = Array.from(versions).map(async (v) => ({
            visVersion: v,
            codebooks: await this.getCodebooks(v),
        }));
        const codebooks = await Promise.all(codebookPromises);

        return new Map(
            Object.assign(
                {},
                ...codebooks.map((c) => ({ [c.visVersion]: c.codebooks }))
            )
        );
    }

    public async getLocationsMap(
        visVersions: VisVersion[]
    ): Promise<Map<VisVersion, Locations>> {
        var invalidVisVersions = visVersions.filter(
            (v) => !VisVersionExtension.isValid(v)
        );
        if (invalidVisVersions.length > 0) {
            throw new Error(
                "Invalid VIS versions provided: " +
                    invalidVisVersions.join(", ")
            );
        }

        const versions = new Set(visVersions);
        const locationPromises = Array.from(versions).map(async (v) => ({
            visVersion: v,
            location: await this.getLocations(v),
        }));
        const locations = await Promise.all(locationPromises);

        return new Map(
            Object.assign(
                {},
                ...locations.map((c) => ({ [c.visVersion]: c.location }))
            )
        );
    }

    public async getVIS(
        visVersion: VisVersion
    ): Promise<{ gmod: Gmod; codebooks: Codebooks; locations: Locations }> {
        const promises = [
            this.getGmod(visVersion),
            this.getCodebooks(visVersion),
            this.getLocations(visVersion),
        ];

        const [gmod, codebooks, locations] = (await Promise.all(promises)) as [
            Gmod,
            Codebooks,
            Locations
        ];

        return { gmod, codebooks, locations };
    }

    public async getVISMap(visVersions: VisVersion[]): Promise<
        Map<
            VisVersion,
            {
                gmod: Gmod;
                codebooks: Codebooks;
                locations: Locations;
            }
        >
    > {
        var invalidVisVersions = visVersions.filter(
            (v) => !VisVersionExtension.isValid(v)
        );
        if (invalidVisVersions.length > 0) {
            throw new Error(
                "Invalid VIS versions provided: " +
                    invalidVisVersions.join(", ")
            );
        }
        const promises = visVersions.map((v) => this.getVIS(v));
        const results = await Promise.all(promises);

        const map = new Map<
            VisVersion,
            { gmod: Gmod; codebooks: Codebooks; locations: Locations }
        >();

        for (const result of results) map.set(result.gmod.visVersion, result);

        return map;
    }

    private async getLocationsDto(
        visVersion: VisVersion
    ): Promise<LocationsDto> {
        let locationDto: Promise<LocationsDto> | undefined =
            this._locationDtoCache.get(visVersion);

        if (locationDto) return await locationDto;

        locationDto = Client.visGetLocation(visVersion);

        this._locationDtoCache.set(visVersion, locationDto);
        return locationDto;
    }

    public async getLocations(visVersion: VisVersion): Promise<Locations> {
        let location: Locations | undefined =
            this._locationCache.get(visVersion);
        if (location) return location;

        location = new Locations(
            visVersion,
            await this.getLocationsDto(visVersion)
        );

        return location;
    }

    public getVisVersions() {
        return VisVersions.all;
    }

    /**@description Rules according to: "ISO19848 5.2.1, Note 1" and "RFC3986 2.3 - Unreserved characters"*/
    public static isISOLocalIdString(value: string): boolean {
        for (const part of value.split("/")) {
            if (!VIS.isISOString(part)) return false;
        }
        return true;
    }

    /**@description Rules according to: "ISO19848 5.2.1, Note 1" and "RFC3986 2.3 - Unreserved characters"*/
    public static isISOString(value: string): boolean {
        for (let i = 0; i < value.length; i++) {
            // User ASCII Decimal instead of HEX Digits
            const code = value.charCodeAt(i);
            if (!VIS.matchAsciiDecimal(code)) return false;
        }
        return true;
    }

    private static matchAsciiDecimal(code: number) {
        // Number
        if (code >= 48 && code <= 57) return true;
        // Large character A-Z
        if (code >= 65 && code <= 90) return true;
        // Small character a-z
        if (code >= 97 && code <= 122) return true;
        // ["-" , "." , "_" , "~"] respectively
        if (code === 45 || code === 46 || code === 95 || code === 126)
            return true;
        return false;
    }
}
