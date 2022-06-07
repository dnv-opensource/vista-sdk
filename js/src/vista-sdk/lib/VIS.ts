import { VisVersion, GmodDto, Gmod, Codebooks } from ".";
import { EmbeddedResource } from "./source-generator/EmbeddedResource";
import { CodebooksDto } from "./types/CodebookDto";
import { VisVersionExtension, VisVersions } from "./VisVersion";

export class VIS {
    public constructor() {
        // TODO Cache
    }

    public static get instance() {
        return new VIS();
    }

    public async getGmodDto(
        visVersion: VisVersion,
        apiUrl?: string
    ): Promise<GmodDto> {
        const dto = await EmbeddedResource.getGmod(
            VisVersionExtension.toVersionString(visVersion),
            apiUrl
        );

        return dto;
    }

    public async getGmod(
        visVersion: VisVersion,
        apiUrl?: string
    ): Promise<Gmod> {
        const dto = await this.getGmodDto(visVersion, apiUrl);

        return new Gmod(visVersion, dto);
    }

    public async getGmodsMap(
        visVersions: VisVersion[]
    ): Promise<Map<VisVersion, Gmod>> {
        var invalidVersions = visVersions.filter(
            (v) => !VisVersionExtension.isValid(v)
        );
        if (invalidVersions.length > 0) {
            throw new Error(
                "Invalid VIS versions provided: " + invalidVersions.join(", ")
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

    public async getCodebooksDto(
        visVersion: string,
        apiUrl?: string
    ): Promise<CodebooksDto> {
        const dto = await EmbeddedResource.getCodebooks(visVersion, apiUrl);
        return dto;
    }

    public async getCodebooks(visVersion: VisVersion, apiUrl?: string) {
        const dto = await this.getCodebooksDto(visVersion, apiUrl);

        return new Codebooks(visVersion, dto);
    }

    public async getCodebooksMap(
        visVersions: VisVersion[]
    ): Promise<Map<VisVersion, Codebooks>> {
        var invalidVersions = visVersions.filter(
            (v) => !VisVersionExtension.isValid(v)
        );
        if (invalidVersions.length > 0) {
            throw new Error(
                "Invalid VIS versions provided: " + invalidVersions.join(", ")
            );
        }

        const versions = new Set(visVersions);
        const codebookPromises = Array.from(versions).map(async (v) => ({
            visVersion: v,
            gmod: await this.getCodebooks(v),
        }));

        const codebooks = await Promise.all(codebookPromises);

        return new Map(
            Object.assign(
                {},
                ...codebooks.map((g) => ({ [g.visVersion]: g.gmod }))
            )
        );
    }

    public getVisVersions() {
        return VisVersions.all;
    }
}
