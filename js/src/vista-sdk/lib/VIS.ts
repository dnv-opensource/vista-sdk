import { CodebooksDto } from "./types/CodebookDto";
import { VisVersion, VisVersionExtension, VisVersions } from "./VisVersion";
import { Client } from "./Client";

import LRUCache from "lru-cache";
import { GmodDto } from "./types/GmodDto";
import { Gmod } from "./Gmod";
import { Codebooks } from "./Codebooks";

export class VIS {
    private readonly _gmodDtoCache: LRUCache<VisVersion, Promise<GmodDto>>;
    private readonly _gmodCache: LRUCache<VisVersion, Gmod>;
    private readonly _codebooksDtoCache: LRUCache<
        VisVersion,
        Promise<CodebooksDto>
    >;
    private readonly _codebooksCache: LRUCache<VisVersion, Codebooks>;

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
        ttl: 60 * 60 * 1000,
    };

    private async getGmodDto(visVersion: VisVersion): Promise<GmodDto> {
        let gmodDto: Promise<GmodDto> | undefined =
            this._gmodDtoCache.get(visVersion);
        if (gmodDto != undefined) {
            return await gmodDto;
        }

        gmodDto = Client.visGetGmod(visVersion);
        this._gmodDtoCache.set(visVersion, gmodDto);
        return await gmodDto;
    }

    public async getGmod(visVersion: VisVersion): Promise<Gmod> {
        let gmod: Gmod | undefined = this._gmodCache.get(visVersion);
        if (gmod != undefined) {
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
        if (codebooksDto != undefined) return await codebooksDto;

        codebooksDto = Client.visGetCodebooks(visVersion);
        this._codebooksDtoCache.set(visVersion, codebooksDto);
        return await codebooksDto;
    }

    public async getCodebooks(visVersion: VisVersion): Promise<Codebooks> {
        let codebooks: Codebooks | undefined =
            this._codebooksCache.get(visVersion);
        if (codebooks != undefined) return await codebooks;

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

    public getVisVersions() {
        return VisVersions.all;
    }
}
