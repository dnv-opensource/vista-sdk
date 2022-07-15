import * as fs from "fs-extra";
import * as zlib from "zlib";
import { GmodDto } from "../types/GmodDto";

export class EmbeddedResource {
    public static readonly RESOURCE_DIR = "./lib/resources";

    public static getResourceNames() {
        console.log(`__dirname: [${__dirname}] end__dirname`);
        return fs.readdirSync(EmbeddedResource.RESOURCE_DIR);
    }

    public static async getGmodVisVersions(): Promise<string[]> {
        const resources = this.getResourceNames().filter(
            (r) =>
                r.endsWith(".gz") &&
                r.includes("gmod") &&
                !r.includes("versioning")
        );

        if (resources.length === 0)
            throw new Error("Couldn't find any Gmod resource");

        const versions: string[] = [];

        for (const resource of resources) {
            const gmod = await this.readJsonGzip<GmodDto>(
                EmbeddedResource.RESOURCE_DIR + "/" + resource
            );

            versions.push(gmod.visRelease);
        }

        return versions;
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
