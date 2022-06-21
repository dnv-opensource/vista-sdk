import { VisVersion, GmodDto, Gmod, Codebooks } from ".";
import { CodebooksDto } from "./types/CodebookDto";
import { VisVersionExtension, VisVersions } from "./VisVersion";
import { ResourceFiles } from "./source-generator/ResourceFiles";

export class VIS {
    public constructor() {}
    public static get instance() {
        return new VIS();
    }

    private async getGmodDto(visVersion: VisVersion): Promise<GmodDto> {
        const dto = await ResourceFiles.readGmodFile<GmodDto>(
            VisVersionExtension.toVersionString(visVersion)
        );
        return dto;
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

    private async getCodebooksDto(
        visVersion: VisVersion
    ): Promise<CodebooksDto> {
        const dto = await ResourceFiles.readCodebooksFile<CodebooksDto>(
            VisVersionExtension.toString(visVersion)
        );
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
