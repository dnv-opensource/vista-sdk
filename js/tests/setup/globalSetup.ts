import { mkdirSync, writeFileSync } from "fs";
import { join } from "path";
import { Client } from "../../lib";
import { VisVersions } from "../../lib/VisVersion";

export default async function globalSetup() {
    const cacheDir = join(__dirname, ".vis-cache");
    mkdirSync(cacheDir, { recursive: true });

    // Fetch all VIS data and cache to disk for workers to read
    for (const version of VisVersions.all) {
        const gmod = await Client.visGetGmod(version);
        const codebooks = await Client.visGetCodebooks(version);
        const locations = await Client.visGetLocation(version);

        const dtos = {
            gmodDto: gmod,
            codebooksDto: codebooks,
            locationsDto: locations,
        };
        console.debug("Populating cache for VIS version", version);

        writeFileSync(
            join(cacheDir, `${version}.json`),
            JSON.stringify(dtos),
            "utf-8"
        );
    }
}
