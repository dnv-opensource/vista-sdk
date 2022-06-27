import { VisVersion, GmodDto, Gmod, Codebooks } from ".";
import { CodebooksDto } from "./types/CodebookDto";
import { VisVersionExtension, VisVersions } from "./VisVersion";
import { Client } from "./Client";

import LRUCache from 'lru-cache';

export class VIS {
    private readonly  _gmodDtoCache : LRUCache<VisVersion, GmodDto>;
    private readonly  _gmodCache : LRUCache<VisVersion, Gmod>;
    private readonly  _codebooksDtoCache : LRUCache<VisVersion, CodebooksDto>;
    private readonly  _codebooksCache : LRUCache<VisVersion, Codebooks>;

    public constructor() {
        this._gmodDtoCache = new LRUCache(this.options);
        this._gmodCache = new LRUCache(this.options);
        this._codebooksDtoCache = new LRUCache(this.options);
        this._codebooksCache = new LRUCache(this.options);
    }
    public static get instance() {

        return new VIS();
    }
    private readonly options = {
    max: 10,
    //maxSize: 100,
    ttl: 60 * 60 * 1000 // 60 minutes
    }

    private async getGmodDto(visVersion: VisVersion): Promise<GmodDto> {
        const getCache = this._gmodDtoCache.get(visVersion);
        if (getCache == undefined)
        {
            const dto = await Client.visGetGmod(visVersion);
            this._gmodDtoCache.set(visVersion, dto);
            this._gmodDtoCache.get(visVersion);
        }
        return this._gmodDtoCache.get(visVersion) as GmodDto;
    }

    public async getGmod(visVersion: VisVersion): Promise<Gmod> {
        const dto = await this.getGmodDto(visVersion);
        return new Gmod(visVersion, dto);
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
        const dto = await Client.visGetCodebooks(visVersion);
        return dto;
    }

    public async getCodebooks(visVersion: VisVersion): Promise<Codebooks> {
        const dto = await this.getCodebooksDto(visVersion);
        return new Codebooks(visVersion, dto);
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

    public getVisVersions() {
        return VisVersions.all;
    }
}
