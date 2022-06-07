import * as fs from "fs-extra";
import * as zlib from "zlib";
import { GmodDto } from "..";
import { CodebooksDto } from "../types/CodebookDto";

export class EmbeddedResource { 
    public static readonly RESOURCE_DIR = "./resources";

    public static async getGmodVisVersions(apiUrl?: string) {
        return this.isNode()
            ? this.getGmodVisVersions__node()
            : this.getGmodVisVersions__browser(apiUrl);
    }

    public static async getGmod(
        visVersion: string,
        apiUrl?: string
    ): Promise<GmodDto> {
        if (apiUrl) {
        }
        return this.isNode()
            ? this.getGmod__node(visVersion)
            : this.getGmod__browser(apiUrl);
    }

    public static async getCodebooks(visVersion: string, apiUrl?: string) {
        return this.isNode()
            ? this.getCodebooks__node(visVersion)
            : this.getCodebooks__browser(apiUrl);
    }

    public static getResourceNames() {
        console.log(`__dirname: [${__dirname}] end__dirname`);
        return this.isNode()
            ? this.getResourceNames__node()
            : this.getResourceNames__browser();
    }

    private static async getGmodVisVersions__node(): Promise<string[]> {
        const resources = this.getResourceNames().filter(
            (r) => r.endsWith(".gz") && r.includes("gmod")
        );

        if (resources.length === 0)
            throw new Error("Couldnt find any Gmod resource");

        const versions: string[] = [];

        for (const resource of resources) {
            const gmod = await this.readJsonGzip<GmodDto>(
                EmbeddedResource.RESOURCE_DIR + "/" + resource
            );

            versions.push(gmod.visRelease);
        }

        return versions;
    }

    private static async getGmodVisVersions__browser(
        apiUrl?: string
    ): Promise<string[]> {
        if (!apiUrl) throw new Error("Api url required for browser usage");

        return (await (await fetch(apiUrl)).json()) as string[];
    }

    private static async getGmod__node(visVersion: string): Promise<GmodDto> {
        const resource = this.getResourceNames().filter(
            (r) =>
                r.endsWith(".gz") &&
                r.includes("gmod") &&
                r.includes(visVersion)
        )[0];

        if (!resource)
            throw new Error(
                "Couldnt find Gmod resource for vis version: " + visVersion
            );

        return this.readJsonGzip<GmodDto>(
            EmbeddedResource.RESOURCE_DIR + "/" + resource
        );
    }

    private static async getGmod__browser(apiUrl?: string): Promise<GmodDto> {
        if (!apiUrl) throw new Error("Api url required for browser usage");

        return (await (await fetch(apiUrl)).json()) as GmodDto;
    }

    private static async getCodebooks__node(
        visVersion: string
    ): Promise<CodebooksDto> {
        const resource = this.getResourceNames().filter(
            (r) =>
                r.endsWith(".gz") &&
                r.includes("codebooks") &&
                r.includes(visVersion)
        )[0];

        if (!resource)
            throw new Error(
                "Couldnt find Codebook resource for vis version: " + visVersion
            );

        return this.readJsonGzip<CodebooksDto>(
            EmbeddedResource.RESOURCE_DIR + "/" + resource
        );
    }

    private static async getCodebooks__browser(
        apiUrl?: string
    ): Promise<CodebooksDto> {
        if (!apiUrl) throw new Error("Api url required for browser usage");

        return (await (await fetch(apiUrl)).json()) as CodebooksDto;
    }

    public static getResourceNames__node(): string[] {
        return fs.readdirSync(EmbeddedResource.RESOURCE_DIR);
    }

    public static getResourceNames__browser(): string[] {
        throw new Error("Not implemented");
    }

    public static isNode() {
        return typeof window === "undefined";
    }

    public static async readJsonGzip<T>(file: string): Promise<T> {
        if (!file.endsWith(".gz")) throw new Error("Invalid resource file");

        const jsonStr = await new Promise<string>((res) => {
            const unzip = zlib.createUnzip();
            const inputStream = fs.createReadStream(file);
            const stream = inputStream.pipe(unzip);

            const segments: Set<Buffer> = new Set();

            stream.on("data", (data) => {
                segments.add(data);
            });

            stream.on("end", () => {
                const result = Buffer.concat(Array.from(segments)).toString();
                res(result);
            });
        });
        return JSON.parse(jsonStr);
    }
}
