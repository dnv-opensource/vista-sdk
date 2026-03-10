import { VIS } from ".";
import { Codebook } from "./Codebook";
import { CodebookName } from "./CodebookName";
import { MetadataTag } from "./MetadataTag";
import { CodebooksDto, CodebookDto } from "./types/CodebookDto";
import { VisVersion } from "./VisVersion";

export class Codebooks {
    public visVersion: VisVersion;
    private readonly _codebooks: Map<CodebookName, Codebook>;

    public constructor(visVersion: VisVersion, dto: CodebooksDto) {
        this.visVersion = visVersion;
        this._codebooks = new Map<CodebookName, Codebook>();

        for (const typeDto of dto.items) {
            const type = new Codebook(typeDto);
            this._codebooks.set(type.name, type);
        }

        const detailsCodebook = new Codebook({
            name: "detail",
            values: {},
        } as CodebookDto);
        this._codebooks.set(detailsCodebook.name, detailsCodebook);
    }

    public getCodebook(name: CodebookName): Codebook {
        const codebook = this._codebooks.get(name);
        if (!codebook) throw new Error(`Invalid codebook name: ${name}`);
        return codebook;
    }

    public get codebooks(): Map<CodebookName, Codebook> {
        return this._codebooks;
    }

    public tryCreateTag(
        name: CodebookName,
        value?: string,
    ): MetadataTag | undefined {
        return this.getCodebook(name).tryCreateTag(value);
    }

    public createTag(name: CodebookName, value?: string): MetadataTag {
        return this.getCodebook(name).createTag(value);
    }

    public static async tryCreateTagAsync(
        name: CodebookName,
        value: string,
        visVersion: VisVersion,
    ) {
        const codebook = await VIS.instance.getCodebooks(visVersion);

        return codebook.tryCreateTag(name, value);
    }

    public static async createTagAsync(
        name: CodebookName,
        value: string,
        visVersion: VisVersion,
    ) {
        const tag = await this.tryCreateTagAsync(name, value, visVersion);

        if (!tag) throw new Error("Failed to create metadata tag");

        return tag;
    }
}
